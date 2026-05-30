using System.Text;
using System.Text.Json;
using WarInc.Api.Common;

namespace WarInc.Api.GameServer;

public static class GameServerInventoryEndpoints
{
    public static void MapGameServerInventoryEndpoints(this WebApplication app)
    {
        MapLegacy(app, "/api_SrvGiveItem.aspx", LegacyGiveItemAsync);
        MapLegacy(app, "/api_SrvGiveItemInMinutes.aspx", LegacyGiveItemInMinutesAsync);
        MapLegacy(app, "/api_SrvRemoveItem.aspx", LegacyRemoveItemAsync);

        app.MapGet("/v1/gameserver/give-item", JsonGiveItemAsync);
        app.MapPost("/v1/gameserver/give-item", JsonGiveItemAsync);

        app.MapGet("/v1/gameserver/remove-item", JsonRemoveItemAsync);
        app.MapPost("/v1/gameserver/remove-item", JsonRemoveItemAsync);
    }

    private static void MapLegacy(
        WebApplication app,
        string path,
        Delegate handler)
    {
        app.MapGet(path, handler);
        app.MapPost(path, handler);

        app.MapGet("/api" + path, handler);
        app.MapPost("/api" + path, handler);
    }

    private static async Task<IResult> LegacyGiveItemAsync(
        HttpContext http,
        GameServerInventoryService inventory)
    {
        var data = await ReadRequestDataAsync(http);

        var customerId = ReadULongAny(
            data,
            "CustomerID",
            "CustomerId",
            "customerId",
            "customerid",
            "s_id",
            "UserID",
            "userid");

        var itemId = ReadIntAny(
            data,
            0,
            "ItemID",
            "ItemId",
            "itemId",
            "itemid");

        var quantity = ReadIntAny(
            data,
            1,
            "Quantity",
            "quantity",
            "Qty",
            "qty",
            "Amount",
            "amount");

        var expDays = ReadIntAny(
            data,
            2000,
            "ExpDays",
            "expDays",
            "Days",
            "days",
            "Duration",
            "duration",
            "LeasedDays",
            "leasedDays");

        var result = await inventory.GiveItemAsync(
            customerId,
            itemId,
            quantity,
            expDays);

        return LegacyText(result);
    }

    private static async Task<IResult> LegacyGiveItemInMinutesAsync(
        HttpContext http,
        GameServerInventoryService inventory)
    {
        var data = await ReadRequestDataAsync(http);

        var customerId = ReadULongAny(
            data,
            "CustomerID",
            "CustomerId",
            "customerId",
            "customerid",
            "s_id",
            "UserID",
            "userid");

        var itemId = ReadIntAny(
            data,
            0,
            "ItemID",
            "ItemId",
            "itemId",
            "itemid");

        var quantity = ReadIntAny(
            data,
            1,
            "Quantity",
            "quantity",
            "Qty",
            "qty",
            "Amount",
            "amount");

        var expMinutes = ReadIntAny(
            data,
            2000 * 24 * 60,
            "ExpMinutes",
            "expMinutes",
            "Minutes",
            "minutes",
            "Mins",
            "mins",
            "Duration",
            "duration",
            "LeasedMinutes",
            "leasedMinutes");

        var result = await inventory.GiveItemInMinutesAsync(
            customerId,
            itemId,
            quantity,
            expMinutes);

        return LegacyText(result);
    }

    private static async Task<IResult> LegacyRemoveItemAsync(
        HttpContext http,
        GameServerInventoryService inventory)
    {
        var data = await ReadRequestDataAsync(http);

        var customerId = ReadULongAny(
            data,
            "CustomerID",
            "CustomerId",
            "customerId",
            "customerid",
            "s_id",
            "UserID",
            "userid");

        var itemId = ReadIntAny(
            data,
            0,
            "ItemID",
            "ItemId",
            "itemId",
            "itemid");

        var quantity = ReadIntAny(
            data,
            1,
            "Quantity",
            "quantity",
            "Qty",
            "qty",
            "Amount",
            "amount");

        var removeAll = ReadBoolAny(
            data,
            false,
            "RemoveAll",
            "removeAll",
            "DeleteAll",
            "deleteAll",
            "All",
            "all");

        var result = await inventory.RemoveItemAsync(
            customerId,
            itemId,
            quantity,
            removeAll);

        return LegacyText(result);
    }

    private static async Task<IResult> JsonGiveItemAsync(
        HttpContext http,
        GameServerInventoryService inventory)
    {
        var data = await ReadRequestDataAsync(http);

        var customerId = ReadULongAny(
            data,
            "customerId",
            "CustomerId",
            "CustomerID",
            "s_id");

        var itemId = ReadIntAny(
            data,
            0,
            "itemId",
            "ItemId",
            "ItemID");

        var quantity = ReadIntAny(
            data,
            1,
            "quantity",
            "Quantity",
            "qty",
            "Qty",
            "amount",
            "Amount");

        var expDays = ReadIntAny(
            data,
            2000,
            "expDays",
            "ExpDays",
            "days",
            "Days");

        var expMinutes = ReadIntAny(
            data,
            0,
            "expMinutes",
            "ExpMinutes",
            "minutes",
            "Minutes");

        var result = expMinutes > 0
            ? await inventory.GiveItemInMinutesAsync(customerId, itemId, quantity, expMinutes)
            : await inventory.GiveItemAsync(customerId, itemId, quantity, expDays);

        return Results.Json(result);
    }

    private static async Task<IResult> JsonRemoveItemAsync(
        HttpContext http,
        GameServerInventoryService inventory)
    {
        var data = await ReadRequestDataAsync(http);

        var customerId = ReadULongAny(
            data,
            "customerId",
            "CustomerId",
            "CustomerID",
            "s_id");

        var itemId = ReadIntAny(
            data,
            0,
            "itemId",
            "ItemId",
            "ItemID");

        var quantity = ReadIntAny(
            data,
            1,
            "quantity",
            "Quantity",
            "qty",
            "Qty",
            "amount",
            "Amount");

        var removeAll = ReadBoolAny(
            data,
            false,
            "removeAll",
            "RemoveAll",
            "deleteAll",
            "DeleteAll");

        var result = await inventory.RemoveItemAsync(
            customerId,
            itemId,
            quantity,
            removeAll);

        return Results.Json(result);
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
                        if (item.Value.ValueKind == JsonValueKind.String)
                        {
                            data[item.Key] = item.Value.GetString() ?? "";
                        }
                        else if (item.Value.ValueKind == JsonValueKind.Number)
                        {
                            data[item.Key] = item.Value.GetRawText();
                        }
                        else if (item.Value.ValueKind == JsonValueKind.True)
                        {
                            data[item.Key] = "1";
                        }
                        else if (item.Value.ValueKind == JsonValueKind.False)
                        {
                            data[item.Key] = "0";
                        }
                        else
                        {
                            data[item.Key] = item.Value.GetRawText();
                        }
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

    private static IResult Text(string value)
    {
        return Results.Text(value, "text/plain", Encoding.UTF8);
    }
}