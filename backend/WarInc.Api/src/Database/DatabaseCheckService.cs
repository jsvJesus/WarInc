using Dapper;
using MySqlConnector;

namespace WarInc.Api.Database;

public sealed class DatabaseCheckService
{
    private readonly WarIncDb _db;

    private static readonly string[] RequiredTables =
    [
        "loginid",
        "logins",
        "loginsessions",
        "accountinfo",

        "profiledata",
        "profile_chars",
        "stats",
        "achievements",

        "inventory",
        "inventory_fps",

        "items_gear",
        "items_weapons",
        "items_generic",
        "items_packages",
        "items_attachments",
        "items_upgradedata",
        "items_lootdata",

        "friendsmap",
        "clandata",
        "claninvites",
        "clanapplications",
        "clanevents",

        "leaderboard",
        "leaderboard1",
        "leaderboard7",
        "leaderboard30",

        "dataretentionbonuses",
        "datagamerewards",
        "dataskill2price",
        "dataskillprice",
        "datarankpoints",
        "dataclanxp",

        "masterserverinfo",

        "financialtransactions",
        "dbg_gptransactions",
        "dbg_hwinfo",
        "dbg_srvloginfo",
        "dbg_userroundresults",
        "dbg_lootrewards",
        "cheatlog"
    ];

    private static readonly DatabaseRequiredColumn[] RequiredColumns =
    [
        new("loginid", "CustomerID", "int"),
        new("loginid", "AccountName", "string"),
        new("loginid", "Password", "string"),
        new("loginid", "MD5Password", "string"),
        new("loginid", "AccountStatus", "int"),
        new("loginid", "GamePoints", "int"),
        new("loginid", "HonorPoints", "int"),
        new("loginid", "SkillPoints", "int"),
        new("loginid", "Gamertag", "string"),
        new("loginid", "dateregistered", "datetime"),
        new("loginid", "lastlogindate", "datetime"),
        new("loginid", "lastloginIP", "string"),
        new("loginid", "lastgamedate", "datetime"),
        new("loginid", "ReferralID", "int"),
        new("loginid", "lastjoineddate", "datetime"),
        new("loginid", "ClanID", "int"),
        new("loginid", "ClanRank", "int"),
        new("loginid", "GameDollars", "int"),
        new("loginid", "Faction1Score", "int"),
        new("loginid", "Faction2Score", "int"),
        new("loginid", "Faction3Score", "int"),
        new("loginid", "Faction4Score", "int"),
        new("loginid", "Faction5Score", "int"),
        new("loginid", "lastRetBonusDate", "datetime"),
        new("loginid", "IsFPSEnabled", "int"),
        new("loginid", "reg_sid", "string"),
        new("loginid", "ClanContributedXP", "int"),
        new("loginid", "ClanContributedGP", "int"),
        new("loginid", "IsDeveloper", "int"),

        new("logins", "LoginID", "int"),
        new("logins", "CustomerID", "int"),
        new("logins", "LoginTime", "datetime"),
        new("logins", "IP", "string"),
        new("logins", "LoginSource", "int"),

        new("loginsessions", "CustomerID", "int"),
        new("loginsessions", "SessionKey", "string"),
        new("loginsessions", "SessionID", "int"),
        new("loginsessions", "LoginIP", "string"),
        new("loginsessions", "TimeLogged", "datetime"),
        new("loginsessions", "TimeUpdated", "datetime"),
        new("loginsessions", "GameSessionID", "bigint"),

        new("accountinfo", "CustomerID", "int"),
        new("accountinfo", "email", "string"),

        new("profiledata", "CustomerID", "int"),
        new("profiledata", "Skills", "string"),
        new("profiledata", "Achievements", "string"),
        new("profiledata", "Abilities", "string"),

        new("profile_chars", "LoadoutID", "int"),
        new("profile_chars", "CustomerID", "int"),
        new("profile_chars", "Class", "int"),
        new("profile_chars", "HonorPoints", "int"),
        new("profile_chars", "TimePlayed", "int"),
        new("profile_chars", "Loadout", "string"),
        new("profile_chars", "Skills", "string"),
        new("profile_chars", "SpendSP1", "int"),
        new("profile_chars", "SpendSP2", "int"),
        new("profile_chars", "SpendSP3", "int"),

        new("stats", "CustomerID", "int"),
        new("stats", "Kills", "int"),
        new("stats", "Deaths", "int"),
        new("stats", "ShotsFired", "int"),
        new("stats", "ShotsHits", "int"),
        new("stats", "Headshots", "int"),
        new("stats", "AssistKills", "int"),
        new("stats", "Wins", "int"),
        new("stats", "Losses", "int"),
        new("stats", "CaptureNeutralPoints", "int"),
        new("stats", "CaptureEnemyPoints", "int"),
        new("stats", "TimePlayed", "int"),
        new("stats", "CheatAttempts", "int"),

        new("achievements", "CustomerID", "int"),
        new("achievements", "AchID", "int"),
        new("achievements", "Value", "int"),
        new("achievements", "Unlocked", "int"),

        new("inventory", "CustomerID", "int"),
        new("inventory", "ItemID", "int"),
        new("inventory", "LeasedUntil", "datetime"),
        new("inventory", "Quantity", "int"),
        new("inventory", "UpSlot1", "int"),
        new("inventory", "UpSlot2", "int"),
        new("inventory", "UpSlot3", "int"),
        new("inventory", "UpSlot4", "int"),
        new("inventory", "UpSlot5", "int"),

        new("inventory_fps", "CustomerID", "int"),
        new("inventory_fps", "WeaponID", "int"),
        new("inventory_fps", "AttachmentID", "int"),
        new("inventory_fps", "LeasedUntil", "datetime"),
        new("inventory_fps", "Slot", "int"),
        new("inventory_fps", "IsEquipped", "int"),

        new("items_gear", "ItemID", "int"),
        new("items_gear", "FNAME", "string"),
        new("items_gear", "Name", "string"),
        new("items_gear", "Description", "string"),
        new("items_gear", "Category", "int"),
        new("items_gear", "Weight", "int"),
        new("items_gear", "DamagePerc", "int"),
        new("items_gear", "DamageMax", "int"),
        new("items_gear", "Price1", "int"),
        new("items_gear", "Price7", "int"),
        new("items_gear", "Price30", "int"),
        new("items_gear", "PriceP", "int"),
        new("items_gear", "GPrice1", "int"),
        new("items_gear", "GPrice7", "int"),
        new("items_gear", "GPrice30", "int"),
        new("items_gear", "GPriceP", "int"),
        new("items_gear", "IsNew", "int"),
        new("items_gear", "LevelRequired", "int"),

        new("items_weapons", "ItemID", "int"),
        new("items_weapons", "FNAME", "string"),
        new("items_weapons", "Category", "int"),
        new("items_weapons", "Name", "string"),
        new("items_weapons", "Description", "string"),
        new("items_weapons", "Damage", "float"),
        new("items_weapons", "DamageDecay", "float"),
        new("items_weapons", "NumClips", "int"),
        new("items_weapons", "Clipsize", "int"),
        new("items_weapons", "RateOfFire", "int"),
        new("items_weapons", "Spread", "float"),
        new("items_weapons", "Recoil", "float"),
        new("items_weapons", "Price1", "int"),
        new("items_weapons", "Price7", "int"),
        new("items_weapons", "Price30", "int"),
        new("items_weapons", "PriceP", "int"),
        new("items_weapons", "GPrice1", "int"),
        new("items_weapons", "GPrice7", "int"),
        new("items_weapons", "GPrice30", "int"),
        new("items_weapons", "GPriceP", "int"),
        new("items_weapons", "IsNew", "int"),
        new("items_weapons", "LevelRequired", "int"),
        new("items_weapons", "IsFPS", "int"),
        new("items_weapons", "FPSAttach0", "int"),
        new("items_weapons", "FPSAttach1", "int"),
        new("items_weapons", "FPSAttach2", "int"),
        new("items_weapons", "FPSAttach3", "int"),
        new("items_weapons", "FPSAttach4", "int"),
        new("items_weapons", "FPSAttach5", "int"),
        new("items_weapons", "FPSAttach6", "int"),
        new("items_weapons", "FPSAttach7", "int"),
        new("items_weapons", "FPSAttach8", "int"),

        new("items_generic", "ItemID", "int"),
        new("items_generic", "FNAME", "string"),
        new("items_generic", "Category", "int"),
        new("items_generic", "Name", "string"),
        new("items_generic", "Description", "string"),
        new("items_generic", "Price1", "int"),
        new("items_generic", "Price7", "int"),
        new("items_generic", "Price30", "int"),
        new("items_generic", "PriceP", "int"),
        new("items_generic", "GPrice1", "int"),
        new("items_generic", "GPrice7", "int"),
        new("items_generic", "GPrice30", "int"),
        new("items_generic", "GPriceP", "int"),
        new("items_generic", "IsNew", "int"),
        new("items_generic", "LevelRequired", "int"),
        new("items_generic", "IsStackable", "int"),

        new("items_packages", "ItemID", "int"),
        new("items_packages", "FNAME", "string"),
        new("items_packages", "Name", "string"),
        new("items_packages", "Description", "string"),
        new("items_packages", "Category", "int"),
        new("items_packages", "Price1", "int"),
        new("items_packages", "Price7", "int"),
        new("items_packages", "Price30", "int"),
        new("items_packages", "PriceP", "int"),
        new("items_packages", "GPrice1", "int"),
        new("items_packages", "GPrice7", "int"),
        new("items_packages", "GPrice30", "int"),
        new("items_packages", "GPriceP", "int"),
        new("items_packages", "IsNew", "int"),
        new("items_packages", "LevelRequired", "int"),
        new("items_packages", "IsEnabled", "int"),
        new("items_packages", "AddGP", "int"),
        new("items_packages", "AddSP", "int"),
        new("items_packages", "Item1_ID", "int"),
        new("items_packages", "Item1_Exp", "int"),
        new("items_packages", "Item2_ID", "int"),
        new("items_packages", "Item2_Exp", "int"),
        new("items_packages", "Item3_ID", "int"),
        new("items_packages", "Item3_Exp", "int"),
        new("items_packages", "Item4_ID", "int"),
        new("items_packages", "Item4_Exp", "int"),
        new("items_packages", "Item5_ID", "int"),
        new("items_packages", "Item5_Exp", "int"),
        new("items_packages", "Item6_ID", "int"),
        new("items_packages", "Item6_Exp", "int"),

        new("items_attachments", "ItemID", "int"),
        new("items_attachments", "FNAME", "string"),
        new("items_attachments", "Type", "int"),
        new("items_attachments", "Name", "string"),
        new("items_attachments", "Description", "string"),
        new("items_attachments", "Damage", "float"),
        new("items_attachments", "Firerate", "float"),
        new("items_attachments", "Spread", "float"),
        new("items_attachments", "Recoil", "float"),
        new("items_attachments", "Clipsize", "int"),
        new("items_attachments", "Category", "int"),
        new("items_attachments", "Price1", "int"),
        new("items_attachments", "Price7", "int"),
        new("items_attachments", "Price30", "int"),
        new("items_attachments", "PriceP", "int"),
        new("items_attachments", "GPrice1", "int"),
        new("items_attachments", "GPrice7", "int"),
        new("items_attachments", "GPrice30", "int"),
        new("items_attachments", "GPriceP", "int"),
        new("items_attachments", "IsNew", "int"),
        new("items_attachments", "LevelRequired", "int"),

        new("items_upgradedata", "ItemID", "int"),
        new("items_upgradedata", "FNAME", "string"),
        new("items_upgradedata", "Category", "int"),
        new("items_upgradedata", "Name", "string"),
        new("items_upgradedata", "Description", "string"),
        new("items_upgradedata", "Price1", "int"),
        new("items_upgradedata", "Price7", "int"),
        new("items_upgradedata", "Price30", "int"),
        new("items_upgradedata", "PriceP", "int"),
        new("items_upgradedata", "LevelRequired", "int"),
        new("items_upgradedata", "GPrice1", "int"),
        new("items_upgradedata", "GPrice7", "int"),
        new("items_upgradedata", "GPrice30", "int"),
        new("items_upgradedata", "GPriceP", "int"),
        new("items_upgradedata", "GPChance", "float"),
        new("items_upgradedata", "GDChance", "float"),
        new("items_upgradedata", "UpgradeID", "int"),
        new("items_upgradedata", "Value", "float"),

        new("items_lootdata", "RecordID", "int"),
        new("items_lootdata", "LootID", "int"),
        new("items_lootdata", "Chance", "float"),
        new("items_lootdata", "ItemID", "int"),
        new("items_lootdata", "ExpDaysMin", "int"),
        new("items_lootdata", "ExpDaysMax", "int"),
        new("items_lootdata", "GDMin", "int"),
        new("items_lootdata", "GDMax", "int"),
        new("items_lootdata", "GDIfHave", "int"),

        new("friendsmap", "CustomerID", "int"),
        new("friendsmap", "FriendID", "int"),
        new("friendsmap", "FriendStatus", "int"),
        new("friendsmap", "DateAdded", "datetime"),

        new("clandata", "ClanID", "int"),
        new("clandata", "ClanName", "string"),
        new("clandata", "ClanTag", "string"),
        new("clandata", "ClanXP", "int"),
        new("clandata", "ClanLevel", "int"),
        new("clandata", "ClanGP", "int"),
        new("clandata", "OwnerID", "int"),
        new("clandata", "MaxClanMembers", "int"),
        new("clandata", "NumClanMembers", "int"),
        new("clandata", "ClanLore", "string"),
        new("clandata", "Announcement", "string"),
        new("clandata", "ClanCreateDate", "datetime"),

        new("claninvites", "ClanInviteID", "int"),
        new("claninvites", "ClanID", "int"),
        new("claninvites", "InviterID", "int"),
        new("claninvites", "CustomerID", "int"),
        new("claninvites", "ExpireTime", "datetime"),

        new("clanapplications", "ClanApplicationID", "int"),
        new("clanapplications", "ClanID", "int"),
        new("clanapplications", "CustomerID", "int"),
        new("clanapplications", "ExpireTime", "datetime"),
        new("clanapplications", "ApplicationText", "string"),
        new("clanapplications", "IsProcessed", "int"),

        new("clanevents", "ClanEventID", "int"),
        new("clanevents", "ClanID", "int"),
        new("clanevents", "EventDate", "datetime"),
        new("clanevents", "EventType", "int"),
        new("clanevents", "EventRank", "int"),
        new("clanevents", "Text1", "string"),

        new("leaderboard", "Pos", "int"),
        new("leaderboard", "CustomerID", "int"),
        new("leaderboard", "gamertag", "string"),
        new("leaderboard", "Rank", "int"),
        new("leaderboard", "HonorPoints", "int"),
        new("leaderboard", "Wins", "int"),
        new("leaderboard", "Losses", "int"),
        new("leaderboard", "Kills", "int"),
        new("leaderboard", "Deaths", "int"),
        new("leaderboard", "ShotsFired", "int"),
        new("leaderboard", "ShotsHit", "int"),
        new("leaderboard", "TimePlayed", "int"),
        new("leaderboard", "HavePremium", "int"),

        new("leaderboard1", "CustomerID", "int"),
        new("leaderboard7", "CustomerID", "int"),
        new("leaderboard30", "CustomerID", "int"),

        new("dataretentionbonuses", "Day", "int"),
        new("dataretentionbonuses", "Bonus", "int"),

        new("datagamerewards", "RewardID", "int"),
        new("datagamerewards", "RewardName", "string"),
        new("datagamerewards", "GD_CQ", "int"),
        new("datagamerewards", "HP_CQ", "int"),
        new("datagamerewards", "GD_DM", "int"),
        new("datagamerewards", "HP_DM", "int"),
        new("datagamerewards", "GD_SB", "int"),
        new("datagamerewards", "HP_SB", "int"),

        new("dataskill2price", "SkillID", "int"),
        new("dataskill2price", "Lv1", "int"),
        new("dataskill2price", "Lv2", "int"),
        new("dataskill2price", "Lv3", "int"),
        new("dataskill2price", "Lv4", "int"),
        new("dataskill2price", "Lv5", "int"),

        new("masterserverinfo", "ServerID", "int"),
        new("masterserverinfo", "LastUpdated", "datetime"),
        new("masterserverinfo", "CreateGameKey", "int"),
        new("masterserverinfo", "IP", "string"),

        new("financialtransactions", "CustomerID", "int"),
        new("financialtransactions", "TransactionID", "string"),
        new("financialtransactions", "TransactionType", "int"),
        new("financialtransactions", "DateTime", "datetime"),
        new("financialtransactions", "Amount", "float"),
        new("financialtransactions", "ResponseCode", "string"),
        new("financialtransactions", "AprovalCode", "string"),
        new("financialtransactions", "ItemID", "string"),

        new("dbg_gptransactions", "TransactionID", "int"),
        new("dbg_gptransactions", "CustomerID", "int"),
        new("dbg_gptransactions", "TransactionTime", "datetime"),
        new("dbg_gptransactions", "Amount", "int"),
        new("dbg_gptransactions", "Reason", "string"),
        new("dbg_gptransactions", "Previous", "int"),

        new("dbg_hwinfo", "ComputerID", "bigint"),
        new("dbg_hwinfo", "CustomerID", "int"),
        new("dbg_hwinfo", "ReportDate", "datetime"),
        new("dbg_hwinfo", "CPUName", "string"),
        new("dbg_hwinfo", "CPUBrand", "string"),
        new("dbg_hwinfo", "CPUFreq", "int"),
        new("dbg_hwinfo", "TotalMemory", "int"),
        new("dbg_hwinfo", "DisplayW", "int"),
        new("dbg_hwinfo", "DisplayH", "int"),
        new("dbg_hwinfo", "gfxErrors", "int"),
        new("dbg_hwinfo", "gfxVendorId", "int"),
        new("dbg_hwinfo", "gfxDeviceId", "int"),
        new("dbg_hwinfo", "gfxDescription", "string"),
        new("dbg_hwinfo", "OSVersion", "string"),

        new("dbg_srvloginfo", "RecordID", "int"),
        new("dbg_srvloginfo", "ReportTime", "datetime"),
        new("dbg_srvloginfo", "IsProcessed", "int"),
        new("dbg_srvloginfo", "CustomerID", "int"),
        new("dbg_srvloginfo", "CustomerIP", "string"),
        new("dbg_srvloginfo", "GameSessionID", "bigint"),
        new("dbg_srvloginfo", "CheatID", "int"),
        new("dbg_srvloginfo", "RepeatCount", "int"),
        new("dbg_srvloginfo", "Msg", "string"),
        new("dbg_srvloginfo", "Data", "string"),

        new("dbg_userroundresults", "IP", "string"),
        new("dbg_userroundresults", "GameSessionID", "bigint"),
        new("dbg_userroundresults", "CustomerID", "int"),
        new("dbg_userroundresults", "GamePoints", "int"),
        new("dbg_userroundresults", "HonorPoints", "int"),
        new("dbg_userroundresults", "SkillPoints", "int"),
        new("dbg_userroundresults", "Kills", "int"),
        new("dbg_userroundresults", "Deaths", "int"),
        new("dbg_userroundresults", "ShotsFired", "int"),
        new("dbg_userroundresults", "ShotsHits", "int"),
        new("dbg_userroundresults", "Headshots", "int"),
        new("dbg_userroundresults", "AssistKills", "int"),
        new("dbg_userroundresults", "Wins", "int"),
        new("dbg_userroundresults", "Losses", "int"),
        new("dbg_userroundresults", "CaptureNeutralPoints", "int"),
        new("dbg_userroundresults", "CaptureEnemyPoints", "int"),
        new("dbg_userroundresults", "TimePlayed", "int"),
        new("dbg_userroundresults", "GameReportTime", "datetime"),
        new("dbg_userroundresults", "GameDollars", "int"),
        new("dbg_userroundresults", "TeamID", "int"),
        new("dbg_userroundresults", "MapID", "int"),
        new("dbg_userroundresults", "MapType", "int"),

        new("dbg_lootrewards", "RecordID", "int"),
        new("dbg_lootrewards", "ReportTime", "datetime"),
        new("dbg_lootrewards", "CustomerID", "int"),
        new("dbg_lootrewards", "Roll", "float"),
        new("dbg_lootrewards", "LootID", "float"),
        new("dbg_lootrewards", "ItemID", "int"),
        new("dbg_lootrewards", "ExpDays", "int"),
        new("dbg_lootrewards", "GD", "int"),

        new("cheatlog", "ID", "int"),
        new("cheatlog", "SessionID", "bigint"),
        new("cheatlog", "CustomerID", "int"),
        new("cheatlog", "CheatID", "int"),
        new("cheatlog", "ReportTime", "datetime")
    ];

    private static readonly string[] TablesAllowedEmpty =
    [
        "logins",
        "loginsessions",
        "accountinfo",
        "profiledata",
        "inventory",
        "inventory_fps",
        "friendsmap",
        "clandata",
        "claninvites",
        "clanapplications",
        "clanevents",
        "achievements",
        "financialtransactions",
        "dbg_gptransactions",
        "dbg_hwinfo",
        "dbg_srvloginfo",
        "dbg_userroundresults",
        "dbg_lootrewards",
        "cheatlog",
        "items_upgradedata"
    ];

    public DatabaseCheckService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<DatabaseCheckResponse> CheckAsync()
    {
        await using var db = _db.CreateConnection();

        var existingTables = await db.QueryAsync<string>(
            """
            SELECT TABLE_NAME
            FROM INFORMATION_SCHEMA.TABLES
            WHERE TABLE_SCHEMA = DATABASE();
            """);

        var existingTableSet = existingTables
            .ToHashSet(StringComparer.OrdinalIgnoreCase);

        var missingTables = RequiredTables
            .Where(table => !existingTableSet.Contains(table))
            .OrderBy(table => table, StringComparer.OrdinalIgnoreCase)
            .ToArray();

        var columns = (await db.QueryAsync<ColumnMetaRow>(
            """
            SELECT
                TABLE_NAME AS TableName,
                COLUMN_NAME AS ColumnName,
                COLUMN_TYPE AS ColumnType,
                DATA_TYPE AS DataType,
                IS_NULLABLE AS IsNullable,
                COLUMN_KEY AS ColumnKey,
                COALESCE(COLUMN_DEFAULT, '') AS ColumnDefault,
                EXTRA AS Extra
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = DATABASE();
            """)).ToList();

        var columnByKey = columns
            .ToDictionary(
                x => $"{x.TableName}.{x.ColumnName}",
                x => x,
                StringComparer.OrdinalIgnoreCase);

        var missingColumns = RequiredColumns
            .Where(x => existingTableSet.Contains(x.Table))
            .Where(x => !columnByKey.ContainsKey($"{x.Table}.{x.Column}"))
            .Select(x => new DatabaseMissingColumnDto(x.Table, x.Column, x.ExpectedType))
            .OrderBy(x => x.Table, StringComparer.OrdinalIgnoreCase)
            .ThenBy(x => x.Column, StringComparer.OrdinalIgnoreCase)
            .ToArray();

        var typeMismatches = RequiredColumns
            .Where(x => existingTableSet.Contains(x.Table))
            .Where(x => columnByKey.ContainsKey($"{x.Table}.{x.Column}"))
            .Select(x =>
            {
                var actual = columnByKey[$"{x.Table}.{x.Column}"];
                return new
                {
                    Required = x,
                    Actual = actual
                };
            })
            .Where(x => !MatchesType(x.Required.ExpectedType, x.Actual.DataType, x.Actual.ColumnType))
            .Select(x => new DatabaseColumnTypeMismatchDto(
                x.Required.Table,
                x.Required.Column,
                x.Required.ExpectedType,
                x.Actual.ColumnType))
            .OrderBy(x => x.Table, StringComparer.OrdinalIgnoreCase)
            .ThenBy(x => x.Column, StringComparer.OrdinalIgnoreCase)
            .ToArray();

        var tableStats = await GetTableStatsAsync(db, existingTableSet);

        var warnings = new List<string>();

        if (missingTables.Length > 0)
            warnings.Add("Database has missing required tables.");

        if (missingColumns.Length > 0)
            warnings.Add("Database has missing required columns.");

        if (typeMismatches.Length > 0)
            warnings.Add("Database has column type mismatches.");

        foreach (var table in tableStats.Where(x => x.Rows <= 0 && !x.AllowedEmpty))
            warnings.Add($"Table `{table.Table}` is empty.");

        var ok =
            missingTables.Length == 0 &&
            missingColumns.Length == 0 &&
            typeMismatches.Length == 0;

        return new DatabaseCheckResponse(
            ok,
            missingTables,
            missingColumns,
            typeMismatches,
            tableStats,
            warnings
        );
    }

    public async Task<DatabaseSmokeResponse> SmokeAsync()
    {
        var schema = await CheckAsync();
        var steps = new List<DatabaseSmokeStepDto>
        {
            new(
                "schema_check",
                schema.Ok,
                schema.Ok ? "OK" : "SCHEMA_HAS_ERRORS",
                null)
        };

        steps.Add(await SmokeScalarAsync(
            "database_ping",
            "SELECT 1;",
            true));

        steps.Add(await SmokeCountAsync(
            "accounts_count",
            "loginid"));

        steps.Add(await SmokeQueryAsync(
            "auth_login_columns",
            """
            SELECT
                CustomerID,
                AccountName,
                Password,
                MD5Password,
                AccountStatus,
                IsDeveloper
            FROM loginid
            ORDER BY CustomerID ASC
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "auth_session_columns",
            """
            SELECT
                CustomerID,
                SessionKey,
                SessionID,
                LoginIP,
                TimeLogged,
                TimeUpdated,
                GameSessionID
            FROM loginsessions
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "profile_full_read",
            """
            SELECT
                l.CustomerID,
                l.AccountStatus,
                l.GamePoints,
                l.GameDollars,
                l.HonorPoints,
                l.SkillPoints,
                l.Gamertag,
                l.Faction1Score,
                l.Faction2Score,
                l.Faction3Score,
                l.Faction4Score,
                l.Faction5Score,
                l.ClanID,
                l.ClanRank,
                l.IsFPSEnabled,
                l.IsDeveloper,
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
            ORDER BY l.CustomerID ASC
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "profile_chars_read",
            """
            SELECT
                LoadoutID,
                CustomerID,
                Class,
                HonorPoints,
                TimePlayed,
                Loadout,
                Skills,
                SpendSP1,
                SpendSP2,
                SpendSP3
            FROM profile_chars
            ORDER BY LoadoutID ASC
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "inventory_read",
            """
            SELECT
                CustomerID,
                ItemID,
                LeasedUntil,
                Quantity,
                UpSlot1,
                UpSlot2,
                UpSlot3,
                UpSlot4,
                UpSlot5
            FROM inventory
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "inventory_fps_read",
            """
            SELECT
                CustomerID,
                WeaponID,
                AttachmentID,
                LeasedUntil,
                Slot,
                IsEquipped
            FROM inventory_fps
            LIMIT 1;
            """));

        steps.Add(await SmokeScalarAsync(
            "shop_items_union",
            """
            SELECT COUNT(*)
            FROM
            (
                SELECT ItemID, Category, Price1, Price7, Price30, PriceP, GPrice1, GPrice7, GPrice30, GPriceP FROM items_gear
                UNION ALL
                SELECT ItemID, Category, Price1, Price7, Price30, PriceP, GPrice1, GPrice7, GPrice30, GPriceP FROM items_weapons
                UNION ALL
                SELECT ItemID, Category, Price1, Price7, Price30, PriceP, GPrice1, GPrice7, GPrice30, GPriceP FROM items_generic
                UNION ALL
                SELECT ItemID, Category, Price1, Price7, Price30, PriceP, GPrice1, GPrice7, GPrice30, GPriceP FROM items_attachments
                UNION ALL
                SELECT ItemID, Category, Price1, Price7, Price30, PriceP, GPrice1, GPrice7, GPrice30, GPriceP FROM items_packages
            ) x;
            """,
            false));

        steps.Add(await SmokeQueryAsync(
            "items_info_weapons",
            """
            SELECT
                ItemID,
                FNAME,
                Category,
                Name,
                Description,
                Damage,
                DamageDecay,
                NumClips,
                Clipsize,
                RateOfFire,
                Spread,
                Recoil,
                LevelRequired
            FROM items_weapons
            ORDER BY ItemID ASC
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "items_info_gear",
            """
            SELECT
                ItemID,
                FNAME,
                Name,
                Description,
                Category,
                Weight,
                DamagePerc,
                DamageMax,
                LevelRequired
            FROM items_gear
            ORDER BY ItemID ASC
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "purchase_price_read",
            """
            SELECT
                ItemID,
                Category,
                Price1,
                Price7,
                Price30,
                PriceP,
                GPrice1,
                GPrice7,
                GPrice30,
                GPriceP
            FROM items_generic
            ORDER BY ItemID ASC
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "purchase_upgrade_price_read",
            """
            SELECT
                ItemID,
                Category,
                Price1,
                Price7,
                Price30,
                PriceP,
                GPrice1,
                GPrice7,
                GPrice30,
                GPriceP,
                GPChance,
                GDChance,
                UpgradeID,
                Value
            FROM items_upgradedata
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "financialtransactions_read",
            """
            SELECT
                CustomerID,
                TransactionID,
                TransactionType,
                DateTime,
                Amount,
                ResponseCode,
                AprovalCode,
                ItemID
            FROM financialtransactions
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "leaderboard_read",
            """
            SELECT
                Pos,
                CustomerID,
                gamertag,
                Rank,
                HonorPoints,
                Wins,
                Losses,
                Kills,
                Deaths,
                ShotsFired,
                ShotsHit,
                TimePlayed,
                HavePremium
            FROM leaderboard
            ORDER BY Pos ASC
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "retention_read",
            """
            SELECT
                Day,
                Bonus
            FROM dataretentionbonuses
            ORDER BY Day ASC
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "game_rewards_read",
            """
            SELECT
                RewardID,
                RewardName,
                GD_CQ,
                HP_CQ,
                GD_DM,
                HP_DM,
                GD_SB,
                HP_SB
            FROM datagamerewards
            ORDER BY RewardID ASC
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "lootdata_read",
            """
            SELECT
                RecordID,
                LootID,
                Chance,
                ItemID,
                ExpDaysMin,
                ExpDaysMax,
                GDMin,
                GDMax,
                GDIfHave
            FROM items_lootdata
            ORDER BY RecordID ASC
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "telemetry_hwinfo_read",
            """
            SELECT
                ComputerID,
                CustomerID,
                ReportDate,
                CPUName,
                CPUBrand,
                CPUFreq,
                TotalMemory,
                DisplayW,
                DisplayH,
                gfxErrors,
                gfxVendorId,
                gfxDeviceId,
                gfxDescription,
                OSVersion
            FROM dbg_hwinfo
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "server_log_read",
            """
            SELECT
                RecordID,
                ReportTime,
                IsProcessed,
                CustomerID,
                CustomerIP,
                GameSessionID,
                CheatID,
                RepeatCount,
                Msg,
                Data
            FROM dbg_srvloginfo
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "round_results_read",
            """
            SELECT
                IP,
                GameSessionID,
                CustomerID,
                GamePoints,
                HonorPoints,
                SkillPoints,
                Kills,
                Deaths,
                ShotsFired,
                ShotsHits,
                Headshots,
                AssistKills,
                Wins,
                Losses,
                CaptureNeutralPoints,
                CaptureEnemyPoints,
                TimePlayed,
                GameReportTime,
                GameDollars,
                TeamID,
                MapID,
                MapType
            FROM dbg_userroundresults
            LIMIT 1;
            """));

        steps.Add(await SmokeQueryAsync(
            "cheatlog_read",
            """
            SELECT
                ID,
                SessionID,
                CustomerID,
                CheatID,
                ReportTime
            FROM cheatlog
            LIMIT 1;
            """));

        var warnings = new List<string>();

        foreach (var warning in schema.Warnings)
            warnings.Add(warning);

        foreach (var failed in steps.Where(x => !x.Ok))
            warnings.Add($"{failed.Name}: {failed.Message}");

        return new DatabaseSmokeResponse(
            steps.All(x => x.Ok),
            schema,
            steps,
            warnings
        );
    }

    public async Task<DatabaseSchemaResponse> GetSchemaAsync()
    {
        await using var db = _db.CreateConnection();

        var rows = await db.QueryAsync<ColumnMetaRow>(
            """
            SELECT
                TABLE_NAME AS TableName,
                COLUMN_NAME AS ColumnName,
                COLUMN_TYPE AS ColumnType,
                DATA_TYPE AS DataType,
                IS_NULLABLE AS IsNullable,
                COLUMN_KEY AS ColumnKey,
                COALESCE(COLUMN_DEFAULT, '') AS ColumnDefault,
                EXTRA AS Extra
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = DATABASE()
            ORDER BY TABLE_NAME, ORDINAL_POSITION;
            """);

        var tables = rows
            .GroupBy(x => x.TableName, StringComparer.OrdinalIgnoreCase)
            .Select(group => new DatabaseTableSchemaDto(
                group.Key,
                group.Select(x => new DatabaseColumnSchemaDto(
                    x.ColumnName,
                    x.ColumnType,
                    string.Equals(x.IsNullable, "YES", StringComparison.OrdinalIgnoreCase),
                    x.ColumnKey ?? "",
                    x.ColumnDefault ?? "",
                    x.Extra ?? ""
                )).ToArray()
            ))
            .OrderBy(x => x.Table, StringComparer.OrdinalIgnoreCase)
            .ToArray();

        var warnings = new List<string>();

        if (tables.Length == 0)
            warnings.Add("No tables found in selected database.");

        return new DatabaseSchemaResponse(
            true,
            tables,
            warnings
        );
    }

    private async Task<DatabaseSmokeStepDto> SmokeCountAsync(
        string name,
        string table)
    {
        try
        {
            await using var db = _db.CreateConnection();

            var count = await db.ExecuteScalarAsync<long>(
                $"SELECT COUNT(*) FROM `{table}`;");

            return new DatabaseSmokeStepDto(name, true, "OK", count);
        }
        catch (Exception e)
        {
            return new DatabaseSmokeStepDto(name, false, e.Message, null);
        }
    }

    private async Task<DatabaseSmokeStepDto> SmokeScalarAsync(
        string name,
        string sql,
        bool ignoreCount)
    {
        try
        {
            await using var db = _db.CreateConnection();

            var value = await db.ExecuteScalarAsync<long>(sql);

            return new DatabaseSmokeStepDto(
                name,
                true,
                "OK",
                ignoreCount ? null : value);
        }
        catch (Exception e)
        {
            return new DatabaseSmokeStepDto(name, false, e.Message, null);
        }
    }

    private async Task<DatabaseSmokeStepDto> SmokeQueryAsync(
        string name,
        string sql)
    {
        try
        {
            await using var db = _db.CreateConnection();

            await db.QueryFirstOrDefaultAsync(sql);

            return new DatabaseSmokeStepDto(name, true, "OK", null);
        }
        catch (Exception e)
        {
            return new DatabaseSmokeStepDto(name, false, e.Message, null);
        }
    }

    private static bool MatchesType(
        string expected,
        string dataType,
        string columnType)
    {
        expected = Normalize(expected);
        dataType = Normalize(dataType);
        columnType = Normalize(columnType);

        return expected switch
        {
            "string" => dataType is "char" or "varchar" or "text" or "mediumtext" or "longtext",
            "int" => dataType is "tinyint" or "smallint" or "mediumint" or "int" or "integer",
            "bigint" => dataType == "bigint",
            "float" => dataType is "float" or "double" or "decimal",
            "datetime" => dataType is "datetime" or "timestamp" or "date",
            _ => columnType.Contains(expected, StringComparison.OrdinalIgnoreCase)
        };
    }

    private async Task<IReadOnlyList<DatabaseTableStatDto>> GetTableStatsAsync(
        MySqlConnection db,
        HashSet<string> existingTableSet)
    {
        var allowedEmpty = TablesAllowedEmpty
            .ToHashSet(StringComparer.OrdinalIgnoreCase);

        var result = new List<DatabaseTableStatDto>();

        foreach (var table in RequiredTables)
        {
            if (!existingTableSet.Contains(table))
                continue;

            var count = await db.ExecuteScalarAsync<long>(
                $"SELECT COUNT(*) FROM `{table}`;");

            result.Add(new DatabaseTableStatDto(
                table,
                count,
                allowedEmpty.Contains(table)
            ));
        }

        return result
            .OrderBy(x => x.Table, StringComparer.OrdinalIgnoreCase)
            .ToArray();
    }

    private static string Normalize(string value)
    {
        return (value ?? "").Trim().ToLowerInvariant();
    }

    private sealed class ColumnMetaRow
    {
        public string TableName { get; set; } = "";
        public string ColumnName { get; set; } = "";
        public string ColumnType { get; set; } = "";
        public string DataType { get; set; } = "";
        public string IsNullable { get; set; } = "";
        public string ColumnKey { get; set; } = "";
        public string ColumnDefault { get; set; } = "";
        public string Extra { get; set; } = "";
    }
}