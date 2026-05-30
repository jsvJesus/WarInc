namespace WarInc.Api.MysteryBox;

public sealed record MysteryBoxRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    int BoxId,
    int ItemId);

public sealed record MysteryBoxItemDto(
    int ItemId,
    int Quantity,
    int Weight,
    int Rarity,
    int IsJackpot);

public sealed record MysteryBoxInfoResponse(
    bool Ok,
    int Code,
    string Message,
    IReadOnlyList<MysteryBoxItemDto> Items);

public sealed record MysteryBoxRollResponse(
    bool Ok,
    int Code,
    string Message,
    int BoxId,
    int ItemId,
    int Quantity,
    int MinutesLeft);

public sealed record MysteryBoxSellResponse(
    bool Ok,
    int Code,
    string Message,
    int BoxId,
    int ItemId,
    int Currency,
    int Amount);