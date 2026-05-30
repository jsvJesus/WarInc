using System.Collections.Concurrent;
using Microsoft.Extensions.Options;
using WarInc.Api.Common;
using WarInc.Api.Config;

namespace WarInc.Api.GameServer;

public static class GameServerEndpoints
{
    public static void MapGameServerEndpoints(this WebApplication app)
    {
        app.MapPost("/internal/gameserver/register", (
            HttpContext http,
            GameServerRegisterRequest request,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = service.Register(request);
            return Results.Json(result);
        });

        app.MapPost("/internal/gameserver/heartbeat", (
            HttpContext http,
            GameServerHeartbeatRequest request,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = service.Heartbeat(request);
            return Results.Json(result);
        });

        app.MapPost("/internal/gameserver/unregister", (
            HttpContext http,
            GameServerIdRequest request,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = service.Unregister(request.ServerId);
            return Results.Json(result);
        });

        app.MapPost("/internal/gameserver/player/join", (
            HttpContext http,
            GameServerPlayerRequest request,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = service.PlayerJoin(request);
            return Results.Json(result);
        });

        app.MapPost("/internal/gameserver/player/leave", (
            HttpContext http,
            GameServerPlayerRequest request,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = service.PlayerLeave(request);
            return Results.Json(result);
        });

        app.MapPost("/internal/gameserver/report", (
            HttpContext http,
            GameServerReportRequest request,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = service.Report(request);
            return Results.Json(result);
        });

        app.MapGet("/internal/gameserver/list", (
            HttpContext http,
            GameServerService service,
            IOptions<WarIncOptions> options) =>
        {
            if (!IsInternalRequest(http, options))
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            return Results.Json(new
            {
                ok = true,
                code = 0,
                message = "OK",
                servers = service.List()
            });
        });
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
    private readonly ConcurrentDictionary<string, GameServerState> _servers = new();

    public object Register(GameServerRegisterRequest request)
    {
        if (string.IsNullOrWhiteSpace(request.ServerId))
            return new ApiResponse(false, 400, "BAD_SERVER_ID");

        var now = DateTimeOffset.UtcNow;

        var state = new GameServerState
        {
            ServerId = request.ServerId.Trim(),
            Name = request.Name?.Trim() ?? "",
            Region = request.Region?.Trim() ?? "",
            Map = request.Map?.Trim() ?? "",
            Mode = request.Mode?.Trim() ?? "",
            PublicAddress = request.PublicAddress?.Trim() ?? "",
            PublicPort = request.PublicPort,
            MaxPlayers = Math.Max(0, request.MaxPlayers),
            CurrentPlayers = Math.Clamp(request.CurrentPlayers, 0, Math.Max(0, request.MaxPlayers)),
            CreatedAt = now,
            LastHeartbeatAt = now,
            Players = new ConcurrentDictionary<ulong, GameServerPlayerState>()
        };

        _servers[state.ServerId] = state;

        return new
        {
            ok = true,
            code = 0,
            message = "OK",
            server = state.ToDto()
        };
    }

    public object Heartbeat(GameServerHeartbeatRequest request)
    {
        if (!_servers.TryGetValue(request.ServerId, out var state))
            return new ApiResponse(false, 404, "SERVER_NOT_FOUND");

        state.CurrentPlayers = Math.Clamp(request.CurrentPlayers, 0, state.MaxPlayers);
        state.LastHeartbeatAt = DateTimeOffset.UtcNow;
        state.Status = string.IsNullOrWhiteSpace(request.Status) ? "online" : request.Status.Trim();

        return new
        {
            ok = true,
            code = 0,
            message = "OK",
            server = state.ToDto()
        };
    }

    public object Unregister(string serverId)
    {
        if (string.IsNullOrWhiteSpace(serverId))
            return new ApiResponse(false, 400, "BAD_SERVER_ID");

        _servers.TryRemove(serverId, out _);

        return new ApiResponse(true, 0, "OK");
    }

    public object PlayerJoin(GameServerPlayerRequest request)
    {
        if (!_servers.TryGetValue(request.ServerId, out var state))
            return new ApiResponse(false, 404, "SERVER_NOT_FOUND");

        if (request.CustomerId == 0)
            return new ApiResponse(false, 400, "BAD_CUSTOMER_ID");

        state.Players[request.CustomerId] = new GameServerPlayerState
        {
            CustomerId = request.CustomerId,
            Gamertag = request.Gamertag?.Trim() ?? "",
            JoinedAt = DateTimeOffset.UtcNow
        };

        state.CurrentPlayers = state.Players.Count;
        state.LastHeartbeatAt = DateTimeOffset.UtcNow;

        return new
        {
            ok = true,
            code = 0,
            message = "OK",
            server = state.ToDto()
        };
    }

    public object PlayerLeave(GameServerPlayerRequest request)
    {
        if (!_servers.TryGetValue(request.ServerId, out var state))
            return new ApiResponse(false, 404, "SERVER_NOT_FOUND");

        if (request.CustomerId == 0)
            return new ApiResponse(false, 400, "BAD_CUSTOMER_ID");

        state.Players.TryRemove(request.CustomerId, out _);
        state.CurrentPlayers = state.Players.Count;
        state.LastHeartbeatAt = DateTimeOffset.UtcNow;

        return new
        {
            ok = true,
            code = 0,
            message = "OK",
            server = state.ToDto()
        };
    }

    public object Report(GameServerReportRequest request)
    {
        if (!_servers.TryGetValue(request.ServerId, out var state))
            return new ApiResponse(false, 404, "SERVER_NOT_FOUND");

        state.LastReportAt = DateTimeOffset.UtcNow;
        state.LastReport = request.Report ?? "";
        state.LastHeartbeatAt = DateTimeOffset.UtcNow;

        return new ApiResponse(true, 0, "OK");
    }

    public object[] List()
    {
        var now = DateTimeOffset.UtcNow;

        foreach (var pair in _servers.ToArray())
        {
            if (now - pair.Value.LastHeartbeatAt > TimeSpan.FromMinutes(10))
                _servers.TryRemove(pair.Key, out _);
        }

        return _servers.Values
            .OrderByDescending(x => x.LastHeartbeatAt)
            .Select(x => x.ToDto())
            .ToArray();
    }
}

public sealed class GameServerState
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
    public string Status { get; set; } = "online";
    public DateTimeOffset CreatedAt { get; set; }
    public DateTimeOffset LastHeartbeatAt { get; set; }
    public DateTimeOffset? LastReportAt { get; set; }
    public string LastReport { get; set; } = "";
    public ConcurrentDictionary<ulong, GameServerPlayerState> Players { get; set; } = new();

    public object ToDto()
    {
        return new
        {
            serverId = ServerId,
            name = Name,
            region = Region,
            map = Map,
            mode = Mode,
            publicAddress = PublicAddress,
            publicPort = PublicPort,
            maxPlayers = MaxPlayers,
            currentPlayers = CurrentPlayers,
            status = Status,
            createdAt = CreatedAt,
            lastHeartbeatAt = LastHeartbeatAt,
            lastReportAt = LastReportAt,
            players = Players.Values
                .OrderBy(x => x.JoinedAt)
                .Select(x => new
                {
                    customerId = x.CustomerId,
                    gamertag = x.Gamertag,
                    joinedAt = x.JoinedAt
                })
                .ToArray()
        };
    }
}

public sealed class GameServerPlayerState
{
    public ulong CustomerId { get; set; }
    public string Gamertag { get; set; } = "";
    public DateTimeOffset JoinedAt { get; set; }
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
    int CurrentPlayers);

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
    string? Report);