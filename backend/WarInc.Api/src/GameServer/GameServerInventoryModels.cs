namespace WarInc.Api.GameServer;

public sealed record GameServerInventoryActionResponse(
    bool Ok,
    int Code,
    string Message,
    ulong CustomerId,
    int ItemId,
    int Quantity,
    int ExpMinutes
);