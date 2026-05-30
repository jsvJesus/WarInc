using System.Globalization;
using System.Text;
using Dapper;
using MySqlConnector;
using WarInc.Api.Common;
using WarInc.Api.Database;

namespace WarInc.Api.MysteryBox;

public sealed class MysteryBoxService
{
    private readonly WarIncDb _db;

    public MysteryBoxService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<MysteryBoxInfoResponse> GetInfoAsync(
        ulong customerId,
        int lootId)
    {
        if (lootId <= 0)
        {
            return new MysteryBoxInfoResponse(
                true,
                0,
                "OK",
                lootId,
                0,
                Array.Empty<MysteryBoxItemDto>());
        }

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "items_lootdata"))
        {
            return new MysteryBoxInfoResponse(
                true,
                0,
                "OK",
                lootId,
                0,
                Array.Empty<MysteryBoxItemDto>());
        }

        var category = await GetItemCategoryAsync(db, null, lootId);
        var loot = await LoadLootAsync(db, null, lootId);

        var items = loot
            .Select(x => new MysteryBoxItemDto(
                x.ItemID,
                x.Chance,
                x.ItemID == 0 ? x.GDMin : x.ExpDaysMin,
                x.ItemID == 0 ? x.GDMax : x.ExpDaysMax,
                x.GDIfHave))
            .ToArray();

        return new MysteryBoxInfoResponse(
            true,
            0,
            "OK",
            lootId,
            category,
            items);
    }

    public async Task<MysteryBoxRollResponse> RollAsync(
        ulong customerId,
        int boxId,
        int buyIdx)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return RollFail(6, "BAD_CUSTOMER_ID", boxId);

        if (boxId <= 0)
            return RollFail(6, "BAD_BOX_ID", boxId);

        if (!TryGetBuyInfo(buyIdx, out var buyDays, out var isGameDollars))
            return RollFail(6, "BAD_BUY_IDX", boxId);

        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            if (!await TableExistsAsync(db, "items_lootdata"))
            {
                await tx.RollbackAsync();
                return RollFail(6, "MISSING_LOOT_TABLE", boxId);
            }

            var category = await GetItemCategoryAsync(db, tx, boxId);

            if (category != 3 && isGameDollars)
            {
                await tx.RollbackAsync();
                return RollFail(6, "CANT_BUY_LOOT_FOR_GD", boxId);
            }

            var loot = await LoadLootAsync(db, tx, boxId);

            if (loot.Count == 0)
            {
                await tx.RollbackAsync();
                return RollFail(6, "EMPTY_LOOT_BOX", boxId);
            }

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
                new { CustomerId = (int)customerId },
                tx);

            if (account == null)
            {
                await tx.RollbackAsync();
                return RollFail(6, "ACCOUNT_NOT_FOUND", boxId);
            }

            var price = await GetBuyItemPriceAsync(db, tx, boxId);

            if (price == null || price.IsEnabled == 0)
            {
                await tx.RollbackAsync();
                return RollFail(6, "BAD_BOX_PRICE", boxId);
            }

            var finalPrice = GetFinalPrice(price, buyDays, isGameDollars);

            if (finalPrice <= 0)
            {
                await tx.RollbackAsync();
                return RollFail(6, "BAD_BOX_PRICE", boxId);
            }

            var currentBalance = isGameDollars
                ? account.GameDollars
                : account.GamePoints;

            if (currentBalance < finalPrice)
            {
                await tx.RollbackAsync();

                return new MysteryBoxRollResponse(
                    false,
                    7,
                    isGameDollars ? "NOT_ENOUGH_GD" : "NOT_ENOUGH_GP",
                    boxId,
                    0,
                    0,
                    0,
                    currentBalance,
                    0);
            }

            if (isGameDollars)
            {
                await db.ExecuteAsync(
                    """
                    UPDATE loginid
                    SET GameDollars = GameDollars - @Price
                    WHERE CustomerID = @CustomerId;
                    """,
                    new
                    {
                        CustomerId = (int)customerId,
                        Price = finalPrice
                    },
                    tx);
            }
            else
            {
                await db.ExecuteAsync(
                    """
                    UPDATE loginid
                    SET GamePoints = GamePoints - @Price
                    WHERE CustomerID = @CustomerId;
                    """,
                    new
                    {
                        CustomerId = (int)customerId,
                        Price = finalPrice
                    },
                    tx);
            }

            await InsertFinancialTransactionAsync(
                db,
                tx,
                (int)customerId,
                isGameDollars ? 2001 : 2000,
                finalPrice,
                boxId);

            var win = RollWinning(loot);

            var finalGd = win.GD;
            var expDays = win.ExpDays;
            var wonItemId = win.ItemID;

            if (wonItemId == 0)
            {
                if (finalGd > 0)
                {
                    await db.ExecuteAsync(
                        """
                        UPDATE loginid
                        SET GameDollars = GameDollars + @Amount
                        WHERE CustomerID = @CustomerId;
                        """,
                        new
                        {
                            CustomerId = (int)customerId,
                            Amount = finalGd
                        },
                        tx);
                }
            }
            else
            {
                var alreadyHasActive = await HasActiveInventoryItemAsync(
                    db,
                    tx,
                    (int)customerId,
                    wonItemId);

                if (alreadyHasActive && win.GDIfHave > 0)
                {
                    finalGd = win.GDIfHave;

                    await db.ExecuteAsync(
                        """
                        UPDATE loginid
                        SET GameDollars = GameDollars + @Amount
                        WHERE CustomerID = @CustomerId;
                        """,
                        new
                        {
                            CustomerId = (int)customerId,
                            Amount = finalGd
                        },
                        tx);
                }
                else
                {
                    finalGd = 0;
                    await AddItemToInventoryAsync(
                        db,
                        tx,
                        (int)customerId,
                        wonItemId,
                        expDays);
                }
            }

            await InsertLootRewardDebugAsync(
                db,
                tx,
                (int)customerId,
                win.Roll,
                boxId,
                wonItemId,
                expDays,
                finalGd);

            var balance = await db.ExecuteScalarAsync<int>(
                isGameDollars
                    ? "SELECT GameDollars FROM loginid WHERE CustomerID = @CustomerId;"
                    : "SELECT GamePoints FROM loginid WHERE CustomerID = @CustomerId;",
                new { CustomerId = (int)customerId },
                tx);

            await tx.CommitAsync();

            return new MysteryBoxRollResponse(
                true,
                0,
                "OK",
                boxId,
                wonItemId,
                expDays,
                finalGd,
                balance,
                win.Roll);
        }
        catch
        {
            await tx.RollbackAsync();
            return RollFail(5, "MYSTERY_BOX_ROLL_FAILED", boxId);
        }
    }

    public async Task<MysteryBoxSellResponse> SellAsync(
        ulong customerId,
        int boxId,
        int itemId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return new MysteryBoxSellResponse(false, 6, "BAD_CUSTOMER_ID", itemId, 0);

        var sellItemId = itemId > 0 ? itemId : boxId;

        if (sellItemId <= 0)
            return new MysteryBoxSellResponse(false, 6, "BAD_ITEM_ID", sellItemId, 0);

        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var current = await db.QueryFirstOrDefaultAsync<InventoryQuantityRow>(
                """
                SELECT
                    Quantity AS Quantity
                FROM inventory
                WHERE CustomerID = @CustomerId
                  AND ItemID = @ItemId
                FOR UPDATE;
                """,
                new
                {
                    CustomerId = (int)customerId,
                    ItemId = sellItemId
                },
                tx);

            if (current == null)
            {
                await tx.CommitAsync();
                return new MysteryBoxSellResponse(true, 0, "OK", sellItemId, 0);
            }

            if (current.Quantity > 1)
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
                        CustomerId = (int)customerId,
                        ItemId = sellItemId
                    },
                    tx);
            }
            else
            {
                await db.ExecuteAsync(
                    """
                    DELETE FROM inventory
                    WHERE CustomerID = @CustomerId
                      AND ItemID = @ItemId;
                    """,
                    new
                    {
                        CustomerId = (int)customerId,
                        ItemId = sellItemId
                    },
                    tx);
            }

            await tx.CommitAsync();

            return new MysteryBoxSellResponse(true, 0, "OK", sellItemId, 1);
        }
        catch
        {
            await tx.RollbackAsync();
            return new MysteryBoxSellResponse(false, 5, "SELL_FAILED", sellItemId, 0);
        }
    }

    public string BuildLegacyInfoXml(MysteryBoxInfoResponse info)
    {
        var xml = new StringBuilder();

        xml.Append("<?xml version=\"1.0\"?>\n");
        xml.Append("<box>");

        foreach (var item in info.Items)
        {
            xml.Append("<e ");
            LegacyUtil.AppendXmlAttr(xml, "ID", item.ItemId);
            LegacyUtil.AppendXmlAttr(xml, "v1", item.Value1);
            LegacyUtil.AppendXmlAttr(xml, "v2", item.Value2);
            xml.Append("/>\n");
        }

        xml.Append("</box>");

        return xml.ToString();
    }

    public string BuildLegacyRollText(MysteryBoxRollResponse result)
    {
        if (!result.Ok)
            return "WO_" + LegacyUtil.NormalizeLegacyErrorCode(result.Code);

        return "WO_0" +
               result.WonItemId.ToString(CultureInfo.InvariantCulture) + " " +
               result.ExpDays.ToString(CultureInfo.InvariantCulture) + " " +
               result.GameDollars.ToString(CultureInfo.InvariantCulture) + " " +
               result.Balance.ToString(CultureInfo.InvariantCulture);
    }

    public string BuildLegacySellText(MysteryBoxSellResponse result)
    {
        if (!result.Ok)
            return "WO_" + LegacyUtil.NormalizeLegacyErrorCode(result.Code);

        return "WO_0";
    }

    private static MysteryBoxRollResponse RollFail(
        int code,
        string message,
        int lootId)
    {
        return new MysteryBoxRollResponse(
            false,
            code,
            message,
            lootId,
            0,
            0,
            0,
            0,
            0);
    }

    private static async Task<List<LootEntryRow>> LoadLootAsync(
        MySqlConnection db,
        MySqlTransaction? tx,
        int lootId)
    {
        var rows = (await db.QueryAsync<LootEntryRow>(
            """
            SELECT
                RecordID AS RecordID,
                LootID AS LootID,
                COALESCE(Chance, 0) AS Chance,
                COALESCE(ItemID, 0) AS ItemID,
                COALESCE(ExpDaysMin, 0) AS ExpDaysMin,
                COALESCE(ExpDaysMax, 0) AS ExpDaysMax,
                COALESCE(GDMin, 0) AS GDMin,
                COALESCE(GDMax, 0) AS GDMax,
                COALESCE(GDIfHave, 0) AS GDIfHave
            FROM items_lootdata
            WHERE LootID = @LootId
            ORDER BY RecordID ASC;
            """,
            new { LootId = lootId },
            tx)).ToList();

        var total = rows.Sum(x => Math.Max(0, x.Chance));

        if (total <= 0)
            return rows;

        var cumulative = 0.0;

        foreach (var row in rows)
        {
            cumulative += Math.Max(0, row.Chance) / total;
            row.NormalizedChance = cumulative;
        }

        if (rows.Count > 0)
            rows[^1].NormalizedChance = 1.0;

        return rows;
    }

    private static WinData RollWinning(List<LootEntryRow> loot)
    {
        var roll = Random.Shared.NextDouble();

        foreach (var entry in loot)
        {
            if (roll > entry.NormalizedChance)
                continue;

            if (entry.ItemID <= 0)
            {
                return new WinData
                {
                    Roll = roll,
                    ItemID = 0,
                    ExpDays = 0,
                    GD = RandomRangeInclusive(entry.GDMin, entry.GDMax),
                    GDIfHave = 0
                };
            }

            return new WinData
            {
                Roll = roll,
                ItemID = entry.ItemID,
                ExpDays = RandomRangeInclusive(entry.ExpDaysMin, entry.ExpDaysMax),
                GD = 0,
                GDIfHave = entry.GDIfHave
            };
        }

        var fallback = loot[^1];

        return new WinData
        {
            Roll = roll,
            ItemID = fallback.ItemID,
            ExpDays = RandomRangeInclusive(fallback.ExpDaysMin, fallback.ExpDaysMax),
            GD = fallback.ItemID == 0
                ? RandomRangeInclusive(fallback.GDMin, fallback.GDMax)
                : 0,
            GDIfHave = fallback.GDIfHave
        };
    }

    private static int RandomRangeInclusive(int min, int max)
    {
        if (min > max)
            (min, max) = (max, min);

        if (max <= min)
            return min;

        return Random.Shared.Next(min, max + 1);
    }

    private static async Task<bool> HasActiveInventoryItemAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int customerId,
        int itemId)
    {
        var count = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM inventory
            WHERE CustomerID = @CustomerId
              AND ItemID = @ItemId
              AND Quantity > 0
              AND LeasedUntil > UTC_TIMESTAMP();
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId
            },
            tx);

        return count > 0;
    }

    private static async Task AddItemToInventoryAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int customerId,
        int itemId,
        int expDays)
    {
        if (expDays <= 0)
            expDays = 1;

        var current = await db.QueryFirstOrDefaultAsync<InventoryQuantityRow>(
            """
            SELECT
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

        var isStackable = await db.ExecuteScalarAsync<int?>(
            """
            SELECT IsStackable
            FROM items_generic
            WHERE ItemID = @ItemId
            LIMIT 1;
            """,
            new { ItemId = itemId },
            tx);

        if ((isStackable ?? 0) > 0)
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

        return null;
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

            case 9:
                buyDays = 1;
                return true;
            case 10:
                buyDays = 7;
                return true;
            case 11:
                buyDays = 30;
                return true;
            case 12:
                buyDays = 2000;
                return true;

            default:
                return false;
        }
    }

    private static async Task<int> GetItemCategoryAsync(
        MySqlConnection db,
        MySqlTransaction? tx,
        int itemId)
    {
        var category = await QueryCategoryAsync(db, tx, "items_generic", itemId);

        if (category.HasValue)
            return category.Value;

        category = await QueryCategoryAsync(db, tx, "items_packages", itemId);

        if (category.HasValue)
            return category.Value;

        category = await QueryCategoryAsync(db, tx, "items_weapons", itemId);

        if (category.HasValue)
            return category.Value;

        category = await QueryCategoryAsync(db, tx, "items_gear", itemId);

        if (category.HasValue)
            return category.Value;

        category = await QueryCategoryAsync(db, tx, "items_attachments", itemId);

        return category ?? 0;
    }

    private static async Task<int?> QueryCategoryAsync(
        MySqlConnection db,
        MySqlTransaction? tx,
        string table,
        int itemId)
    {
        if (!await TableExistsAsync(db, table))
            return null;

        var sql = $"SELECT Category FROM {table} WHERE ItemID = @ItemId LIMIT 1;";

        return await db.ExecuteScalarAsync<int?>(
            sql,
            new { ItemId = itemId },
            tx);
    }

    private static async Task InsertFinancialTransactionAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int customerId,
        int transactionType,
        int amount,
        int itemId)
    {
        if (!await TableExistsAsync(db, "financialtransactions"))
            return;

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
                'MYSTERYBOX',
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
                TransactionType = transactionType,
                Amount = amount,
                ItemId = itemId.ToString(CultureInfo.InvariantCulture)
            },
            tx);
    }

    private static async Task InsertLootRewardDebugAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int customerId,
        double roll,
        int lootId,
        int itemId,
        int expDays,
        int gd)
    {
        if (!await TableExistsAsync(db, "dbg_lootrewards"))
            return;

        await db.ExecuteAsync(
            """
            INSERT INTO dbg_lootrewards
            (
                ReportTime,
                CustomerID,
                Roll,
                LootID,
                ItemID,
                ExpDays,
                GD
            )
            VALUES
            (
                UTC_TIMESTAMP(),
                @CustomerId,
                @Roll,
                @LootId,
                @ItemId,
                @ExpDays,
                @GD
            );
            """,
            new
            {
                CustomerId = customerId,
                Roll = roll,
                LootId = lootId,
                ItemId = itemId,
                ExpDays = expDays,
                GD = gd
            },
            tx);
    }

    private static async Task<bool> TableExistsAsync(
        MySqlConnection db,
        string table)
    {
        var count = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM information_schema.tables
            WHERE table_schema = DATABASE()
              AND table_name = @Table;
            """,
            new { Table = table });

        return count > 0;
    }

    private sealed class LootEntryRow
    {
        public int RecordID { get; set; }
        public int LootID { get; set; }
        public double Chance { get; set; }
        public int ItemID { get; set; }
        public int ExpDaysMin { get; set; }
        public int ExpDaysMax { get; set; }
        public int GDMin { get; set; }
        public int GDMax { get; set; }
        public int GDIfHave { get; set; }
        public double NormalizedChance { get; set; }
    }

    private sealed class WinData
    {
        public double Roll { get; set; }
        public int ItemID { get; set; }
        public int ExpDays { get; set; }
        public int GD { get; set; }
        public int GDIfHave { get; set; }
    }

    private sealed class AccountBalanceRow
    {
        public int CustomerId { get; set; }
        public int GamePoints { get; set; }
        public int GameDollars { get; set; }
    }

    private sealed class BuyItemPriceRow
    {
        public int ItemId { get; set; }
        public int Category { get; set; }
        public int IsEnabled { get; set; }
        public int Price1 { get; set; }
        public int Price7 { get; set; }
        public int Price30 { get; set; }
        public int PricePerm { get; set; }
        public int GPrice1 { get; set; }
        public int GPrice7 { get; set; }
        public int GPrice30 { get; set; }
        public int GPricePerm { get; set; }
    }

    private sealed class InventoryQuantityRow
    {
        public int Quantity { get; set; }
    }
}