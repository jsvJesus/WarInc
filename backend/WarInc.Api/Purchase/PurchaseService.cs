using System.Globalization;
using Dapper;
using MySqlConnector;
using WarInc.Api.Database;

namespace WarInc.Api.Purchase;

public sealed class PurchaseService
{
    private readonly WarIncDb _db;

    public PurchaseService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<BuyItemResponse> BuyItemAsync(
        ulong customerId,
        int itemId,
        int buyIdx,
        int param1)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new BuyItemResponse(false, 6, "bad CustomerID", 0, false);

        if (itemId <= 0)
            return new BuyItemResponse(false, 6, "bad ItemID", 0, false);

        if (!TryGetBuyInfo(buyIdx, out var buyDays, out var isGameDollars))
            return new BuyItemResponse(false, 6, "bad BuyIdx", 0, false);

        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var account = await db.QueryFirstOrDefaultAsync<AccountBalanceRow>(
                """
                SELECT
                    CustomerID AS CustomerId,
                    GamePoints AS GamePoints,
                    GameDollars AS GameDollars
                FROM loginid
                WHERE CustomerID = @CustomerId
                FOR UPDATE;
                """,
                new { CustomerId = customerId },
                tx);

            if (account == null)
            {
                await tx.RollbackAsync();
                return new BuyItemResponse(false, 6, "no CustomerID", 0, isGameDollars);
            }

            var priceRow = await GetBuyItemPriceAsync(db, tx, itemId);

            if (priceRow == null)
            {
                await tx.RollbackAsync();
                return new BuyItemResponse(false, 6, "bad GetPrice", 0, isGameDollars);
            }

            var finalPrice = GetFinalPrice(priceRow, buyDays, isGameDollars);

            if (finalPrice <= 0 || priceRow.IsEnabled == 0)
            {
                await tx.RollbackAsync();
                return new BuyItemResponse(false, 6, "bad GetPrice", 0, isGameDollars);
            }

            var currentBalance = isGameDollars
                ? account.GameDollars
                : account.GamePoints;

            if (currentBalance < finalPrice)
            {
                await tx.RollbackAsync();

                return new BuyItemResponse(
                    false,
                    7,
                    isGameDollars ? "Not Enough GD" : "Not Enough GP",
                    currentBalance,
                    isGameDollars);
            }

            var execResult = await ExecuteBoughtItemAsync(
                db,
                tx,
                customerId,
                priceRow,
                buyDays,
                param1);

            if (!execResult.Ok)
            {
                await tx.RollbackAsync();
                return new BuyItemResponse(false, 7, execResult.Message, currentBalance, isGameDollars);
            }

            if (isGameDollars)
            {
                await db.ExecuteAsync(
                    """
                    UPDATE loginid
                    SET GameDollars = GameDollars - @FinalPrice
                    WHERE CustomerID = @CustomerId;
                    """,
                    new
                    {
                        CustomerId = customerId,
                        FinalPrice = finalPrice
                    },
                    tx);
            }
            else
            {
                await db.ExecuteAsync(
                    """
                    UPDATE loginid
                    SET GamePoints = GamePoints - @FinalPrice
                    WHERE CustomerID = @CustomerId;
                    """,
                    new
                    {
                        CustomerId = customerId,
                        FinalPrice = finalPrice
                    },
                    tx);
            }

            await db.ExecuteAsync(
                """
                INSERT INTO financialtransactions
                (
                    CustomerID,
                    TransactionID,
                    TransactionType,
                    DateTime,
                    Amount,
                    ResponseCode,
                    AprovalCode,
                    ItemID
                )
                VALUES
                (
                    @CustomerId,
                    'INGAME',
                    @TransactionType,
                    UTC_TIMESTAMP(),
                    @Amount,
                    '1',
                    'APPROVED',
                    @ItemId
                );
                """,
                new
                {
                    CustomerId = customerId,
                    TransactionType = GetTransactionType(buyDays, isGameDollars),
                    Amount = finalPrice,
                    ItemId = itemId.ToString(CultureInfo.InvariantCulture)
                },
                tx);

            var newBalance = await db.ExecuteScalarAsync<int>(
                isGameDollars
                    ? "SELECT GameDollars FROM loginid WHERE CustomerID = @CustomerId;"
                    : "SELECT GamePoints FROM loginid WHERE CustomerID = @CustomerId;",
                new { CustomerId = customerId },
                tx);

            await tx.CommitAsync();

            return new BuyItemResponse(
                true,
                0,
                "OK",
                newBalance,
                isGameDollars);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return new BuyItemResponse(false, 5, e.Message, 0, isGameDollars);
        }
    }

    private static bool TryGetBuyInfo(
        int buyIdx,
        out int buyDays,
        out bool isGameDollars)
    {
        buyDays = 0;
        isGameDollars = false;

        switch (buyIdx)
        {
            case 1:
                buyDays = 1;
                return true;

            case 2:
                buyDays = 7;
                return true;

            case 3:
                buyDays = 30;
                return true;

            case 4:
                buyDays = 2000;
                return true;

            case 5:
                buyDays = 1;
                isGameDollars = true;
                return true;

            case 6:
                buyDays = 7;
                isGameDollars = true;
                return true;

            case 7:
                buyDays = 30;
                isGameDollars = true;
                return true;

            case 8:
                buyDays = 2000;
                isGameDollars = true;
                return true;

            default:
                return false;
        }
    }

    private static int GetFinalPrice(
        BuyItemPriceRow item,
        int buyDays,
        bool isGameDollars)
    {
        if (!isGameDollars)
        {
            return buyDays switch
            {
                1 => item.Price1,
                7 => item.Price7,
                30 => item.Price30,
                2000 => item.PricePerm,
                _ => 0
            };
        }

        return buyDays switch
        {
            1 => item.GPrice1,
            7 => item.GPrice7,
            30 => item.GPrice30,
            2000 => item.GPricePerm,
            _ => 0
        };
    }

    private static int GetTransactionType(
        int buyDays,
        bool isGameDollars)
    {
        if (isGameDollars)
            return buyDays == 2000 ? 3001 : 2001;

        return buyDays == 2000 ? 3000 : 2000;
    }

    private static async Task<BuyItemPriceRow?> GetBuyItemPriceAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int itemId)
    {
        if (itemId >= 20000 && itemId < 99999)
        {
            return await db.QueryFirstOrDefaultAsync<BuyItemPriceRow>(
                """
                SELECT
                    ItemID AS ItemId,
                    1 AS ItemType,
                    Category AS Category,
                    1 AS IsEnabled,
                    Price1 AS Price1,
                    Price7 AS Price7,
                    Price30 AS Price30,
                    PriceP AS PricePerm,
                    GPrice1 AS GPrice1,
                    GPrice7 AS GPrice7,
                    GPrice30 AS GPrice30,
                    GPriceP AS GPricePerm
                FROM items_gear
                WHERE ItemID = @ItemId
                LIMIT 1;
                """,
                new { ItemId = itemId },
                tx);
        }

        if (itemId >= 100000 && itemId < 190000)
        {
            return await db.QueryFirstOrDefaultAsync<BuyItemPriceRow>(
                """
                SELECT
                    ItemID AS ItemId,
                    2 AS ItemType,
                    Category AS Category,
                    1 AS IsEnabled,
                    Price1 AS Price1,
                    Price7 AS Price7,
                    Price30 AS Price30,
                    PriceP AS PricePerm,
                    GPrice1 AS GPrice1,
                    GPrice7 AS GPrice7,
                    GPrice30 AS GPrice30,
                    GPriceP AS GPricePerm
                FROM items_weapons
                WHERE ItemID = @ItemId
                LIMIT 1;
                """,
                new { ItemId = itemId },
                tx);
        }

        if (itemId >= 300000 && itemId < 390000)
        {
            return await db.QueryFirstOrDefaultAsync<BuyItemPriceRow>(
                """
                SELECT
                    ItemID AS ItemId,
                    3 AS ItemType,
                    Category AS Category,
                    1 AS IsEnabled,
                    Price1 AS Price1,
                    Price7 AS Price7,
                    Price30 AS Price30,
                    PriceP AS PricePerm,
                    GPrice1 AS GPrice1,
                    GPrice7 AS GPrice7,
                    GPrice30 AS GPrice30,
                    GPriceP AS GPricePerm
                FROM items_generic
                WHERE ItemID = @ItemId
                LIMIT 1;
                """,
                new { ItemId = itemId },
                tx);
        }

        if (itemId >= 400000 && itemId < 490000)
        {
            return await db.QueryFirstOrDefaultAsync<BuyItemPriceRow>(
                """
                SELECT
                    ItemID AS ItemId,
                    5 AS ItemType,
                    Category AS Category,
                    1 AS IsEnabled,
                    Price1 AS Price1,
                    Price7 AS Price7,
                    Price30 AS Price30,
                    PriceP AS PricePerm,
                    GPrice1 AS GPrice1,
                    GPrice7 AS GPrice7,
                    GPrice30 AS GPrice30,
                    GPriceP AS GPricePerm
                FROM items_attachments
                WHERE ItemID = @ItemId
                LIMIT 1;
                """,
                new { ItemId = itemId },
                tx);
        }

        if (itemId >= 500000 && itemId < 590000)
        {
            return await db.QueryFirstOrDefaultAsync<BuyItemPriceRow>(
                """
                SELECT
                    ItemID AS ItemId,
                    4 AS ItemType,
                    Category AS Category,
                    IsEnabled AS IsEnabled,
                    Price1 AS Price1,
                    Price7 AS Price7,
                    Price30 AS Price30,
                    PriceP AS PricePerm,
                    GPrice1 AS GPrice1,
                    GPrice7 AS GPrice7,
                    GPrice30 AS GPrice30,
                    GPriceP AS GPricePerm
                FROM items_packages
                WHERE ItemID = @ItemId
                LIMIT 1;
                """,
                new { ItemId = itemId },
                tx);
        }

        if (itemId >= 600000 && itemId < 700000)
        {
            return await db.QueryFirstOrDefaultAsync<BuyItemPriceRow>(
                """
                SELECT
                    ItemID AS ItemId,
                    6 AS ItemType,
                    Category AS Category,
                    1 AS IsEnabled,
                    Price1 AS Price1,
                    Price7 AS Price7,
                    Price30 AS Price30,
                    PriceP AS PricePerm,
                    GPrice1 AS GPrice1,
                    GPrice7 AS GPrice7,
                    GPrice30 AS GPrice30,
                    GPriceP AS GPricePerm
                FROM items_upgradedata
                WHERE ItemID = @ItemId
                LIMIT 1;
                """,
                new { ItemId = itemId },
                tx);
        }

        return null;
    }

    private static async Task<BuyExecResult> ExecuteBoughtItemAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        ulong customerId,
        BuyItemPriceRow item,
        int buyDays,
        int param1)
    {
        if (item.ItemType == 5)
        {
            await AddAttachmentToUserAsync(db, tx, customerId, param1, item.ItemId, buyDays);
            return BuyExecResult.Success();
        }

        if (item.ItemType == 4)
            return await AddPackageToUserAsync(db, tx, customerId, item.ItemId);

        if (item.ItemType == 6)
            return BuyExecResult.Success();

        if (item.ItemId == 301142)
            return BuyExecResult.Success();

        if (item.ItemId == 301055)
            return BuyExecResult.Success();

        if (item.ItemId == 301045)
            return BuyExecResult.Success();

        if (item.ItemId == 301046)
        {
            await db.ExecuteAsync(
                """
                UPDATE stats
                SET ShotsFired = 0,
                    ShotsHits = 0
                WHERE CustomerID = @CustomerId;
                """,
                new { CustomerId = customerId },
                tx);

            return BuyExecResult.Success();
        }

        if (item.ItemId == 301047)
        {
            await db.ExecuteAsync(
                """
                UPDATE stats
                SET Kills = 0,
                    Deaths = 0
                WHERE CustomerID = @CustomerId;
                """,
                new { CustomerId = customerId },
                tx);

            return BuyExecResult.Success();
        }

        if (item.ItemId == 301049)
        {
            await db.ExecuteAsync(
                """
                UPDATE stats
                SET Wins = 0,
                    Losses = 0
                WHERE CustomerID = @CustomerId;
                """,
                new { CustomerId = customerId },
                tx);

            return BuyExecResult.Success();
        }

        if (item.ItemId == 301048)
        {
            await db.ExecuteAsync(
                """
                UPDATE profile_chars
                SET Skills = '0000000000000000000000000000000',
                    SpendSP1 = 0,
                    SpendSP2 = 0,
                    SpendSP3 = 0
                WHERE CustomerID = @CustomerId;
                """,
                new { CustomerId = customerId },
                tx);

            return BuyExecResult.Success();
        }

        if (item.ItemType == 3)
        {
            if (item.Category == 3 || item.Category == 6)
                return BuyExecResult.Success();

            if (item.Category == 7)
            {
                await RemoveOneItemFromUserAsync(db, tx, customerId, item.ItemId);
                return BuyExecResult.Success();
            }
        }

        if (item.ItemId == 301107)
        {
            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET GameDollars = GameDollars + 10000
                WHERE CustomerID = @CustomerId;
                """,
                new { CustomerId = customerId },
                tx);

            return BuyExecResult.Success();
        }

        await AddItemToUserAsync(db, tx, customerId, item.ItemId, buyDays);
        return BuyExecResult.Success();
    }

    private static async Task AddItemToUserAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        ulong customerId,
        int itemId,
        int expDays)
    {
        if (itemId >= 100000 && itemId <= 199999)
            await AddDefaultAttachmentsToUserAsync(db, tx, customerId, itemId, expDays);

        var current = await db.QueryFirstOrDefaultAsync<InventoryLeaseRow>(
            """
            SELECT
                LeasedUntil AS LeasedUntil,
                Quantity AS Quantity
            FROM inventory
            WHERE CustomerID = @CustomerId
              AND ItemID = @ItemId
            FOR UPDATE;
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId
            },
            tx);

        if (current == null)
        {
            await db.ExecuteAsync(
                """
                INSERT INTO inventory
                (
                    CustomerID,
                    ItemID,
                    LeasedUntil,
                    Quantity,
                    UpSlot1,
                    UpSlot2,
                    UpSlot3,
                    UpSlot4,
                    UpSlot5
                )
                VALUES
                (
                    @CustomerId,
                    @ItemId,
                    DATE_ADD(UTC_TIMESTAMP(), INTERVAL @ExpDays DAY),
                    1,
                    0,
                    0,
                    0,
                    0,
                    0
                );
                """,
                new
                {
                    CustomerId = customerId,
                    ItemId = itemId,
                    ExpDays = expDays
                },
                tx);

            return;
        }

        await db.ExecuteAsync(
            """
            UPDATE inventory
            SET LeasedUntil =
                DATE_ADD(
                    CASE
                        WHEN LeasedUntil < UTC_TIMESTAMP() THEN UTC_TIMESTAMP()
                        ELSE LeasedUntil
                    END,
                    INTERVAL @ExpDays DAY
                )
            WHERE CustomerID = @CustomerId
              AND ItemID = @ItemId;
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId,
                ExpDays = expDays
            },
            tx);

        var isStackable = await db.ExecuteScalarAsync<int>(
            """
            SELECT IFNULL(IsStackable, 0)
            FROM items_generic
            WHERE ItemID = @ItemId
            LIMIT 1;
            """,
            new { ItemId = itemId },
            tx);

        if (isStackable > 0)
        {
            await db.ExecuteAsync(
                """
                UPDATE inventory
                SET Quantity = Quantity + 1
                WHERE CustomerID = @CustomerId
                  AND ItemID = @ItemId;
                """,
                new
                {
                    CustomerId = customerId,
                    ItemId = itemId
                },
                tx);
        }
    }

    private static async Task AddDefaultAttachmentsToUserAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        ulong customerId,
        int weaponId,
        int expDays)
    {
        var row = await db.QueryFirstOrDefaultAsync<DefaultWeaponAttachmentsRow>(
            """
            SELECT
                IsFPS AS IsFps,
                FPSAttach0 AS FpsAttach0,
                FPSAttach1 AS FpsAttach1,
                FPSAttach2 AS FpsAttach2,
                FPSAttach3 AS FpsAttach3,
                FPSAttach4 AS FpsAttach4,
                FPSAttach5 AS FpsAttach5,
                FPSAttach6 AS FpsAttach6,
                FPSAttach7 AS FpsAttach7,
                FPSAttach8 AS FpsAttach8
            FROM items_weapons
            WHERE ItemID = @WeaponId
            LIMIT 1;
            """,
            new { WeaponId = weaponId },
            tx);

        if (row == null || row.IsFps <= 0)
            return;

        var attachments = new[]
        {
            row.FpsAttach0,
            row.FpsAttach1,
            row.FpsAttach2,
            row.FpsAttach3,
            row.FpsAttach4,
            row.FpsAttach5,
            row.FpsAttach6,
            row.FpsAttach7,
            row.FpsAttach8
        };

        foreach (var attachmentId in attachments)
        {
            if (attachmentId <= 0)
                continue;

            await AddAttachmentToUserAsync(
                db,
                tx,
                customerId,
                weaponId,
                attachmentId,
                expDays);
        }
    }

    private static async Task AddAttachmentToUserAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        ulong customerId,
        int weaponId,
        int attachmentId,
        int expDays)
    {
        var slot = await db.ExecuteScalarAsync<int?>(
            """
            SELECT Type
            FROM items_attachments
            WHERE ItemID = @AttachmentId
            LIMIT 1;
            """,
            new { AttachmentId = attachmentId },
            tx);

        if (slot == null)
            return;

        var current = await db.QueryFirstOrDefaultAsync<FpsAttachmentLeaseRow>(
            """
            SELECT LeasedUntil AS LeasedUntil
            FROM inventory_fps
            WHERE CustomerID = @CustomerId
              AND WeaponID = @WeaponId
              AND AttachmentID = @AttachmentId
            FOR UPDATE;
            """,
            new
            {
                CustomerId = customerId,
                WeaponId = weaponId,
                AttachmentId = attachmentId
            },
            tx);

        if (current == null)
        {
            await db.ExecuteAsync(
                """
                UPDATE inventory_fps
                SET IsEquipped = 0
                WHERE CustomerID = @CustomerId
                  AND WeaponID = @WeaponId
                  AND Slot = @Slot;
                """,
                new
                {
                    CustomerId = customerId,
                    WeaponId = weaponId,
                    Slot = slot.Value
                },
                tx);

            await db.ExecuteAsync(
                """
                INSERT INTO inventory_fps
                (
                    CustomerID,
                    WeaponID,
                    AttachmentID,
                    LeasedUntil,
                    Slot,
                    IsEquipped
                )
                VALUES
                (
                    @CustomerId,
                    @WeaponId,
                    @AttachmentId,
                    DATE_ADD(UTC_TIMESTAMP(), INTERVAL @ExpDays DAY),
                    @Slot,
                    1
                );
                """,
                new
                {
                    CustomerId = customerId,
                    WeaponId = weaponId,
                    AttachmentId = attachmentId,
                    ExpDays = expDays,
                    Slot = slot.Value
                },
                tx);

            return;
        }

        await db.ExecuteAsync(
            """
            UPDATE inventory_fps
            SET LeasedUntil =
                DATE_ADD(
                    CASE
                        WHEN LeasedUntil < UTC_TIMESTAMP() THEN UTC_TIMESTAMP()
                        ELSE LeasedUntil
                    END,
                    INTERVAL @ExpDays DAY
                )
            WHERE CustomerID = @CustomerId
              AND WeaponID = @WeaponId
              AND AttachmentID = @AttachmentId;
            """,
            new
            {
                CustomerId = customerId,
                WeaponId = weaponId,
                AttachmentId = attachmentId,
                ExpDays = expDays
            },
            tx);
    }

    private static async Task<BuyExecResult> AddPackageToUserAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        ulong customerId,
        int packageId)
    {
        var package = await db.QueryFirstOrDefaultAsync<BuyPackageRow>(
            """
            SELECT
                IsEnabled AS IsEnabled,
                AddGP AS AddGP,
                AddSP AS AddSP,
                Item1_ID AS Item1Id,
                Item1_Exp AS Item1Exp,
                Item2_ID AS Item2Id,
                Item2_Exp AS Item2Exp,
                Item3_ID AS Item3Id,
                Item3_Exp AS Item3Exp,
                Item4_ID AS Item4Id,
                Item4_Exp AS Item4Exp,
                Item5_ID AS Item5Id,
                Item5_Exp AS Item5Exp,
                Item6_ID AS Item6Id,
                Item6_Exp AS Item6Exp
            FROM items_packages
            WHERE ItemID = @PackageId
            LIMIT 1;
            """,
            new { PackageId = packageId },
            tx);

        if (package == null)
            return BuyExecResult.Fail("bad package");

        if (package.IsEnabled == 0)
            return BuyExecResult.Fail("package disabled");

        await AddPackageItemAsync(db, tx, customerId, package.Item1Id, package.Item1Exp);
        await AddPackageItemAsync(db, tx, customerId, package.Item2Id, package.Item2Exp);
        await AddPackageItemAsync(db, tx, customerId, package.Item3Id, package.Item3Exp);
        await AddPackageItemAsync(db, tx, customerId, package.Item4Id, package.Item4Exp);
        await AddPackageItemAsync(db, tx, customerId, package.Item5Id, package.Item5Exp);
        await AddPackageItemAsync(db, tx, customerId, package.Item6Id, package.Item6Exp);

        await db.ExecuteAsync(
            """
            UPDATE loginid
            SET
                GameDollars = GameDollars + @AddGP,
                SkillPoints = SkillPoints + @AddSP
            WHERE CustomerID = @CustomerId;
            """,
            new
            {
                CustomerId = customerId,
                package.AddGP,
                package.AddSP
            },
            tx);

        return BuyExecResult.Success();
    }

    private static async Task AddPackageItemAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        ulong customerId,
        int itemId,
        int expDays)
    {
        if (itemId <= 0)
            return;

        if (expDays <= 0)
            expDays = 2000;

        await AddItemToUserAsync(db, tx, customerId, itemId, expDays);
    }

    private static async Task RemoveOneItemFromUserAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        ulong customerId,
        int itemId)
    {
        var quantity = await db.ExecuteScalarAsync<int?>(
            """
            SELECT Quantity
            FROM inventory
            WHERE CustomerID = @CustomerId
              AND ItemID = @ItemId
            FOR UPDATE;
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId
            },
            tx);

        if (quantity == null)
            return;

        if (quantity.Value > 1)
        {
            await db.ExecuteAsync(
                """
                UPDATE inventory
                SET Quantity = Quantity - 1
                WHERE CustomerID = @CustomerId
                  AND ItemID = @ItemId;
                """,
                new
                {
                    CustomerId = customerId,
                    ItemId = itemId
                },
                tx);

            return;
        }

        await db.ExecuteAsync(
            """
            DELETE FROM inventory
            WHERE CustomerID = @CustomerId
              AND ItemID = @ItemId;
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId
            },
            tx);
    }
}