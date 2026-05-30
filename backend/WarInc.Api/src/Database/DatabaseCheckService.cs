using Dapper;

namespace WarInc.Api.Database;

public sealed class DatabaseCheckService
{
    private readonly WarIncDb _db;

    private static readonly string[] RequiredTables =
    {
        "loginid",
        "logins",
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
        "leaderboard",
        "leaderboard1",
        "leaderboard7",
        "leaderboard30",
        "dataretentionbonuses",
        "financialtransactions"
    };

    private static readonly DatabaseRequiredColumn[] RequiredColumns =
    {
        new("loginid", "CustomerID", "number"),
        new("loginid", "email", "string"),
        new("loginid", "password", "string"),

        new("logins", "CustomerID", "number"),
        new("logins", "SessionID", "number"),

        new("items_gear", "ItemID", "number"),
        new("items_gear", "Category", "number"),
        new("items_gear", "Name", "string"),
        new("items_gear", "FNAME", "string"),
        new("items_gear", "Description", "string"),
        new("items_gear", "LevelRequired", "number"),
        new("items_gear", "Price1", "number"),
        new("items_gear", "Price7", "number"),
        new("items_gear", "Price30", "number"),
        new("items_gear", "PriceP", "number"),
        new("items_gear", "GPrice1", "number"),
        new("items_gear", "GPrice7", "number"),
        new("items_gear", "GPrice30", "number"),
        new("items_gear", "GPriceP", "number"),

        new("items_weapons", "ItemID", "number"),
        new("items_weapons", "Category", "number"),
        new("items_weapons", "Name", "string"),
        new("items_weapons", "FNAME", "string"),
        new("items_weapons", "Description", "string"),
        new("items_weapons", "LevelRequired", "number"),
        new("items_weapons", "Price1", "number"),
        new("items_weapons", "Price7", "number"),
        new("items_weapons", "Price30", "number"),
        new("items_weapons", "PriceP", "number"),
        new("items_weapons", "GPrice1", "number"),
        new("items_weapons", "GPrice7", "number"),
        new("items_weapons", "GPrice30", "number"),
        new("items_weapons", "GPriceP", "number"),

        new("items_generic", "ItemID", "number"),
        new("items_generic", "Category", "number"),
        new("items_generic", "Name", "string"),
        new("items_generic", "FNAME", "string"),
        new("items_generic", "Description", "string"),

        new("items_packages", "ItemID", "number"),
        new("items_packages", "Name", "string"),

        new("items_attachments", "ItemID", "number"),
        new("items_attachments", "Category", "number"),
        new("items_attachments", "Name", "string"),

        new("items_lootdata", "LootID", "number"),
        new("items_lootdata", "ItemID", "number"),

        new("inventory", "CustomerID", "number"),
        new("inventory", "ItemID", "number"),

        new("inventory_fps", "CustomerID", "number"),
        new("inventory_fps", "LoadoutID", "number"),

        new("profile_chars", "CustomerID", "number"),
        new("profile_chars", "CharID", "number"),

        new("stats", "CustomerID", "number"),

        new("friendsmap", "CustomerID", "number"),
        new("friendsmap", "FriendID", "number"),

        new("clandata", "ClanID", "number"),
        new("clandata", "OwnerID", "number"),
        new("clandata", "ClanName", "string"),
        new("clandata", "ClanTag", "string"),

        new("claninvites", "ClanID", "number"),
        new("claninvites", "CustomerID", "number"),

        new("clanapplications", "ClanID", "number"),
        new("clanapplications", "CustomerID", "number"),

        new("leaderboard", "CustomerID", "number"),
        new("leaderboard1", "CustomerID", "number"),
        new("leaderboard7", "CustomerID", "number"),
        new("leaderboard30", "CustomerID", "number"),

        new("dataretentionbonuses", "CustomerID", "number"),

        new("financialtransactions", "CustomerID", "number")
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

        var emptyImportantTables = await GetEmptyImportantTablesAsync();

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

    private async Task<IReadOnlyList<string>> GetEmptyImportantTablesAsync()
    {
        await using var db = _db.CreateConnection();

        var result = new List<string>();

        foreach (var table in RequiredTables)
        {
            var exists = await db.ExecuteScalarAsync<int>(
                """
                SELECT COUNT(*)
                FROM INFORMATION_SCHEMA.TABLES
                WHERE TABLE_SCHEMA = DATABASE()
                  AND TABLE_NAME = @Table;
                """,
                new { Table = table }
            );

            if (exists <= 0)
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