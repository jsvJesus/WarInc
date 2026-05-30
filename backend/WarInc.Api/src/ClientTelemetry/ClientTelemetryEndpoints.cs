using System.Text;
using System.Text.Json;
using WarInc.Api.Auth;
using WarInc.Api.Common;

namespace WarInc.Api.ClientTelemetry;

public static class ClientTelemetryEndpoints
{
    public static void MapClientTelemetryEndpoints(this WebApplication app)
    {
        MapLegacy(app, "/api_LoginSessionPoller.aspx", LegacyLoginSessionPollerAsync);
        MapLegacy(app, "/api_SetRSUpdateStatus.aspx", LegacySetRsUpdateStatusAsync);
        MapLegacy(app, "/api_ReportHWInfo.aspx", LegacyReportHardwareAsync);
        MapLegacy(app, "/api_ReportHWInfo_Customer.aspx", LegacyReportHardwareAsync);
        MapLegacy(app, "/api_ClientUpdateAchievements.aspx", LegacyUpdateAchievementsAsync);

        app.MapGet("/v1/session/poll", JsonSessionPollerAsync);
        app.MapPost("/v1/session/poll", JsonSessionPollerAsync);

        app.MapGet("/v1/client/status", JsonSetRsUpdateStatusAsync);
        app.MapPost("/v1/client/status", JsonSetRsUpdateStatusAsync);

        app.MapGet("/v1/hardware/report", JsonReportHardwareAsync);
        app.MapPost("/v1/hardware/report", JsonReportHardwareAsync);

        app.MapGet("/v1/achievements/update", JsonUpdateAchievementsAsync);
        app.MapPost("/v1/achievements/update", JsonUpdateAchievementsAsync);
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

    private static async Task<IResult> LegacyLoginSessionPollerAsync(
        HttpContext http,
        AuthService auth)
    {
        var data = await ReadRequestDataAsync(http);

        var customerId = LegacyUtil.ParseULong(ReadAny(
            data,
            "s_id",
            "CustomerID",
            "CustomerId",
            "customerId"));

        var sessionId = LegacyUtil.ParseULong(ReadAny(
            data,
            "s_key",
            "SessionID",
            "SessionId",
            "sessionId"));

        if (customerId == 0 || sessionId == 0)
            return Text("WO_0");

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);

        if (!session.Ok)
            return Text("WO_1");

        await auth.TouchLegacySessionAsync(customerId, sessionId);

        return Text("WO_0");
    }

    private static async Task<IResult> LegacySetRsUpdateStatusAsync(
        HttpContext http,
        AuthService auth,
        ClientTelemetryService telemetry)
    {
        var data = await ReadRequestDataAsync(http);

        if (!await CheckLegacySessionIfPresentAsync(data, auth))
            return Text("WO_1");

        await telemetry.LogClientStatusAsync(data, GetRemoteIp(http));

        return Text("WO_0");
    }

    private static async Task<IResult> LegacyReportHardwareAsync(
        HttpContext http,
        AuthService auth,
        ClientTelemetryService telemetry)
    {
        var data = await ReadRequestDataAsync(http);

        if (!await CheckLegacySessionIfPresentAsync(data, auth))
            return Text("WO_1");

        await telemetry.ReportHardwareAsync(data, GetRemoteIp(http));

        return Text("WO_0");
    }

    private static async Task<IResult> LegacyUpdateAchievementsAsync(
        HttpContext http,
        AuthService auth,
        ClientTelemetryService telemetry)
    {
        var data = await ReadRequestDataAsync(http);

        if (!await CheckLegacySessionIfPresentAsync(data, auth))
            return Text("WO_1");

        var ok = await telemetry.UpdateAchievementsAsync(data);

        return Text(ok ? "WO_0" : "WO_5");
    }

    private static async Task<IResult> JsonSessionPollerAsync(
        HttpContext http,
        AuthService auth)
    {
        var data = await ReadRequestDataAsync(http);

        var customerId = LegacyUtil.ParseULong(ReadAny(
            data,
            "customerId",
            "CustomerId",
            "CustomerID",
            "s_id"));

        var sessionId = LegacyUtil.ParseULong(ReadAny(
            data,
            "sessionId",
            "SessionId",
            "SessionID",
            "s_key"));

        var token = ReadAny(
            data,
            "token",
            "Token",
            "s_token");

        if (customerId == 0 || sessionId == 0)
            return Results.Json(new { ok = false, code = 400, message = "BAD_SESSION" });

        if (string.IsNullOrWhiteSpace(token))
        {
            var legacySession = await auth.CheckLegacySessionAsync(customerId, sessionId);

            if (!legacySession.Ok)
                return Results.Json(new { ok = false, code = legacySession.Code, message = legacySession.Message });

            await auth.TouchLegacySessionAsync(customerId, sessionId);

            return Results.Json(new { ok = true, code = 0, message = "OK" });
        }

        var session = await auth.CheckSessionAsync(customerId, sessionId, token);

        if (!session.Ok)
            return Results.Json(new { ok = false, code = session.Code, message = session.Message });

        await auth.TouchLegacySessionAsync(customerId, sessionId);

        return Results.Json(new { ok = true, code = 0, message = "OK" });
    }

    private static async Task<IResult> JsonSetRsUpdateStatusAsync(
        HttpContext http,
        AuthService auth,
        ClientTelemetryService telemetry)
    {
        var data = await ReadRequestDataAsync(http);

        if (!await CheckJsonOrLegacySessionIfPresentAsync(data, auth))
            return Results.Json(new { ok = false, code = 401, message = "BAD_SESSION" });

        await telemetry.LogClientStatusAsync(data, GetRemoteIp(http));

        return Results.Json(new { ok = true, code = 0, message = "OK" });
    }

    private static async Task<IResult> JsonReportHardwareAsync(
        HttpContext http,
        AuthService auth,
        ClientTelemetryService telemetry)
    {
        var data = await ReadRequestDataAsync(http);

        if (!await CheckJsonOrLegacySessionIfPresentAsync(data, auth))
            return Results.Json(new { ok = false, code = 401, message = "BAD_SESSION" });

        await telemetry.ReportHardwareAsync(data, GetRemoteIp(http));

        return Results.Json(new { ok = true, code = 0, message = "OK" });
    }

    private static async Task<IResult> JsonUpdateAchievementsAsync(
        HttpContext http,
        AuthService auth,
        ClientTelemetryService telemetry)
    {
        var data = await ReadRequestDataAsync(http);

        if (!await CheckJsonOrLegacySessionIfPresentAsync(data, auth))
            return Results.Json(new { ok = false, code = 401, message = "BAD_SESSION" });

        var ok = await telemetry.UpdateAchievementsAsync(data);

        return Results.Json(new
        {
            ok,
            code = ok ? 0 : 5,
            message = ok ? "OK" : "UPDATE_ACHIEVEMENTS_FAILED"
        });
    }

    private static async Task<bool> CheckLegacySessionIfPresentAsync(
        Dictionary<string, string> data,
        AuthService auth)
    {
        var customerId = LegacyUtil.ParseULong(ReadAny(
            data,
            "s_id",
            "CustomerID",
            "CustomerId",
            "customerId"));

        var sessionId = LegacyUtil.ParseULong(ReadAny(
            data,
            "s_key",
            "SessionID",
            "SessionId",
            "sessionId"));

        if (customerId == 0 || sessionId == 0)
            return true;

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);

        return session.Ok;
    }

    private static async Task<bool> CheckJsonOrLegacySessionIfPresentAsync(
        Dictionary<string, string> data,
        AuthService auth)
    {
        var customerId = LegacyUtil.ParseULong(ReadAny(
            data,
            "customerId",
            "CustomerId",
            "CustomerID",
            "s_id"));

        var sessionId = LegacyUtil.ParseULong(ReadAny(
            data,
            "sessionId",
            "SessionId",
            "SessionID",
            "s_key"));

        var token = ReadAny(
            data,
            "token",
            "Token",
            "s_token");

        if (customerId == 0 || sessionId == 0)
            return true;

        if (string.IsNullOrWhiteSpace(token))
        {
            var legacySession = await auth.CheckLegacySessionAsync(customerId, sessionId);
            return legacySession.Ok;
        }

        var session = await auth.CheckSessionAsync(customerId, sessionId, token);
        return session.Ok;
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
                        else
                        {
                            data[item.Key] = item.Value.GetRawText();
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