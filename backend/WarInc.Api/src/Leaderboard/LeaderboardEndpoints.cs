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

        var position = ReadInt(data, "pos", 0);

        if (position == 0)
            position = ReadInt(data, "start", 0);

        if (position == 0)
            position = ReadInt(data, "offset", 0);

        var size = ReadInt(data, "size", 0);

        if (size == 0)
            size = ReadInt(data, "limit", 0);

        if (size == 0)
            size = ReadInt(data, "count", 0);

        var result = await leaderboard.GetLeaderboardAsync(position, size);
        var xml = leaderboard.BuildLegacyLeaderboardXml(result);

        return Results.Text(xml, "text/xml", Encoding.UTF8);
    }

    private static async Task<IResult> JsonLeaderboardAsync(
        HttpContext http,
        LeaderboardService leaderboard)
    {
        var data = await ReadRequestDataAsync(http);

        var position = ReadInt(data, "position", 0);

        if (position == 0)
            position = ReadInt(data, "pos", 0);

        if (position == 0)
            position = ReadInt(data, "start", 0);

        if (position == 0)
            position = ReadInt(data, "offset", 0);

        var size = ReadInt(data, "size", 50);

        if (size == 50)
            size = ReadInt(data, "limit", 50);

        if (size == 50)
            size = ReadInt(data, "count", 50);

        var result = await leaderboard.GetLeaderboardAsync(position, size);

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

    private static int ReadInt(
        Dictionary<string, string> data,
        string key,
        int defaultValue)
    {
        if (!data.TryGetValue(key, out var value))
            return defaultValue;

        var parsed = LegacyUtil.ParseInt(value);

        if (parsed < 0)
            return defaultValue;

        return parsed;
    }
}