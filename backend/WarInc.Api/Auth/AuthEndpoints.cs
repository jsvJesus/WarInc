using System.Text;
using Microsoft.Extensions.Options;
using WarInc.Api.Common;
using WarInc.Api.Config;

namespace WarInc.Api.Auth;

public static class AuthEndpoints
{
    public static void MapAuthEndpoints(this WebApplication app)
    {
        app.MapPost("/v1/auth/login", async (
            HttpContext http,
            LoginRequest request,
            AuthService auth) =>
        {
            var result = await auth.LoginAsync(
                http,
                request.Username,
                request.Password);

            return Results.Json(result);
        });

        app.MapPost("/v1/auth/check", async (
            CheckSessionRequest request,
            AuthService auth) =>
        {
            var result = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            return Results.Json(result);
        });

        app.MapPost("/v1/auth/logout", async (
            CheckSessionRequest request,
            AuthService auth) =>
        {
            if (request.CustomerId == 0 || request.SessionId == 0 || string.IsNullOrWhiteSpace(request.Token))
                return Results.Json(new ApiResponse(false, 400, "BAD_SESSION"));

            var ok = await auth.LogoutAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!ok)
                return Results.Json(new ApiResponse(false, 404, "SESSION_NOT_FOUND"));

            return Results.Json(new ApiResponse(true, 0, "OK"));
        });

        app.MapPost("/internal/session/validate", async (
            HttpContext http,
            CheckSessionRequest request,
            AuthService auth,
            IOptions<WarIncOptions> options) =>
        {
            var key = http.Request.Headers["X-WarInc-Internal-Key"].ToString();

            if (string.IsNullOrWhiteSpace(options.Value.InternalApiKey) || key != options.Value.InternalApiKey)
                return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

            var result = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            return Results.Json(new InternalSessionValidateResponse(
                result.Ok,
                request.CustomerId,
                request.SessionId,
                result.Ok ? result.AccountStatus : 0));
        });

        app.MapPost("/api_Login.aspx", async (
            HttpContext http,
            AuthService auth) =>
        {
            var form = await http.Request.ReadFormAsync();

            var username = form["username"].ToString();
            var password = form["password"].ToString();

            var result = await auth.LoginAsync(
                http,
                username,
                password);

            if (!result.Ok)
                return Results.Text("WO_00 0 0", "text/plain", Encoding.UTF8);

            return Results.Text(
                $"WO_0{result.CustomerId} {result.SessionId} {result.AccountStatus}",
                "text/plain",
                Encoding.UTF8);
        });

        app.MapPost("/api_CheckLoginSession.aspx", LegacyCheckLoginSessionAsync);
        app.MapPost("/api/api_CheckLoginSession.aspx", LegacyCheckLoginSessionAsync);

        app.MapPost("/api_UpdateLoginSession.aspx", LegacyUpdateLoginSessionAsync);
        app.MapPost("/api/api_UpdateLoginSession.aspx", LegacyUpdateLoginSessionAsync);
        
        app.MapPost("/api_SteamLogin.aspx", LegacySteamLoginAsync);
        app.MapPost("/api/api_SteamLogin.aspx", LegacySteamLoginAsync);

        app.MapPost("/api_SteamCreateAcc.aspx", LegacySteamCreateAccountAsync);
        app.MapPost("/api/api_SteamCreateAcc.aspx", LegacySteamCreateAccountAsync);

        app.MapPost("/api_Gamersfirst.aspx", LegacyGamersFirstAsync);
        app.MapPost("/api/api_Gamersfirst.aspx", LegacyGamersFirstAsync);

        app.MapPost("/dev/create-account", async (
            HttpContext http,
            CreateAccountRequest request,
            AuthService auth) =>
        {
            var result = await auth.CreateAccountAsync(http, request);
            return Results.Json(result);
        });
    }

    private static async Task<IResult> LegacyCheckLoginSessionAsync(
        HttpContext http,
        AuthService auth)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var result = await auth.CheckLegacySessionAsync(
            customerId,
            sessionId);

        if (!result.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        return Results.Text("WO_0", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyUpdateLoginSessionAsync(
        HttpContext http,
        AuthService auth)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var result = await auth.CheckLegacySessionAsync(
            customerId,
            sessionId);

        if (!result.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var touched = await auth.TouchLegacySessionAsync(
            customerId,
            sessionId);

        if (!touched)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        return Results.Text("WO_0", "text/plain", Encoding.UTF8);
    }
    
    private static Task<IResult> LegacySteamLoginAsync(HttpContext http)
    {
        return Task.FromResult<IResult>(
            Results.Text("WO_00 0 0", "text/plain", Encoding.UTF8));
    }

    private static async Task<IResult> LegacySteamCreateAccountAsync(
        HttpContext http,
        AuthService auth)
    {
        var form = await http.Request.ReadFormAsync();

        var username = form["username"].ToString();
        var password = form["password"].ToString();
        var email = form["email"].ToString();

        var result = await auth.CreateAccountAsync(
            http,
            new CreateAccountRequest(
                username,
                password,
                email,
                false));

        if (!result.Ok)
            return Results.Text(
                $"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}",
                "text/plain",
                Encoding.UTF8);

        return Results.Text("WO_0", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyGamersFirstAsync(
        HttpContext http,
        AuthService auth)
    {
        var form = await http.Request.ReadFormAsync();

        var func = form["func"].ToString();

        if (func == "login")
            return Results.Text("WO_00 0 0 0 0", "text/plain", Encoding.UTF8);

        if (func == "create")
        {
            var username = form["username"].ToString();
            var password = form["password"].ToString();
            var email = form["email"].ToString();

            var result = await auth.CreateAccountAsync(
                http,
                new CreateAccountRequest(
                    username,
                    password,
                    email,
                    false));

            if (!result.Ok)
                return Results.Text(
                    $"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}",
                    "text/plain",
                    Encoding.UTF8);

            return Results.Text("WO_0", "text/plain", Encoding.UTF8);
        }

        return Results.Text("WO_6 bad func", "text/plain", Encoding.UTF8);
    }
}