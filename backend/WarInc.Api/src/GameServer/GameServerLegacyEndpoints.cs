using System.Text;
using System.Text.Json;
using WarInc.Api.Common;

namespace WarInc.Api.GameServer;

public static class GameServerLegacyEndpoints
{
    public static void MapGameServerLegacyEndpoints(this WebApplication app)
    {
        MapLegacy(app, "/api_SrvSetCreateGameKey2.aspx", LegacySetCreateGameKeyAsync);
        MapLegacy(app, "/api_GetCreateGameKey3.aspx", LegacyGetCreateGameKeyAsync);
        MapLegacy(app, "/api_SrvAddUserRoundResult4.aspx", LegacyAddUserRoundResultAsync);
        MapLegacy(app, "/api_SrvAddWeaponStats.aspx", LegacyAddWeaponStatsAsync);
        MapLegacy(app, "/api_SrvUpdateAchievements.aspx", LegacyUpdateAchievementsAsync);
        MapLegacy(app, "/api_SrvAddLogInfo.aspx", LegacyAddLogInfoAsync);
        MapLegacy(app, "/api_SrvUploadLogFile.aspx", LegacyUploadLogFileAsync);
        MapLegacy(app, "/api_SrvAddCheatAttempts.aspx", LegacyAddCheatAttemptsAsync);
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

    private static async Task<IResult> LegacySetCreateGameKeyAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);

        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid", "s_id");

        if (string.IsNullOrWhiteSpace(serverId))
            serverId = "legacy-" + Guid.NewGuid().ToString("N");

        var name = ReadAny(data, "name", "Name", "ServerName", "srvname");

        if (string.IsNullOrWhiteSpace(name))
            name = "Legacy GameServer";

        var map = ReadAny(data, "map", "Map", "MapName", "GBMapName");

        if (string.IsNullOrWhiteSpace(map))
            map = "unknown";

        var mode = ReadAny(data, "mode", "Mode", "GameMode");

        if (string.IsNullOrWhiteSpace(mode))
            mode = "legacy";

        var publicAddress = ReadAny(data, "ip", "IP", "addr", "address", "PublicAddress");

        if (string.IsNullOrWhiteSpace(publicAddress))
            publicAddress = GetRemoteIp(http);

        var port = ReadIntAny(data, 0, "port", "Port", "GamePort", "PublicPort");
        var maxPlayers = ReadIntAny(data, 0, "maxplayers", "MaxPlayers", "maxplr", "MaxPlayersLimit");
        var currentPlayers = ReadIntAny(data, 0, "players", "curplayers", "CurrentPlayers", "numplayers");

        service.Register(new GameServerRegisterRequest(
            serverId,
            name,
            "",
            map,
            mode,
            publicAddress,
            port,
            maxPlayers,
            currentPlayers));

        return Text("WO_0");
    }

    private static async Task<IResult> LegacyGetCreateGameKeyAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);

        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid", "s_id");

        if (string.IsNullOrWhiteSpace(serverId))
            serverId = "legacy-" + Guid.NewGuid().ToString("N");

        var name = ReadAny(data, "name", "Name", "ServerName", "srvname");

        if (string.IsNullOrWhiteSpace(name))
            name = "Legacy GameServer";

        var map = ReadAny(data, "map", "Map", "MapName", "GBMapName");

        if (string.IsNullOrWhiteSpace(map))
            map = "unknown";

        var mode = ReadAny(data, "mode", "Mode", "GameMode");

        if (string.IsNullOrWhiteSpace(mode))
            mode = "legacy";

        var publicAddress = ReadAny(data, "ip", "IP", "addr", "address", "PublicAddress");

        if (string.IsNullOrWhiteSpace(publicAddress))
            publicAddress = GetRemoteIp(http);

        var port = ReadIntAny(data, 0, "port", "Port", "GamePort", "PublicPort");
        var maxPlayers = ReadIntAny(data, 0, "maxplayers", "MaxPlayers", "maxplr", "MaxPlayersLimit");
        var currentPlayers = ReadIntAny(data, 0, "players", "curplayers", "CurrentPlayers", "numplayers");

        service.Register(new GameServerRegisterRequest(
            serverId,
            name,
            "",
            map,
            mode,
            publicAddress,
            port,
            maxPlayers,
            currentPlayers));

        return Text("WO_00");
    }

    private static async Task<IResult> LegacyAddUserRoundResultAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);

        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid");

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            var customerId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "CustomerId", "customerid", "s_id"));
            var gamertag = ReadAny(data, "Gamertag", "gamertag", "gt", "name");

            if (customerId != 0)
            {
                service.PlayerLeave(new GameServerPlayerRequest(
                    serverId,
                    customerId,
                    gamertag));
            }

            service.Report(new GameServerReportRequest(
                serverId,
                BuildReport("round_result", data)));
        }

        return Text("WO_0");
    }

    private static async Task<IResult> LegacyAddWeaponStatsAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);
        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid");

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            service.Report(new GameServerReportRequest(
                serverId,
                BuildReport("weapon_stats", data)));
        }

        return Text("WO_0");
    }

    private static async Task<IResult> LegacyUpdateAchievementsAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);
        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid");

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            service.Report(new GameServerReportRequest(
                serverId,
                BuildReport("achievements", data)));
        }

        return Text("WO_0");
    }

    private static async Task<IResult> LegacyAddLogInfoAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);
        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid");

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            service.Report(new GameServerReportRequest(
                serverId,
                BuildReport("log_info", data)));
        }

        return Text("WO_0");
    }

    private static async Task<IResult> LegacyUploadLogFileAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);
        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid");

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            service.Report(new GameServerReportRequest(
                serverId,
                BuildReport("upload_log_file", data)));
        }

        return Text("WO_0");
    }

    private static async Task<IResult> LegacyAddCheatAttemptsAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);
        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid");

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            service.Report(new GameServerReportRequest(
                serverId,
                BuildReport("cheat_attempt", data)));
        }

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
                    }
                }
            }
            catch
            {
            }
        }

        return data;
    }

    private static string ReadAny(Dictionary<string, string> data, params string[] keys)
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

            if (parsed >= 0)
                return parsed;
        }

        return defaultValue;
    }

    private static string BuildReport(
        string type,
        Dictionary<string, string> data)
    {
        var sb = new StringBuilder();

        sb.Append(type);
        sb.Append(": ");

        var first = true;

        foreach (var item in data.OrderBy(x => x.Key, StringComparer.OrdinalIgnoreCase))
        {
            if (!first)
                sb.Append("; ");

            first = false;

            sb.Append(item.Key);
            sb.Append('=');
            sb.Append(item.Value);
        }

        return sb.ToString();
    }

    private static string GetRemoteIp(HttpContext http)
    {
        var forwarded = http.Request.Headers["X-Forwarded-For"].ToString();

        if (!string.IsNullOrWhiteSpace(forwarded))
            return forwarded.Split(',')[0].Trim();

        return http.Connection.RemoteIpAddress?.ToString() ?? "";
    }

    private static IResult Text(string value)
    {
        return Results.Text(value, "text/plain", Encoding.UTF8);
    }
}