using System.Text;
using Dapper;
using MySqlConnector;
using WarInc.Api.Common;
using WarInc.Api.Database;

namespace WarInc.Api.GameRewards;

public sealed class GameRewardsService
{
    private readonly WarIncDb _db;

    private static readonly GameRewardDto[] DefaultRewards =
    {
        new(1, "Kill", 2, 10, 5, 10, 10, 10),
        new(2, "Headshot", 10, 20, 10, 20, 10, 10),
        new(3, "AssistedKill", 1, 10, 1, 5, 10, 10),
        new(4, "ExplosionKill", 100, 50, 10, 10, 10, 10),
        new(5, "FriendlyKill", 0, -20, 0, 0, 0, -20),
        new(6, "AvengeKill", 10, 100, 10, 10, 10, 10),
        new(7, "RevengeKill", 20, 250, 20, 15, 10, 10),

        new(14, "Killstreak2", 100, 50, 10, 10, 0, 0),
        new(15, "Killstreak3", 150, 100, 20, 20, 0, 0),
        new(16, "Killstreak4", 200, 125, 50, 35, 0, 0),
        new(17, "Killstreak5", 500, 150, 100, 50, 0, 0),
        new(18, "Killstreak6", 1000, 200, 250, 75, 0, 0),

        new(100, "CaptureNeutral", 100, 250, 0, 0, 0, 0),
        new(101, "CaptureEnemy", 200, 500, 0, 0, 0, 0),
        new(102, "KillCloseToYourFlag", 50, 50, 0, 0, 0, 0),
        new(103, "KillCloseToTheirFlag", 50, 20, 0, 0, 0, 0),
        new(104, "KillAtSpawn", 0, 0, 0, 0, 0, 0),

        new(120, "Bomb_PlantBomb", 0, 0, 0, 0, 10, 200),
        new(121, "Bomb_DiffuseBomb", 0, 0, 0, 0, 20, 200),
        new(122, "Bomb_WinRoundKills", 0, 0, 0, 0, 10, 10),
        new(123, "Bomb_WinRound", 0, 0, 0, 0, 25, 50),
        new(124, "Bomb_Pickup", 0, 0, 0, 0, 5, 0),

        new(140, "Win", 500, 500, 500, 500, 500, 500),
        new(141, "Loss", 200, 250, 200, 250, 200, 250),
        new(142, "BestPlayerInTeam", 150, 250, 150, 250, 150, 250),
        new(143, "BestPlayerOverall", 250, 500, 250, 500, 250, 500),

        new(200, "BlackopResupply", 10, 10, 10, 10, 10, 10),
        new(201, "UseUAV", 0, 50, 0, 50, 0, 50),
        new(202, "UAVAssist", 50, 50, 50, 50, 50, 50),
        new(203, "DestroyMine", 0, 20, 0, 20, 0, 20),

        new(300, "FirstCaptureNeutral", 150, 100, 0, 0, 0, 0),
        new(301, "FirstCaptureEnemy", 250, 200, 0, 0, 0, 0),
        new(302, "Kill5NotDying", 25, 50, 25, 50, 25, 50),
        new(303, "Kill10NotDying", 65, 100, 65, 100, 65, 100),
        new(304, "Kill25NotDying", 150, 200, 150, 200, 150, 200),
        new(305, "Kill50NotDying", 150, 200, 150, 200, 150, 200),
        new(306, "KillGrenadeAfterDeath", 10, 100, 10, 100, 10, 100),
        new(307, "KillEnemyWithHisGun", 50, 50, 50, 50, 50, 50)
    };

    public GameRewardsService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<GameRewardsResponse> GetRewardsAsync()
    {
        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "datagamerewards"))
        {
            return new GameRewardsResponse(
                true,
                0,
                "OK",
                DefaultRewards);
        }

        var rows = (await db.QueryAsync<GameRewardRow>(
            """
            SELECT
                RewardID AS RewardId,
                RewardName AS RewardName,
                GD_CQ AS GdCq,
                HP_CQ AS HpCq,
                GD_DM AS GdDm,
                HP_DM AS HpDm,
                GD_SB AS GdSb,
                HP_SB AS HpSb
            FROM datagamerewards
            ORDER BY RewardID;
            """
        )).ToArray();

        if (rows.Length == 0)
        {
            return new GameRewardsResponse(
                true,
                0,
                "OK",
                DefaultRewards);
        }

        var rewards = rows
            .Select(x => new GameRewardDto(
                x.RewardId,
                NormalizeRewardName(x.RewardName),
                x.GdCq,
                x.HpCq,
                x.GdDm,
                x.HpDm,
                x.GdSb,
                x.HpSb))
            .ToArray();

        return new GameRewardsResponse(
            true,
            0,
            "OK",
            rewards);
    }

    public string BuildLegacyXml(GameRewardsResponse response)
    {
        var xml = new StringBuilder();

        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append("<rewards>");

        foreach (var reward in response.Rewards)
        {
            xml.Append("<rwd ");
            LegacyUtil.AppendXmlAttr(xml, "ID", reward.RewardId);
            LegacyUtil.AppendXmlAttr(xml, "Name", reward.Name);
            LegacyUtil.AppendXmlAttr(xml, "GD_CQ", reward.GdCq);
            LegacyUtil.AppendXmlAttr(xml, "HP_CQ", reward.HpCq);
            LegacyUtil.AppendXmlAttr(xml, "GD_DM", reward.GdDm);
            LegacyUtil.AppendXmlAttr(xml, "HP_DM", reward.HpDm);
            LegacyUtil.AppendXmlAttr(xml, "GD_SB", reward.GdSb);
            LegacyUtil.AppendXmlAttr(xml, "HP_SB", reward.HpSb);
            xml.Append("/>\n");
        }

        xml.Append("</rewards>");

        return xml.ToString();
    }

    private static string NormalizeRewardName(string value)
    {
        var name = (value ?? "").Trim();

        if (name.StartsWith("RWD_", StringComparison.OrdinalIgnoreCase))
            name = name[4..];

        return name;
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

    private sealed class GameRewardRow
    {
        public int RewardId { get; set; }
        public string RewardName { get; set; } = "";
        public int GdCq { get; set; }
        public int HpCq { get; set; }
        public int GdDm { get; set; }
        public int HpDm { get; set; }
        public int GdSb { get; set; }
        public int HpSb { get; set; }
    }
}