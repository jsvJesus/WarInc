namespace WarInc.Api.Leaderboard;

public sealed record LeaderboardEntryDto(
    int Position,
    ulong CustomerId,
    string Gamertag,
    long Score,
    int Rank,
    int HonorPoints,
    int Kills,
    int Deaths,
    int Wins,
    int Losses,
    int ShotsFired,
    int ShotsHit,
    int TimePlayed,
    int HavePremium);

public sealed record LeaderboardResponse(
    bool Ok,
    int Code,
    string Message,
    int Position,
    int Size,
    int TableId,
    string TableName,
    IReadOnlyList<LeaderboardEntryDto> Entries);