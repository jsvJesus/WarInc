namespace WarInc.Api.Retention;

public sealed record RetentionRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token);

public sealed record RetentionDayDto(
    int Day,
    int Claimed,
    int RewardId,
    int RewardAmount,
    string RewardType);

public sealed record RetentionInfoResponse(
    bool Ok,
    int Code,
    string Message,
    int CurrentDay,
    int MaxDay,
    IReadOnlyList<RetentionDayDto> Days);

public sealed record RetentionClaimResponse(
    bool Ok,
    int Code,
    string Message,
    int ClaimedDay,
    IReadOnlyList<RetentionDayDto> Rewards);