namespace WarInc.Api.Profile;

public record ProfileRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    bool JoiningGame);

public sealed class ProfileBundle
{
    public bool Ok { get; set; }
    public int Code { get; set; }
    public string Message { get; set; } = "";

    public ProfileAccountRow? Account { get; set; }

    public List<ProfileLoadoutRow> Loadouts { get; set; } = new();
    public List<ProfileAchievementRow> Achievements { get; set; } = new();
    public List<ProfileFpsAttachmentRow> FpsAttachments { get; set; } = new();
    public List<ProfileInventoryRow> Inventory { get; set; } = new();
    public List<int> NewItems { get; set; } = new();

    public ProfileStatsWindowRow DailyStats { get; set; } = new();
    public ProfileStatsWindowRow WeeklyStats { get; set; } = new();

    public static ProfileBundle Fail(int code, string message)
    {
        return new ProfileBundle
        {
            Ok = false,
            Code = code,
            Message = message
        };
    }
}

public sealed class ProfileAccountRow
{
    public ulong CustomerId { get; set; }
    public int AccountStatus { get; set; }

    public int GamePoints { get; set; }
    public int GameDollars { get; set; }
    public int HonorPoints { get; set; }
    public int SkillPoints { get; set; }

    public string Gamertag { get; set; } = "";

    public int Faction1Score { get; set; }
    public int Faction2Score { get; set; }
    public int Faction3Score { get; set; }
    public int Faction4Score { get; set; }
    public int Faction5Score { get; set; }

    public int ClanId { get; set; }
    public int ClanRank { get; set; }
    public string ClanTag { get; set; } = "";
    public int ClanTagColor { get; set; }

    public int IsFPSEnabled { get; set; }
    public int IsDeveloper { get; set; }

    public int Kills { get; set; }
    public int Deaths { get; set; }
    public int ShotsFired { get; set; }
    public int ShotsHits { get; set; }
    public int Headshots { get; set; }
    public int AssistKills { get; set; }
    public int Wins { get; set; }
    public int Losses { get; set; }
    public int CaptureNeutralPoints { get; set; }
    public int CaptureEnemyPoints { get; set; }
    public int TimePlayed { get; set; }

    public string Skills { get; set; } = "";
    public string Abilities { get; set; } = "";
}

public sealed class ProfileLoadoutRow
{
    public int LoadoutId { get; set; }
    public int Class { get; set; }
    public int HonorPoints { get; set; }
    public int TimePlayed { get; set; }
    public string Loadout { get; set; } = "";
    public string Skills { get; set; } = "";
    public int SpendSP1 { get; set; }
    public int SpendSP2 { get; set; }
    public int SpendSP3 { get; set; }
}

public sealed class ProfileAchievementRow
{
    public int AchId { get; set; }
    public int Value { get; set; }
    public int Unlocked { get; set; }
}

public sealed class ProfileFpsAttachmentRow
{
    public int WeaponId { get; set; }
    public int AttachmentId { get; set; }
    public int MinutesLeft { get; set; }
    public int IsEquipped { get; set; }
}

public sealed class ProfileInventoryRow
{
    public int ItemId { get; set; }
    public int MinutesLeft { get; set; }
    public int Quantity { get; set; }
}

public sealed class ProfileStatsWindowRow
{
    public int DailyGames { get; set; }
    public int Kills { get; set; }
    public int Headshots { get; set; }
    public int CaptureFlags { get; set; }
    public int MatchesCQ { get; set; }
    public int MatchesDM { get; set; }
    public int MatchesSB { get; set; }
}