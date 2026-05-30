namespace WarInc.Api.Purchase;

public record BuyItemRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    int ItemId,
    int BuyIdx,
    int Param1 = 0);

public record BuyItemResponse(
    bool Ok,
    int Code,
    string Message,
    int Balance,
    bool IsGameDollars);

public sealed class AccountBalanceRow
{
    public ulong CustomerId { get; set; }
    public int GamePoints { get; set; }
    public int GameDollars { get; set; }
}

public sealed class BuyItemPriceRow
{
    public int ItemId { get; set; }
    public int ItemType { get; set; }
    public int Category { get; set; }
    public int IsEnabled { get; set; } = 1;

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}

public sealed class BuyPackageRow
{
    public int IsEnabled { get; set; }

    public int AddGP { get; set; }
    public int AddSP { get; set; }

    public int Item1Id { get; set; }
    public int Item1Exp { get; set; }
    public int Item2Id { get; set; }
    public int Item2Exp { get; set; }
    public int Item3Id { get; set; }
    public int Item3Exp { get; set; }
    public int Item4Id { get; set; }
    public int Item4Exp { get; set; }
    public int Item5Id { get; set; }
    public int Item5Exp { get; set; }
    public int Item6Id { get; set; }
    public int Item6Exp { get; set; }
}

public sealed class DefaultWeaponAttachmentsRow
{
    public int IsFps { get; set; }

    public int FpsAttach0 { get; set; }
    public int FpsAttach1 { get; set; }
    public int FpsAttach2 { get; set; }
    public int FpsAttach3 { get; set; }
    public int FpsAttach4 { get; set; }
    public int FpsAttach5 { get; set; }
    public int FpsAttach6 { get; set; }
    public int FpsAttach7 { get; set; }
    public int FpsAttach8 { get; set; }
}

public sealed class InventoryLeaseRow
{
    public DateTime LeasedUntil { get; set; }
    public int Quantity { get; set; }
}

public sealed class FpsAttachmentLeaseRow
{
    public DateTime LeasedUntil { get; set; }
}

public sealed class BuyExecResult
{
    public bool Ok { get; set; }
    public string Message { get; set; } = "";

    public static BuyExecResult Success()
    {
        return new BuyExecResult
        {
            Ok = true,
            Message = "OK"
        };
    }

    public static BuyExecResult Fail(string message)
    {
        return new BuyExecResult
        {
            Ok = false,
            Message = message
        };
    }
}