using System.Text;
using System.Text.Json;
using WarInc.Api.Common;

namespace WarInc.Api.Leaderboard;

public static class LeaderboardEndpoints
{
    public static void MapLeaderboardEndpoints(this WebApplication app)
    {
        app.MapGet("/api_LeaderboardGet.aspx", LegacyLeaderboardAsync);
        app.MapGet("/api/api_LeaderboardGet.aspx", LegacyLeaderboardAsync);

        app.MapPost("/api_LeaderboardGet.aspx", LegacyLeaderboardAsync);
        app.MapPost("/api/api_LeaderboardGet.aspx", LegacyLeaderboardAsync);

        app.MapGet("/v1/leaderboard/get", JsonLeaderboardAsync);
        app.MapPost("/v1/leaderboard/get", JsonLeaderboardAsync);
    }

    private static async Task<IResult> LegacyLeaderboardAsync(
        HttpContext http,
        LeaderboardService leaderboard)
    {
        var data = await ReadRequestDataAsync(http);

        var customerId = LegacyUtil.ParseULong(ReadAny(
            data,
            "CustomerID",
            "CustomerId",
            "customerId",
            "s_id"));

        var position = ReadIntAny(
            data,
            0,
            "pos",
            "start",
            "offset",
            "StartPos");

        var tableId = ReadIntAny(
            data,
            3,
            "t",
            "table",
            "tableId",
            "TableID",
            "TableId");

        var size = ReadIntAny(
            data,
            100,
            "size",
            "limit",
            "count");

        var result = await leaderboard.GetLeaderboardAsync(
            customerId,
            position,
            size,
            tableId);

        var xml = leaderboard.BuildLegacyLeaderboardXml(result);

        return Results.Text(xml, "text/xml", Encoding.UTF8);
    }

    private static async Task<IResult> JsonLeaderboardAsync(
        HttpContext http,
        LeaderboardService leaderboard)
    {
        var data = await ReadRequestDataAsync(http);

        var customerId = LegacyUtil.ParseULong(ReadAny(
            data,
            "customerId",
            "CustomerId",
            "CustomerID",
            "s_id"));

        var position = ReadIntAny(
            data,
            0,
            "position",
            "pos",
            "start",
            "offset",
            "StartPos");

        var tableId = ReadIntAny(
            data,
            3,
            "tableId",
            "TableId",
            "TableID",
            "table",
            "t");

        var size = ReadIntAny(
            data,
            100,
            "size",
            "limit",
            "count");

        var result = await leaderboard.GetLeaderboardAsync(
            customerId,
            position,
            size,
            tableId);

        return Results.Json(result);
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

            var parsed = LegacyUtil.ParseInt(value);

            return parsed;
        }

        return defaultValue;
    }
}