using System.Text;
using Microsoft.Extensions.Options;
using WarInc.Api.Auth;
using WarInc.Api.Common;
using WarInc.Api.Config;
using WarInc.Api.Security;

namespace WarInc.Api.Purchase;

public static class PurchaseEndpoints
{
    public static void MapPurchaseEndpoints(this WebApplication app)
    {
        app.MapPost("/api_BuyItem3.aspx", LegacyBuyItem3Async).RequireRateLimiting("buy");
        app.MapPost("/api/api_BuyItem3.aspx", LegacyBuyItem3Async).RequireRateLimiting("buy");

        app.MapPost("/v1/shop/buy", async (
            HttpContext http,
            BuyItemRequest request,
            AuthService auth,
            PurchaseService purchase,
            SecurityAuditService audit) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
            {
                await audit.LogAsync(
                    http,
                    "buy_denied",
                    session.Message,
                    request.CustomerId,
                    "",
                    request);

                return Results.Json(new BuyItemResponse(
                    false,
                    session.Code,
                    session.Message,
                    0,
                    false));
            }

            var result = await purchase.BuyItemAsync(
                request.CustomerId,
                request.ItemId,
                request.BuyIdx,
                request.Param1);

            if (!result.Ok)
            {
                await audit.LogAsync(
                    http,
                    "buy_failed",
                    result.Message,
                    request.CustomerId,
                    "",
                    request);
            }

            return Results.Json(result);
        }).RequireRateLimiting("buy");
    }

    private static async Task<IResult> LegacyBuyItem3Async(
        HttpContext http,
        AuthService auth,
        PurchaseService purchase,
        IOptions<WarIncOptions> options,
        SecurityAuditService audit)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());
        var token = ReadAny(form, "s_token", "token", "Token", "SessionKey");

        var itemId = LegacyUtil.ParseInt(form["ItemID"].ToString());
        var buyIdx = LegacyUtil.ParseInt(form["BuyIdx"].ToString());
        var param1 = LegacyUtil.ParseInt(form["Param1"].ToString());

        var session = await auth.CheckLegacySessionWithTokenModeAsync(
            customerId,
            sessionId,
            token,
            options.Value.RequireLegacySessionToken);

        if (!session.Ok)
        {
            await audit.LogAsync(
                http,
                "legacy_buy_denied",
                session.Message,
                customerId,
                "",
                new
                {
                    customerId,
                    sessionId,
                    hasToken = !string.IsNullOrWhiteSpace(token),
                    itemId,
                    buyIdx,
                    param1
                });

            return Results.Text("WO_1", "text/plain", Encoding.UTF8);
        }

        var result = await purchase.BuyItemAsync(
            customerId,
            itemId,
            buyIdx,
            param1);

        if (!result.Ok)
        {
            await audit.LogAsync(
                http,
                "legacy_buy_failed",
                result.Message,
                customerId,
                "",
                new
                {
                    customerId,
                    itemId,
                    buyIdx,
                    param1
                });

            return Results.Text(
                $"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}",
                "text/plain",
                Encoding.UTF8);
        }

        return Results.Text(
            $"WO_0{result.Balance}",
            "text/plain",
            Encoding.UTF8);
    }

    private static string ReadAny(
        IFormCollection form,
        params string[] keys)
    {
        foreach (var key in keys)
        {
            var value = form[key].ToString();

            if (!string.IsNullOrWhiteSpace(value))
                return value;
        }

        return "";
    }
}