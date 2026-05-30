using Dapper;

namespace WarInc.Api.Database;

public sealed class DatabaseCheckService
{
    private readonly WarIncDb _db;

    private static readonly string[] RequiredTables =
    {
        "loginid",
        "logins",
        "loginsessions",
        "accountinfo",

        "items_gear",
        "items_weapons",
        "items_generic",
        "items_packages",
        "items_attachments",
        "items_lootdata",

        "inventory",
        "inventory_fps",
        "profile_chars",
        "stats",

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

        "achievements",

        "financialtransactions",
        "dbg_gptransactions",
        "dbg_hwinfo",
        "dbg_srvloginfo",
        "dbg_userroundresults",
        "dbg_lootrewards",
        "cheatlog"
    };

    private static readonly DatabaseRequiredColumn[] RequiredColumns =
    {
        new("loginid", "CustomerID", "int"),
        new("loginid", "AccountName", "string"),
        new("loginid", "Password", "string"),
        new("loginid", "MD5Password", "string"),
        new("loginid", "AccountStatus", "int"),
        new("loginid", "GamePoints", "int"),
        new("loginid", "HonorPoints", "int"),
        new("loginid", "SkillPoints", "int"),
        new("loginid", "Gamertag", "string"),
        new("loginid", "ClanID", "int"),
        new("loginid", "ClanRank", "int"),
        new("loginid", "GameDollars", "int"),
        new("loginid", "lastlogindate", "datetime"),
        new("loginid", "lastloginIP", "string"),
        new("loginid", "lastRetBonusDate", "datetime"),
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

        new("items_gear", "ItemID", "int"),
        new("items_gear", "FNAME", "string"),
        new("items_gear", "Name", "string"),
        new("items_gear", "Description", "string"),
        new("items_gear", "Category", "int"),
        new("items_gear", "Weight", "float"),
        new("items_gear", "DamagePerc", "float"),
        new("items_gear", "DamageMax", "int"),
        new("items_gear", "LevelRequired", "int"),
        new("items_gear", "Price1", "int"),
        new("items_gear", "Price7", "int"),
        new("items_gear", "Price30", "int"),
        new("items_gear", "PriceP", "int"),
        new("items_gear", "GPrice1", "int"),
        new("items_gear", "GPrice7", "int"),
        new("items_gear", "GPrice30", "int"),
        new("items_gear", "GPriceP", "int"),

        new("items_weapons", "ItemID", "int"),
        new("items_weapons", "FNAME", "string"),
        new("items_weapons", "Category", "int"),
        new("items_weapons", "Name", "string"),
        new("items_weapons", "Description", "string"),
        new("items_weapons", "Damage", "int"),
        new("items_weapons", "DamageDecay", "int"),
        new("items_weapons", "NumClips", "int"),
        new("items_weapons", "Clipsize", "int"),
        new("items_weapons", "RateOfFire", "int"),
        new("items_weapons", "Spread", "float"),
        new("items_weapons", "Recoil", "float"),
        new("items_weapons", "LevelRequired", "int"),
        new("items_weapons", "Price1", "int"),
        new("items_weapons", "Price7", "int"),
        new("items_weapons", "Price30", "int"),
        new("items_weapons", "PriceP", "int"),
        new("items_weapons", "GPrice1", "int"),
        new("items_weapons", "GPrice7", "int"),
        new("items_weapons", "GPrice30", "int"),
        new("items_weapons", "GPriceP", "int"),

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
        new("items_attachments", "Category", "int"),
        new("items_attachments", "Price1", "int"),
        new("items_attachments", "Price7", "int"),
        new("items_attachments", "Price30", "int"),
        new("items_attachments", "PriceP", "int"),
        new("items_attachments", "GPrice1", "int"),
        new("items_attachments", "GPrice7", "int"),
        new("items_attachments", "GPrice30", "int"),
        new("items_attachments", "GPriceP", "int"),
        new("items_attachments", "LevelRequired", "int"),

        new("items_lootdata", "RecordID", "int"),
        new("items_lootdata", "LootID", "int"),
        new("items_lootdata", "Chance", "float"),
        new("items_lootdata", "ItemID", "int"),
        new("items_lootdata", "ExpDaysMin", "int"),
        new("items_lootdata", "ExpDaysMax", "int"),
        new("items_lootdata", "GDMin", "int"),
        new("items_lootdata", "GDMax", "int"),
        new("items_lootdata", "GDIfHave", "int"),

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
        new("stats", "Wins", "int"),
        new("stats", "Losses", "int"),
        new("stats", "TimePlayed", "int"),
        new("stats", "CheatAttempts", "int"),

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

        new("achievements", "CustomerID", "int"),
        new("achievements", "AchID", "int"),
        new("achievements", "Value", "int"),
        new("achievements", "Unlocked", "int"),

        new("financialtransactions", "CustomerID", "int"),
        new("financialtransactions", "TransactionID", "int"),
        new("financialtransactions", "TransactionType", "int"),
        new("financialtransactions", "DateTime", "datetime"),
        new("financialtransactions", "Amount", "decimal"),
        new("financialtransactions", "ResponseCode", "int"),
        new("financialtransactions", "AprovalCode", "string"),
        new("financialtransactions", "ItemID", "int"),

        new("dbg_gptransactions", "TransactionID", "int"),
        new("dbg_gptransactions", "CustomerID", "int"),
        new("dbg_gptransactions", "TransactionTime", "datetime"),
        new("dbg_gptransactions", "Amount", "int"),
        new("dbg_gptransactions", "Reason", "string"),
        new("dbg_gptransactions", "Previous", "int"),

        new("dbg_hwinfo", "ComputerID", "string"),
        new("dbg_hwinfo", "CustomerID", "int"),
        new("dbg_hwinfo", "ReportDate", "datetime"),
        new("dbg_hwinfo", "CPUName", "string"),
        new("dbg_hwinfo", "CPUBrand", "string"),
        new("dbg_hwinfo", "TotalMemory", "int"),

        new("dbg_srvloginfo", "RecordID", "int"),
        new("dbg_srvloginfo", "ReportTime", "datetime"),
        new("dbg_srvloginfo", "CustomerID", "int"),
        new("dbg_srvloginfo", "GameSessionID", "bigint"),
        new("dbg_srvloginfo", "CheatID", "int"),
        new("dbg_srvloginfo", "Msg", "string"),

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

        new("dbg_lootrewards", "RecordID", "int"),
        new("dbg_lootrewards", "ReportTime", "datetime"),
        new("dbg_lootrewards", "CustomerID", "int"),
        new("dbg_lootrewards", "Roll", "int"),
        new("dbg_lootrewards", "LootID", "int"),
        new("dbg_lootrewards", "ItemID", "int"),
        new("dbg_lootrewards", "ExpDays", "int"),
        new("dbg_lootrewards", "GD", "int"),

        new("cheatlog", "ID", "int"),
        new("cheatlog", "SessionID", "int"),
        new("cheatlog", "CustomerID", "int"),
        new("cheatlog", "CheatID", "int"),
        new("cheatlog", "ReportTime", "datetime")
    };

    private static readonly string[] TablesAllowedEmpty =
    {
        "logins",
        "loginsessions",
        "accountinfo",
        "inventory",
        "inventory_fps",
        "friendsmap",
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
        "cheatlog"
    };

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
            """
        );

        var existingTableSet = existingTables
            .ToHashSet(StringComparer.OrdinalIgnoreCase);

        var missingTables = RequiredTables
            .Where(table => !existingTableSet.Contains(table))
            .OrderBy(table => table, StringComparer.OrdinalIgnoreCase)
            .ToArray();

        var existingColumns = await db.QueryAsync<ExistingColumnRow>(
            """
            SELECT
                TABLE_NAME AS TableName,
                COLUMN_NAME AS ColumnName
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = DATABASE();
            """
        );

        var columnSet = existingColumns
            .Select(x => $"{x.TableName}.{x.ColumnName}")
            .ToHashSet(StringComparer.OrdinalIgnoreCase);

        var missingColumns = RequiredColumns
            .Where(x => existingTableSet.Contains(x.Table))
            .Where(x => !columnSet.Contains($"{x.Table}.{x.Column}"))
            .Select(x => new DatabaseMissingColumnDto(x.Table, x.Column, x.ExpectedType))
            .OrderBy(x => x.Table, StringComparer.OrdinalIgnoreCase)
            .ThenBy(x => x.Column, StringComparer.OrdinalIgnoreCase)
            .ToArray();

        var warnings = new List<string>();

        if (missingTables.Length > 0)
            warnings.Add("Database has missing required tables. Do not cleanup legacy tables yet.");

        if (missingColumns.Length > 0)
            warnings.Add("Database has missing required columns. Some legacy or v1 endpoints can fail.");

        var emptyImportantTables = await GetEmptyImportantTablesAsync(existingTableSet);

        foreach (var table in emptyImportantTables)
            warnings.Add($"Table `{table}` is empty.");

        var ok = missingTables.Length == 0 && missingColumns.Length == 0;

        return new DatabaseCheckResponse(
            ok,
            missingTables,
            missingColumns,
            warnings
        );
    }

    public async Task<DatabaseSchemaResponse> GetSchemaAsync()
    {
        await using var db = _db.CreateConnection();

        var rows = await db.QueryAsync<SchemaColumnRow>(
            """
            SELECT
                TABLE_NAME AS TableName,
                COLUMN_NAME AS ColumnName,
                COLUMN_TYPE AS ColumnType,
                IS_NULLABLE AS IsNullable,
                COLUMN_KEY AS ColumnKey,
                COALESCE(COLUMN_DEFAULT, '') AS ColumnDefault,
                EXTRA AS Extra
            FROM INFORMATION_SCHEMA.COLUMNS
            WHERE TABLE_SCHEMA = DATABASE()
            ORDER BY TABLE_NAME, ORDINAL_POSITION;
            """
        );

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

    private async Task<IReadOnlyList<string>> GetEmptyImportantTablesAsync(
        HashSet<string> existingTableSet)
    {
        await using var db = _db.CreateConnection();

        var allowedEmpty = TablesAllowedEmpty
            .ToHashSet(StringComparer.OrdinalIgnoreCase);

        var result = new List<string>();

        foreach (var table in RequiredTables)
        {
            if (!existingTableSet.Contains(table))
                continue;

            if (allowedEmpty.Contains(table))
                continue;

            var count = await db.ExecuteScalarAsync<long>($"SELECT COUNT(*) FROM `{table}`;");

            if (count <= 0)
                result.Add(table);
        }

        return result;
    }

    private sealed class ExistingColumnRow
    {
        public string TableName { get; set; } = "";
        public string ColumnName { get; set; } = "";
    }

    private sealed class SchemaColumnRow
    {
        public string TableName { get; set; } = "";
        public string ColumnName { get; set; } = "";
        public string ColumnType { get; set; } = "";
        public string IsNullable { get; set; } = "";
        public string ColumnKey { get; set; } = "";
        public string ColumnDefault { get; set; } = "";
        public string Extra { get; set; } = "";
    }
}