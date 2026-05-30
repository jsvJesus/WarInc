namespace WarInc.Api.Database;

public static class DatabaseCheckEndpoints
{
    public static void MapDatabaseCheckEndpoints(this WebApplication app)
    {
        app.MapGet("/internal/database/check", async (DatabaseCheckService service) =>
        {
            var result = await service.CheckAsync();
            return Results.Json(result);
        });

        app.MapGet("/internal/database/schema", async (DatabaseCheckService service) =>
        {
            var result = await service.GetSchemaAsync();
            return Results.Json(result);
        });
    }
}