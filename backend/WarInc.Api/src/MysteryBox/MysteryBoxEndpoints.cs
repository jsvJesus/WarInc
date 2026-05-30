using System.Text;
using System.Text.Json;
using WarInc.Api.Auth;
using WarInc.Api.Common;

namespace WarInc.Api.MysteryBox;

public static class MysteryBoxEndpoints
{
    public static void MapMysteryBoxEndpoints(this WebApplication app)
    {
        app.MapGet("/api_MysteryBox.aspx", LegacyMysteryBoxAsync);
        app.MapGet("/api/api_MysteryBox.aspx", LegacyMysteryBoxAsync);

        app.MapPost("/api_MysteryBox.aspx", LegacyMysteryBoxAsync);
        app.MapPost("/api/api_MysteryBox.aspx", LegacyMysteryBoxAsync);

        app.MapGet("/v1/mystery-box/info", JsonInfoAsync);
        app.MapPost("/v1/mystery-box/info", JsonInfoAsync);

        app.MapGet("/v1/mystery-box/roll", JsonRollAsync);
        app.MapPost("/v1/mystery-box/roll", JsonRollAsync);

        app.MapGet("/v1/mystery-box/sell", JsonSellAsync);
        app.MapPost("/v1/mystery-box/sell", JsonSellAsync);
    }

    private static async Task<IResult> LegacyMysteryBoxAsync(
        HttpContext http,
        AuthService auth,
        MysteryBoxService mysteryBox)
    {
        var data = await ReadRequestDataAsync(http);
        var func = ReadAny(data, "func").Trim();

        var customerId = LegacyUtil.ParseULong(ReadAny(
            data,
            "s_id",
            "CustomerID",
            "CustomerId",
            "customerId"));

        var sessionId = LegacyUtil.ParseULong(ReadAny(
            data,
            "s_key",
            "SessionID",
            "SessionId",
            "sessionId"));

        if (!string.Equals(func, "fEEaTest001", StringComparison.OrdinalIgnoreCase))
        {
            var session = await auth.CheckLegacySessionAsync(
                customerId,
                sessionId);

            if (!session.Ok)
                return Results.Text("WO_1", "text/plain", Encoding.UTF8);
        }

        var lootId = ReadIntAny(
            data,
            0,
            "LootID",
            "LootId",
            "lootId",
            "boxId",
            "BoxId",
            "BoxID",
            "id");

        var itemId = ReadIntAny(
            data,
            0,
            "ItemID",
            "ItemId",
            "itemId",
            "itemid");

        var buyIdx = ReadIntAny(
            data,
            0,
            "BuyIdx",
            "buyIdx",
            "buyidx");

        if (string.Equals(func, "info", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(func, "fEEaTest001", StringComparison.OrdinalIgnoreCase))
        {
            var infoLootId = lootId > 0 ? lootId : itemId;
            var info = await mysteryBox.GetInfoAsync(customerId, infoLootId);
            var xml = mysteryBox.BuildLegacyInfoXml(info);

            return Results.Text(xml, "text/xml", Encoding.UTF8);
        }

        if (string.Equals(func, "roll", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(func, "open", StringComparison.OrdinalIgnoreCase))
        {
            var rollBoxId = itemId > 0 ? itemId : lootId;

            var roll = await mysteryBox.RollAsync(
                customerId,
                rollBoxId,
                buyIdx);

            var text = mysteryBox.BuildLegacyRollText(roll);

            return Results.Text(text, "text/plain", Encoding.UTF8);
        }

        if (string.Equals(func, "sell", StringComparison.OrdinalIgnoreCase))
        {
            var sellItemId = itemId > 0 ? itemId : lootId;

            var sell = await mysteryBox.SellAsync(
                customerId,
                sellItemId,
                sellItemId);

            var text = mysteryBox.BuildLegacySellText(sell);

            return Results.Text(text, "text/plain", Encoding.UTF8);
        }

        return Results.Text("WO_5", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> JsonInfoAsync(
        HttpContext http,
        AuthService auth,
        MysteryBoxService mysteryBox)
    {
        var request = await ReadJsonRequestAsync(http);

        var session = await auth.CheckSessionAsync(
            request.CustomerId,
            request.SessionId,
            request.Token);

        if (!session.Ok)
        {
            return Results.Json(new MysteryBoxInfoResponse(
                false,
                session.Code,
                session.Message,
                request.BoxId,
                0,
                Array.Empty<MysteryBoxItemDto>()));
        }

        var result = await mysteryBox.GetInfoAsync(
            request.CustomerId,
            request.BoxId);

        return Results.Json(result);
    }

    private static async Task<IResult> JsonRollAsync(
        HttpContext http,
        AuthService auth,
        MysteryBoxService mysteryBox)
    {
        var request = await ReadJsonRequestAsync(http);

        var session = await auth.CheckSessionAsync(
            request.CustomerId,
            request.SessionId,
            request.Token);

        if (!session.Ok)
        {
            return Results.Json(new MysteryBoxRollResponse(
                false,
                session.Code,
                session.Message,
                request.BoxId,
                0,
                0,
                0,
                0,
                0));
        }

        var result = await mysteryBox.RollAsync(
            request.CustomerId,
            request.BoxId,
            request.BuyIdx);

        return Results.Json(result);
    }

    private static async Task<IResult> JsonSellAsync(
        HttpContext http,
        AuthService auth,
        MysteryBoxService mysteryBox)
    {
        var request = await ReadJsonRequestAsync(http);

        var session = await auth.CheckSessionAsync(
            request.CustomerId,
            request.SessionId,
            request.Token);

        if (!session.Ok)
        {
            return Results.Json(new MysteryBoxSellResponse(
                false,
                session.Code,
                session.Message,
                request.ItemId,
                0));
        }

        var result = await mysteryBox.SellAsync(
            request.CustomerId,
            request.BoxId,
            request.ItemId);

        return Results.Json(result);
    }

    private static async Task<MysteryBoxRequest> ReadJsonRequestAsync(HttpContext http)
    {
        var data = await ReadRequestDataAsync(http);

        var customerId = LegacyUtil.ParseULong(ReadAny(
            data,
            "customerId",
            "CustomerId",
            "CustomerID",
            "s_id"));

        var sessionId = LegacyUtil.ParseULong(ReadAny(
            data,
            "sessionId",
            "SessionId",
            "SessionID",
            "s_key"));

        var token = ReadAny(
            data,
            "token",
            "Token",
            "s_token");

        var boxId = ReadIntAny(
            data,
            0,
            "boxId",
            "BoxId",
            "BoxID",
            "boxid",
            "LootID",
            "LootId",
            "lootId",
            "id");

        var itemId = ReadIntAny(
            data,
            0,
            "itemId",
            "ItemId",
            "ItemID",
            "itemid");

        var buyIdx = ReadIntAny(
            data,
            0,
            "buyIdx",
            "BuyIdx",
            "buyidx");

        return new MysteryBoxRequest(
            customerId,
            sessionId,
            token,
            boxId,
            itemId,
            buyIdx);
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
                    }
                }
            }
            catch
            {
            }
        }

        return data;
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

            return LegacyUtil.ParseInt(value);
        }

        return defaultValue;
    }
}