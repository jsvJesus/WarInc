using System.Text;
using Dapper;
using MySqlConnector;
using WarInc.Api.Common;
using WarInc.Api.Database;

namespace WarInc.Api.Leaderboard;

public sealed class LeaderboardService
{
    private const int LegacyRowsToFetch = 100;

    private readonly WarIncDb _db;

    public LeaderboardService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<LeaderboardResponse> GetLeaderboardAsync(
        int position,
        int size)
    {
        return await GetLeaderboardAsync(
            0,
            position,
            size,
            3);
    }

    public async Task<LeaderboardResponse> GetLeaderboardAsync(
        ulong customerId,
        int position,
        int size,
        int tableId)
    {
        var tableName = ResolveTableName(tableId);

        if (size <= 0)
            size = LegacyRowsToFetch;

        size = Math.Clamp(size, 1, 100);

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, tableName))
        {
            return new LeaderboardResponse(
                true,
                0,
                "OK",
                Math.Max(0, position),
                0,
                tableId,
                tableName,
                Array.Empty<LeaderboardEntryDto>());
        }

        var totalRows = 0;
        var startPos = position;

        if (startPos < 0)
        {
            totalRows = await GetTotalRowsAsync(db, tableName);
            startPos = await GetPlayerCenteredStartPosAsync(
                db,
                tableName,
                customerId);

            if (startPos < 0)
                startPos = 0;
        }

        if (startPos < 0)
            startPos = 0;

        var rows = await QueryLeaderboardRowsAsync(
            db,
            tableName,
            startPos,
            size);

        var entries = rows
            .Select(row => new LeaderboardEntryDto(
                row.Pos,
                (ulong)Math.Max(0, row.CustomerID),
                row.Gamertag ?? "",
                row.HonorPoints,
                row.Rank,
                row.HonorPoints,
                row.Kills,
                row.Deaths,
                row.Wins,
                row.Losses,
                row.ShotsFired,
                row.ShotsHit,
                row.TimePlayed,
                row.HavePremium))
            .ToArray();

        return new LeaderboardResponse(
            true,
            0,
            "OK",
            startPos,
            totalRows,
            tableId,
            tableName,
            entries);
    }

    public string BuildLegacyLeaderboardXml(LeaderboardResponse leaderboard)
    {
        var xml = new StringBuilder();

        xml.Append("<?xml version=\"1.0\"?>\n");

        xml.Append("<leaderboard ");
        LegacyUtil.AppendXmlAttr(xml, "pos", leaderboard.Position);
        LegacyUtil.AppendXmlAttr(xml, "size", leaderboard.Size);
        xml.Append(">");

        foreach (var entry in leaderboard.Entries)
        {
            xml.Append("<f ");
            LegacyUtil.AppendXmlAttr(xml, "GT", entry.Gamertag);
            LegacyUtil.AppendXmlAttr(xml, "XP", entry.HonorPoints);
            LegacyUtil.AppendXmlAttr(xml, "k", entry.Kills);
            LegacyUtil.AppendXmlAttr(xml, "d", entry.Deaths);
            LegacyUtil.AppendXmlAttr(xml, "w", entry.Wins);
            LegacyUtil.AppendXmlAttr(xml, "l", entry.Losses);
            LegacyUtil.AppendXmlAttr(xml, "f", entry.ShotsFired);
            LegacyUtil.AppendXmlAttr(xml, "h", entry.ShotsHit);
            LegacyUtil.AppendXmlAttr(xml, "t", entry.TimePlayed);
            LegacyUtil.AppendXmlAttr(xml, "p", entry.HavePremium);
            xml.Append("/>");
        }

        xml.Append("</leaderboard>");

        return xml.ToString();
    }

    private static async Task<LeaderboardDbRow[]> QueryLeaderboardRowsAsync(
        MySqlConnection db,
        string tableName,
        int startPos,
        int size)
    {
        var sql =
            $"""
            SELECT
                Pos,
                CustomerID,
                gamertag AS Gamertag,
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
            FROM {tableName}
            WHERE Pos > @StartPos
              AND Pos <= @EndPos
            ORDER BY HonorPoints DESC, Pos ASC;
            """;

        var rows = await db.QueryAsync<LeaderboardDbRow>(
            sql,
            new
            {
                StartPos = startPos,
                EndPos = startPos + size
            });

        return rows.ToArray();
    }

    private static async Task<int> GetPlayerCenteredStartPosAsync(
        MySqlConnection db,
        string tableName,
        ulong customerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return 0;

        var sql =
            $"""
            SELECT Pos
            FROM {tableName}
            WHERE CustomerID = @CustomerId
            LIMIT 1;
            """;

        var playerPos = await db.ExecuteScalarAsync<int?>(
            sql,
            new { CustomerId = (int)customerId });

        if (!playerPos.HasValue || playerPos.Value <= 0)
            return 0;

        var startPos = playerPos.Value - (LegacyRowsToFetch / 2);

        return Math.Max(0, startPos);
    }

    private static async Task<int> GetTotalRowsAsync(
        MySqlConnection db,
        string tableName)
    {
        var sql = $"SELECT COUNT(*) FROM {tableName};";

        return await db.ExecuteScalarAsync<int>(sql);
    }

    private static string ResolveTableName(int tableId)
    {
        return tableId switch
        {
            0 => "leaderboard1",
            1 => "leaderboard7",
            2 => "leaderboard30",
            _ => "leaderboard"
        };
    }

    private static async Task<bool> TableExistsAsync(
        MySqlConnection db,
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

    private sealed class LeaderboardDbRow
    {
        public int Pos { get; set; }
        public int CustomerID { get; set; }
        public string Gamertag { get; set; } = "";
        public int Rank { get; set; }
        public int HonorPoints { get; set; }
        public int Wins { get; set; }
        public int Losses { get; set; }
        public int Kills { get; set; }
        public int Deaths { get; set; }
        public int ShotsFired { get; set; }
        public int ShotsHit { get; set; }
        public int TimePlayed { get; set; }
        public int HavePremium { get; set; }
    }
}