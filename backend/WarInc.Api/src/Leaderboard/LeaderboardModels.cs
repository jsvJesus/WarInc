namespace WarInc.Api.Leaderboard;

public sealed record LeaderboardEntryDto(
    int Position,
    ulong CustomerId,
    string Gamertag,
    long Score,
    int Kills,
    int Deaths,
    int Wins,
    int Losses,
    int TimePlayed);

public sealed record LeaderboardResponse(
    bool Ok,
    int Code,
    string Message,
    int Position,
    int Size,
    IReadOnlyList<LeaderboardEntryDto> Entries);