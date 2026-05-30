using Dapper;
using MySqlConnector;
using WarInc.Api.Database;

namespace WarInc.Api.GameServer;

public sealed class GameServerInventoryService
{
    private readonly WarIncDb _db;

    public GameServerInventoryService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<GameServerInventoryActionResponse> GiveItemAsync(
        ulong customerId,
        int itemId,
        int quantity,
        int expDays)
    {
        var expMinutes = expDays > 0
            ? expDays * 24 * 60
            : 2000 * 24 * 60;

        return await GiveItemInMinutesAsync(
            customerId,
            itemId,
            quantity,
            expMinutes);
    }

    public async Task<GameServerInventoryActionResponse> GiveItemInMinutesAsync(
        ulong customerId,
        int itemId,
        int quantity,
        int expMinutes)
    {
        if (customerId == 0 || customerId > int.MaxValue)
        {
            return new GameServerInventoryActionResponse(
                false,
                6,
                "BAD_CUSTOMER_ID",
                customerId,
                itemId,
                quantity,
                expMinutes);
        }

        if (itemId <= 0)
        {
            return new GameServerInventoryActionResponse(
                false,
                6,
                "BAD_ITEM_ID",
                customerId,
                itemId,
                quantity,
                expMinutes);
        }

        if (quantity <= 0)
            quantity = 1;

        if (quantity > 999999)
            quantity = 999999;

        if (expMinutes <= 0)
            expMinutes = 2000 * 24 * 60;

        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        if (!await TableExistsAsync(db, "inventory"))
        {
            return new GameServerInventoryActionResponse(
                false,
                5,
                "MISSING_INVENTORY_TABLE",
                customerId,
                itemId,
                quantity,
                expMinutes);
        }

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var accountExists = await db.ExecuteScalarAsync<int>(
                """
                SELECT COUNT(*)
                FROM loginid
                WHERE CustomerID = @CustomerId;
                """,
                new { CustomerId = (int)customerId },
                tx);

            if (accountExists <= 0)
            {
                await tx.RollbackAsync();

                return new GameServerInventoryActionResponse(
                    false,
                    6,
                    "ACCOUNT_NOT_FOUND",
                    customerId,
                    itemId,
                    quantity,
                    expMinutes);
            }

            await AddItemToUserAsync(
                db,
                tx,
                (int)customerId,
                itemId,
                quantity,
                expMinutes);

            await LogServerActionAsync(
                db,
                tx,
                (int)customerId,
                0,
                "srv_give_item",
                $"ItemID={itemId}; Quantity={quantity}; ExpMinutes={expMinutes}");

            await tx.CommitAsync();

            return new GameServerInventoryActionResponse(
                true,
                0,
                "OK",
                customerId,
                itemId,
                quantity,
                expMinutes);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();

            return new GameServerInventoryActionResponse(
                false,
                5,
                e.Message,
                customerId,
                itemId,
                quantity,
                expMinutes);
        }
    }

    public async Task<GameServerInventoryActionResponse> RemoveItemAsync(
        ulong customerId,
        int itemId,
        int quantity,
        bool removeAll)
    {
        if (customerId == 0 || customerId > int.MaxValue)
        {
            return new GameServerInventoryActionResponse(
                false,
                6,
                "BAD_CUSTOMER_ID",
                customerId,
                itemId,
                quantity,
                0);
        }

        if (itemId <= 0)
        {
            return new GameServerInventoryActionResponse(
                false,
                6,
                "BAD_ITEM_ID",
                customerId,
                itemId,
                quantity,
                0);
        }

        if (quantity <= 0)
            quantity = 1;

        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        if (!await TableExistsAsync(db, "inventory"))
        {
            return new GameServerInventoryActionResponse(
                false,
                5,
                "MISSING_INVENTORY_TABLE",
                customerId,
                itemId,
                quantity,
                0);
        }

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            await RemoveItemFromUserAsync(
                db,
                tx,
                (int)customerId,
                itemId,
                quantity,
                removeAll);

            await LogServerActionAsync(
                db,
                tx,
                (int)customerId,
                0,
                "srv_remove_item",
                $"ItemID={itemId}; Quantity={quantity}; RemoveAll={removeAll}");

            await tx.CommitAsync();

            return new GameServerInventoryActionResponse(
                true,
                0,
                "OK",
                customerId,
                itemId,
                quantity,
                0);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();

            return new GameServerInventoryActionResponse(
                false,
                5,
                e.Message,
                customerId,
                itemId,
                quantity,
                0);
        }
    }

    private static async Task AddItemToUserAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int customerId,
        int itemId,
        int quantity,
        int expMinutes)
    {
        if (itemId >= 100000 && itemId <= 199999)
        {
            await AddDefaultAttachmentsToUserAsync(
                db,
                tx,
                customerId,
                itemId,
                expMinutes);
        }

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
                    DATE_ADD(UTC_TIMESTAMP(), INTERVAL @ExpMinutes MINUTE),
                    @Quantity,
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
                    ExpMinutes = expMinutes,
                    Quantity = quantity
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
                    INTERVAL @ExpMinutes MINUTE
                )
            WHERE CustomerID = @CustomerId
              AND ItemID = @ItemId;
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId,
                ExpMinutes = expMinutes
            },
            tx);

        var isStackable = await IsStackableAsync(db, tx, itemId);

        if (isStackable)
        {
            await db.ExecuteAsync(
                """
                UPDATE inventory
                SET Quantity = Quantity + @Quantity
                WHERE CustomerID = @CustomerId
                  AND ItemID = @ItemId;
                """,
                new
                {
                    CustomerId = customerId,
                    ItemId = itemId,
                    Quantity = quantity
                },
                tx);
        }
    }

    private static async Task RemoveItemFromUserAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int customerId,
        int itemId,
        int quantity,
        bool removeAll)
    {
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
            await RemoveFpsLinksAsync(
                db,
                tx,
                customerId,
                itemId);

            return;
        }

        if (removeAll || current.Quantity <= quantity)
        {
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

            await RemoveFpsLinksAsync(
                db,
                tx,
                customerId,
                itemId);

            return;
        }

        await db.ExecuteAsync(
            """
            UPDATE inventory
            SET Quantity = Quantity - @Quantity
            WHERE CustomerID = @CustomerId
              AND ItemID = @ItemId;
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId,
                Quantity = quantity
            },
            tx);
    }

    private static async Task RemoveFpsLinksAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int customerId,
        int itemId)
    {
        if (!await TableExistsAsync(db, "inventory_fps"))
            return;

        if (itemId >= 100000 && itemId <= 199999)
        {
            await db.ExecuteAsync(
                """
                DELETE FROM inventory_fps
                WHERE CustomerID = @CustomerId
                  AND WeaponID = @WeaponID;
                """,
                new
                {
                    CustomerId = customerId,
                    WeaponID = itemId
                },
                tx);
        }

        if (itemId >= 400000 && itemId <= 499999)
        {
            await db.ExecuteAsync(
                """
                DELETE FROM inventory_fps
                WHERE CustomerID = @CustomerId
                  AND AttachmentID = @AttachmentID;
                """,
                new
                {
                    CustomerId = customerId,
                    AttachmentID = itemId
                },
                tx);
        }
    }

    private static async Task AddDefaultAttachmentsToUserAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int customerId,
        int weaponId,
        int expMinutes)
    {
        if (!await TableExistsAsync(db, "items_weapons"))
            return;

        if (!await TableExistsAsync(db, "inventory_fps"))
            return;

        var row = await db.QueryFirstOrDefaultAsync<DefaultWeaponAttachmentsRow>(
            """
            SELECT
                IFNULL(IsFPS, 0) AS IsFps,
                IFNULL(FPSAttach0, 0) AS FpsAttach0,
                IFNULL(FPSAttach1, 0) AS FpsAttach1,
                IFNULL(FPSAttach2, 0) AS FpsAttach2,
                IFNULL(FPSAttach3, 0) AS FpsAttach3,
                IFNULL(FPSAttach4, 0) AS FpsAttach4,
                IFNULL(FPSAttach5, 0) AS FpsAttach5,
                IFNULL(FPSAttach6, 0) AS FpsAttach6,
                IFNULL(FPSAttach7, 0) AS FpsAttach7,
                IFNULL(FPSAttach8, 0) AS FpsAttach8
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
                expMinutes);
        }
    }

    private static async Task AddAttachmentToUserAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int customerId,
        int weaponId,
        int attachmentId,
        int expMinutes)
    {
        if (!await TableExistsAsync(db, "items_attachments"))
            return;

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
                    DATE_ADD(UTC_TIMESTAMP(), INTERVAL @ExpMinutes MINUTE),
                    @Slot,
                    1
                );
                """,
                new
                {
                    CustomerId = customerId,
                    WeaponId = weaponId,
                    AttachmentId = attachmentId,
                    ExpMinutes = expMinutes,
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
                    INTERVAL @ExpMinutes MINUTE
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
                ExpMinutes = expMinutes
            },
            tx);
    }

    private static async Task<bool> IsStackableAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int itemId)
    {
        if (!await TableExistsAsync(db, "items_generic"))
            return false;

        var value = await db.ExecuteScalarAsync<int?>(
            """
            SELECT IFNULL(IsStackable, 0)
            FROM items_generic
            WHERE ItemID = @ItemId
            LIMIT 1;
            """,
            new { ItemId = itemId },
            tx);

        return (value ?? 0) > 0;
    }

    private static async Task LogServerActionAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        int customerId,
        long gameSessionId,
        string action,
        string data)
    {
        if (!await TableExistsAsync(db, "dbg_srvloginfo"))
            return;

        await db.ExecuteAsync(
            """
            INSERT INTO dbg_srvloginfo
            (
                ReportTime,
                IsProcessed,
                CustomerID,
                CustomerIP,
                GameSessionID,
                CheatID,
                RepeatCount,
                Msg,
                Data
            )
            VALUES
            (
                UTC_TIMESTAMP(),
                0,
                @CustomerId,
                '',
                @GameSessionId,
                0,
                1,
                @Msg,
                @Data
            );
            """,
            new
            {
                CustomerId = customerId,
                GameSessionId = gameSessionId,
                Msg = action,
                Data = data
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

    private sealed class InventoryLeaseRow
    {
        public DateTime LeasedUntil { get; set; }
        public int Quantity { get; set; }
    }

    private sealed class FpsAttachmentLeaseRow
    {
        public DateTime LeasedUntil { get; set; }
    }

    private sealed class DefaultWeaponAttachmentsRow
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
}