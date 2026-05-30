using System.Text;
using Dapper;
using MySqlConnector;
using WarInc.Api.Common;
using WarInc.Api.Database;

namespace WarInc.Api.Profile;

public sealed class ProfileService
{
    private readonly WarIncDb _db;

    public ProfileService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<ProfileBundle> GetProfileDataAsync(
        ulong customerId,
        bool joiningGame)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return ProfileBundle.Fail(400, "BAD_CUSTOMER_ID");

        await using var db = _db.CreateConnection();

        if (joiningGame)
        {
            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET lastjoineddate = UTC_TIMESTAMP()
                WHERE CustomerID = @CustomerId;
                """,
                new { CustomerId = customerId });
        }

        var account = await db.QueryFirstOrDefaultAsync<ProfileAccountRow>(
            """
            SELECT
                l.CustomerID AS CustomerId,
                l.AccountStatus AS AccountStatus,
                l.GamePoints AS GamePoints,
                l.GameDollars AS GameDollars,
                l.HonorPoints AS HonorPoints,
                l.SkillPoints AS SkillPoints,
                l.Gamertag AS Gamertag,

                l.Faction1Score AS Faction1Score,
                l.Faction2Score AS Faction2Score,
                l.Faction3Score AS Faction3Score,
                l.Faction4Score AS Faction4Score,
                l.Faction5Score AS Faction5Score,

                l.ClanID AS ClanId,
                l.ClanRank AS ClanRank,
                l.IsFPSEnabled AS IsFPSEnabled,
                l.IsDeveloper AS IsDeveloper,

                IFNULL(s.Kills, 0) AS Kills,
                IFNULL(s.Deaths, 0) AS Deaths,
                IFNULL(s.ShotsFired, 0) AS ShotsFired,
                IFNULL(s.ShotsHits, 0) AS ShotsHits,
                IFNULL(s.Headshots, 0) AS Headshots,
                IFNULL(s.AssistKills, 0) AS AssistKills,
                IFNULL(s.Wins, 0) AS Wins,
                IFNULL(s.Losses, 0) AS Losses,
                IFNULL(s.CaptureNeutralPoints, 0) AS CaptureNeutralPoints,
                IFNULL(s.CaptureEnemyPoints, 0) AS CaptureEnemyPoints,
                IFNULL(s.TimePlayed, 0) AS TimePlayed,

                IFNULL(pd.Skills, '') AS Skills,
                IFNULL(pd.Abilities, '') AS Abilities,

                IFNULL(cd.ClanTag, '') AS ClanTag,
                IFNULL(cd.ClanTagColor, 0) AS ClanTagColor
            FROM loginid l
            LEFT JOIN stats s ON s.CustomerID = l.CustomerID
            LEFT JOIN profiledata pd ON pd.CustomerID = l.CustomerID
            LEFT JOIN clandata cd ON cd.ClanID = l.ClanID
            WHERE l.CustomerID = @CustomerId
            LIMIT 1;
            """,
            new { CustomerId = customerId });

        if (account == null)
            return ProfileBundle.Fail(404, "ACCOUNT_NOT_FOUND");

        var loadouts = (await db.QueryAsync<ProfileLoadoutRow>(
            """
            SELECT
                LoadoutID AS LoadoutId,
                Class AS Class,
                HonorPoints AS HonorPoints,
                TimePlayed AS TimePlayed,
                Loadout AS Loadout,
                Skills AS Skills,
                SpendSP1 AS SpendSP1,
                SpendSP2 AS SpendSP2,
                SpendSP3 AS SpendSP3
            FROM profile_chars
            WHERE CustomerID = @CustomerId
            ORDER BY LoadoutID ASC;
            """,
            new { CustomerId = customerId })).ToList();

        var achievements = (await db.QueryAsync<ProfileAchievementRow>(
            """
            SELECT
                AchID AS AchId,
                Value AS Value,
                Unlocked AS Unlocked
            FROM achievements
            WHERE CustomerID = @CustomerId
            ORDER BY AchID ASC;
            """,
            new { CustomerId = customerId })).ToList();

        var fpsAttachments = (await db.QueryAsync<ProfileFpsAttachmentRow>(
            """
            SELECT
                WeaponID AS WeaponId,
                AttachmentID AS AttachmentId,
                GREATEST(TIMESTAMPDIFF(MINUTE, UTC_TIMESTAMP(), LeasedUntil), 0) AS MinutesLeft,
                IsEquipped AS IsEquipped
            FROM inventory_fps
            WHERE CustomerID = @CustomerId
              AND LeasedUntil > UTC_TIMESTAMP()
            ORDER BY WeaponID ASC, Slot ASC;
            """,
            new { CustomerId = customerId })).ToList();

        var inventory = (await db.QueryAsync<ProfileInventoryRow>(
            """
            SELECT
                ItemID AS ItemId,
                GREATEST(TIMESTAMPDIFF(MINUTE, UTC_TIMESTAMP(), LeasedUntil), 0) AS MinutesLeft,
                Quantity AS Quantity
            FROM inventory
            WHERE CustomerID = @CustomerId
              AND LeasedUntil > UTC_TIMESTAMP()
            ORDER BY ItemID ASC;
            """,
            new { CustomerId = customerId })).ToList();

        var newItems = (await db.QueryAsync<int>(
            """
            SELECT ItemID
            FROM items_weapons
            WHERE IsNew > 0
              AND (Price1 + Price7 + Price30 + PriceP + GPrice1 + GPrice7 + GPrice30 + GPriceP) > 0

            UNION

            SELECT ItemID
            FROM items_gear
            WHERE IsNew > 0
              AND (Price1 + Price7 + Price30 + PriceP + GPrice1 + GPrice7 + GPrice30 + GPriceP) > 0

            UNION

            SELECT ItemID
            FROM items_generic
            WHERE IsNew > 0
              AND (Price1 + Price7 + Price30 + PriceP + GPrice1 + GPrice7 + GPrice30 + GPriceP) > 0

            UNION

            SELECT ItemID
            FROM items_packages
            WHERE IsNew > 0
              AND (Price1 + Price7 + Price30 + PriceP + GPrice1 + GPrice7 + GPrice30 + GPriceP) > 0

            ORDER BY ItemID ASC;
            """)).ToList();

        var now = DateTime.UtcNow;
        var dayStart = now.Date;
        var dayEnd = dayStart.AddDays(1);

        var weekStart = now.Date.AddDays(-(int)now.DayOfWeek);
        var weekEnd = weekStart.AddDays(7);

        var dailyStats = await GetProfileStatsWindowAsync(db, customerId, dayStart, dayEnd);
        var weeklyStats = await GetProfileStatsWindowAsync(db, customerId, weekStart, weekEnd);

        return new ProfileBundle
        {
            Ok = true,
            Code = 0,
            Message = "OK",
            Account = account,
            Loadouts = loadouts,
            Achievements = achievements,
            FpsAttachments = fpsAttachments,
            Inventory = inventory,
            NewItems = newItems,
            DailyStats = dailyStats,
            WeeklyStats = weeklyStats
        };
    }

    public string BuildLegacyProfileXml(ProfileBundle profile)
    {
        var account = profile.Account ?? throw new Exception("Profile account is null");

        var now = DateTime.Now;
        var curTime = $"{now.Year} {now.Month} {now.Day} {now.Hour} {now.Minute}";

        var xml = new StringBuilder();

        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append("<account ");

        LegacyUtil.AppendXmlAttr(xml, "CustomerID", account.CustomerId);
        LegacyUtil.AppendXmlAttr(xml, "AccountStatus", account.AccountStatus);
        LegacyUtil.AppendXmlAttr(xml, "gamertag", account.Gamertag.TrimEnd());
        LegacyUtil.AppendXmlAttr(xml, "gamepoints", account.GamePoints);
        LegacyUtil.AppendXmlAttr(xml, "GameDollars", account.GameDollars);
        LegacyUtil.AppendXmlAttr(xml, "HonorPoints", account.HonorPoints);
        LegacyUtil.AppendXmlAttr(xml, "SkillPoints", account.SkillPoints);

        LegacyUtil.AppendXmlAttr(xml, "ClanID", account.ClanId);
        LegacyUtil.AppendXmlAttr(xml, "ClanRank", account.ClanRank);
        LegacyUtil.AppendXmlAttr(xml, "ClanTag", account.ClanTag);
        LegacyUtil.AppendXmlAttr(xml, "ClanTagColor", account.ClanTagColor);

        LegacyUtil.AppendXmlAttr(xml, "Kills", account.Kills);
        LegacyUtil.AppendXmlAttr(xml, "Deaths", account.Deaths);
        LegacyUtil.AppendXmlAttr(xml, "ShotsFired", account.ShotsFired);
        LegacyUtil.AppendXmlAttr(xml, "ShotsHits", account.ShotsHits);
        LegacyUtil.AppendXmlAttr(xml, "Headshots", account.Headshots);
        LegacyUtil.AppendXmlAttr(xml, "AssistKills", account.AssistKills);
        LegacyUtil.AppendXmlAttr(xml, "Wins", account.Wins);
        LegacyUtil.AppendXmlAttr(xml, "Losses", account.Losses);
        LegacyUtil.AppendXmlAttr(xml, "CaptureNeutralPoints", account.CaptureNeutralPoints);
        LegacyUtil.AppendXmlAttr(xml, "CaptureEnemyPoints", account.CaptureEnemyPoints);
        LegacyUtil.AppendXmlAttr(xml, "TimePlayed", account.TimePlayed);

        LegacyUtil.AppendXmlAttr(xml, "Abilities", account.Abilities);
        LegacyUtil.AppendXmlAttr(xml, "F1S", account.Faction1Score);
        LegacyUtil.AppendXmlAttr(xml, "F2S", account.Faction2Score);
        LegacyUtil.AppendXmlAttr(xml, "F3S", account.Faction3Score);
        LegacyUtil.AppendXmlAttr(xml, "F4S", account.Faction4Score);
        LegacyUtil.AppendXmlAttr(xml, "F5S", account.Faction5Score);
        LegacyUtil.AppendXmlAttr(xml, "time", curTime);
        LegacyUtil.AppendXmlAttr(xml, "IsFPSEnabled", account.IsFPSEnabled);

        if (account.IsDeveloper != 0)
            LegacyUtil.AppendXmlAttr(xml, "IsDev", account.IsDeveloper);

        xml.Append(">\n");

        xml.Append("<loadouts>\n");
        foreach (var loadout in profile.Loadouts)
        {
            xml.Append("<l ");
            LegacyUtil.AppendXmlAttr(xml, "id", loadout.LoadoutId);
            LegacyUtil.AppendXmlAttr(xml, "cl", loadout.Class);
            LegacyUtil.AppendXmlAttr(xml, "xp", loadout.HonorPoints);
            LegacyUtil.AppendXmlAttr(xml, "tm", loadout.TimePlayed);
            LegacyUtil.AppendXmlAttr(xml, "lo", loadout.Loadout);
            LegacyUtil.AppendXmlAttr(xml, "sp1", loadout.SpendSP1);
            LegacyUtil.AppendXmlAttr(xml, "sp2", loadout.SpendSP2);
            LegacyUtil.AppendXmlAttr(xml, "sp3", loadout.SpendSP3);
            LegacyUtil.AppendXmlAttr(xml, "sv", TrimLegacySkills(loadout.Skills));
            xml.Append("/>\n");
        }
        xml.Append("</loadouts>\n");

        xml.Append("<achievements>\n");
        foreach (var achievement in profile.Achievements)
        {
            xml.Append("<a ");
            LegacyUtil.AppendXmlAttr(xml, "id", achievement.AchId);
            LegacyUtil.AppendXmlAttr(xml, "v", achievement.Value);
            LegacyUtil.AppendXmlAttr(xml, "u", achievement.Unlocked);
            xml.Append("/>\n");
        }
        xml.Append("</achievements>\n");

        xml.Append("<fpsattach>\n");
        foreach (var item in profile.FpsAttachments)
        {
            xml.Append("<i ");
            LegacyUtil.AppendXmlAttr(xml, "wi", item.WeaponId);
            LegacyUtil.AppendXmlAttr(xml, "ai", item.AttachmentId);
            LegacyUtil.AppendXmlAttr(xml, "ml", item.MinutesLeft);

            if (item.IsEquipped > 0)
                LegacyUtil.AppendXmlAttr(xml, "eq", 1);

            xml.Append("/>\n");
        }
        xml.Append("</fpsattach>\n");

        xml.Append("<inventory>\n");
        foreach (var item in profile.Inventory)
        {
            xml.Append("<i ");
            LegacyUtil.AppendXmlAttr(xml, "id", item.ItemId);
            LegacyUtil.AppendXmlAttr(xml, "ml", item.MinutesLeft);

            if (item.Quantity > 1)
                LegacyUtil.AppendXmlAttr(xml, "qt", item.Quantity);

            xml.Append("/>\n");
        }
        xml.Append("</inventory>\n");

        xml.Append("<nis>\n");
        foreach (var itemId in profile.NewItems)
        {
            xml.Append("<i ");
            LegacyUtil.AppendXmlAttr(xml, "id", itemId);
            xml.Append("/>\n");
        }
        xml.Append("</nis>\n");

        AppendLegacyStatsXml(xml, "sday", profile.DailyStats);
        AppendLegacyStatsXml(xml, "sweek", profile.WeeklyStats);

        xml.Append("</account>");

        return xml.ToString();
    }

    public async Task EnsureStarterProfileAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        ulong customerId)
    {
        var loadoutCount = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM profile_chars
            WHERE CustomerID = @CustomerId;
            """,
            new { CustomerId = customerId },
            tx);

        if (loadoutCount > 0)
            return;

        const string loadout = "20061 0 20019 20026 0 0 0 0 0 0 101002 101158 101004";

        await db.ExecuteAsync(
            """
            INSERT INTO profile_chars
            (
                CustomerID,
                Class,
                HonorPoints,
                TimePlayed,
                Loadout,
                Skills,
                SpendSP1,
                SpendSP2,
                SpendSP3
            )
            VALUES
            (
                @CustomerId,
                0,
                0,
                0,
                @Loadout,
                '0000000000000000000000000000000',
                0,
                0,
                0
            );
            """,
            new
            {
                CustomerId = customerId,
                Loadout = loadout
            },
            tx);

        var starterItems = new[]
        {
            20061,
            20019,
            20026,
            101002,
            101158,
            101004
        };

        foreach (var itemId in starterItems)
        {
            await db.ExecuteAsync(
                """
                INSERT INTO inventory
                (
                    CustomerID,
                    ItemID,
                    LeasedUntil,
                    Quantity,
                    UpSlot1,
                    UpSlot2,
                    UpSlot3,
                    UpSlot4,
                    UpSlot5
                )
                SELECT
                    @CustomerId,
                    @ItemId,
                    DATE_ADD(UTC_TIMESTAMP(), INTERVAL 2000 DAY),
                    1,
                    0,
                    0,
                    0,
                    0,
                    0
                WHERE NOT EXISTS
                (
                    SELECT 1
                    FROM inventory
                    WHERE CustomerID = @CustomerId
                      AND ItemID = @ItemId
                    LIMIT 1
                );
                """,
                new
                {
                    CustomerId = customerId,
                    ItemId = itemId
                },
                tx);
        }
    }

    private static async Task<ProfileStatsWindowRow> GetProfileStatsWindowAsync(
        MySqlConnection db,
        ulong customerId,
        DateTime start,
        DateTime end)
    {
        return await db.QueryFirstAsync<ProfileStatsWindowRow>(
            """
            SELECT
                COUNT(*) AS DailyGames,
                IFNULL(SUM(Kills), 0) AS Kills,
                IFNULL(SUM(Headshots), 0) AS Headshots,
                IFNULL(SUM(CaptureEnemyPoints), 0) AS CaptureFlags,

                IFNULL(SUM(CASE WHEN MapType = 0 THEN 1 ELSE 0 END), 0) AS MatchesCQ,
                IFNULL(SUM(CASE WHEN MapType = 1 THEN 1 ELSE 0 END), 0) AS MatchesDM,
                IFNULL(SUM(CASE WHEN MapType = 3 THEN 1 ELSE 0 END), 0) AS MatchesSB
            FROM dbg_userroundresults
            WHERE CustomerID = @CustomerId
              AND GameReportTime >= @Start
              AND GameReportTime < @End;
            """,
            new
            {
                CustomerId = customerId,
                Start = start,
                End = end
            });
    }

    private static void AppendLegacyStatsXml(
        StringBuilder xml,
        string nodeName,
        ProfileStatsWindowRow stats)
    {
        xml.Append("<");
        xml.Append(nodeName);
        xml.Append(" ");

        LegacyUtil.AppendXmlAttr(xml, "dg", stats.DailyGames);
        LegacyUtil.AppendXmlAttr(xml, "ki", stats.Kills);
        LegacyUtil.AppendXmlAttr(xml, "hs", stats.Headshots);
        LegacyUtil.AppendXmlAttr(xml, "cf", stats.CaptureFlags);
        LegacyUtil.AppendXmlAttr(xml, "mcq", stats.MatchesCQ);
        LegacyUtil.AppendXmlAttr(xml, "mdm", stats.MatchesDM);
        LegacyUtil.AppendXmlAttr(xml, "msb", stats.MatchesSB);

        xml.Append("/>\n");
    }

    private static string TrimLegacySkills(string? value)
    {
        return (value ?? "").TrimEnd(' ', '0');
    }
}