using System.IO.Compression;
using System.Text;
using System.Text.Json;
using Microsoft.Extensions.Options;
using WarInc.Api.Common;
using WarInc.Api.Config;

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
        GameServerService service,
        IOptions<WarIncOptions> options)
    {
        var data = await ReadRequestDataAsync(http);

        if (!CheckServerKey(data, options))
            return Text("WO_3 bad key");

        var createGameKey = ReadIntAny(data, 0, "CreateGameKey", "createGameKey", "gameKey", "key");
        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid", "s_id");

        if (string.IsNullOrWhiteSpace(serverId) && createGameKey > 0)
            serverId = createGameKey.ToString();

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

        await service.RegisterAsync(new GameServerRegisterRequest(
            serverId,
            name,
            "",
            map,
            mode,
            publicAddress,
            port,
            maxPlayers,
            currentPlayers,
            createGameKey));

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

        await service.RegisterAsync(new GameServerRegisterRequest(
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
        var remoteIp = GetRemoteIp(http);

        var ok = await service.WriteRoundResultAsync(data, remoteIp);

        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid");

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            var customerId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "CustomerId", "customerid", "s_id"));
            var gamertag = ReadAny(data, "Gamertag", "gamertag", "gt", "name");

            if (customerId != 0)
            {
                await service.PlayerLeaveAsync(new GameServerPlayerRequest(
                    serverId,
                    customerId,
                    gamertag));
            }

            await service.ReportAsync(new GameServerReportRequest(
                serverId,
                BuildReport("round_result", data),
                "round_result"));
        }

        return Text(ok ? "WO_0" : "WO_5");
    }

    private static async Task<IResult> LegacyAddWeaponStatsAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);
        var ok = await service.WriteWeaponStatsAsync(data);

        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid");

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            await service.ReportAsync(new GameServerReportRequest(
                serverId,
                BuildReport("weapon_stats", data),
                "weapon_stats"));
        }

        return Text(ok ? "WO_0" : "WO_5");
    }

    private static async Task<IResult> LegacyUpdateAchievementsAsync(
        HttpContext http,
        GameServerService service,
        IOptions<WarIncOptions> options)
    {
        var data = await ReadRequestDataAsync(http);

        if (!CheckServerKey(data, options))
            return Text("WO_3 bad key");

        var ok = await service.UpdateAchievementsAsync(data);

        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid");

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            await service.ReportAsync(new GameServerReportRequest(
                serverId,
                BuildReport("achievements", data),
                "achievements"));
        }

        return Text(ok ? "WO_0" : "WO_5");
    }

    private static async Task<IResult> LegacyAddLogInfoAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);
        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid");

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            await service.ReportAsync(new GameServerReportRequest(
                serverId,
                BuildReport("log_info", data),
                "log_info"));
        }

        return Text("WO_0");
    }

    private static async Task<IResult> LegacyUploadLogFileAsync(
        HttpContext http,
        GameServerService service,
        IOptions<WarIncOptions> options)
    {
        var data = await ReadRequestDataAsync(http);

        if (!CheckServerKey(data, options))
            return Text("WO_3 bad key");

        await SaveUploadedLegacyFilesAsync(http, data);

        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid");

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            await service.ReportAsync(new GameServerReportRequest(
                serverId,
                BuildReport("upload_log_file", data),
                "upload_log_file"));
        }

        return Text("WO_0");
    }

    private static async Task<IResult> LegacyAddCheatAttemptsAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);

        var ok = await service.WriteCheatAttemptAsync(data);

        var serverId = ReadAny(data, "serverid", "ServerID", "ServerId", "sid");

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            await service.ReportAsync(new GameServerReportRequest(
                serverId,
                BuildReport("cheat_attempt", data),
                "cheat_attempt"));
        }

        return Text(ok ? "WO_0" : "WO_5");
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
                            data[item.Key] = item.Value.GetString() ?? "";
                        else if (item.Value.ValueKind == JsonValueKind.Number)
                            data[item.Key] = item.Value.GetRawText();
                        else if (item.Value.ValueKind == JsonValueKind.True)
                            data[item.Key] = "1";
                        else if (item.Value.ValueKind == JsonValueKind.False)
                            data[item.Key] = "0";
                        else
                            data[item.Key] = item.Value.GetRawText();
                    }
                }
            }
            catch
            {
            }
        }

        return data;
    }

    private static async Task SaveUploadedLegacyFilesAsync(
        HttpContext http,
        Dictionary<string, string> data)
    {
        if (!http.Request.HasFormContentType)
            return;

        try
        {
            var form = await http.Request.ReadFormAsync();

            if (form.Files.Count == 0)
                return;

            var sessionId = ReadAny(data, "sessionId", "SessionID", "sid", "s_id");

            if (string.IsNullOrWhiteSpace(sessionId))
                sessionId = DateTimeOffset.UtcNow.ToUnixTimeSeconds().ToString();

            sessionId = SafeFileName(sessionId);

            var root = Path.Combine(AppContext.BaseDirectory, "legacy-uploads");
            var day = DateTime.UtcNow.ToString("yyyy-MM-dd");

            var logsDir = Path.Combine(root, "logs", day);
            Directory.CreateDirectory(logsDir);

            var logFile = form.Files.FirstOrDefault(x => string.Equals(x.Name, "logFile", StringComparison.OrdinalIgnoreCase));
            if (logFile != null && logFile.Length > 0)
            {
                var logSize = ReadIntAny(data, 0, "logSize", "LogSize");
                var bytes = await ReadMaybeGZipAsync(logFile, logSize);
                await File.WriteAllBytesAsync(Path.Combine(logsDir, $"GS_{sessionId}.txt"), bytes);
            }

            var dmpFile = form.Files.FirstOrDefault(x => string.Equals(x.Name, "dmpFile", StringComparison.OrdinalIgnoreCase));
            if (dmpFile != null && dmpFile.Length > 0)
            {
                var crashDir = Path.Combine(root, "crash");
                Directory.CreateDirectory(crashDir);

                var dmpSize = ReadIntAny(data, 0, "dmpSize", "DmpSize");
                var bytes = await ReadMaybeGZipAsync(dmpFile, dmpSize);

                await File.WriteAllBytesAsync(Path.Combine(crashDir, $"GS_{sessionId}.dmp"), bytes);
            }

            var picturesDir = Path.Combine(root, "pictures");
            Directory.CreateDirectory(picturesDir);

            for (var i = 0; i < 99; i++)
            {
                var fileName = "jpgFile" + i;
                var jpg = form.Files.FirstOrDefault(x => string.Equals(x.Name, fileName, StringComparison.OrdinalIgnoreCase));

                if (jpg == null || jpg.Length <= 0)
                    continue;

                var jpgPath = Path.Combine(picturesDir, $"GS_{sessionId}_{i:00}.jpg");

                await using var input = jpg.OpenReadStream();
                await using var output = File.Create(jpgPath);
                await input.CopyToAsync(output);
            }
        }
        catch
        {
        }
    }

    private static async Task<byte[]> ReadMaybeGZipAsync(
        IFormFile file,
        int expectedSize)
    {
        if (expectedSize > 0 || file.FileName.EndsWith(".gz", StringComparison.OrdinalIgnoreCase))
        {
            try
            {
                await using var input = file.OpenReadStream();
                await using var output = new MemoryStream();

                using (var gzip = new GZipStream(input, CompressionMode.Decompress))
                {
                    await gzip.CopyToAsync(output);
                }

                return output.ToArray();
            }
            catch
            {
            }
        }

        await using var rawInput = file.OpenReadStream();
        await using var rawOutput = new MemoryStream();
        await rawInput.CopyToAsync(rawOutput);

        return rawOutput.ToArray();
    }

    private static bool CheckServerKey(
        Dictionary<string, string> data,
        IOptions<WarIncOptions> options)
    {
        var expected = options.Value.InternalApiKey;

        if (string.IsNullOrWhiteSpace(expected))
            return true;

        var actual = ReadAny(data, "skey1", "SKey1", "serverKey", "ServerKey", "key", "Key");

        return actual == expected;
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

    private static string SafeFileName(string value)
    {
        value = value.Trim();

        var sb = new StringBuilder(value.Length);

        foreach (var ch in value)
        {
            if (char.IsLetterOrDigit(ch) || ch == '_' || ch == '-')
                sb.Append(ch);
        }

        if (sb.Length == 0)
            return DateTimeOffset.UtcNow.ToUnixTimeSeconds().ToString();

        return sb.ToString();
    }

    private static IResult Text(string value)
    {
        return Results.Text(value, "text/plain", Encoding.UTF8);
    }
}