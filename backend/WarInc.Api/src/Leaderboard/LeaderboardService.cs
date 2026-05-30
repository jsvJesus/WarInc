using System.Net;
using System.Text;
using Dapper;
using MySqlConnector;
using WarInc.Api.Common;
using WarInc.Api.Database;

namespace WarInc.Api.Leaderboard;

public sealed class LeaderboardService
{
    private readonly WarIncDb _db;

    public LeaderboardService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<LeaderboardResponse> GetLeaderboardAsync(int position, int size)
    {
        position = Math.Max(0, position);
        size = Math.Clamp(size, 0, 100);

        if (size <= 0)
        {
            return new LeaderboardResponse(
                true,
                0,
                "OK",
                position,
                0,
                Array.Empty<LeaderboardEntryDto>());
        }

        await using var db = _db.CreateConnection();

        var hasLoginId = await TableExistsAsync(db, "loginid");
        var hasStats = await TableExistsAsync(db, "stats");

        if (!hasLoginId || !hasStats)
        {
            return new LeaderboardResponse(
                true,
                0,
                "OK",
                position,
                0,
                Array.Empty<LeaderboardEntryDto>());
        }

        try
        {
            var rows = (await db.QueryAsync<LeaderboardRow>(
                """
                SELECT
                    l.CustomerID AS CustomerId,
                    COALESCE(NULLIF(l.Gamertag, ''), NULLIF(l.AccountName, ''), CONCAT('Player', l.CustomerID)) AS Gamertag,
                    COALESCE(s.Kills, 0) AS Kills,
                    COALESCE(s.Deaths, 0) AS Deaths,
                    COALESCE(s.Wins, 0) AS Wins,
                    COALESCE(s.Losses, 0) AS Losses,
                    COALESCE(s.TimePlayed, 0) AS TimePlayed,
                    (
                        COALESCE(s.Kills, 0) * 10 +
                        COALESCE(s.Wins, 0) * 50 +
                        COALESCE(s.AssistKills, 0) * 3 -
                        COALESCE(s.Deaths, 0) * 2 +
                        FLOOR(COALESCE(s.TimePlayed, 0) / 60)
                    ) AS Score
                FROM loginid l
                LEFT JOIN stats s ON s.CustomerID = l.CustomerID
                WHERE COALESCE(l.AccountStatus, 0) < 200
                ORDER BY
                    Score DESC,
                    COALESCE(s.Kills, 0) DESC,
                    COALESCE(s.Deaths, 0) ASC,
                    l.CustomerID ASC
                LIMIT @Size OFFSET @Position;
                """,
                new
                {
                    Position = position,
                    Size = size
                })).ToArray();

            var entries = rows
                .Select((row, index) => new LeaderboardEntryDto(
                    position + index + 1,
                    row.CustomerId,
                    row.Gamertag,
                    row.Score,
                    row.Kills,
                    row.Deaths,
                    row.Wins,
                    row.Losses,
                    row.TimePlayed))
                .ToArray();

            return new LeaderboardResponse(
                true,
                0,
                "OK",
                position,
                entries.Length,
                entries);
        }
        catch
        {
            return new LeaderboardResponse(
                true,
                0,
                "OK",
                position,
                0,
                Array.Empty<LeaderboardEntryDto>());
        }
    }

    public string BuildLegacyLeaderboardXml(LeaderboardResponse leaderboard)
    {
        if (leaderboard.Entries.Count == 0)
            return $"<leaderboard pos=\"{leaderboard.Position}\" size=\"0\"></leaderboard>";

        var xml = new StringBuilder();

        xml.Append("<leaderboard ");
        LegacyUtil.AppendXmlAttr(xml, "pos", leaderboard.Position);
        LegacyUtil.AppendXmlAttr(xml, "size", leaderboard.Entries.Count);
        xml.Append(">");

        foreach (var entry in leaderboard.Entries)
        {
            xml.Append("<r ");
            LegacyUtil.AppendXmlAttr(xml, "pos", entry.Position);
            LegacyUtil.AppendXmlAttr(xml, "CustomerID", entry.CustomerId);
            LegacyUtil.AppendXmlAttr(xml, "gt", entry.Gamertag);
            LegacyUtil.AppendXmlAttr(xml, "score", entry.Score);
            LegacyUtil.AppendXmlAttr(xml, "kills", entry.Kills);
            LegacyUtil.AppendXmlAttr(xml, "deaths", entry.Deaths);
            LegacyUtil.AppendXmlAttr(xml, "wins", entry.Wins);
            LegacyUtil.AppendXmlAttr(xml, "losses", entry.Losses);
            LegacyUtil.AppendXmlAttr(xml, "time", entry.TimePlayed);
            xml.Append("/>");
        }

        xml.Append("</leaderboard>");

        return xml.ToString();
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

    private sealed class LeaderboardRow
    {
        public ulong CustomerId { get; set; }
        public string Gamertag { get; set; } = "";
        public long Score { get; set; }
        public int Kills { get; set; }
        public int Deaths { get; set; }
        public int Wins { get; set; }
        public int Losses { get; set; }
        public int TimePlayed { get; set; }
    }
}