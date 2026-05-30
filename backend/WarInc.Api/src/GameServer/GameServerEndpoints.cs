using Dapper;
using Microsoft.Extensions.Options;
using MySqlConnector;
using WarInc.Api.Common;
using WarInc.Api.Config;
using WarInc.Api.Database;
using System.Text;
using System.Text.Json;

namespace WarInc.Api.GameServer;

public static class GameServerEndpoints
{
    public static void MapGameServerEndpoints(this WebApplication app)
    {
        app.MapPost("/internal/gameserver/register", async (
            HttpContext http,
            GameServerRegisterRequest request,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = await service.RegisterAsync(request);
            return Results.Json(result);
        });

        app.MapPost("/internal/gameserver/heartbeat", async (
            HttpContext http,
            GameServerHeartbeatRequest request,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = await service.HeartbeatAsync(request);
            return Results.Json(result);
        });

        app.MapPost("/internal/gameserver/unregister", async (
            HttpContext http,
            GameServerIdRequest request,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = await service.UnregisterAsync(request.ServerId);
            return Results.Json(result);
        });

        app.MapPost("/internal/gameserver/player/join", async (
            HttpContext http,
            GameServerPlayerRequest request,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = await service.PlayerJoinAsync(request);
            return Results.Json(result);
        });

        app.MapPost("/internal/gameserver/player/leave", async (
            HttpContext http,
            GameServerPlayerRequest request,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = await service.PlayerLeaveAsync(request);
            return Results.Json(result);
        });

        app.MapPost("/internal/gameserver/report", async (
            HttpContext http,
            GameServerReportRequest request,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = await service.ReportAsync(request);
            return Results.Json(result);
        });

        app.MapGet("/internal/gameserver/list", async (
            HttpContext http,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var servers = await service.ListAsync();

            return Results.Json(new
            {
                ok = true,
                code = 0,
                message = "OK",
                servers
            });
        });
        
        app.MapGet("/v1/gameserver/result", JsonGameServerResultAsync);
        app.MapPost("/v1/gameserver/result", JsonGameServerResultAsync);

        app.MapGet("/v1/gameserver/weapon-stats", JsonGameServerWeaponStatsAsync);
        app.MapPost("/v1/gameserver/weapon-stats", JsonGameServerWeaponStatsAsync);

        app.MapPost("/internal/gameserver/list", async (
            HttpContext http,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var servers = await service.ListAsync();

            return Results.Json(new
            {
                ok = true,
                code = 0,
                message = "OK",
                servers
            });
        });
    }
    
        private static async Task<IResult> JsonGameServerResultAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);
        var remoteIp = GetRemoteIp(http);

        var customerId = ReadULongAny(
            data,
            "CustomerID",
            "CustomerId",
            "customerId",
            "customerid",
            "s_id",
            "UserID",
            "userid");

        if (customerId == 0)
        {
            return Results.Json(new
            {
                ok = false,
                code = 400,
                message = "BAD_CUSTOMER_ID"
            });
        }

        var serverId = ReadAny(
            data,
            "serverid",
            "ServerID",
            "ServerId",
            "serverId",
            "sid");

        var gamertag = ReadAny(
            data,
            "Gamertag",
            "gamertag",
            "gt",
            "name");

        var ok = await service.WriteRoundResultAsync(data, remoteIp);

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            await service.PlayerLeaveAsync(new GameServerPlayerRequest(
                serverId,
                customerId,
                gamertag));

            await service.ReportAsync(new GameServerReportRequest(
                serverId,
                BuildReport("round_result", data),
                "round_result"));
        }

        return Results.Json(new
        {
            ok,
            code = ok ? 0 : 500,
            message = ok ? "OK" : "WRITE_FAILED",
            action = "gameserver_result",
            customerId,
            serverId,
            written = ok
        });
    }

    private static async Task<IResult> JsonGameServerWeaponStatsAsync(
        HttpContext http,
        GameServerService service)
    {
        var data = await ReadRequestDataAsync(http);

        var itemId = ReadIntAny(
            data,
            0,
            "ItemID",
            "ItemId",
            "itemId",
            "WeaponID",
            "WeaponId",
            "weaponId");

        if (itemId <= 0)
        {
            return Results.Json(new
            {
                ok = false,
                code = 400,
                message = "BAD_ITEM_ID"
            });
        }

        var customerId = ReadULongAny(
            data,
            "CustomerID",
            "CustomerId",
            "customerId",
            "customerid",
            "s_id",
            "UserID",
            "userid");

        var serverId = ReadAny(
            data,
            "serverid",
            "ServerID",
            "ServerId",
            "serverId",
            "sid");

        var ok = await service.WriteWeaponStatsAsync(data);

        if (!string.IsNullOrWhiteSpace(serverId))
        {
            await service.ReportAsync(new GameServerReportRequest(
                serverId,
                BuildReport("weapon_stats", data),
                "weapon_stats"));
        }

        return Results.Json(new
        {
            ok,
            code = ok ? 0 : 500,
            message = ok ? "OK" : "WRITE_FAILED",
            action = "gameserver_weapon_stats",
            itemId,
            customerId,
            serverId,
            written = ok
        });
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

            return data;
        }

        if (http.Request.ContentLength.GetValueOrDefault() <= 0)
            return data;

        var contentType = http.Request.ContentType ?? "";

        if (!contentType.Contains("application/json", StringComparison.OrdinalIgnoreCase))
            return data;

        try
        {
            using var doc = await JsonDocument.ParseAsync(http.Request.Body);

            if (doc.RootElement.ValueKind != JsonValueKind.Object)
                return data;

            foreach (var prop in doc.RootElement.EnumerateObject())
            {
                data[prop.Name] = prop.Value.ValueKind switch
                {
                    JsonValueKind.String => prop.Value.GetString() ?? "",
                    JsonValueKind.Number => prop.Value.GetRawText(),
                    JsonValueKind.True => "1",
                    JsonValueKind.False => "0",
                    JsonValueKind.Null => "",
                    _ => prop.Value.GetRawText()
                };
            }
        }
        catch
        {
            return data;
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

    private static ulong ReadULongAny(
        Dictionary<string, string> data,
        params string[] keys)
    {
        foreach (var key in keys)
        {
            if (!data.TryGetValue(key, out var value))
                continue;

            return LegacyUtil.ParseULong(value);
        }

        return 0;
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

    private static bool IsInternalRequest(
        HttpContext http,
        IOptions<WarIncOptions> options)
    {
        var expected = options.Value.InternalApiKey;
        var actual = http.Request.Headers["X-WarInc-Internal-Key"].ToString();

        if (string.IsNullOrWhiteSpace(expected))
            return false;

        return actual == expected;
    }
}

public sealed class GameServerService
{
    private readonly WarIncDb _db;

    public GameServerService(WarIncDb db)
    {
        _db = db;
    }

    public async Task<object> RegisterAsync(GameServerRegisterRequest request)
    {
        var serverId = NormalizeServerId(request.ServerId);

        if (string.IsNullOrWhiteSpace(serverId))
            return new ApiResponse(false, 400, "BAD_SERVER_ID");

        await using var db = _db.CreateConnection();
        await EnsureSchemaAsync(db);

        var serverIntId = StableServerIntId(serverId);
        var createGameKey = request.CreateGameKey > 0 ? request.CreateGameKey : serverIntId;
        var maxPlayers = Math.Max(0, request.MaxPlayers);
        var currentPlayers = ClampCurrentPlayers(request.CurrentPlayers, maxPlayers);

        var exists = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM masterserverinfo
            WHERE ServerGuid = @ServerGuid;
            """,
            new { ServerGuid = serverId });

        if (exists > 0)
        {
            await db.ExecuteAsync(
                """
                UPDATE masterserverinfo
                SET
                    ServerID = @ServerID,
                    LastUpdated = UTC_TIMESTAMP(),
                    CreateGameKey = @CreateGameKey,
                    IP = @IP,
                    ServerName = @ServerName,
                    RegionName = @RegionName,
                    MapName = @MapName,
                    GameMode = @GameMode,
                    PublicAddress = @PublicAddress,
                    PublicPort = @PublicPort,
                    MaxPlayers = @MaxPlayers,
                    CurrentPlayers = @CurrentPlayers,
                    Status = 'online',
                    LastHeartbeatAt = UTC_TIMESTAMP()
                WHERE ServerGuid = @ServerGuid;
                """,
                new
                {
                    ServerID = serverIntId,
                    ServerGuid = serverId,
                    CreateGameKey = createGameKey,
                    IP = Trim(request.PublicAddress, 64),
                    ServerName = Trim(request.Name, 128),
                    RegionName = Trim(request.Region, 32),
                    MapName = Trim(request.Map, 64),
                    GameMode = Trim(request.Mode, 32),
                    PublicAddress = Trim(request.PublicAddress, 64),
                    PublicPort = Math.Max(0, request.PublicPort),
                    MaxPlayers = maxPlayers,
                    CurrentPlayers = currentPlayers
                });
        }
        else
        {
            await db.ExecuteAsync(
                """
                INSERT INTO masterserverinfo
                (
                    ServerID,
                    LastUpdated,
                    CreateGameKey,
                    IP,
                    ServerGuid,
                    ServerName,
                    RegionName,
                    MapName,
                    GameMode,
                    PublicAddress,
                    PublicPort,
                    MaxPlayers,
                    CurrentPlayers,
                    Status,
                    CreatedAt,
                    LastHeartbeatAt
                )
                VALUES
                (
                    @ServerID,
                    UTC_TIMESTAMP(),
                    @CreateGameKey,
                    @IP,
                    @ServerGuid,
                    @ServerName,
                    @RegionName,
                    @MapName,
                    @GameMode,
                    @PublicAddress,
                    @PublicPort,
                    @MaxPlayers,
                    @CurrentPlayers,
                    'online',
                    UTC_TIMESTAMP(),
                    UTC_TIMESTAMP()
                );
                """,
                new
                {
                    ServerID = serverIntId,
                    ServerGuid = serverId,
                    CreateGameKey = createGameKey,
                    IP = Trim(request.PublicAddress, 64),
                    ServerName = Trim(request.Name, 128),
                    RegionName = Trim(request.Region, 32),
                    MapName = Trim(request.Map, 64),
                    GameMode = Trim(request.Mode, 32),
                    PublicAddress = Trim(request.PublicAddress, 64),
                    PublicPort = Math.Max(0, request.PublicPort),
                    MaxPlayers = maxPlayers,
                    CurrentPlayers = currentPlayers
                });
        }

        var server = await GetServerDtoAsync(db, serverId);

        return new
        {
            ok = true,
            code = 0,
            message = "OK",
            server
        };
    }

    public async Task<object> HeartbeatAsync(GameServerHeartbeatRequest request)
    {
        var serverId = NormalizeServerId(request.ServerId);

        if (string.IsNullOrWhiteSpace(serverId))
            return new ApiResponse(false, 400, "BAD_SERVER_ID");

        await using var db = _db.CreateConnection();
        await EnsureSchemaAsync(db);

        var row = await db.QueryFirstOrDefaultAsync<GameServerDbRow>(
            """
            SELECT
                ServerGuid AS ServerId,
                COALESCE(MaxPlayers, 0) AS MaxPlayers
            FROM masterserverinfo
            WHERE ServerGuid = @ServerGuid
            LIMIT 1;
            """,
            new { ServerGuid = serverId });

        if (row == null)
            return new ApiResponse(false, 404, "SERVER_NOT_FOUND");

        var currentPlayers = ClampCurrentPlayers(request.CurrentPlayers, row.MaxPlayers);
        var status = string.IsNullOrWhiteSpace(request.Status) ? "online" : Trim(request.Status, 16);

        await db.ExecuteAsync(
            """
            UPDATE masterserverinfo
            SET
                LastUpdated = UTC_TIMESTAMP(),
                LastHeartbeatAt = UTC_TIMESTAMP(),
                CurrentPlayers = @CurrentPlayers,
                Status = @Status
            WHERE ServerGuid = @ServerGuid;
            """,
            new
            {
                ServerGuid = serverId,
                CurrentPlayers = currentPlayers,
                Status = status
            });

        var server = await GetServerDtoAsync(db, serverId);

        return new
        {
            ok = true,
            code = 0,
            message = "OK",
            server
        };
    }

    public async Task<object> UnregisterAsync(string serverIdRaw)
    {
        var serverId = NormalizeServerId(serverIdRaw);

        if (string.IsNullOrWhiteSpace(serverId))
            return new ApiResponse(false, 400, "BAD_SERVER_ID");

        await using var db = _db.CreateConnection();
        await EnsureSchemaAsync(db);

        await db.ExecuteAsync(
            """
            UPDATE masterserverinfo
            SET
                LastUpdated = UTC_TIMESTAMP(),
                Status = 'offline',
                CurrentPlayers = 0
            WHERE ServerGuid = @ServerGuid;

            DELETE FROM gameserver_activeplayers
            WHERE ServerGuid = @ServerGuid;
            """,
            new { ServerGuid = serverId });

        return new ApiResponse(true, 0, "OK");
    }

    public async Task<object> PlayerJoinAsync(GameServerPlayerRequest request)
    {
        var serverId = NormalizeServerId(request.ServerId);

        if (string.IsNullOrWhiteSpace(serverId))
            return new ApiResponse(false, 400, "BAD_SERVER_ID");

        if (request.CustomerId == 0 || request.CustomerId > int.MaxValue)
            return new ApiResponse(false, 400, "BAD_CUSTOMER_ID");

        await using var db = _db.CreateConnection();
        await EnsureSchemaAsync(db);

        if (!await ServerExistsAsync(db, serverId))
            return new ApiResponse(false, 404, "SERVER_NOT_FOUND");

        var gamertag = Trim(request.Gamertag, 32);

        if (string.IsNullOrWhiteSpace(gamertag))
            gamertag = await ResolveGamertagAsync(db, (int)request.CustomerId);

        await db.ExecuteAsync(
            """
            DELETE FROM gameserver_activeplayers
            WHERE ServerGuid = @ServerGuid
              AND CustomerID = @CustomerID;

            INSERT INTO gameserver_activeplayers
            (
                ServerGuid,
                CustomerID,
                Gamertag,
                JoinedAt,
                LastSeenAt
            )
            VALUES
            (
                @ServerGuid,
                @CustomerID,
                @Gamertag,
                UTC_TIMESTAMP(),
                UTC_TIMESTAMP()
            );

            UPDATE masterserverinfo
            SET
                CurrentPlayers =
                (
                    SELECT COUNT(*)
                    FROM gameserver_activeplayers
                    WHERE ServerGuid = @ServerGuid
                ),
                LastUpdated = UTC_TIMESTAMP(),
                LastHeartbeatAt = UTC_TIMESTAMP(),
                Status = 'online'
            WHERE ServerGuid = @ServerGuid;
            """,
            new
            {
                ServerGuid = serverId,
                CustomerID = (int)request.CustomerId,
                Gamertag = gamertag
            });

        var server = await GetServerDtoAsync(db, serverId);

        return new
        {
            ok = true,
            code = 0,
            message = "OK",
            server
        };
    }

    public async Task<object> PlayerLeaveAsync(GameServerPlayerRequest request)
    {
        var serverId = NormalizeServerId(request.ServerId);

        if (string.IsNullOrWhiteSpace(serverId))
            return new ApiResponse(false, 400, "BAD_SERVER_ID");

        if (request.CustomerId == 0 || request.CustomerId > int.MaxValue)
            return new ApiResponse(false, 400, "BAD_CUSTOMER_ID");

        await using var db = _db.CreateConnection();
        await EnsureSchemaAsync(db);

        if (!await ServerExistsAsync(db, serverId))
            return new ApiResponse(false, 404, "SERVER_NOT_FOUND");

        await db.ExecuteAsync(
            """
            DELETE FROM gameserver_activeplayers
            WHERE ServerGuid = @ServerGuid
              AND CustomerID = @CustomerID;

            UPDATE masterserverinfo
            SET
                CurrentPlayers =
                (
                    SELECT COUNT(*)
                    FROM gameserver_activeplayers
                    WHERE ServerGuid = @ServerGuid
                ),
                LastUpdated = UTC_TIMESTAMP(),
                LastHeartbeatAt = UTC_TIMESTAMP()
            WHERE ServerGuid = @ServerGuid;
            """,
            new
            {
                ServerGuid = serverId,
                CustomerID = (int)request.CustomerId
            });

        var server = await GetServerDtoAsync(db, serverId);

        return new
        {
            ok = true,
            code = 0,
            message = "OK",
            server
        };
    }

    public async Task<object> ReportAsync(GameServerReportRequest request)
    {
        var serverId = NormalizeServerId(request.ServerId);

        if (string.IsNullOrWhiteSpace(serverId))
            return new ApiResponse(false, 400, "BAD_SERVER_ID");

        await using var db = _db.CreateConnection();
        await EnsureSchemaAsync(db);

        if (!await ServerExistsAsync(db, serverId))
            return new ApiResponse(false, 404, "SERVER_NOT_FOUND");

        var reportType = string.IsNullOrWhiteSpace(request.ReportType)
            ? "generic"
            : Trim(request.ReportType, 32);

        var report = Trim(request.Report ?? "", 65535);

        await db.ExecuteAsync(
            """
            INSERT INTO gameserver_reports
            (
                ServerGuid,
                ReportType,
                ReportTime,
                RawData
            )
            VALUES
            (
                @ServerGuid,
                @ReportType,
                UTC_TIMESTAMP(),
                @RawData
            );

            UPDATE masterserverinfo
            SET
                LastUpdated = UTC_TIMESTAMP(),
                LastHeartbeatAt = UTC_TIMESTAMP(),
                LastReportAt = UTC_TIMESTAMP(),
                LastReport = @RawData
            WHERE ServerGuid = @ServerGuid;
            """,
            new
            {
                ServerGuid = serverId,
                ReportType = reportType,
                RawData = report
            });

        return new ApiResponse(true, 0, "OK");
    }

    public async Task<object[]> ListAsync()
    {
        await using var db = _db.CreateConnection();
        await EnsureSchemaAsync(db);
        await CleanupStaleServersAsync(db);

        var servers = await db.QueryAsync<GameServerDbRow>(
            """
            SELECT
                ServerGuid AS ServerId,
                ServerName AS Name,
                RegionName AS Region,
                MapName AS Map,
                GameMode AS Mode,
                PublicAddress,
                PublicPort,
                MaxPlayers,
                CurrentPlayers,
                Status,
                CreatedAt,
                LastHeartbeatAt,
                LastReportAt,
                LastReport
            FROM masterserverinfo
            WHERE ServerGuid <> ''
              AND Status <> 'offline'
              AND LastHeartbeatAt >= DATE_SUB(UTC_TIMESTAMP(), INTERVAL 10 MINUTE)
            ORDER BY LastHeartbeatAt DESC, ServerName ASC;
            """);

        var result = new List<object>();

        foreach (var server in servers)
            result.Add(await BuildServerDtoAsync(db, server));

        return result.ToArray();
    }

    public async Task<bool> WriteRoundResultAsync(Dictionary<string, string> data, string remoteIp)
    {
        var customerId = ReadIntAny(data, 0, "CustomerID", "CustomerId", "customerId", "customerid", "s_id", "UserID", "userid");

        if (customerId <= 0)
            return true;

        var gameSessionId = ReadLongAny(data, 0, "GameSessionID", "GameSessionId", "gameSessionId", "g_id", "SessionID", "sessionId");
        var gamePoints = ReadIntAny(data, 0, "GamePoints", "GP", "gamePoints");
        var honorPoints = ReadIntAny(data, 0, "HonorPoints", "Honor", "HP", "XP", "Exp", "Experience");
        var skillPoints = ReadIntAny(data, 0, "SkillPoints", "SP", "skillPoints");
        var kills = ReadIntAny(data, 0, "Kills", "kills");
        var deaths = ReadIntAny(data, 0, "Deaths", "deaths");
        var shotsFired = ReadIntAny(data, 0, "ShotsFired", "shotsFired", "SF");
        var shotsHits = ReadIntAny(data, 0, "ShotsHits", "ShotsHit", "shotsHits", "SH");
        var headshots = ReadIntAny(data, 0, "Headshots", "HeadShots", "headshots");
        var assistKills = ReadIntAny(data, 0, "AssistKills", "Assists", "assistKills");
        var wins = ReadIntAny(data, 0, "Wins", "wins");
        var losses = ReadIntAny(data, 0, "Losses", "losses");
        var captureNeutralPoints = ReadIntAny(data, 0, "CaptureNeutralPoints", "CapNeutral", "captureNeutralPoints");
        var captureEnemyPoints = ReadIntAny(data, 0, "CaptureEnemyPoints", "CapEnemy", "captureEnemyPoints");
        var timePlayed = ReadIntAny(data, 0, "TimePlayed", "timePlayed", "PlayTime", "playTime");
        var gameDollars = ReadIntAny(data, 0, "GameDollars", "GD", "Dollars", "Money");
        var teamId = ReadIntAny(data, 0, "TeamID", "TeamId", "teamId");
        var mapId = ReadIntAny(data, 0, "MapID", "MapId", "mapId");
        var mapType = ReadIntAny(data, 0, "MapType", "mapType");

        await using var db = _db.CreateConnection();
        await EnsureSchemaAsync(db);

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            if (await TableExistsAsync(db, "dbg_userroundresults"))
            {
                await db.ExecuteAsync(
                    """
                    INSERT INTO dbg_userroundresults
                    (
                        IP,
                        GameSessionID,
                        CustomerID,
                        GamePoints,
                        HonorPoints,
                        SkillPoints,
                        Kills,
                        Deaths,
                        ShotsFired,
                        ShotsHits,
                        Headshots,
                        AssistKills,
                        Wins,
                        Losses,
                        CaptureNeutralPoints,
                        CaptureEnemyPoints,
                        TimePlayed,
                        GameReportTime,
                        GameDollars,
                        TeamID,
                        MapID,
                        MapType
                    )
                    VALUES
                    (
                        @IP,
                        @GameSessionID,
                        @CustomerID,
                        @GamePoints,
                        @HonorPoints,
                        @SkillPoints,
                        @Kills,
                        @Deaths,
                        @ShotsFired,
                        @ShotsHits,
                        @Headshots,
                        @AssistKills,
                        @Wins,
                        @Losses,
                        @CaptureNeutralPoints,
                        @CaptureEnemyPoints,
                        @TimePlayed,
                        UTC_TIMESTAMP(),
                        @GameDollars,
                        @TeamID,
                        @MapID,
                        @MapType
                    );
                    """,
                    new
                    {
                        IP = Trim(remoteIp, 32),
                        GameSessionID = gameSessionId,
                        CustomerID = customerId,
                        GamePoints = gamePoints,
                        HonorPoints = honorPoints,
                        SkillPoints = skillPoints,
                        Kills = kills,
                        Deaths = deaths,
                        ShotsFired = shotsFired,
                        ShotsHits = shotsHits,
                        Headshots = headshots,
                        AssistKills = assistKills,
                        Wins = wins,
                        Losses = losses,
                        CaptureNeutralPoints = captureNeutralPoints,
                        CaptureEnemyPoints = captureEnemyPoints,
                        TimePlayed = timePlayed,
                        GameDollars = gameDollars,
                        TeamID = teamId,
                        MapID = mapId,
                        MapType = mapType
                    },
                    tx);
            }

            if (await TableExistsAsync(db, "loginid"))
            {
                await db.ExecuteAsync(
                    """
                    UPDATE loginid
                    SET
                        GamePoints = GamePoints + @GamePoints,
                        HonorPoints = HonorPoints + @HonorPoints,
                        SkillPoints = SkillPoints + @SkillPoints,
                        GameDollars = GameDollars + @GameDollars,
                        lastgamedate = UTC_TIMESTAMP()
                    WHERE CustomerID = @CustomerID;
                    """,
                    new
                    {
                        CustomerID = customerId,
                        GamePoints = gamePoints,
                        HonorPoints = honorPoints,
                        SkillPoints = skillPoints,
                        GameDollars = gameDollars
                    },
                    tx);
            }

            if (await TableExistsAsync(db, "stats"))
            {
                var rows = await db.ExecuteAsync(
                    """
                    UPDATE stats
                    SET
                        Kills = Kills + @Kills,
                        Deaths = Deaths + @Deaths,
                        ShotsFired = ShotsFired + @ShotsFired,
                        ShotsHits = ShotsHits + @ShotsHits,
                        Headshots = Headshots + @Headshots,
                        AssistKills = AssistKills + @AssistKills,
                        Wins = Wins + @Wins,
                        Losses = Losses + @Losses,
                        CaptureNeutralPoints = CaptureNeutralPoints + @CaptureNeutralPoints,
                        CaptureEnemyPoints = CaptureEnemyPoints + @CaptureEnemyPoints,
                        TimePlayed = TimePlayed + @TimePlayed
                    WHERE CustomerID = @CustomerID;
                    """,
                    new
                    {
                        CustomerID = customerId,
                        Kills = kills,
                        Deaths = deaths,
                        ShotsFired = shotsFired,
                        ShotsHits = shotsHits,
                        Headshots = headshots,
                        AssistKills = assistKills,
                        Wins = wins,
                        Losses = losses,
                        CaptureNeutralPoints = captureNeutralPoints,
                        CaptureEnemyPoints = captureEnemyPoints,
                        TimePlayed = timePlayed
                    },
                    tx);

                if (rows <= 0)
                {
                    await db.ExecuteAsync(
                        """
                        INSERT INTO stats
                        (
                            CustomerID,
                            Kills,
                            Deaths,
                            ShotsFired,
                            ShotsHits,
                            Headshots,
                            AssistKills,
                            Wins,
                            Losses,
                            CaptureNeutralPoints,
                            CaptureEnemyPoints,
                            TimePlayed,
                            CheatAttempts
                        )
                        VALUES
                        (
                            @CustomerID,
                            @Kills,
                            @Deaths,
                            @ShotsFired,
                            @ShotsHits,
                            @Headshots,
                            @AssistKills,
                            @Wins,
                            @Losses,
                            @CaptureNeutralPoints,
                            @CaptureEnemyPoints,
                            @TimePlayed,
                            0
                        );
                        """,
                        new
                        {
                            CustomerID = customerId,
                            Kills = kills,
                            Deaths = deaths,
                            ShotsFired = shotsFired,
                            ShotsHits = shotsHits,
                            Headshots = headshots,
                            AssistKills = assistKills,
                            Wins = wins,
                            Losses = losses,
                            CaptureNeutralPoints = captureNeutralPoints,
                            CaptureEnemyPoints = captureEnemyPoints,
                            TimePlayed = timePlayed
                        },
                        tx);
                }
            }

            if (await TableExistsAsync(db, "leaderboard"))
            {
                var gamertag = Trim(ReadAny(data, "Gamertag", "gamertag", "gt", "name"), 32);

                if (string.IsNullOrWhiteSpace(gamertag))
                    gamertag = await ResolveGamertagAsync(db, customerId, tx);

                var rows = await db.ExecuteAsync(
                    """
                    UPDATE leaderboard
                    SET
                        gamertag = @Gamertag,
                        HonorPoints = HonorPoints + @HonorPoints,
                        Kills = Kills + @Kills,
                        Deaths = Deaths + @Deaths,
                        ShotsFired = ShotsFired + @ShotsFired,
                        ShotsHit = ShotsHit + @ShotsHits,
                        TimePlayed = TimePlayed + @TimePlayed,
                        Wins = Wins + @Wins,
                        Losses = Losses + @Losses
                    WHERE CustomerID = @CustomerID;
                    """,
                    new
                    {
                        CustomerID = customerId,
                        Gamertag = gamertag,
                        HonorPoints = honorPoints,
                        Kills = kills,
                        Deaths = deaths,
                        ShotsFired = shotsFired,
                        ShotsHits = shotsHits,
                        TimePlayed = timePlayed,
                        Wins = wins,
                        Losses = losses
                    },
                    tx);

                if (rows <= 0)
                {
                    await db.ExecuteAsync(
                        """
                        INSERT INTO leaderboard
                        (
                            CustomerID,
                            gamertag,
                            Rank,
                            HonorPoints,
                            Wins,
                            Losses,
                            Kills,
                            Deaths,
                            ShotsFired,
                            ShotsHit,
                            TimePlayed,
                            HavePremium
                        )
                        VALUES
                        (
                            @CustomerID,
                            @Gamertag,
                            0,
                            @HonorPoints,
                            @Wins,
                            @Losses,
                            @Kills,
                            @Deaths,
                            @ShotsFired,
                            @ShotsHits,
                            @TimePlayed,
                            0
                        );
                        """,
                        new
                        {
                            CustomerID = customerId,
                            Gamertag = gamertag,
                            HonorPoints = honorPoints,
                            Wins = wins,
                            Losses = losses,
                            Kills = kills,
                            Deaths = deaths,
                            ShotsFired = shotsFired,
                            ShotsHits = shotsHits,
                            TimePlayed = timePlayed
                        },
                        tx);
                }
            }

            await tx.CommitAsync();
            return true;
        }
        catch
        {
            await tx.RollbackAsync();
            return false;
        }
    }

    public async Task<bool> WriteWeaponStatsAsync(Dictionary<string, string> data)
    {
        var itemId = ReadIntAny(data, 0, "ItemID", "ItemId", "itemId", "WeaponID", "WeaponId", "weaponId");
        var customerId = ReadIntAny(data, 0, "CustomerID", "CustomerId", "customerId", "s_id");
        var shotsFired = ReadIntAny(data, 0, "ShotsFired", "shotsFired", "SF");
        var shotsHits = ReadIntAny(data, 0, "ShotsHits", "ShotsHit", "shotsHits", "SH");
        var killsCq = ReadIntAny(data, -1, "KillsCQ", "killsCQ");
        var killsDm = ReadIntAny(data, 0, "KillsDM", "killsDM");
        var killsSb = ReadIntAny(data, 0, "KillsSB", "killsSB");

        if (killsCq < 0)
            killsCq = ReadIntAny(data, 0, "Kills", "kills");

        if (itemId <= 0)
            return true;

        await using var db = _db.CreateConnection();
        await EnsureSchemaAsync(db);

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            await db.ExecuteAsync(
                """
                INSERT INTO gameserver_weaponstats_log
                (
                    ItemID,
                    CustomerID,
                    ShotsFired,
                    ShotsHits,
                    KillsCQ,
                    KillsDM,
                    KillsSB,
                    ReportTime,
                    RawData
                )
                VALUES
                (
                    @ItemID,
                    @CustomerID,
                    @ShotsFired,
                    @ShotsHits,
                    @KillsCQ,
                    @KillsDM,
                    @KillsSB,
                    UTC_TIMESTAMP(),
                    @RawData
                );
                """,
                new
                {
                    ItemID = itemId,
                    CustomerID = customerId,
                    ShotsFired = shotsFired,
                    ShotsHits = shotsHits,
                    KillsCQ = killsCq,
                    KillsDM = killsDm,
                    KillsSB = killsSb,
                    RawData = Trim(BuildDataString(data), 65535)
                },
                tx);

            if (await TableExistsAsync(db, "items_weapons"))
            {
                await db.ExecuteAsync(
                    """
                    UPDATE items_weapons
                    SET
                        ShotsFired = ShotsFired + @ShotsFired,
                        ShotsHits = ShotsHits + @ShotsHits,
                        KillsCQ = KillsCQ + @KillsCQ,
                        KillsDM = KillsDM + @KillsDM,
                        KillsSB = KillsSB + @KillsSB
                    WHERE ItemID = @ItemID;
                    """,
                    new
                    {
                        ItemID = itemId,
                        ShotsFired = shotsFired,
                        ShotsHits = shotsHits,
                        KillsCQ = killsCq,
                        KillsDM = killsDm,
                        KillsSB = killsSb
                    },
                    tx);
            }

            await tx.CommitAsync();
            return true;
        }
        catch
        {
            await tx.RollbackAsync();
            return false;
        }
    }

    public async Task<bool> UpdateAchievementsAsync(Dictionary<string, string> data)
    {
        var customerId = ReadIntAny(data, 0, "CustomerID", "CustomerId", "customerId", "s_id");

        if (customerId <= 0)
            return true;

        var updates = ReadAchievementUpdates(data);

        if (updates.Count == 0)
            return true;

        await using var db = _db.CreateConnection();

        if (!await TableExistsAsync(db, "achievements"))
            return true;

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            foreach (var update in updates)
            {
                if (update.AchId <= 0)
                    continue;

                var rows = await db.ExecuteAsync(
                    """
                    UPDATE achievements
                    SET
                        Value = @Value,
                        Unlocked = @Unlocked
                    WHERE CustomerID = @CustomerID
                      AND AchID = @AchID;
                    """,
                    new
                    {
                        CustomerID = customerId,
                        AchID = update.AchId,
                        Value = update.Value,
                        Unlocked = update.Unlocked
                    },
                    tx);

                if (rows > 0)
                    continue;

                await db.ExecuteAsync(
                    """
                    INSERT INTO achievements
                    (
                        CustomerID,
                        AchID,
                        Value,
                        Unlocked
                    )
                    VALUES
                    (
                        @CustomerID,
                        @AchID,
                        @Value,
                        @Unlocked
                    );
                    """,
                    new
                    {
                        CustomerID = customerId,
                        AchID = update.AchId,
                        Value = update.Value,
                        Unlocked = update.Unlocked
                    },
                    tx);
            }

            await tx.CommitAsync();
            return true;
        }
        catch
        {
            await tx.RollbackAsync();
            return false;
        }
    }

    public async Task<bool> WriteCheatAttemptAsync(Dictionary<string, string> data)
    {
        var customerId = ReadIntAny(data, 0, "CustomerID", "CustomerId", "customerId", "s_id");
        var sessionId = ReadLongAny(data, 0, "SessionID", "sessionId", "GameSessionID", "gameSessionId");
        var cheatId = ReadIntAny(data, 0, "CheatID", "cheatId", "Cheat", "cheat");

        await using var db = _db.CreateConnection();

        await db.OpenAsync();
        await using var tx = await db.BeginTransactionAsync();

        try
        {
            if (await TableExistsAsync(db, "cheatlog"))
            {
                await db.ExecuteAsync(
                    """
                    INSERT INTO cheatlog
                    (
                        SessionID,
                        CustomerID,
                        CheatID,
                        ReportTime
                    )
                    VALUES
                    (
                        @SessionID,
                        @CustomerID,
                        @CheatID,
                        UTC_TIMESTAMP()
                    );
                    """,
                    new
                    {
                        SessionID = sessionId,
                        CustomerID = customerId,
                        CheatID = cheatId
                    },
                    tx);
            }

            if (customerId > 0 && await TableExistsAsync(db, "stats"))
            {
                var rows = await db.ExecuteAsync(
                    """
                    UPDATE stats
                    SET CheatAttempts = CheatAttempts + 1
                    WHERE CustomerID = @CustomerID;
                    """,
                    new { CustomerID = customerId },
                    tx);

                if (rows <= 0)
                {
                    await db.ExecuteAsync(
                        """
                        INSERT INTO stats
                        (
                            CustomerID,
                            Kills,
                            Deaths,
                            ShotsFired,
                            ShotsHits,
                            Headshots,
                            AssistKills,
                            Wins,
                            Losses,
                            CaptureNeutralPoints,
                            CaptureEnemyPoints,
                            TimePlayed,
                            CheatAttempts
                        )
                        VALUES
                        (
                            @CustomerID,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            1
                        );
                        """,
                        new { CustomerID = customerId },
                        tx);
                }
            }

            await tx.CommitAsync();
            return true;
        }
        catch
        {
            await tx.RollbackAsync();
            return false;
        }
    }

    private async Task EnsureSchemaAsync(MySqlConnection db)
    {
        await db.ExecuteAsync(
            """
            CREATE TABLE IF NOT EXISTS masterserverinfo
            (
                ServerID int(10) NOT NULL,
                LastUpdated datetime NOT NULL,
                CreateGameKey int(10) NOT NULL,
                IP varchar(64) NOT NULL,
                ServerGuid varchar(64) NOT NULL DEFAULT '',
                ServerName varchar(128) NOT NULL DEFAULT '',
                RegionName varchar(32) NOT NULL DEFAULT '',
                MapName varchar(64) NOT NULL DEFAULT '',
                GameMode varchar(32) NOT NULL DEFAULT '',
                PublicAddress varchar(64) NOT NULL DEFAULT '',
                PublicPort int(10) NOT NULL DEFAULT 0,
                MaxPlayers int(10) NOT NULL DEFAULT 0,
                CurrentPlayers int(10) NOT NULL DEFAULT 0,
                Status varchar(16) NOT NULL DEFAULT 'offline',
                CreatedAt datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
                LastHeartbeatAt datetime NOT NULL DEFAULT '1970-01-01 00:00:01',
                LastReportAt datetime DEFAULT NULL,
                LastReport mediumtext DEFAULT NULL
            ) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
            """);

        await AddColumnIfMissingAsync(db, "masterserverinfo", "ServerGuid", "varchar(64) NOT NULL DEFAULT ''");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "ServerName", "varchar(128) NOT NULL DEFAULT ''");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "RegionName", "varchar(32) NOT NULL DEFAULT ''");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "MapName", "varchar(64) NOT NULL DEFAULT ''");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "GameMode", "varchar(32) NOT NULL DEFAULT ''");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "PublicAddress", "varchar(64) NOT NULL DEFAULT ''");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "PublicPort", "int(10) NOT NULL DEFAULT 0");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "MaxPlayers", "int(10) NOT NULL DEFAULT 0");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "CurrentPlayers", "int(10) NOT NULL DEFAULT 0");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "Status", "varchar(16) NOT NULL DEFAULT 'offline'");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "CreatedAt", "datetime NOT NULL DEFAULT CURRENT_TIMESTAMP");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "LastHeartbeatAt", "datetime NOT NULL DEFAULT '1970-01-01 00:00:01'");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "LastReportAt", "datetime DEFAULT NULL");
        await AddColumnIfMissingAsync(db, "masterserverinfo", "LastReport", "mediumtext DEFAULT NULL");

        await db.ExecuteAsync(
            """
            UPDATE masterserverinfo
            SET ServerGuid = CONCAT('legacy-', ServerID)
            WHERE ServerGuid = '';

            UPDATE masterserverinfo
            SET PublicAddress = IP
            WHERE PublicAddress = '';

            UPDATE masterserverinfo
            SET CreatedAt = LastUpdated
            WHERE CreatedAt <= '1970-01-01 00:00:01';

            UPDATE masterserverinfo
            SET LastHeartbeatAt = LastUpdated
            WHERE LastHeartbeatAt <= '1970-01-01 00:00:01';
            """);

        await CreateIndexIfMissingAsync(db, "masterserverinfo", "idx_masterserverinfo_serverguid", "ServerGuid");
        await CreateIndexIfMissingAsync(db, "masterserverinfo", "idx_masterserverinfo_heartbeat", "LastHeartbeatAt");

        await db.ExecuteAsync(
            """
            CREATE TABLE IF NOT EXISTS gameserver_activeplayers
            (
                ServerGuid varchar(64) NOT NULL,
                CustomerID int(10) NOT NULL,
                Gamertag varchar(32) NOT NULL DEFAULT '',
                JoinedAt datetime NOT NULL,
                LastSeenAt datetime NOT NULL,
                PRIMARY KEY (ServerGuid, CustomerID),
                KEY idx_gameserver_activeplayers_customer (CustomerID)
            ) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
            """);

        await db.ExecuteAsync(
            """
            CREATE TABLE IF NOT EXISTS gameserver_reports
            (
                ReportID bigint(20) NOT NULL AUTO_INCREMENT,
                ServerGuid varchar(64) NOT NULL,
                ReportType varchar(32) NOT NULL,
                ReportTime datetime NOT NULL,
                RawData mediumtext DEFAULT NULL,
                PRIMARY KEY (ReportID),
                KEY idx_gameserver_reports_server_time (ServerGuid, ReportTime),
                KEY idx_gameserver_reports_type_time (ReportType, ReportTime)
            ) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
            """);

        await db.ExecuteAsync(
            """
            CREATE TABLE IF NOT EXISTS gameserver_weaponstats_log
            (
                ID bigint(20) NOT NULL AUTO_INCREMENT,
                ItemID int(10) NOT NULL,
                CustomerID int(10) NOT NULL DEFAULT 0,
                ShotsFired int(10) NOT NULL DEFAULT 0,
                ShotsHits int(10) NOT NULL DEFAULT 0,
                KillsCQ int(10) NOT NULL DEFAULT 0,
                KillsDM int(10) NOT NULL DEFAULT 0,
                KillsSB int(10) NOT NULL DEFAULT 0,
                ReportTime datetime NOT NULL,
                RawData mediumtext DEFAULT NULL,
                PRIMARY KEY (ID),
                KEY idx_gameserver_weaponstats_item (ItemID),
                KEY idx_gameserver_weaponstats_customer (CustomerID),
                KEY idx_gameserver_weaponstats_time (ReportTime)
            ) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;
            """);
    }

    private static async Task AddColumnIfMissingAsync(
        MySqlConnection db,
        string table,
        string column,
        string definition)
    {
        var exists = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM information_schema.columns
            WHERE table_schema = DATABASE()
              AND table_name = @Table
              AND column_name = @Column;
            """,
            new
            {
                Table = table,
                Column = column
            });

        if (exists > 0)
            return;

        await db.ExecuteAsync($"ALTER TABLE `{table}` ADD COLUMN `{column}` {definition};");
    }

    private static async Task CreateIndexIfMissingAsync(
        MySqlConnection db,
        string table,
        string index,
        string column)
    {
        var exists = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM information_schema.statistics
            WHERE table_schema = DATABASE()
              AND table_name = @Table
              AND index_name = @IndexName;
            """,
            new
            {
                Table = table,
                IndexName = index
            });

        if (exists > 0)
            return;

        await db.ExecuteAsync($"CREATE INDEX `{index}` ON `{table}` (`{column}`);");
    }

    private static async Task CleanupStaleServersAsync(MySqlConnection db)
    {
        await db.ExecuteAsync(
            """
            UPDATE masterserverinfo
            SET
                Status = 'offline',
                CurrentPlayers = 0
            WHERE ServerGuid <> ''
              AND LastHeartbeatAt < DATE_SUB(UTC_TIMESTAMP(), INTERVAL 10 MINUTE);

            DELETE p
            FROM gameserver_activeplayers p
            INNER JOIN masterserverinfo s ON s.ServerGuid = p.ServerGuid
            WHERE s.LastHeartbeatAt < DATE_SUB(UTC_TIMESTAMP(), INTERVAL 10 MINUTE)
               OR s.Status = 'offline';
            """);
    }

    private static async Task<bool> ServerExistsAsync(MySqlConnection db, string serverId)
    {
        var exists = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM masterserverinfo
            WHERE ServerGuid = @ServerGuid;
            """,
            new { ServerGuid = serverId });

        return exists > 0;
    }

    private static async Task<object?> GetServerDtoAsync(MySqlConnection db, string serverId)
    {
        var row = await db.QueryFirstOrDefaultAsync<GameServerDbRow>(
            """
            SELECT
                ServerGuid AS ServerId,
                ServerName AS Name,
                RegionName AS Region,
                MapName AS Map,
                GameMode AS Mode,
                PublicAddress,
                PublicPort,
                MaxPlayers,
                CurrentPlayers,
                Status,
                CreatedAt,
                LastHeartbeatAt,
                LastReportAt,
                LastReport
            FROM masterserverinfo
            WHERE ServerGuid = @ServerGuid
            LIMIT 1;
            """,
            new { ServerGuid = serverId });

        if (row == null)
            return null;

        return await BuildServerDtoAsync(db, row);
    }

    private static async Task<object> BuildServerDtoAsync(MySqlConnection db, GameServerDbRow row)
    {
        var players = await db.QueryAsync<GameServerPlayerDbRow>(
            """
            SELECT
                CustomerID,
                Gamertag,
                JoinedAt,
                LastSeenAt
            FROM gameserver_activeplayers
            WHERE ServerGuid = @ServerGuid
            ORDER BY JoinedAt ASC;
            """,
            new { ServerGuid = row.ServerId });

        return new
        {
            serverId = row.ServerId,
            name = row.Name,
            region = row.Region,
            map = row.Map,
            mode = row.Mode,
            publicAddress = row.PublicAddress,
            publicPort = row.PublicPort,
            maxPlayers = row.MaxPlayers,
            currentPlayers = row.CurrentPlayers,
            status = row.Status,
            createdAt = row.CreatedAt,
            lastHeartbeatAt = row.LastHeartbeatAt,
            lastReportAt = row.LastReportAt,
            lastReport = row.LastReport,
            players = players.Select(x => new
            {
                customerId = x.CustomerID,
                gamertag = x.Gamertag,
                joinedAt = x.JoinedAt,
                lastSeenAt = x.LastSeenAt
            }).ToArray()
        };
    }

    private static async Task<string> ResolveGamertagAsync(
        MySqlConnection db,
        int customerId,
        MySqlTransaction? tx = null)
    {
        if (customerId <= 0)
            return "";

        if (!await TableExistsAsync(db, "loginid"))
            return "";

        var value = await db.ExecuteScalarAsync<string?>(
            """
            SELECT COALESCE(NULLIF(Gamertag, ''), NULLIF(AccountName, ''), CONCAT('Player', CustomerID))
            FROM loginid
            WHERE CustomerID = @CustomerID
            LIMIT 1;
            """,
            new { CustomerID = customerId },
            tx);

        return Trim(value ?? "", 32);
    }

    private static async Task<bool> TableExistsAsync(MySqlConnection db, string table)
    {
        var count = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM information_schema.tables
            WHERE table_schema = DATABASE()
              AND table_name = @Table;
            """,
            new { Table = table });

        return count > 0;
    }

    private static List<AchievementUpdate> ReadAchievementUpdates(Dictionary<string, string> data)
    {
        var result = new List<AchievementUpdate>();

        var singleAchId = ReadIntAny(
            data,
            0,
            "AchID",
            "AchId",
            "achId",
            "AchievementID",
            "achievementId",
            "id");

        var singleValue = ReadIntAny(
            data,
            0,
            "AchVal",
            "AchValue",
            "AchVAL",
            "Value",
            "value",
            "Val",
            "val",
            "v");

        var singleUnlocked = ReadIntAny(
            data,
            0,
            "AchUnl",
            "AchUnlocked",
            "AchUNL",
            "Unlocked",
            "unlocked",
            "u");

        if (singleAchId > 0)
            result.Add(new AchievementUpdate(singleAchId, singleValue, singleUnlocked));

        var numAch = ReadIntAny(data, 256, "NumAch", "numAch", "NumAchievements", "numAchievements");

        if (numAch <= 0)
            numAch = 256;

        if (numAch > 256)
            numAch = 256;

        for (var i = 0; i < numAch; i++)
        {
            var achId = ReadIntAny(
                data,
                0,
                "AchID" + i,
                "AchId" + i,
                "achId" + i,
                "AchievementID" + i,
                "AID" + i,
                "aid" + i,
                "A" + i,
                "a" + i);

            if (achId <= 0)
                continue;

            var value = ReadIntAny(
                data,
                0,
                "AchVal" + i,
                "AchValue" + i,
                "AchVAL" + i,
                "Value" + i,
                "value" + i,
                "Val" + i,
                "val" + i,
                "V" + i,
                "v" + i);

            var unlocked = ReadIntAny(
                data,
                0,
                "AchUnl" + i,
                "AchUnlocked" + i,
                "AchUNL" + i,
                "Unlocked" + i,
                "unlocked" + i,
                "U" + i,
                "u" + i);

            result.Add(new AchievementUpdate(achId, value, unlocked));
        }

        return result
            .GroupBy(x => x.AchId)
            .Select(x => x.Last())
            .ToList();
    }

    private static string NormalizeServerId(string value)
    {
        return Trim(value ?? "", 64);
    }

    private static int StableServerIntId(string serverId)
    {
        unchecked
        {
            var hash = 23;

            foreach (var ch in serverId)
                hash = hash * 31 + ch;

            if (hash == int.MinValue)
                hash = 1;

            hash = Math.Abs(hash);

            if (hash == 0)
                hash = 1;

            return hash;
        }
    }

    private static int ClampCurrentPlayers(int currentPlayers, int maxPlayers)
    {
        currentPlayers = Math.Max(0, currentPlayers);

        if (maxPlayers <= 0)
            return currentPlayers;

        return Math.Clamp(currentPlayers, 0, maxPlayers);
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
            return parsed;
        }

        return defaultValue;
    }

    private static long ReadLongAny(
        Dictionary<string, string> data,
        long defaultValue,
        params string[] keys)
    {
        foreach (var key in keys)
        {
            if (!data.TryGetValue(key, out var value))
                continue;

            if (long.TryParse(value, out var parsed))
                return parsed;

            return defaultValue;
        }

        return defaultValue;
    }

    private static string BuildDataString(Dictionary<string, string> data)
    {
        var sb = new System.Text.StringBuilder();
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

    private static string Trim(string? value, int max)
    {
        value ??= "";
        value = value.Trim();

        if (value.Length <= max)
            return value;

        return value[..max];
    }

    private sealed class GameServerDbRow
    {
        public string ServerId { get; set; } = "";
        public string Name { get; set; } = "";
        public string Region { get; set; } = "";
        public string Map { get; set; } = "";
        public string Mode { get; set; } = "";
        public string PublicAddress { get; set; } = "";
        public int PublicPort { get; set; }
        public int MaxPlayers { get; set; }
        public int CurrentPlayers { get; set; }
        public string Status { get; set; } = "";
        public DateTime CreatedAt { get; set; }
        public DateTime LastHeartbeatAt { get; set; }
        public DateTime? LastReportAt { get; set; }
        public string? LastReport { get; set; }
    }

    private sealed class GameServerPlayerDbRow
    {
        public int CustomerID { get; set; }
        public string Gamertag { get; set; } = "";
        public DateTime JoinedAt { get; set; }
        public DateTime LastSeenAt { get; set; }
    }

    private sealed record AchievementUpdate(
        int AchId,
        int Value,
        int Unlocked);
}

public record GameServerRegisterRequest(
    string ServerId,
    string Name,
    string Region,
    string Map,
    string Mode,
    string PublicAddress,
    int PublicPort,
    int MaxPlayers,
    int CurrentPlayers,
    int CreateGameKey = 0);

public record GameServerHeartbeatRequest(
    string ServerId,
    int CurrentPlayers,
    string? Status);

public record GameServerIdRequest(
    string ServerId);

public record GameServerPlayerRequest(
    string ServerId,
    ulong CustomerId,
    string? Gamertag);

public record GameServerReportRequest(
    string ServerId,
    string? Report,
    string? ReportType = null);