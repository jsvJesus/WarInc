using System.Text;

namespace WarInc.Api.Shop;

public static class ShopEndpoints
{
    public static void MapShopEndpoints(this WebApplication app)
    {
        app.MapGet("/v1/shop/items", async (ShopService shop) =>
        {
            var result = await shop.GetShopItemsAsync();
            return Results.Json(result);
        });

        app.MapPost("/api_GetShop5.aspx", LegacyGetShop5Async);
        app.MapPost("/api/api_GetShop5.aspx", LegacyGetShop5Async);

        app.MapPost("/api_GetItemsInfo.aspx", LegacyGetItemsInfoAsync);
        app.MapPost("/api/api_GetItemsInfo.aspx", LegacyGetItemsInfoAsync);
    }

    private static async Task<IResult> LegacyGetShop5Async(ShopService shop)
    {
        var result = await shop.BuildLegacyShop5Async();
        return Results.Bytes(result, "application/octet-stream");
    }

    private static async Task<IResult> LegacyGetItemsInfoAsync(ShopService shop)
    {
        var xml = await shop.BuildLegacyItemsInfoXmlAsync();
        return Results.Text(xml, "text/xml", Encoding.UTF8);
    }
}