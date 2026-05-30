using System.Text;
using System.Text.Json;
using Microsoft.Extensions.Options;
using WarInc.Api.Common;
using WarInc.Api.Config;
using WarInc.Api.Security;

namespace WarInc.Api.GameServer;

public static class GameServerInventoryEndpoints
{
    public static void MapGameServerInventoryEndpoints(this WebApplication app)
    {
        MapLegacy(app, "/api_SrvGiveItem.aspx", LegacyGiveItemAsync);
        MapLegacy(app, "/api_SrvGiveItemInMinutes.aspx", LegacyGiveItemInMinutesAsync);
        MapLegacy(app, "/api_SrvRemoveItem.aspx", LegacyRemoveItemAsync);

        app.MapGet("/internal/gameserver/give-item", JsonGiveItemAsync).RequireRateLimiting("gameserver");
        app.MapPost("/internal/gameserver/give-item", JsonGiveItemAsync).RequireRateLimiting("gameserver");

        app.MapGet("/internal/gameserver/remove-item", JsonRemoveItemAsync).RequireRateLimiting("gameserver");
        app.MapPost("/internal/gameserver/remove-item", JsonRemoveItemAsync).RequireRateLimiting("gameserver");

        app.MapGet("/v1/gameserver/give-item", JsonGiveItemAsync).RequireRateLimiting("gameserver");
        app.MapPost("/v1/gameserver/give-item", JsonGiveItemAsync).RequireRateLimiting("gameserver");

        app.MapGet("/v1/gameserver/remove-item", JsonRemoveItemAsync).RequireRateLimiting("gameserver");
        app.MapPost("/v1/gameserver/remove-item", JsonRemoveItemAsync).RequireRateLimiting("gameserver");
    }

    private static void MapLegacy(
        WebApplication app,
        string path,
        Delegate handler)
    {
        app.MapGet(path, handler).RequireRateLimiting("gameserver");
        app.MapPost(path, handler).RequireRateLimiting("gameserver");

        app.MapGet("/api" + path, handler).RequireRateLimiting("gameserver");
        app.MapPost("/api" + path, handler).RequireRateLimiting("gameserver");
    }

    private static async Task<IResult> LegacyGiveItemAsync(
        HttpContext http,
        GameServerInventoryService inventory,
        IOptions<WarIncOptions> options,
        SecurityAuditService audit)
    {
        var data = await ReadRequestDataAsync(http);

        if (!await CheckGameServerAsync(http, data, options, audit, "legacy_give_item"))
            return Text("WO_3 bad key");

        var customerId = ReadULongAny(data, "CustomerID", "CustomerId", "customerId", "customerid", "s_id", "UserID", "userid");
        var itemId = ReadIntAny(data, 0, "ItemID", "ItemId", "itemId", "itemid");
        var quantity = ReadIntAny(data, 1, "Quantity", "quantity", "Qty", "qty", "Amount", "amount");
        var expDays = ReadIntAny(data, 2000, "ExpDays", "expDays", "Days", "days", "Duration", "duration", "LeasedDays", "leasedDays");

        var result = await inventory.GiveItemAsync(customerId, itemId, quantity, expDays);

        if (!result.Ok)
            await audit.LogAsync(http, "legacy_give_item_failed", result.Message, customerId, ReadServerId(data), data);

        return LegacyText(result);
    }

    private static async Task<IResult> LegacyGiveItemInMinutesAsync(
        HttpContext http,
        GameServerInventoryService inventory,
        IOptions<WarIncOptions> options,
        SecurityAuditService audit)
    {
        var data = await ReadRequestDataAsync(http);

        if (!await CheckGameServerAsync(http, data, options, audit, "legacy_give_item_minutes"))
            return Text("WO_3 bad key");

        var customerId = ReadULongAny(data, "CustomerID", "CustomerId", "customerId", "customerid", "s_id", "UserID", "userid");
        var itemId = ReadIntAny(data, 0, "ItemID", "ItemId", "itemId", "itemid");
        var quantity = ReadIntAny(data, 1, "Quantity", "quantity", "Qty", "qty", "Amount", "amount");
        var expMinutes = ReadIntAny(data, 2000 * 24 * 60, "ExpMinutes", "expMinutes", "Minutes", "minutes", "Mins", "mins", "Duration", "duration", "LeasedMinutes", "leasedMinutes");

        var result = await inventory.GiveItemInMinutesAsync(customerId, itemId, quantity, expMinutes);

        if (!result.Ok)
            await audit.LogAsync(http, "legacy_give_item_minutes_failed", result.Message, customerId, ReadServerId(data), data);

        return LegacyText(result);
    }

    private static async Task<IResult> LegacyRemoveItemAsync(
        HttpContext http,
        GameServerInventoryService inventory,
        IOptions<WarIncOptions> options,
        SecurityAuditService audit)
    {
        var data = await ReadRequestDataAsync(http);

        if (!await CheckGameServerAsync(http, data, options, audit, "legacy_remove_item"))
            return Text("WO_3 bad key");

        var customerId = ReadULongAny(data, "CustomerID", "CustomerId", "customerId", "customerid", "s_id", "UserID", "userid");
        var itemId = ReadIntAny(data, 0, "ItemID", "ItemId", "itemId", "itemid");
        var quantity = ReadIntAny(data, 1, "Quantity", "quantity", "Qty", "qty", "Amount", "amount");
        var removeAll = ReadBoolAny(data, false, "RemoveAll", "removeAll", "DeleteAll", "deleteAll", "All", "all");

        var result = await inventory.RemoveItemAsync(customerId, itemId, quantity, removeAll);

        if (!result.Ok)
            await audit.LogAsync(http, "legacy_remove_item_failed", result.Message, customerId, ReadServerId(data), data);

        return LegacyText(result);
    }

    private static async Task<IResult> JsonGiveItemAsync(
        HttpContext http,
        GameServerInventoryService inventory,
        IOptions<WarIncOptions> options,
        SecurityAuditService audit)
    {
        var data = await ReadRequestDataAsync(http);

        if (!await CheckGameServerAsync(http, data, options, audit, "json_give_item"))
        {
            return Results.Json(new GameServerInventoryActionResponse(
                false,
                403,
                "FORBIDDEN",
                0,
                0,
                0,
                0));
        }

        var customerId = ReadULongAny(data, "customerId", "CustomerId", "CustomerID", "s_id");
        var itemId = ReadIntAny(data, 0, "itemId", "ItemId", "ItemID");
        var quantity = ReadIntAny(data, 1, "quantity", "Quantity", "qty", "Qty", "amount", "Amount");
        var expDays = ReadIntAny(data, 2000, "expDays", "ExpDays", "days", "Days");
        var expMinutes = ReadIntAny(data, 0, "expMinutes", "ExpMinutes", "minutes", "Minutes");

        var result = expMinutes > 0
            ? await inventory.GiveItemInMinutesAsync(customerId, itemId, quantity, expMinutes)
            : await inventory.GiveItemAsync(customerId, itemId, quantity, expDays);

        if (!result.Ok)
            await audit.LogAsync(http, "json_give_item_failed", result.Message, customerId, ReadServerId(data), data);

        return Results.Json(result);
    }

    private static async Task<IResult> JsonRemoveItemAsync(
        HttpContext http,
        GameServerInventoryService inventory,
        IOptions<WarIncOptions> options,
        SecurityAuditService audit)
    {
        var data = await ReadRequestDataAsync(http);

        if (!await CheckGameServerAsync(http, data, options, audit, "json_remove_item"))
        {
            return Results.Json(new GameServerInventoryActionResponse(
                false,
                403,
                "FORBIDDEN",
                0,
                0,
                0,
                0));
        }

        var customerId = ReadULongAny(data, "customerId", "CustomerId", "CustomerID", "s_id");
        var itemId = ReadIntAny(data, 0, "itemId", "ItemId", "ItemID");
        var quantity = ReadIntAny(data, 1, "quantity", "Quantity", "qty", "Qty", "amount", "Amount");
        var removeAll = ReadBoolAny(data, false, "removeAll", "RemoveAll", "deleteAll", "DeleteAll");

        var result = await inventory.RemoveItemAsync(customerId, itemId, quantity, removeAll);

        if (!result.Ok)
            await audit.LogAsync(http, "json_remove_item_failed", result.Message, customerId, ReadServerId(data), data);

        return Results.Json(result);
    }

    private static async Task<bool> CheckGameServerAsync(
        HttpContext http,
        Dictionary<string, string> data,
        IOptions<WarIncOptions> options,
        SecurityAuditService audit,
        string action)
    {
        if (RequestSecurity.IsGameServerAllowed(http, options.Value, data, out var reason))
            return true;

        await audit.LogAsync(
            http,
            action + "_denied",
            reason,
            ReadULongAny(data, "CustomerID", "CustomerId", "customerId", "s_id"),
            ReadServerId(data),
            data);

        return false;
    }

    private static IResult LegacyText(GameServerInventoryActionResponse result)
    {
        if (!result.Ok)
            return Text("WO_" + LegacyUtil.NormalizeLegacyErrorCode(result.Code));

        return Text("WO_0");
    }

    private static async Task<Dictionary<string, string>> ReadRequestDataAsync(HttpContext http)
    {
        var data = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

        foreach (var item in http.Request.Query)
            data[item.Key] = item.Value.ToString();

        if (http.Request.HasFormContentType)
        {
            var form = await http.Request.ReadFormAsync();

            foreach (var item in form)
                data[item.Key] = item.Value.ToString();
        }
        else if (http.Request.ContentLength > 0)
        {
            try
            {
                var body = await JsonSerializer.DeserializeAsync<Dictionary<string, JsonElement>>(
                    http.Request.Body,
                    new JsonSerializerOptions
                    {
                        PropertyNameCaseInsensitive = true
                    });

                if (body != null)
                {
                    foreach (var item in body)
                    {
                        data[item.Key] = item.Value.ValueKind switch
                        {
                            JsonValueKind.String => item.Value.GetString() ?? "",
                            JsonValueKind.Number => item.Value.GetRawText(),
                            JsonValueKind.True => "1",
                            JsonValueKind.False => "0",
                            JsonValueKind.Null => "",
                            _ => item.Value.GetRawText()
                        };
                    }
                }
            }
            catch
            {
            }
        }

        return data;
    }

    private static ulong ReadULongAny(
        Dictionary<string, string> data,
        params string[] keys)
    {
        foreach (var key in keys)
        {
            if (!data.TryGetValue(key, out var value))
                continue;

            return LegacyUtil.ParseULong(value);
        }

        return 0;
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

            return LegacyUtil.ParseInt(value);
        }

        return defaultValue;
    }

    private static bool ReadBoolAny(
        Dictionary<string, string> data,
        bool defaultValue,
        params string[] keys)
    {
        foreach (var key in keys)
        {
            if (!data.TryGetValue(key, out var value))
                continue;

            value = value.Trim();

            if (value == "1")
                return true;

            if (value == "0")
                return false;

            if (bool.TryParse(value, out var parsed))
                return parsed;

            return defaultValue;
        }

        return defaultValue;
    }

    private static string ReadServerId(Dictionary<string, string> data)
    {
        return RequestSecurity.ReadAny(
            data,
            "serverid",
            "ServerID",
            "ServerId",
            "serverId",
            "sid");
    }

    private static IResult Text(string value)
    {
        return Results.Text(value, "text/plain", Encoding.UTF8);
    }
}