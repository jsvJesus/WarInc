using System.Net;
using System.Text;
using Dapper;
using MySqlConnector;
using WarInc.Api.Common;
using WarInc.Api.Database;

namespace WarInc.Api.Social;

public sealed class SocialService
{
    private const int ClanRankMember = 1;
    private const int ClanRankOfficer = 50;
    private const int ClanRankGeneral = 90;
    private const int ClanRankOwner = 99;

    private readonly WarIncDb _db;

    public SocialService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<FriendsListResponse> GetFriendsAsync(ulong customerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new FriendsListResponse(true, 0, "OK", Array.Empty<SocialFriendDto>());

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "friendsmap"))
            return new FriendsListResponse(true, 0, "OK", Array.Empty<SocialFriendDto>());

        var rows = await db.QueryAsync<SocialFriendDto>(
            """
            SELECT
                CASE
                    WHEN f.CustomerID = @CustomerId THEN f.FriendID
                    ELSE f.CustomerID
                END AS CustomerId,
                COALESCE(
                    NULLIF(l.Gamertag, ''),
                    NULLIF(l.AccountName, ''),
                    CONCAT('Player', CASE
                        WHEN f.CustomerID = @CustomerId THEN f.FriendID
                        ELSE f.CustomerID
                    END)
                ) AS Gamertag,
                COALESCE(f.FriendStatus, 0) AS Status,
                CASE WHEN s.CustomerID IS NULL THEN 0 ELSE 1 END AS Online,
                COALESCE(lb.Rank, 0) AS Level,
                COALESCE(l.HonorPoints, 0) AS XP
            FROM friendsmap f
            LEFT JOIN loginid l
                ON l.CustomerID = CASE
                    WHEN f.CustomerID = @CustomerId THEN f.FriendID
                    ELSE f.CustomerID
                END
            LEFT JOIN loginsessions s
                ON s.CustomerID = l.CustomerID
               AND s.TimeUpdated >= DATE_SUB(UTC_TIMESTAMP(), INTERVAL 15 MINUTE)
            LEFT JOIN leaderboard lb
                ON lb.CustomerID = l.CustomerID
            WHERE (f.CustomerID = @CustomerId OR f.FriendID = @CustomerId)
              AND COALESCE(f.FriendStatus, 0) = 1
            GROUP BY CustomerId, Gamertag, Status, Online, Level, XP
            ORDER BY Gamertag;
            """,
            new { CustomerId = (int)customerId });

        return new FriendsListResponse(true, 0, "OK", rows.ToArray());
    }

    public async Task<FriendsPendingResponse> GetFriendPendingRequestsAsync(ulong customerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new FriendsPendingResponse(true, 0, "OK", Array.Empty<SocialFriendPendingDto>(), Array.Empty<SocialFriendPendingDto>());

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "friendsmap"))
            return new FriendsPendingResponse(true, 0, "OK", Array.Empty<SocialFriendPendingDto>(), Array.Empty<SocialFriendPendingDto>());

        var incoming = await db.QueryAsync<SocialFriendPendingDto>(
            """
            SELECT
                f.CustomerID AS CustomerId,
                COALESCE(NULLIF(l.Gamertag, ''), NULLIF(l.AccountName, ''), CONCAT('Player', f.CustomerID)) AS Gamertag,
                1 AS Incoming,
                COALESCE(f.DateAdded, UTC_TIMESTAMP()) AS DateAdded
            FROM friendsmap f
            LEFT JOIN loginid l ON l.CustomerID = f.CustomerID
            WHERE f.FriendID = @CustomerId
              AND COALESCE(f.FriendStatus, 0) = 0
            ORDER BY DateAdded DESC;
            """,
            new { CustomerId = (int)customerId });

        var outgoing = await db.QueryAsync<SocialFriendPendingDto>(
            """
            SELECT
                f.FriendID AS CustomerId,
                COALESCE(NULLIF(l.Gamertag, ''), NULLIF(l.AccountName, ''), CONCAT('Player', f.FriendID)) AS Gamertag,
                0 AS Incoming,
                COALESCE(f.DateAdded, UTC_TIMESTAMP()) AS DateAdded
            FROM friendsmap f
            LEFT JOIN loginid l ON l.CustomerID = f.FriendID
            WHERE f.CustomerID = @CustomerId
              AND COALESCE(f.FriendStatus, 0) = 0
            ORDER BY DateAdded DESC;
            """,
            new { CustomerId = (int)customerId });

        return new FriendsPendingResponse(true, 0, "OK", incoming.ToArray(), outgoing.ToArray());
    }

    public async Task<bool> AddFriendRequestAsync(ulong customerId, ulong friendCustomerId, string? friendName)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return true;

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "friendsmap"))
            return true;

        var friendId = friendCustomerId;

        if (friendId == 0 && !string.IsNullOrWhiteSpace(friendName))
            friendId = await ResolveCustomerIdByNameAsync(db, friendName);

        if (friendId == 0 || friendId > int.MaxValue || friendId == customerId)
            return true;

        var reversePending = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM friendsmap
            WHERE CustomerID = @FriendId
              AND FriendID = @CustomerId
              AND COALESCE(FriendStatus, 0) = 0;
            """,
            new
            {
                CustomerId = (int)customerId,
                FriendId = (int)friendId
            });

        if (reversePending > 0)
        {
            await AnswerFriendRequestAsync(customerId, friendId, true);
            return true;
        }

        var exists = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM friendsmap
            WHERE (CustomerID = @CustomerId AND FriendID = @FriendId)
               OR (CustomerID = @FriendId AND FriendID = @CustomerId);
            """,
            new
            {
                CustomerId = (int)customerId,
                FriendId = (int)friendId
            });

        if (exists > 0)
            return true;

        await db.ExecuteAsync(
            """
            INSERT INTO friendsmap
            (
                CustomerID,
                FriendID,
                FriendStatus,
                DateAdded
            )
            VALUES
            (
                @CustomerId,
                @FriendId,
                0,
                UTC_TIMESTAMP()
            );
            """,
            new
            {
                CustomerId = (int)customerId,
                FriendId = (int)friendId
            });

        return true;
    }

    public async Task<bool> AnswerFriendRequestAsync(ulong customerId, ulong friendCustomerId, bool accept)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return true;

        if (friendCustomerId == 0 || friendCustomerId > int.MaxValue || friendCustomerId == customerId)
            return true;

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "friendsmap"))
            return true;

        if (!accept)
        {
            await db.ExecuteAsync(
                """
                DELETE FROM friendsmap
                WHERE (CustomerID = @FriendId AND FriendID = @CustomerId)
                   OR (CustomerID = @CustomerId AND FriendID = @FriendId);
                """,
                new
                {
                    CustomerId = (int)customerId,
                    FriendId = (int)friendCustomerId
                });

            return true;
        }

        await db.ExecuteAsync(
            """
            UPDATE friendsmap
            SET
                FriendStatus = 1,
                DateAdded = COALESCE(DateAdded, UTC_TIMESTAMP())
            WHERE CustomerID = @FriendId
              AND FriendID = @CustomerId;
            """,
            new
            {
                CustomerId = (int)customerId,
                FriendId = (int)friendCustomerId
            });

        await InsertFriendRowIfMissingAsync(db, (int)friendCustomerId, (int)customerId, 1);
        await InsertFriendRowIfMissingAsync(db, (int)customerId, (int)friendCustomerId, 1);

        return true;
    }

    public async Task<bool> RemoveFriendAsync(ulong customerId, ulong friendCustomerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return true;

        if (friendCustomerId == 0 || friendCustomerId > int.MaxValue)
            return true;

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "friendsmap"))
            return true;

        await db.ExecuteAsync(
            """
            DELETE FROM friendsmap
            WHERE (CustomerID = @CustomerId AND FriendID = @FriendId)
               OR (CustomerID = @FriendId AND FriendID = @CustomerId);
            """,
            new
            {
                CustomerId = (int)customerId,
                FriendId = (int)friendCustomerId
            });

        return true;
    }

    public async Task<ClanResponse> GetClanAsync(ulong customerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new ClanResponse(true, 0, "OK", null);

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "clandata"))
            return new ClanResponse(true, 0, "OK", null);

        var clan = await db.QueryFirstOrDefaultAsync<SocialClanDto>(
            """
            SELECT
                c.ClanID AS ClanId,
                c.ClanName AS Name,
                c.ClanTag AS Tag,
                c.OwnerID AS OwnerCustomerId,
                COALESCE(c.ClanLevel, 0) AS Level,
                COALESCE(c.ClanXP, 0) AS XP,
                (
                    SELECT COUNT(*)
                    FROM loginid l2
                    WHERE l2.ClanID = c.ClanID
                ) AS MemberCount,
                COALESCE(c.Announcement, '') AS Announcement
            FROM loginid l
            INNER JOIN clandata c ON c.ClanID = l.ClanID
            WHERE l.CustomerID = @CustomerId
              AND l.ClanID > 0
            LIMIT 1;
            """,
            new { CustomerId = (int)customerId });

        return new ClanResponse(true, 0, "OK", clan);
    }

    public async Task<ClanResponse> CreateClanAsync(ulong customerId, string nameRaw, string tagRaw)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new ClanResponse(false, 400, "BAD_CUSTOMER_ID", null);

        var name = NormalizeClanName(nameRaw);
        var tag = NormalizeClanTag(tagRaw);

        if (name.Length < 3 || name.Length > 64)
            return new ClanResponse(false, 400, "BAD_CLAN_NAME", null);

        if (tag.Length < 2 || tag.Length > 4)
            return new ClanResponse(false, 400, "BAD_CLAN_TAG", null);

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "clandata"))
            return new ClanResponse(true, 0, "OK", new SocialClanDto(0, name, tag, customerId, 0, 0, 1, ""));

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var account = await GetAccountClanRowAsync(db, customerId, tx);

            if (account == null)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 404, "ACCOUNT_NOT_FOUND", null);
            }

            if (account.ClanId > 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "ALREADY_IN_CLAN", null);
            }

            var exists = await db.ExecuteScalarAsync<int>(
                """
                SELECT COUNT(*)
                FROM clandata
                WHERE ClanName = @Name
                   OR ClanTag = @Tag;
                """,
                new { Name = name, Tag = tag },
                tx);

            if (exists > 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "CLAN_EXISTS", null);
            }

            var clanId = await db.ExecuteScalarAsync<int>(
                """
                INSERT INTO clandata
                (
                    ClanName,
                    ClanNameColor,
                    ClanTag,
                    ClanTagColor,
                    ClanEmblemID,
                    ClanEmblemColor,
                    ClanXP,
                    ClanLevel,
                    ClanGP,
                    OwnerID,
                    MaxClanMembers,
                    NumClanMembers,
                    ClanLore,
                    Announcement,
                    ClanCreateDate
                )
                VALUES
                (
                    @Name,
                    0,
                    @Tag,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    @OwnerId,
                    50,
                    1,
                    '',
                    '',
                    UTC_TIMESTAMP()
                );

                SELECT LAST_INSERT_ID();
                """,
                new
                {
                    Name = name,
                    Tag = tag,
                    OwnerId = (int)customerId
                },
                tx);

            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET
                    ClanID = @ClanId,
                    ClanRank = @Rank,
                    ClanContributedXP = 0,
                    ClanContributedGP = 0
                WHERE CustomerID = @CustomerId;
                """,
                new
                {
                    ClanId = clanId,
                    Rank = ClanRankOwner,
                    CustomerId = (int)customerId
                },
                tx);

            await LogClanEventAsync(db, tx, clanId, 1, ClanRankOwner, (int)customerId, 0, 0, 0, account.Gamertag, "", "clan created");

            await tx.CommitAsync();

            return new ClanResponse(true, 0, "OK", new SocialClanDto((ulong)clanId, name, tag, customerId, 0, 0, 1, ""));
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return new ClanResponse(false, 500, e.Message, null);
        }
    }

    public async Task<IReadOnlyList<SocialClanMemberDto>> GetClanMembersAsync(ulong customerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return Array.Empty<SocialClanMemberDto>();

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "clandata"))
            return Array.Empty<SocialClanMemberDto>();

        var clanId = await GetCustomerClanIdAsync(db, customerId);

        if (clanId <= 0)
            return Array.Empty<SocialClanMemberDto>();

        var rows = await db.QueryAsync<SocialClanMemberDto>(
            """
            SELECT
                l.CustomerID AS CustomerId,
                COALESCE(NULLIF(l.Gamertag, ''), NULLIF(l.AccountName, ''), CONCAT('Player', l.CustomerID)) AS Gamertag,
                COALESCE(l.ClanRank, 0) AS Rank,
                CASE WHEN s.CustomerID IS NULL THEN 0 ELSE 1 END AS Online,
                COALESCE(l.HonorPoints, 0) AS XP,
                COALESCE(l.ClanContributedXP, 0) AS ContributedXP,
                COALESCE(l.ClanContributedGP, 0) AS ContributedGP
            FROM loginid l
            LEFT JOIN loginsessions s
                ON s.CustomerID = l.CustomerID
               AND s.TimeUpdated >= DATE_SUB(UTC_TIMESTAMP(), INTERVAL 15 MINUTE)
            WHERE l.ClanID = @ClanId
            GROUP BY l.CustomerID, Gamertag, Rank, Online, XP, ContributedXP, ContributedGP
            ORDER BY Rank DESC, Gamertag ASC;
            """,
            new { ClanId = clanId });

        return rows.ToArray();
    }

    public async Task<IReadOnlyList<SocialClanInviteDto>> GetClanInvitesAsync(ulong customerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return Array.Empty<SocialClanInviteDto>();

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "claninvites"))
            return Array.Empty<SocialClanInviteDto>();

        var account = await GetAccountClanRowAsync(db, customerId);
        var result = new List<SocialClanInviteDto>();

        var incoming = await db.QueryAsync<SocialClanInviteDto>(
            """
            SELECT
                i.ClanInviteID AS InviteId,
                i.ClanID AS ClanId,
                COALESCE(c.ClanName, '') AS ClanName,
                COALESCE(c.ClanTag, '') AS ClanTag,
                i.InviterID AS InviterCustomerId,
                COALESCE(NULLIF(inv.Gamertag, ''), NULLIF(inv.AccountName, ''), CONCAT('Player', i.InviterID)) AS InviterGamertag,
                i.CustomerID AS TargetCustomerId,
                COALESCE(NULLIF(t.Gamertag, ''), NULLIF(t.AccountName, ''), CONCAT('Player', i.CustomerID)) AS TargetGamertag,
                1 AS Incoming,
                i.ExpireTime AS ExpireTime
            FROM claninvites i
            INNER JOIN clandata c ON c.ClanID = i.ClanID
            LEFT JOIN loginid inv ON inv.CustomerID = i.InviterID
            LEFT JOIN loginid t ON t.CustomerID = i.CustomerID
            WHERE i.CustomerID = @CustomerId
              AND i.ExpireTime > UTC_TIMESTAMP()
            ORDER BY i.ExpireTime DESC;
            """,
            new { CustomerId = (int)customerId });

        result.AddRange(incoming);

        if (account != null && account.ClanId > 0 && account.ClanRank >= ClanRankOfficer)
        {
            var outgoing = await db.QueryAsync<SocialClanInviteDto>(
                """
                SELECT
                    i.ClanInviteID AS InviteId,
                    i.ClanID AS ClanId,
                    COALESCE(c.ClanName, '') AS ClanName,
                    COALESCE(c.ClanTag, '') AS ClanTag,
                    i.InviterID AS InviterCustomerId,
                    COALESCE(NULLIF(inv.Gamertag, ''), NULLIF(inv.AccountName, ''), CONCAT('Player', i.InviterID)) AS InviterGamertag,
                    i.CustomerID AS TargetCustomerId,
                    COALESCE(NULLIF(t.Gamertag, ''), NULLIF(t.AccountName, ''), CONCAT('Player', i.CustomerID)) AS TargetGamertag,
                    0 AS Incoming,
                    i.ExpireTime AS ExpireTime
                FROM claninvites i
                INNER JOIN clandata c ON c.ClanID = i.ClanID
                LEFT JOIN loginid inv ON inv.CustomerID = i.InviterID
                LEFT JOIN loginid t ON t.CustomerID = i.CustomerID
                WHERE i.ClanID = @ClanId
                  AND i.ExpireTime > UTC_TIMESTAMP()
                ORDER BY i.ExpireTime DESC;
                """,
                new { ClanId = account.ClanId });

            result.AddRange(outgoing);
        }

        return result.ToArray();
    }

    public async Task<IReadOnlyList<SocialClanApplicationDto>> GetClanApplicationsAsync(ulong customerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return Array.Empty<SocialClanApplicationDto>();

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "clanapplications"))
            return Array.Empty<SocialClanApplicationDto>();

        var account = await GetAccountClanRowAsync(db, customerId);

        if (account == null || account.ClanId <= 0 || account.ClanRank < ClanRankOfficer)
            return Array.Empty<SocialClanApplicationDto>();

        var rows = await db.QueryAsync<SocialClanApplicationDto>(
            """
            SELECT
                a.ClanApplicationID AS ApplicationId,
                a.ClanID AS ClanId,
                COALESCE(c.ClanName, '') AS ClanName,
                COALESCE(c.ClanTag, '') AS ClanTag,
                a.CustomerID AS CustomerId,
                COALESCE(NULLIF(l.Gamertag, ''), NULLIF(l.AccountName, ''), CONCAT('Player', a.CustomerID)) AS Gamertag,
                COALESCE(a.ApplicationText, '') AS Text,
                a.ExpireTime AS ExpireTime
            FROM clanapplications a
            INNER JOIN clandata c ON c.ClanID = a.ClanID
            LEFT JOIN loginid l ON l.CustomerID = a.CustomerID
            WHERE a.ClanID = @ClanId
              AND COALESCE(a.IsProcessed, 0) = 0
              AND a.ExpireTime > UTC_TIMESTAMP()
            ORDER BY a.ExpireTime DESC;
            """,
            new { ClanId = account.ClanId });

        return rows.ToArray();
    }

    public async Task<ClanResponse> InviteToClanAsync(ulong customerId, ulong targetCustomerId, string? targetName)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new ClanResponse(false, 400, "BAD_CUSTOMER_ID", null);

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "claninvites"))
            return new ClanResponse(true, 0, "OK", null);

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var actor = await GetAccountClanRowAsync(db, customerId, tx);

            if (actor == null || actor.ClanId <= 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "NOT_IN_CLAN", null);
            }

            if (actor.ClanRank < ClanRankOfficer)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 403, "NO_CLAN_PERMISSION", null);
            }

            var targetId = targetCustomerId;

            if (targetId == 0 && !string.IsNullOrWhiteSpace(targetName))
                targetId = await ResolveCustomerIdByNameAsync(db, targetName, tx);

            if (targetId == 0 || targetId > int.MaxValue || targetId == customerId)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 400, "BAD_TARGET", null);
            }

            var target = await GetAccountClanRowAsync(db, targetId, tx);

            if (target == null)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 404, "TARGET_NOT_FOUND", null);
            }

            if (target.ClanId > 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "TARGET_ALREADY_IN_CLAN", null);
            }

            if (!await ClanHasFreeSlotAsync(db, tx, actor.ClanId))
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "CLAN_FULL", null);
            }

            await db.ExecuteAsync(
                """
                DELETE FROM claninvites
                WHERE ClanID = @ClanId
                  AND CustomerID = @TargetId;

                INSERT INTO claninvites
                (
                    ClanID,
                    InviterID,
                    CustomerID,
                    ExpireTime
                )
                VALUES
                (
                    @ClanId,
                    @InviterId,
                    @TargetId,
                    DATE_ADD(UTC_TIMESTAMP(), INTERVAL 7 DAY)
                );
                """,
                new
                {
                    ClanId = actor.ClanId,
                    InviterId = (int)customerId,
                    TargetId = (int)targetId
                },
                tx);

            await LogClanEventAsync(db, tx, actor.ClanId, 10, actor.ClanRank, (int)customerId, (int)targetId, 0, 0, actor.Gamertag, target.Gamertag, "invite sent");

            await tx.CommitAsync();

            return await GetClanAsync(customerId);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return new ClanResponse(false, 500, e.Message, null);
        }
    }

    public async Task<ClanResponse> AcceptClanInviteAsync(ulong customerId, ulong clanOrInviteId, bool accept)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new ClanResponse(false, 400, "BAD_CUSTOMER_ID", null);

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "claninvites"))
            return new ClanResponse(true, 0, "OK", null);

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var account = await GetAccountClanRowAsync(db, customerId, tx);

            if (account == null)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 404, "ACCOUNT_NOT_FOUND", null);
            }

            var invite = await db.QueryFirstOrDefaultAsync<ClanInviteRow>(
                """
                SELECT
                    ClanInviteID AS InviteId,
                    ClanID AS ClanId,
                    InviterID AS InviterId,
                    CustomerID AS CustomerId
                FROM claninvites
                WHERE CustomerID = @CustomerId
                  AND ExpireTime > UTC_TIMESTAMP()
                  AND (
                        @Id = 0
                     OR ClanID = @Id
                     OR ClanInviteID = @Id
                  )
                ORDER BY ClanInviteID DESC
                LIMIT 1
                FOR UPDATE;
                """,
                new
                {
                    CustomerId = (int)customerId,
                    Id = (int)Math.Min(clanOrInviteId, (ulong)int.MaxValue)
                },
                tx);

            if (invite == null)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 404, "INVITE_NOT_FOUND", null);
            }

            if (!accept)
            {
                await db.ExecuteAsync(
                    """
                    DELETE FROM claninvites
                    WHERE ClanInviteID = @InviteId;
                    """,
                    new { invite.InviteId },
                    tx);

                await tx.CommitAsync();
                return new ClanResponse(true, 0, "OK", null);
            }

            if (account.ClanId > 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "ALREADY_IN_CLAN", null);
            }

            if (!await ClanHasFreeSlotAsync(db, tx, invite.ClanId))
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "CLAN_FULL", null);
            }

            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET
                    ClanID = @ClanId,
                    ClanRank = @Rank,
                    ClanContributedXP = 0,
                    ClanContributedGP = 0
                WHERE CustomerID = @CustomerId;

                DELETE FROM claninvites
                WHERE CustomerID = @CustomerId;

                UPDATE clanapplications
                SET IsProcessed = 1
                WHERE CustomerID = @CustomerId
                  AND COALESCE(IsProcessed, 0) = 0;
                """,
                new
                {
                    ClanId = invite.ClanId,
                    Rank = ClanRankMember,
                    CustomerId = (int)customerId
                },
                tx);

            await UpdateClanMemberCountAsync(db, tx, invite.ClanId);
            await LogClanEventAsync(db, tx, invite.ClanId, 11, ClanRankMember, (int)customerId, invite.InviterId, 0, 0, account.Gamertag, "", "invite accepted");

            await tx.CommitAsync();

            return await GetClanAsync(customerId);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return new ClanResponse(false, 500, e.Message, null);
        }
    }

    public async Task<ClanResponse> ApplyToClanAsync(ulong customerId, ulong clanId, string? textRaw)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new ClanResponse(false, 400, "BAD_CUSTOMER_ID", null);

        if (clanId == 0 || clanId > int.MaxValue)
            return new ClanResponse(false, 400, "BAD_CLAN_ID", null);

        var text = Trim(WebUtility.HtmlDecode(textRaw ?? ""), 500);

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "clanapplications"))
            return new ClanResponse(true, 0, "OK", null);

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var account = await GetAccountClanRowAsync(db, customerId, tx);

            if (account == null)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 404, "ACCOUNT_NOT_FOUND", null);
            }

            if (account.ClanId > 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "ALREADY_IN_CLAN", null);
            }

            var clanExists = await db.ExecuteScalarAsync<int>(
                """
                SELECT COUNT(*)
                FROM clandata
                WHERE ClanID = @ClanId;
                """,
                new { ClanId = (int)clanId },
                tx);

            if (clanExists <= 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 404, "CLAN_NOT_FOUND", null);
            }

            await db.ExecuteAsync(
                """
                DELETE FROM clanapplications
                WHERE ClanID = @ClanId
                  AND CustomerID = @CustomerId
                  AND COALESCE(IsProcessed, 0) = 0;

                INSERT INTO clanapplications
                (
                    ClanID,
                    CustomerID,
                    ExpireTime,
                    ApplicationText,
                    IsProcessed
                )
                VALUES
                (
                    @ClanId,
                    @CustomerId,
                    DATE_ADD(UTC_TIMESTAMP(), INTERVAL 7 DAY),
                    @Text,
                    0
                );
                """,
                new
                {
                    ClanId = (int)clanId,
                    CustomerId = (int)customerId,
                    Text = text
                },
                tx);

            await LogClanEventAsync(db, tx, (int)clanId, 20, 0, (int)customerId, 0, 0, 0, account.Gamertag, "", "application sent");

            await tx.CommitAsync();

            return new ClanResponse(true, 0, "OK", null);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return new ClanResponse(false, 500, e.Message, null);
        }
    }

    public async Task<ClanResponse> AnswerClanApplicationAsync(ulong customerId, ulong applicationId, ulong targetCustomerId, bool accept)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new ClanResponse(false, 400, "BAD_CUSTOMER_ID", null);

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "clanapplications"))
            return new ClanResponse(true, 0, "OK", null);

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var actor = await GetAccountClanRowAsync(db, customerId, tx);

            if (actor == null || actor.ClanId <= 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "NOT_IN_CLAN", null);
            }

            if (actor.ClanRank < ClanRankOfficer)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 403, "NO_CLAN_PERMISSION", null);
            }

            var app = await db.QueryFirstOrDefaultAsync<ClanApplicationRow>(
                """
                SELECT
                    ClanApplicationID AS ApplicationId,
                    ClanID AS ClanId,
                    CustomerID AS CustomerId
                FROM clanapplications
                WHERE ClanID = @ClanId
                  AND COALESCE(IsProcessed, 0) = 0
                  AND ExpireTime > UTC_TIMESTAMP()
                  AND (
                        ClanApplicationID = @ApplicationId
                     OR CustomerID = @TargetCustomerId
                  )
                ORDER BY ClanApplicationID DESC
                LIMIT 1
                FOR UPDATE;
                """,
                new
                {
                    ClanId = actor.ClanId,
                    ApplicationId = (int)Math.Min(applicationId, (ulong)int.MaxValue),
                    TargetCustomerId = (int)Math.Min(targetCustomerId, (ulong)int.MaxValue)
                },
                tx);

            if (app == null)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 404, "APPLICATION_NOT_FOUND", null);
            }

            await db.ExecuteAsync(
                """
                UPDATE clanapplications
                SET IsProcessed = 1
                WHERE ClanApplicationID = @ApplicationId;
                """,
                new { app.ApplicationId },
                tx);

            if (!accept)
            {
                await LogClanEventAsync(db, tx, actor.ClanId, 21, actor.ClanRank, (int)customerId, app.CustomerId, 0, 0, actor.Gamertag, "", "application rejected");
                await tx.CommitAsync();
                return await GetClanAsync(customerId);
            }

            var target = await GetAccountClanRowAsync(db, (ulong)app.CustomerId, tx);

            if (target == null)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 404, "TARGET_NOT_FOUND", null);
            }

            if (target.ClanId > 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "TARGET_ALREADY_IN_CLAN", null);
            }

            if (!await ClanHasFreeSlotAsync(db, tx, actor.ClanId))
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "CLAN_FULL", null);
            }

            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET
                    ClanID = @ClanId,
                    ClanRank = @Rank,
                    ClanContributedXP = 0,
                    ClanContributedGP = 0
                WHERE CustomerID = @TargetId;

                DELETE FROM claninvites
                WHERE CustomerID = @TargetId;

                UPDATE clanapplications
                SET IsProcessed = 1
                WHERE CustomerID = @TargetId
                  AND COALESCE(IsProcessed, 0) = 0;
                """,
                new
                {
                    ClanId = actor.ClanId,
                    Rank = ClanRankMember,
                    TargetId = app.CustomerId
                },
                tx);

            await UpdateClanMemberCountAsync(db, tx, actor.ClanId);
            await LogClanEventAsync(db, tx, actor.ClanId, 22, actor.ClanRank, (int)customerId, app.CustomerId, 0, 0, actor.Gamertag, target.Gamertag, "application accepted");

            await tx.CommitAsync();

            return await GetClanAsync(customerId);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return new ClanResponse(false, 500, e.Message, null);
        }
    }

    public async Task<ClanResponse> LeaveClanAsync(ulong customerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new ClanResponse(false, 400, "BAD_CUSTOMER_ID", null);

        await using var db = _db.CreateConnection();

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var account = await GetAccountClanRowAsync(db, customerId, tx);

            if (account == null || account.ClanId <= 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "NOT_IN_CLAN", null);
            }

            var clanId = account.ClanId;

            if (account.ClanRank >= ClanRankOwner)
            {
                var successor = await db.QueryFirstOrDefaultAsync<AccountClanRow>(
                    """
                    SELECT
                        CustomerID AS CustomerId,
                        ClanID AS ClanId,
                        ClanRank AS ClanRank,
                        COALESCE(NULLIF(Gamertag, ''), NULLIF(AccountName, ''), CONCAT('Player', CustomerID)) AS Gamertag
                    FROM loginid
                    WHERE ClanID = @ClanId
                      AND CustomerID <> @CustomerId
                    ORDER BY ClanRank DESC, CustomerID ASC
                    LIMIT 1
                    FOR UPDATE;
                    """,
                    new
                    {
                        ClanId = clanId,
                        CustomerId = (int)customerId
                    },
                    tx);

                if (successor == null)
                {
                    await db.ExecuteAsync(
                        """
                        UPDATE loginid
                        SET
                            ClanID = 0,
                            ClanRank = 0,
                            ClanContributedXP = 0,
                            ClanContributedGP = 0
                        WHERE CustomerID = @CustomerId;

                        DELETE FROM claninvites
                        WHERE ClanID = @ClanId;

                        UPDATE clanapplications
                        SET IsProcessed = 1
                        WHERE ClanID = @ClanId
                          AND COALESCE(IsProcessed, 0) = 0;

                        DELETE FROM clandata
                        WHERE ClanID = @ClanId;
                        """,
                        new
                        {
                            CustomerId = (int)customerId,
                            ClanId = clanId
                        },
                        tx);

                    await tx.CommitAsync();
                    return new ClanResponse(true, 0, "OK", null);
                }

                await db.ExecuteAsync(
                    """
                    UPDATE clandata
                    SET OwnerID = @NewOwnerId
                    WHERE ClanID = @ClanId;

                    UPDATE loginid
                    SET ClanRank = @OwnerRank
                    WHERE CustomerID = @NewOwnerId;
                    """,
                    new
                    {
                        NewOwnerId = successor.CustomerId,
                        ClanId = clanId,
                        OwnerRank = ClanRankOwner
                    },
                    tx);
            }

            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET
                    ClanID = 0,
                    ClanRank = 0,
                    ClanContributedXP = 0,
                    ClanContributedGP = 0
                WHERE CustomerID = @CustomerId;
                """,
                new { CustomerId = (int)customerId },
                tx);

            await UpdateClanMemberCountAsync(db, tx, clanId);
            await LogClanEventAsync(db, tx, clanId, 30, account.ClanRank, (int)customerId, 0, 0, 0, account.Gamertag, "", "member left");

            await tx.CommitAsync();

            return new ClanResponse(true, 0, "OK", null);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return new ClanResponse(false, 500, e.Message, null);
        }
    }

    public async Task<ClanResponse> KickClanMemberAsync(ulong customerId, ulong targetCustomerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new ClanResponse(false, 400, "BAD_CUSTOMER_ID", null);

        if (targetCustomerId == 0 || targetCustomerId > int.MaxValue || targetCustomerId == customerId)
            return new ClanResponse(false, 400, "BAD_TARGET", null);

        await using var db = _db.CreateConnection();

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var actor = await GetAccountClanRowAsync(db, customerId, tx);
            var target = await GetAccountClanRowAsync(db, targetCustomerId, tx);

            if (actor == null || actor.ClanId <= 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "NOT_IN_CLAN", null);
            }

            if (target == null || target.ClanId != actor.ClanId)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 404, "TARGET_NOT_IN_CLAN", null);
            }

            if (actor.ClanRank < ClanRankOfficer || target.ClanRank >= actor.ClanRank)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 403, "NO_CLAN_PERMISSION", null);
            }

            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET
                    ClanID = 0,
                    ClanRank = 0,
                    ClanContributedXP = 0,
                    ClanContributedGP = 0
                WHERE CustomerID = @TargetId;
                """,
                new { TargetId = (int)targetCustomerId },
                tx);

            await UpdateClanMemberCountAsync(db, tx, actor.ClanId);
            await LogClanEventAsync(db, tx, actor.ClanId, 31, actor.ClanRank, (int)customerId, (int)targetCustomerId, 0, 0, actor.Gamertag, target.Gamertag, "member kicked");

            await tx.CommitAsync();

            return await GetClanAsync(customerId);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return new ClanResponse(false, 500, e.Message, null);
        }
    }

    public async Task<ClanResponse> PromoteClanMemberAsync(ulong customerId, ulong targetCustomerId)
    {
        return await ChangeClanRankAsync(customerId, targetCustomerId, true);
    }

    public async Task<ClanResponse> DemoteClanMemberAsync(ulong customerId, ulong targetCustomerId)
    {
        return await ChangeClanRankAsync(customerId, targetCustomerId, false);
    }

    public async Task<ClanResponse> SetClanAnnouncementAsync(ulong customerId, string announcementRaw)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new ClanResponse(false, 400, "BAD_CUSTOMER_ID", null);

        var announcement = Trim(WebUtility.HtmlDecode(announcementRaw ?? ""), 512);

        await using var db = _db.CreateConnection();

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var actor = await GetAccountClanRowAsync(db, customerId, tx);

            if (actor == null || actor.ClanId <= 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "NOT_IN_CLAN", null);
            }

            if (actor.ClanRank < ClanRankOfficer)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 403, "NO_CLAN_PERMISSION", null);
            }

            await db.ExecuteAsync(
                """
                UPDATE clandata
                SET Announcement = @Announcement
                WHERE ClanID = @ClanId;
                """,
                new
                {
                    Announcement = announcement,
                    ClanId = actor.ClanId
                },
                tx);

            await LogClanEventAsync(db, tx, actor.ClanId, 40, actor.ClanRank, (int)customerId, 0, 0, 0, actor.Gamertag, "", "announcement updated");

            await tx.CommitAsync();

            return await GetClanAsync(customerId);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return new ClanResponse(false, 500, e.Message, null);
        }
    }

    public string BuildLegacyFriendsXml(IReadOnlyList<SocialFriendDto> friends)
    {
        var xml = new StringBuilder();

        xml.Append("<friends>");

        foreach (var friend in friends)
        {
            xml.Append("<f ");
            LegacyUtil.AppendXmlAttr(xml, "id", friend.CustomerId);
            LegacyUtil.AppendXmlAttr(xml, "gt", friend.Gamertag);
            LegacyUtil.AppendXmlAttr(xml, "st", friend.Status);
            LegacyUtil.AppendXmlAttr(xml, "on", friend.Online);
            LegacyUtil.AppendXmlAttr(xml, "lvl", friend.Level);
            LegacyUtil.AppendXmlAttr(xml, "xp", friend.XP);
            xml.Append("/>");
        }

        xml.Append("</friends>");

        return xml.ToString();
    }

    public string BuildLegacyFriendsPendingXml(FriendsPendingResponse pending)
    {
        var xml = new StringBuilder();

        xml.Append("<requests>");

        foreach (var item in pending.Incoming)
        {
            xml.Append("<r ");
            LegacyUtil.AppendXmlAttr(xml, "id", item.CustomerId);
            LegacyUtil.AppendXmlAttr(xml, "gt", item.Gamertag);
            LegacyUtil.AppendXmlAttr(xml, "in", 1);
            xml.Append("/>");
        }

        foreach (var item in pending.Outgoing)
        {
            xml.Append("<r ");
            LegacyUtil.AppendXmlAttr(xml, "id", item.CustomerId);
            LegacyUtil.AppendXmlAttr(xml, "gt", item.Gamertag);
            LegacyUtil.AppendXmlAttr(xml, "in", 0);
            xml.Append("/>");
        }

        xml.Append("</requests>");

        return xml.ToString();
    }

    public string BuildLegacyFriendsStatsXml(IReadOnlyList<SocialFriendDto> friends)
    {
        var xml = new StringBuilder();

        xml.Append("<friends ");
        LegacyUtil.AppendXmlAttr(xml, "cnt", friends.Count);
        LegacyUtil.AppendXmlAttr(xml, "online", friends.Count(x => x.Online != 0));
        xml.Append(">");

        foreach (var friend in friends)
        {
            xml.Append("<f ");
            LegacyUtil.AppendXmlAttr(xml, "id", friend.CustomerId);
            LegacyUtil.AppendXmlAttr(xml, "on", friend.Online);
            LegacyUtil.AppendXmlAttr(xml, "lvl", friend.Level);
            LegacyUtil.AppendXmlAttr(xml, "xp", friend.XP);
            xml.Append("/>");
        }

        xml.Append("</friends>");

        return xml.ToString();
    }

    public string BuildLegacyClanXml(SocialClanDto? clan)
    {
        if (clan == null)
            return "<clan id=\"0\" name=\"\" tag=\"\" owner=\"0\" level=\"0\" xp=\"0\" members=\"0\" announcement=\"\"></clan>";

        var xml = new StringBuilder();

        xml.Append("<clan ");
        LegacyUtil.AppendXmlAttr(xml, "id", clan.ClanId);
        LegacyUtil.AppendXmlAttr(xml, "name", clan.Name);
        LegacyUtil.AppendXmlAttr(xml, "tag", clan.Tag);
        LegacyUtil.AppendXmlAttr(xml, "tagcolor", 0);
        LegacyUtil.AppendXmlAttr(xml, "owner", clan.OwnerCustomerId);
        LegacyUtil.AppendXmlAttr(xml, "level", clan.Level);
        LegacyUtil.AppendXmlAttr(xml, "xp", clan.XP);
        LegacyUtil.AppendXmlAttr(xml, "members", clan.MemberCount);
        LegacyUtil.AppendXmlAttr(xml, "announcement", clan.Announcement);
        xml.Append("></clan>");

        return xml.ToString();
    }

    public string BuildLegacyClanMembersXml(IReadOnlyList<SocialClanMemberDto> members)
    {
        var xml = new StringBuilder();

        xml.Append("<members>");

        foreach (var member in members)
        {
            xml.Append("<m ");
            LegacyUtil.AppendXmlAttr(xml, "id", member.CustomerId);
            LegacyUtil.AppendXmlAttr(xml, "gt", member.Gamertag);
            LegacyUtil.AppendXmlAttr(xml, "rank", member.Rank);
            LegacyUtil.AppendXmlAttr(xml, "on", member.Online);
            LegacyUtil.AppendXmlAttr(xml, "xp", member.XP);
            LegacyUtil.AppendXmlAttr(xml, "cxp", member.ContributedXP);
            LegacyUtil.AppendXmlAttr(xml, "cgp", member.ContributedGP);
            xml.Append("/>");
        }

        xml.Append("</members>");

        return xml.ToString();
    }

    public string BuildLegacyClanInvitesXml(IReadOnlyList<SocialClanInviteDto> invites)
    {
        var xml = new StringBuilder();

        xml.Append("<invites>");

        foreach (var invite in invites)
        {
            xml.Append("<i ");
            LegacyUtil.AppendXmlAttr(xml, "id", invite.InviteId);
            LegacyUtil.AppendXmlAttr(xml, "cid", invite.ClanId);
            LegacyUtil.AppendXmlAttr(xml, "cn", invite.ClanName);
            LegacyUtil.AppendXmlAttr(xml, "ct", invite.ClanTag);
            LegacyUtil.AppendXmlAttr(xml, "iid", invite.InviterCustomerId);
            LegacyUtil.AppendXmlAttr(xml, "igt", invite.InviterGamertag);
            LegacyUtil.AppendXmlAttr(xml, "tid", invite.TargetCustomerId);
            LegacyUtil.AppendXmlAttr(xml, "tgt", invite.TargetGamertag);
            LegacyUtil.AppendXmlAttr(xml, "in", invite.Incoming);
            xml.Append("/>");
        }

        xml.Append("</invites>");

        return xml.ToString();
    }

    public string BuildLegacyClanApplicationsXml(IReadOnlyList<SocialClanApplicationDto> applications)
    {
        var xml = new StringBuilder();

        xml.Append("<applications>");

        foreach (var app in applications)
        {
            xml.Append("<a ");
            LegacyUtil.AppendXmlAttr(xml, "id", app.ApplicationId);
            LegacyUtil.AppendXmlAttr(xml, "cid", app.ClanId);
            LegacyUtil.AppendXmlAttr(xml, "cn", app.ClanName);
            LegacyUtil.AppendXmlAttr(xml, "ct", app.ClanTag);
            LegacyUtil.AppendXmlAttr(xml, "uid", app.CustomerId);
            LegacyUtil.AppendXmlAttr(xml, "gt", app.Gamertag);
            LegacyUtil.AppendXmlAttr(xml, "txt", app.Text);
            xml.Append("/>");
        }

        xml.Append("</applications>");

        return xml.ToString();
    }

    private async Task<ClanResponse> ChangeClanRankAsync(ulong customerId, ulong targetCustomerId, bool promote)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new ClanResponse(false, 400, "BAD_CUSTOMER_ID", null);

        if (targetCustomerId == 0 || targetCustomerId > int.MaxValue || targetCustomerId == customerId)
            return new ClanResponse(false, 400, "BAD_TARGET", null);

        await using var db = _db.CreateConnection();

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var actor = await GetAccountClanRowAsync(db, customerId, tx);
            var target = await GetAccountClanRowAsync(db, targetCustomerId, tx);

            if (actor == null || actor.ClanId <= 0)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 409, "NOT_IN_CLAN", null);
            }

            if (target == null || target.ClanId != actor.ClanId)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 404, "TARGET_NOT_IN_CLAN", null);
            }

            if (actor.ClanRank < ClanRankOwner || target.ClanRank >= ClanRankOwner)
            {
                await tx.RollbackAsync();
                return new ClanResponse(false, 403, "NO_CLAN_PERMISSION", null);
            }

            var newRank = target.ClanRank;

            if (promote)
            {
                if (newRank < ClanRankOfficer)
                    newRank = ClanRankOfficer;
                else if (newRank < ClanRankGeneral)
                    newRank = ClanRankGeneral;
                else
                    newRank = ClanRankGeneral;
            }
            else
            {
                if (newRank > ClanRankOfficer)
                    newRank = ClanRankOfficer;
                else
                    newRank = ClanRankMember;
            }

            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET ClanRank = @Rank
                WHERE CustomerID = @TargetId;
                """,
                new
                {
                    Rank = newRank,
                    TargetId = (int)targetCustomerId
                },
                tx);

            await LogClanEventAsync(
                db,
                tx,
                actor.ClanId,
                promote ? 32 : 33,
                actor.ClanRank,
                (int)customerId,
                (int)targetCustomerId,
                target.ClanRank,
                newRank,
                actor.Gamertag,
                target.Gamertag,
                promote ? "member promoted" : "member demoted");

            await tx.CommitAsync();

            return await GetClanAsync(customerId);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return new ClanResponse(false, 500, e.Message, null);
        }
    }

    private static async Task InsertFriendRowIfMissingAsync(MySqlConnection db, int customerId, int friendId, int status)
    {
        var exists = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM friendsmap
            WHERE CustomerID = @CustomerId
              AND FriendID = @FriendId;
            """,
            new
            {
                CustomerId = customerId,
                FriendId = friendId
            });

        if (exists > 0)
        {
            await db.ExecuteAsync(
                """
                UPDATE friendsmap
                SET
                    FriendStatus = @Status,
                    DateAdded = COALESCE(DateAdded, UTC_TIMESTAMP())
                WHERE CustomerID = @CustomerId
                  AND FriendID = @FriendId;
                """,
                new
                {
                    CustomerId = customerId,
                    FriendId = friendId,
                    Status = status
                });

            return;
        }

        await db.ExecuteAsync(
            """
            INSERT INTO friendsmap
            (
                CustomerID,
                FriendID,
                FriendStatus,
                DateAdded
            )
            VALUES
            (
                @CustomerId,
                @FriendId,
                @Status,
                UTC_TIMESTAMP()
            );
            """,
            new
            {
                CustomerId = customerId,
                FriendId = friendId,
                Status = status
            });
    }

    private static async Task<ulong> ResolveCustomerIdByNameAsync(MySqlConnection db, string nameRaw, MySqlTransaction? tx = null)
    {
        var name = WebUtility.HtmlDecode(nameRaw ?? "").Trim();

        if (string.IsNullOrWhiteSpace(name))
            return 0;

        var customerId = await db.ExecuteScalarAsync<int?>(
            """
            SELECT CustomerID
            FROM loginid
            WHERE Gamertag = @Name
               OR AccountName = @Name
            ORDER BY CustomerID ASC
            LIMIT 1;
            """,
            new { Name = name },
            tx);

        return customerId.HasValue && customerId.Value > 0 ? (ulong)customerId.Value : 0;
    }

    private static async Task<int> GetCustomerClanIdAsync(MySqlConnection db, ulong customerId)
    {
        return await db.ExecuteScalarAsync<int>(
            """
            SELECT COALESCE(ClanID, 0)
            FROM loginid
            WHERE CustomerID = @CustomerId
            LIMIT 1;
            """,
            new { CustomerId = (int)customerId });
    }

    private static async Task<AccountClanRow?> GetAccountClanRowAsync(MySqlConnection db, ulong customerId, MySqlTransaction? tx = null)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return null;

        return await db.QueryFirstOrDefaultAsync<AccountClanRow>(
            """
            SELECT
                CustomerID AS CustomerId,
                COALESCE(ClanID, 0) AS ClanId,
                COALESCE(ClanRank, 0) AS ClanRank,
                COALESCE(NULLIF(Gamertag, ''), NULLIF(AccountName, ''), CONCAT('Player', CustomerID)) AS Gamertag
            FROM loginid
            WHERE CustomerID = @CustomerId
            LIMIT 1;
            """,
            new { CustomerId = (int)customerId },
            tx);
    }

    private static async Task<bool> ClanHasFreeSlotAsync(MySqlConnection db, MySqlTransaction tx, int clanId)
    {
        var row = await db.QueryFirstOrDefaultAsync<ClanSlotRow>(
            """
            SELECT
                COALESCE(MaxClanMembers, 50) AS MaxMembers,
                (
                    SELECT COUNT(*)
                    FROM loginid
                    WHERE ClanID = @ClanId
                ) AS CurrentMembers
            FROM clandata
            WHERE ClanID = @ClanId
            LIMIT 1
            FOR UPDATE;
            """,
            new { ClanId = clanId },
            tx);

        return row != null && row.CurrentMembers < row.MaxMembers;
    }

    private static async Task UpdateClanMemberCountAsync(MySqlConnection db, MySqlTransaction tx, int clanId)
    {
        await db.ExecuteAsync(
            """
            UPDATE clandata
            SET NumClanMembers =
            (
                SELECT COUNT(*)
                FROM loginid
                WHERE ClanID = @ClanId
            )
            WHERE ClanID = @ClanId;
            """,
            new { ClanId = clanId },
            tx);
    }

    private static async Task LogClanEventAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int clanId,
        int eventType,
        int eventRank,
        int var1,
        int var2,
        int var3,
        int var4,
        string text1,
        string text2,
        string text3)
    {
        if (!await TableExistsAsync(db, "clanevents"))
            return;

        await db.ExecuteAsync(
            """
            INSERT INTO clanevents
            (
                ClanID,
                EventDate,
                EventType,
                EventRank,
                Var1,
                Var2,
                Var3,
                Var4,
                Text1,
                Text2,
                Text3
            )
            VALUES
            (
                @ClanId,
                UTC_TIMESTAMP(),
                @EventType,
                @EventRank,
                @Var1,
                @Var2,
                @Var3,
                @Var4,
                @Text1,
                @Text2,
                @Text3
            );
            """,
            new
            {
                ClanId = clanId,
                EventType = eventType,
                EventRank = eventRank,
                Var1 = var1,
                Var2 = var2,
                Var3 = var3,
                Var4 = var4,
                Text1 = Trim(text1, 64),
                Text2 = Trim(text2, 64),
                Text3 = Trim(text3, 256)
            },
            tx);
    }

    private static async Task<bool> TableExistsAsync(MySqlConnection db, string table)
    {
        var count = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM information_schema.tables
            WHERE table_schema = DATABASE()
              AND table_name = @Table;
            """,
            new { Table = table });

        return count > 0;
    }

    private static string NormalizeClanName(string value)
    {
        return WebUtility.HtmlDecode(value ?? "").Trim();
    }

    private static string NormalizeClanTag(string value)
    {
        return WebUtility.HtmlDecode(value ?? "").Trim().ToUpperInvariant();
    }

    private static string Trim(string value, int max)
    {
        value ??= "";
        value = value.Trim();

        if (value.Length <= max)
            return value;

        return value[..max];
    }

    private sealed class AccountClanRow
    {
        public int CustomerId { get; set; }
        public int ClanId { get; set; }
        public int ClanRank { get; set; }
        public string Gamertag { get; set; } = "";
    }

    private sealed class ClanSlotRow
    {
        public int MaxMembers { get; set; }
        public int CurrentMembers { get; set; }
    }

    private sealed class ClanInviteRow
    {
        public int InviteId { get; set; }
        public int ClanId { get; set; }
        public int InviterId { get; set; }
        public int CustomerId { get; set; }
    }

    private sealed class ClanApplicationRow
    {
        public int ApplicationId { get; set; }
        public int ClanId { get; set; }
        public int CustomerId { get; set; }
    }
}