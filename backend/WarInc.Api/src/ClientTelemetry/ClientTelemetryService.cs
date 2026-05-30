using System.Text;
using Dapper;
using MySqlConnector;
using WarInc.Api.Common;
using WarInc.Api.Database;

namespace WarInc.Api.ClientTelemetry;

public sealed class ClientTelemetryService
{
    private readonly WarIncDb _db;

    public ClientTelemetryService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<bool> ReportHardwareAsync(
        Dictionary<string, string> data,
        string remoteIp)
    {
        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "dbg_hwinfo"))
            return true;

        var computerId = ReadLongAny(data, 0, "ComputerID", "computerId", "ComputerId", "HWID", "hwid", "MachineID", "machineId");
        var customerId = ReadIntAny(data, 0, "CustomerID", "CustomerId", "customerId", "s_id");

        if (computerId <= 0)
            computerId = Math.Abs(remoteIp.GetHashCode());

        await db.ExecuteAsync(
            """
            INSERT INTO dbg_hwinfo
            (
                ComputerID,
                CustomerID,
                ReportDate,
                CPUName,
                CPUBrand,
                CPUFreq,
                TotalMemory,
                DisplayW,
                DisplayH,
                gfxErrors,
                gfxVendorId,
                gfxDeviceId,
                gfxDescription,
                OSVersion
            )
            VALUES
            (
                @ComputerID,
                @CustomerID,
                UTC_TIMESTAMP(),
                @CPUName,
                @CPUBrand,
                @CPUFreq,
                @TotalMemory,
                @DisplayW,
                @DisplayH,
                @GfxErrors,
                @GfxVendorId,
                @GfxDeviceId,
                @GfxDescription,
                @OSVersion
            );
            """,
            new
            {
                ComputerID = computerId,
                CustomerID = customerId,
                CPUName = Trim(ReadAny(data, "CPUName", "cpuName", "cpu", "CPU"), 32),
                CPUBrand = Trim(ReadAny(data, "CPUBrand", "cpuBrand", "CPUBrandString", "brand"), 64),
                CPUFreq = ReadIntAny(data, 0, "CPUFreq", "cpuFreq", "freq", "MHz"),
                TotalMemory = ReadIntAny(data, 0, "TotalMemory", "totalMemory", "memory", "RAM"),
                DisplayW = ReadIntAny(data, 0, "DisplayW", "displayW", "ScreenW", "screenW", "width"),
                DisplayH = ReadIntAny(data, 0, "DisplayH", "displayH", "ScreenH", "screenH", "height"),
                GfxErrors = ReadIntAny(data, 0, "gfxErrors", "GfxErrors", "GraphicsErrors"),
                GfxVendorId = ReadIntAny(data, 0, "gfxVendorId", "GfxVendorId", "VendorID", "vendorId"),
                GfxDeviceId = ReadIntAny(data, 0, "gfxDeviceId", "GfxDeviceId", "DeviceID", "deviceId"),
                GfxDescription = Trim(ReadAny(data, "gfxDescription", "GfxDescription", "GPU", "gpu", "adapter"), 128),
                OSVersion = Trim(ReadAny(data, "OSVersion", "osVersion", "OS", "os"), 32)
            });

        return true;
    }

    public async Task<bool> LogClientStatusAsync(
        Dictionary<string, string> data,
        string remoteIp)
    {
        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "dbg_srvloginfo"))
            return true;

        var customerId = ReadIntAny(data, 0, "CustomerID", "CustomerId", "customerId", "s_id");
        var gameSessionId = ReadLongAny(data, 0, "GameSessionID", "GameSessionId", "gameSessionId", "g_id");
        var status = ReadAny(data, "status", "Status", "rs", "RS", "state", "State");

        if (string.IsNullOrWhiteSpace(status))
            status = "client_status";

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
                @CustomerID,
                @CustomerIP,
                @GameSessionID,
                0,
                1,
                @Msg,
                @Data
            );
            """,
            new
            {
                CustomerID = customerId,
                CustomerIP = Trim(remoteIp, 64),
                GameSessionID = gameSessionId,
                Msg = Trim(status, 512),
                Data = Trim(BuildDataString(data), 4096)
            });

        return true;
    }

    public async Task<bool> UpdateAchievementsAsync(
        Dictionary<string, string> data)
    {
        var customerId = ReadIntAny(data, 0, "CustomerID", "CustomerId", "customerId", "s_id");

        if (customerId <= 0)
            return true;

        var updates = ReadAchievementUpdates(data);

        if (updates.Count == 0)
            return true;

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "achievements"))
            return true;

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            foreach (var update in updates)
            {
                if (update.AchId <= 0)
                    continue;

                var rows = await db.ExecuteAsync(
                    """
                    UPDATE achievements
                    SET
                        Value = @Value,
                        Unlocked = @Unlocked
                    WHERE CustomerID = @CustomerID
                      AND AchID = @AchID;
                    """,
                    new
                    {
                        CustomerID = customerId,
                        AchID = update.AchId,
                        Value = update.Value,
                        Unlocked = update.Unlocked
                    },
                    tx);

                if (rows > 0)
                    continue;

                await db.ExecuteAsync(
                    """
                    INSERT INTO achievements
                    (
                        CustomerID,
                        AchID,
                        Value,
                        Unlocked
                    )
                    VALUES
                    (
                        @CustomerID,
                        @AchID,
                        @Value,
                        @Unlocked
                    );
                    """,
                    new
                    {
                        CustomerID = customerId,
                        AchID = update.AchId,
                        Value = update.Value,
                        Unlocked = update.Unlocked
                    },
                    tx);
            }

            await tx.CommitAsync();
            return true;
        }
        catch
        {
            await tx.RollbackAsync();
            return false;
        }
    }

    private static List<AchievementUpdate> ReadAchievementUpdates(Dictionary<string, string> data)
{
    var result = new List<AchievementUpdate>();

    var singleAchId = ReadIntAny(
        data,
        0,
        "AchID",
        "AchId",
        "achId",
        "AchievementID",
        "achievementId",
        "id");

    var singleValue = ReadIntAny(
        data,
        0,
        "AchVal",
        "AchValue",
        "AchVAL",
        "Value",
        "value",
        "Val",
        "val",
        "v");

    var singleUnlocked = ReadIntAny(
        data,
        0,
        "AchUnl",
        "AchUnlocked",
        "AchUNL",
        "Unlocked",
        "unlocked",
        "u");

    if (singleAchId > 0)
    {
        result.Add(new AchievementUpdate(
            singleAchId,
            singleValue,
            singleUnlocked));
    }

    var numAch = ReadIntAny(data, 256, "NumAch", "numAch", "NumAchievements", "numAchievements");

    if (numAch <= 0)
        numAch = 256;

    if (numAch > 256)
        numAch = 256;

    for (var i = 0; i < numAch; i++)
    {
        var achId = ReadIntAny(
            data,
            0,
            "AchID" + i,
            "AchId" + i,
            "achId" + i,
            "AchievementID" + i,
            "AID" + i,
            "aid" + i,
            "A" + i,
            "a" + i);

        if (achId <= 0)
            continue;

        var value = ReadIntAny(
            data,
            0,
            "AchVal" + i,
            "AchValue" + i,
            "AchVAL" + i,
            "Value" + i,
            "value" + i,
            "Val" + i,
            "val" + i,
            "V" + i,
            "v" + i);

        var unlocked = ReadIntAny(
            data,
            0,
            "AchUnl" + i,
            "AchUnlocked" + i,
            "AchUNL" + i,
            "Unlocked" + i,
            "unlocked" + i,
            "U" + i,
            "u" + i);

        result.Add(new AchievementUpdate(
            achId,
            value,
            unlocked));
    }

    return result
        .GroupBy(x => x.AchId)
        .Select(x => x.Last())
        .ToList();
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

    private static string BuildDataString(Dictionary<string, string> data)
    {
        var sb = new StringBuilder();
        var first = true;

        foreach (var item in data.OrderBy(x => x.Key, StringComparer.OrdinalIgnoreCase))
        {
            if (!first)
                sb.Append("; ");

            first = false;
            sb.Append(item.Key);
            sb.Append('=');
            sb.Append(item.Value);
        }

        return sb.ToString();
    }

    private static string ReadAny(
        Dictionary<string, string> data,
        params string[] keys)
    {
        foreach (var key in keys)
        {
            if (data.TryGetValue(key, out var value))
                return value;
        }

        return "";
    }

    private static int ReadIntAny(
        Dictionary<string, string> data,
        int defaultValue,
        params string[] keys)
    {
        foreach (var key in keys)
        {
            if (!data.TryGetValue(key, out var value))
                continue;

            var parsed = LegacyUtil.ParseInt(value);

            return parsed;
        }

        return defaultValue;
    }

    private static long ReadLongAny(
        Dictionary<string, string> data,
        long defaultValue,
        params string[] keys)
    {
        foreach (var key in keys)
        {
            if (!data.TryGetValue(key, out var value))
                continue;

            if (long.TryParse(value, out var parsed))
                return parsed;
        }

        return defaultValue;
    }

    private static string Trim(string value, int max)
    {
        value ??= "";

        value = value.Trim();

        if (value.Length <= max)
            return value;

        return value[..max];
    }

    private sealed record AchievementUpdate(
        int AchId,
        int Value,
        int Unlocked
    );
}