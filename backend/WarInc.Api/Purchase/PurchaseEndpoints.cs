using System.Text;
using WarInc.Api.Auth;
using WarInc.Api.Common;

namespace WarInc.Api.Purchase;

public static class PurchaseEndpoints
{
    public static void MapPurchaseEndpoints(this WebApplication app)
    {
        app.MapPost("/api_BuyItem3.aspx", LegacyBuyItem3Async);
        app.MapPost("/api/api_BuyItem3.aspx", LegacyBuyItem3Async);

        app.MapPost("/v1/shop/buy", async (
            BuyItemRequest request,
            AuthService auth,
            PurchaseService purchase) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
            {
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

            return Results.Json(result);
        });
    }

    private static async Task<IResult> LegacyBuyItem3Async(
        HttpContext http,
        AuthService auth,
        PurchaseService purchase)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var itemId = LegacyUtil.ParseInt(form["ItemID"].ToString());
        var buyIdx = LegacyUtil.ParseInt(form["BuyIdx"].ToString());
        var param1 = LegacyUtil.ParseInt(form["Param1"].ToString());

        var session = await auth.CheckLegacySessionAsync(
            customerId,
            sessionId);

        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var result = await purchase.BuyItemAsync(
            customerId,
            itemId,
            buyIdx,
            param1);

        if (!result.Ok)
        {
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
}