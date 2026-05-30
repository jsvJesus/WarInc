using System.Text;
using Dapper;
using MySqlConnector;
using WarInc.Api.Common;
using WarInc.Api.Database;

namespace WarInc.Api.Retention;

public sealed class RetentionService
{
    private const int MaxRetentionDays = 15;

    private readonly WarIncDb _db;

    public RetentionService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<RetentionInfoResponse> GetInfoAsync(ulong customerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return EmptyInfo();

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "loginid"))
            return EmptyInfo();

        if (!await TableExistsAsync(db, "dataretentionbonuses"))
            return EmptyInfo();

        var account = await db.QueryFirstOrDefaultAsync<RetentionAccountRow>(
            """
            SELECT
                CustomerID AS CustomerId,
                GamePoints AS GamePoints,
                lastRetBonusDate AS LastRetBonusDate
            FROM loginid
            WHERE CustomerID = @CustomerId
            LIMIT 1;
            """,
            new { CustomerId = (int)customerId });

        if (account == null)
            return EmptyInfo();

        if (account.LastRetBonusDate == null)
        {
            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET lastRetBonusDate = UTC_TIMESTAMP()
                WHERE CustomerID = @CustomerId;
                """,
                new { CustomerId = (int)customerId });

            var initBonuses = await GetBonusDaysAsync(db, 0);

            return new RetentionInfoResponse(
                true,
                0,
                "OK",
                0,
                1440,
                0,
                initBonuses);
        }

        var now = DateTime.UtcNow;
        var retDays = await CheckRetentionDaysAsync(
            db,
            (int)customerId,
            account.LastRetBonusDate.Value,
            now);

        var minutesToNextDay = GetMinutesToNextDay(
            account.LastRetBonusDate.Value,
            now);

        var bonuses = await GetBonusDaysAsync(db, retDays);

        return new RetentionInfoResponse(
            true,
            0,
            "OK",
            retDays,
            minutesToNextDay,
            retDays > 0 ? 1 : 0,
            bonuses);
    }

    public async Task<RetentionClaimResponse> ClaimAsync(ulong customerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
        {
            return new RetentionClaimResponse(
                false,
                400,
                "BAD_CUSTOMER_ID",
                0,
                0,
                0,
                Array.Empty<RetentionDayDto>());
        }

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "loginid"))
        {
            return new RetentionClaimResponse(
                false,
                500,
                "MISSING_LOGINID_TABLE",
                0,
                0,
                0,
                Array.Empty<RetentionDayDto>());
        }

        if (!await TableExistsAsync(db, "dataretentionbonuses"))
        {
            return new RetentionClaimResponse(
                false,
                500,
                "MISSING_RETENTION_TABLE",
                0,
                0,
                0,
                Array.Empty<RetentionDayDto>());
        }

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var account = await db.QueryFirstOrDefaultAsync<RetentionAccountRow>(
                """
                SELECT
                    CustomerID AS CustomerId,
                    GamePoints AS GamePoints,
                    lastRetBonusDate AS LastRetBonusDate
                FROM loginid
                WHERE CustomerID = @CustomerId
                LIMIT 1;
                """,
                new { CustomerId = (int)customerId },
                tx);

            if (account == null)
            {
                await tx.RollbackAsync();

                return new RetentionClaimResponse(
                    false,
                    404,
                    "ACCOUNT_NOT_FOUND",
                    0,
                    0,
                    0,
                    Array.Empty<RetentionDayDto>());
            }

            if (account.LastRetBonusDate == null)
            {
                await db.ExecuteAsync(
                    """
                    UPDATE loginid
                    SET lastRetBonusDate = UTC_TIMESTAMP()
                    WHERE CustomerID = @CustomerId;
                    """,
                    new { CustomerId = (int)customerId },
                    tx);

                await tx.CommitAsync();

                return new RetentionClaimResponse(
                    false,
                    6,
                    "0_RETENTION_DAYS",
                    0,
                    0,
                    account.GamePoints,
                    Array.Empty<RetentionDayDto>());
            }

            var now = DateTime.UtcNow;
            var retDays = await CheckRetentionDaysAsync(
                db,
                tx,
                (int)customerId,
                account.LastRetBonusDate.Value,
                now);

            if (retDays <= 0)
            {
                await tx.RollbackAsync();

                return new RetentionClaimResponse(
                    false,
                    6,
                    "0_RETENTION_DAYS",
                    0,
                    0,
                    account.GamePoints,
                    Array.Empty<RetentionDayDto>());
            }

            var bonus = await GetBonusForDayAsync(db, tx, retDays);

            if (bonus <= 0)
            {
                await tx.RollbackAsync();

                return new RetentionClaimResponse(
                    false,
                    6,
                    "NO_RETENTION_BONUS",
                    retDays,
                    0,
                    account.GamePoints,
                    Array.Empty<RetentionDayDto>());
            }

            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET
                    GamePoints = GamePoints + @Bonus,
                    lastRetBonusDate = UTC_TIMESTAMP()
                WHERE CustomerID = @CustomerId;
                """,
                new
                {
                    CustomerId = (int)customerId,
                    Bonus = bonus
                },
                tx);

            if (await TableExistsAsync(db, "financialtransactions"))
            {
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
                        'RetBonus',
                        2004,
                        UTC_TIMESTAMP(),
                        @Bonus,
                        '1',
                        'APPROVED',
                        @Day
                    );
                    """,
                    new
                    {
                        CustomerId = (int)customerId,
                        Bonus = bonus,
                        Day = retDays.ToString()
                    },
                    tx);
            }

            var balance = await db.ExecuteScalarAsync<int>(
                """
                SELECT GamePoints
                FROM loginid
                WHERE CustomerID = @CustomerId;
                """,
                new { CustomerId = (int)customerId },
                tx);

            await tx.CommitAsync();

            var reward = new RetentionDayDto(
                retDays,
                1,
                bonus,
                bonus,
                "GamePoints");

            return new RetentionClaimResponse(
                true,
                0,
                "OK",
                retDays,
                bonus,
                balance,
                new[] { reward });
        }
        catch
        {
            await tx.RollbackAsync();

            return new RetentionClaimResponse(
                false,
                500,
                "RETENTION_CLAIM_FAILED",
                0,
                0,
                0,
                Array.Empty<RetentionDayDto>());
        }
    }

    public string BuildLegacyInfoXml(RetentionInfoResponse info)
    {
        var xml = new StringBuilder();

        xml.Append("<?xml version=\"1.0\"?>\n");

        xml.Append("<retbonus ");
        LegacyUtil.AppendXmlAttr(xml, "d", info.CurrentDay);
        LegacyUtil.AppendXmlAttr(xml, "m", info.MinutesToNextDay);
        xml.Append(">");

        xml.Append("<days>");

        foreach (var day in info.Days)
        {
            xml.Append("<d ");
            LegacyUtil.AppendXmlAttr(xml, "b", day.RewardAmount);
            xml.Append("/>");
        }

        xml.Append("</days>");
        xml.Append("</retbonus>");

        return xml.ToString();
    }

    public string BuildLegacyClaimText(RetentionClaimResponse result)
    {
        if (!result.Ok)
            return "WO_" + LegacyUtil.NormalizeLegacyErrorCode(result.Code);

        return "WO_0" + result.Balance.ToString(System.Globalization.CultureInfo.InvariantCulture);
    }

    public string BuildLegacyEmptyInfoXml()
    {
        return "<?xml version=\"1.0\"?>\n<retbonus d=\"0\" m=\"0\"><days></days></retbonus>";
    }

    private async Task<RetentionDayDto[]> GetBonusDaysAsync(
        MySqlConnection db,
        int currentDay)
    {
        var rows = await db.QueryAsync<RetentionBonusRow>(
            """
            SELECT
                Day AS Day,
                Bonus AS Bonus
            FROM dataretentionbonuses
            ORDER BY Day ASC;
            """);

        return rows
            .Select(x => new RetentionDayDto(
                x.Day,
                x.Day <= currentDay ? 1 : 0,
                x.Bonus,
                x.Bonus,
                "GamePoints"))
            .ToArray();
    }

    private static async Task<int> GetBonusForDayAsync(
        MySqlConnection db,
        System.Data.Common.DbTransaction tx,
        int day)
    {
        var bonus = await db.ExecuteScalarAsync<int?>(
            """
            SELECT Bonus
            FROM dataretentionbonuses
            WHERE Day = @Day
            LIMIT 1;
            """,
            new { Day = day },
            tx);

        if (bonus.HasValue)
            return bonus.Value;

        var lastBonus = await db.ExecuteScalarAsync<int?>(
            """
            SELECT Bonus
            FROM dataretentionbonuses
            ORDER BY Day DESC
            LIMIT 1;
            """,
            transaction: tx);

        return lastBonus ?? 0;
    }

    private static async Task<int> CheckRetentionDaysAsync(
        MySqlConnection db,
        int customerId,
        DateTime lastRetBonusDate,
        DateTime now)
    {
        return await CheckRetentionDaysAsync(
            db,
            null,
            customerId,
            lastRetBonusDate,
            now);
    }

    private static async Task<int> CheckRetentionDaysAsync(
        MySqlConnection db,
        System.Data.Common.DbTransaction? tx,
        int customerId,
        DateTime lastRetBonusDate,
        DateTime now)
    {
        var dt1 = now.AddDays(-1);
        var dt2 = now;
        var days = 0;

        while (lastRetBonusDate < dt1)
        {
            var logins = await db.ExecuteScalarAsync<int>(
                """
                SELECT COUNT(*)
                FROM logins
                WHERE CustomerID = @CustomerId
                  AND LoginTime > @From
                  AND LoginTime < @To;
                """,
                new
                {
                    CustomerId = customerId,
                    From = dt1,
                    To = dt2
                },
                tx);

            if (logins <= 0)
                break;

            days++;

            if (days > MaxRetentionDays)
                break;

            dt1 = dt1.AddDays(-1);
            dt2 = dt2.AddDays(-1);
        }

        return days;
    }

    private static int GetMinutesToNextDay(
        DateTime lastRetBonusDate,
        DateTime now)
    {
        var nextDay = lastRetBonusDate.AddDays(1);
        var minutes = (int)Math.Floor((nextDay - now).TotalMinutes);
        minutes %= 24 * 60;

        while (minutes < 0)
            minutes += 24 * 60;

        return minutes;
    }

    private static RetentionInfoResponse EmptyInfo()
    {
        return new RetentionInfoResponse(
            true,
            0,
            "OK",
            0,
            0,
            0,
            Array.Empty<RetentionDayDto>());
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

    private sealed class RetentionAccountRow
    {
        public int CustomerId { get; set; }
        public int GamePoints { get; set; }
        public DateTime? LastRetBonusDate { get; set; }
    }

    private sealed class RetentionBonusRow
    {
        public int Day { get; set; }
        public int Bonus { get; set; }
    }
}