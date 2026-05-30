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
        await using var db = _db.CreateConnection();

        var exists = await TableExistsAsync(db, "friends");

        if (!exists)
            return new FriendsListResponse(true, 0, "OK", Array.Empty<SocialFriendDto>());

        var rows = await db.QueryAsync<SocialFriendDto>(
            """
            SELECT
                CASE
                    WHEN f.CustomerID = @CustomerId THEN f.FriendID
                    ELSE f.CustomerID
                END AS CustomerId,
                COALESCE(p.Gamertag, a.AccountName, '') AS Gamertag,
                1 AS Status,
                0 AS Online,
                COALESCE(p.HardcoreLevel, 0) AS Level,
                COALESCE(p.HardcoreXP, 0) AS XP
            FROM friends f
            LEFT JOIN profile_chars p
                ON p.CustomerID = CASE
                    WHEN f.CustomerID = @CustomerId THEN f.FriendID
                    ELSE f.CustomerID
                END
            LEFT JOIN loginid a
                ON a.CustomerID = CASE
                    WHEN f.CustomerID = @CustomerId THEN f.FriendID
                    ELSE f.CustomerID
                END
            WHERE f.CustomerID = @CustomerId
               OR f.FriendID = @CustomerId
            ORDER BY Gamertag;
            """,
            new { CustomerId = customerId });

        return new FriendsListResponse(true, 0, "OK", rows.ToArray());
    }

    public async Task<bool> AddFriendRequestAsync(
        ulong customerId,
        ulong friendCustomerId,
        string? friendName)
    {
        await using var db = _db.CreateConnection();

        var hasFriendRequests = await TableExistsAsync(db, "friend_requests");

        if (hasFriendRequests && friendCustomerId != 0)
        {
            await db.ExecuteAsync(
                """
                INSERT IGNORE INTO friend_requests
                (
                    CustomerID,
                    FriendID,
                    CreatedAt
                )
                VALUES
                (
                    @CustomerId,
                    @FriendId,
                    UTC_TIMESTAMP()
                );
                """,
                new
                {
                    CustomerId = customerId,
                    FriendId = friendCustomerId
                });

            return true;
        }

        return true;
    }

    public async Task<bool> AnswerFriendRequestAsync(
        ulong customerId,
        ulong friendCustomerId,
        bool accept)
    {
        await using var db = _db.CreateConnection();

        var hasFriends = await TableExistsAsync(db, "friends");
        var hasFriendRequests = await TableExistsAsync(db, "friend_requests");

        if (accept && hasFriends && friendCustomerId != 0)
        {
            await db.ExecuteAsync(
                """
                INSERT IGNORE INTO friends
                (
                    CustomerID,
                    FriendID,
                    CreatedAt
                )
                VALUES
                (
                    @CustomerId,
                    @FriendId,
                    UTC_TIMESTAMP()
                );
                """,
                new
                {
                    CustomerId = customerId,
                    FriendId = friendCustomerId
                });
        }

        if (hasFriendRequests && friendCustomerId != 0)
        {
            await db.ExecuteAsync(
                """
                DELETE FROM friend_requests
                WHERE CustomerID = @FriendId
                  AND FriendID = @CustomerId;
                """,
                new
                {
                    CustomerId = customerId,
                    FriendId = friendCustomerId
                });
        }

        return true;
    }

    public async Task<bool> RemoveFriendAsync(ulong customerId, ulong friendCustomerId)
    {
        await using var db = _db.CreateConnection();

        var hasFriends = await TableExistsAsync(db, "friends");

        if (!hasFriends || friendCustomerId == 0)
            return true;

        await db.ExecuteAsync(
            """
            DELETE FROM friends
            WHERE (CustomerID = @CustomerId AND FriendID = @FriendId)
               OR (CustomerID = @FriendId AND FriendID = @CustomerId);
            """,
            new
            {
                CustomerId = customerId,
                FriendId = friendCustomerId
            });

        return true;
    }

    public async Task<ClanResponse> GetClanAsync(ulong customerId)
    {
        await using var db = _db.CreateConnection();

        var hasClans = await TableExistsAsync(db, "clans");
        var hasClanMembers = await TableExistsAsync(db, "clan_members");

        if (!hasClans || !hasClanMembers)
            return new ClanResponse(true, 0, "OK", null);

        var clan = await db.QueryFirstOrDefaultAsync<SocialClanDto>(
            """
            SELECT
                c.ClanID AS ClanId,
                c.Name AS Name,
                c.Tag AS Tag,
                c.OwnerCustomerID AS OwnerCustomerId,
                COALESCE(c.Level, 0) AS Level,
                COALESCE(c.XP, 0) AS XP,
                (
                    SELECT COUNT(*)
                    FROM clan_members cm2
                    WHERE cm2.ClanID = c.ClanID
                ) AS MemberCount
            FROM clan_members cm
            INNER JOIN clans c ON c.ClanID = cm.ClanID
            WHERE cm.CustomerID = @CustomerId
            LIMIT 1;
            """,
            new { CustomerId = customerId });

        return new ClanResponse(true, 0, "OK", clan);
    }

    public async Task<ClanResponse> CreateClanAsync(
        ulong customerId,
        string nameRaw,
        string tagRaw)
    {
        var name = NormalizeClanName(nameRaw);
        var tag = NormalizeClanTag(tagRaw);

        if (name.Length < 3 || name.Length > 32)
            return new ClanResponse(false, 400, "BAD_CLAN_NAME", null);

        if (tag.Length < 2 || tag.Length > 5)
            return new ClanResponse(false, 400, "BAD_CLAN_TAG", null);

        await using var db = _db.CreateConnection();

        var hasClans = await TableExistsAsync(db, "clans");
        var hasClanMembers = await TableExistsAsync(db, "clan_members");

        if (!hasClans || !hasClanMembers)
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

        var alreadyInClan = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM clan_members
            WHERE CustomerID = @CustomerId;
            """,
            new { CustomerId = customerId });

        if (alreadyInClan > 0)
            return new ClanResponse(false, 409, "ALREADY_IN_CLAN", null);

        var nameUsed = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM clans
            WHERE Name = @Name OR Tag = @Tag;
            """,
            new
            {
                Name = name,
                Tag = tag
            });

        if (nameUsed > 0)
            return new ClanResponse(false, 409, "CLAN_EXISTS", null);

        await db.ExecuteAsync(
            """
            INSERT INTO clans
            (
                Name,
                Tag,
                OwnerCustomerID,
                Level,
                XP,
                CreatedAt
            )
            VALUES
            (
                @Name,
                @Tag,
                @OwnerCustomerId,
                0,
                0,
                UTC_TIMESTAMP()
            );
            """,
            new
            {
                Name = name,
                Tag = tag,
                OwnerCustomerId = customerId
            });

        var clanId = await db.ExecuteScalarAsync<ulong>(
            "SELECT LAST_INSERT_ID();");

        await db.ExecuteAsync(
            """
            INSERT INTO clan_members
            (
                ClanID,
                CustomerID,
                Rank,
                JoinedAt
            )
            VALUES
            (
                @ClanId,
                @CustomerId,
                99,
                UTC_TIMESTAMP()
            );
            """,
            new
            {
                ClanId = clanId,
                CustomerId = customerId
            });

        return new ClanResponse(
            true,
            0,
            "OK",
            new SocialClanDto(
                clanId,
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
        LegacyUtil.AppendXmlAttr(xml, "owner", clan.OwnerCustomerId);
        LegacyUtil.AppendXmlAttr(xml, "level", clan.Level);
        LegacyUtil.AppendXmlAttr(xml, "xp", clan.XP);
        LegacyUtil.AppendXmlAttr(xml, "members", clan.MemberCount);
        xml.Append("></clan>");

        return xml.ToString();
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
}