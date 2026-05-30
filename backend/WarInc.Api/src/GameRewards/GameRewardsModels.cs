namespace WarInc.Api.GameRewards;

public sealed record GameRewardsResponse(
    bool Ok,
    int Code,
    string Message,
    IReadOnlyList<GameRewardDto> Rewards
);

public sealed record GameRewardDto(
    int RewardId,
    string Name,
    int GdCq,
    int HpCq,
    int GdDm,
    int HpDm,
    int GdSb,
    int HpSb
);