using System.Text;

namespace WarInc.Api.GameRewards;

public static class GameRewardsEndpoints
{
    public static void MapGameRewardsEndpoints(this WebApplication app)
    {
        app.MapGet("/api_GetDataGameRewards.aspx", LegacyGameRewardsAsync);
        app.MapPost("/api_GetDataGameRewards.aspx", LegacyGameRewardsAsync);

        app.MapGet("/api/api_GetDataGameRewards.aspx", LegacyGameRewardsAsync);
        app.MapPost("/api/api_GetDataGameRewards.aspx", LegacyGameRewardsAsync);

        app.MapGet("/v1/game-rewards/get", JsonGameRewardsAsync);
        app.MapPost("/v1/game-rewards/get", JsonGameRewardsAsync);
    }

    private static async Task<IResult> LegacyGameRewardsAsync(
        GameRewardsService rewards)
    {
        var result = await rewards.GetRewardsAsync();
        var xml = rewards.BuildLegacyXml(result);

        return Results.Text(xml, "text/xml", Encoding.UTF8);
    }

    private static async Task<IResult> JsonGameRewardsAsync(
        GameRewardsService rewards)
    {
        var result = await rewards.GetRewardsAsync();

        return Results.Json(result);
    }
}