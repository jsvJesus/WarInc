namespace WarInc.Api.MysteryBox;

public sealed record MysteryBoxRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    int BoxId,
    int ItemId,
    int BuyIdx);

public sealed record MysteryBoxItemDto(
    int ItemId,
    double Chance,
    int Value1,
    int Value2,
    int GdIfHave);

public sealed record MysteryBoxInfoResponse(
    bool Ok,
    int Code,
    string Message,
    int LootId,
    int Category,
    IReadOnlyList<MysteryBoxItemDto> Items);

public sealed record MysteryBoxRollResponse(
    bool Ok,
    int Code,
    string Message,
    int LootId,
    int WonItemId,
    int ExpDays,
    int GameDollars,
    int Balance,
    double Roll);

public sealed record MysteryBoxSellResponse(
    bool Ok,
    int Code,
    string Message,
    int ItemId,
    int Removed);