using System.Net;
using System.Text;
using Dapper;
using WarInc.Api.Common;
using WarInc.Api.Database;

namespace WarInc.Api.Social;

public sealed class SocialService
{
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

                CASE
                    WHEN s.CustomerID IS NULL THEN 0
                    ELSE 1
                END AS Online,

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
            LEFT JOIN leaderboard lb
                ON lb.CustomerID = l.CustomerID
            WHERE f.CustomerID = @CustomerId
               OR f.FriendID = @CustomerId
            GROUP BY
                CustomerId,
                Gamertag,
                Status,
                Level,
                XP
            ORDER BY Gamertag;
            """,
            new { CustomerId = (int)customerId });

        return new FriendsListResponse(
            true,
            0,
            "OK",
            rows.ToArray());
    }

    public async Task<bool> AddFriendRequestAsync(
        ulong customerId,
        ulong friendCustomerId,
        string? friendName)
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

    public async Task<bool> AnswerFriendRequestAsync(
        ulong customerId,
        ulong friendCustomerId,
        bool accept)
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

        await InsertFriendRowIfMissingAsync(
            db,
            (int)friendCustomerId,
            (int)customerId,
            1);

        await InsertFriendRowIfMissingAsync(
            db,
            (int)customerId,
            (int)friendCustomerId,
            1);

        return true;
    }

    public async Task<bool> RemoveFriendAsync(
        ulong customerId,
        ulong friendCustomerId)
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
                CASE
                    WHEN COALESCE(c.NumClanMembers, 0) > 0 THEN c.NumClanMembers
                    ELSE (
                        SELECT COUNT(*)
                        FROM loginid l2
                        WHERE l2.ClanID = c.ClanID
                    )
                END AS MemberCount
            FROM loginid l
            INNER JOIN clandata c ON c.ClanID = l.ClanID
            WHERE l.CustomerID = @CustomerId
              AND l.ClanID > 0
            LIMIT 1;
            """,
            new { CustomerId = (int)customerId });

        return new ClanResponse(true, 0, "OK", clan);
    }

    public async Task<ClanResponse> CreateClanAsync(
        ulong customerId,
        string nameRaw,
        string tagRaw)
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
        {
            return new ClanResponse(
                true,
                0,
                "OK",
                new SocialClanDto(
                    0,
                    name,
                    tag,
                    customerId,
                    0,
                    0,
                    1));
        }

        var account = await db.QueryFirstOrDefaultAsync<AccountClanRow>(
            """
            SELECT
                CustomerID AS CustomerId,
                ClanID AS ClanId,
                ClanRank AS ClanRank
            FROM loginid
            WHERE CustomerID = @CustomerId
            LIMIT 1;
            """,
            new { CustomerId = (int)customerId });

        if (account == null)
            return new ClanResponse(false, 404, "ACCOUNT_NOT_FOUND", null);

        if (account.ClanId > 0)
            return new ClanResponse(false, 409, "ALREADY_IN_CLAN", null);

        var exists = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM clandata
            WHERE ClanName = @Name
               OR ClanTag = @Tag;
            """,
            new
            {
                Name = name,
                Tag = tag
            });

        if (exists > 0)
            return new ClanResponse(false, 409, "CLAN_EXISTS", null);

        await db.ExecuteAsync(
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
            """,
            new
            {
                Name = name,
                Tag = tag,
                OwnerId = (int)customerId
            });

        var clanId = await db.ExecuteScalarAsync<int>(
            "SELECT LAST_INSERT_ID();");

        await db.ExecuteAsync(
            """
            UPDATE loginid
            SET
                ClanID = @ClanId,
                ClanRank = 99,
                ClanContributedXP = 0,
                ClanContributedGP = 0
            WHERE CustomerID = @CustomerId;
            """,
            new
            {
                ClanId = clanId,
                CustomerId = (int)customerId
            });

        return new ClanResponse(
            true,
            0,
            "OK",
            new SocialClanDto(
                (ulong)clanId,
                name,
                tag,
                customerId,
                0,
                0,
                1));
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
            return "<clan id=\"0\" name=\"\" tag=\"\" owner=\"0\" level=\"0\" xp=\"0\" members=\"0\"></clan>";

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
        xml.Append("></clan>");

        return xml.ToString();
    }

    private static async Task InsertFriendRowIfMissingAsync(
        MySqlConnector.MySqlConnection db,
        int customerId,
        int friendId,
        int status)
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

    private static async Task<ulong> ResolveCustomerIdByNameAsync(
        MySqlConnector.MySqlConnection db,
        string nameRaw)
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
            new { Name = name });

        return customerId.HasValue && customerId.Value > 0
            ? (ulong)customerId.Value
            : 0;
    }

    private static async Task<bool> TableExistsAsync(
        MySqlConnector.MySqlConnection db,
        string table)
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

    private sealed class AccountClanRow
    {
        public int CustomerId { get; set; }
        public int ClanId { get; set; }
        public int ClanRank { get; set; }
    }
}