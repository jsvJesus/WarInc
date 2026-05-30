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

        app.MapPost("/api_Login.aspx", LegacyLoginAsync);
        app.MapPost("/api/api_Login.aspx", LegacyLoginAsync);

        app.MapPost("/api_CheckLoginSession.aspx", LegacyCheckLoginSessionAsync);
        app.MapPost("/api/api_CheckLoginSession.aspx", LegacyCheckLoginSessionAsync);

        app.MapPost("/api_UpdateLoginSession.aspx", LegacyUpdateLoginSessionAsync);
        app.MapPost("/api/api_UpdateLoginSession.aspx", LegacyUpdateLoginSessionAsync);

        app.MapPost("/api_GNALogin.aspx", LegacyGnaLoginAsync);
        app.MapPost("/api/api_GNALogin.aspx", LegacyGnaLoginAsync);

        app.MapPost("/api_GNAGetBalance.aspx", LegacyGnaGetBalanceAsync);
        app.MapPost("/api/api_GNAGetBalance.aspx", LegacyGnaGetBalanceAsync);

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

    private static async Task<IResult> LegacyLoginAsync(
        HttpContext http,
        AuthService auth)
    {
        var data = await ReadRequestValuesAsync(http);

        var username = ReadAny(data, "username", "Username", "login", "AccountName");
        var password = ReadAny(data, "password", "Password", "pass");

        var result = await auth.LoginAsync(
            http,
            username,
            password);

        if (!result.Ok)
            return Text("WO_00 0 0");

        return Text($"WO_0{result.CustomerId} {result.SessionId} {result.AccountStatus}");
    }

    private static async Task<IResult> LegacyCheckLoginSessionAsync(
        HttpContext http,
        AuthService auth)
    {
        var data = await ReadRequestValuesAsync(http);

        var customerId = LegacyUtil.ParseULong(ReadAny(data, "s_id", "CustomerID", "CustomerId", "customerId"));
        var sessionId = LegacyUtil.ParseULong(ReadAny(data, "s_key", "SessionID", "SessionId", "sessionId"));

        var result = await auth.CheckLegacySessionAsync(
            customerId,
            sessionId);

        if (!result.Ok)
            return Text("WO_1");

        return Text("WO_0");
    }

    private static async Task<IResult> LegacyUpdateLoginSessionAsync(
        HttpContext http,
        AuthService auth)
    {
        var data = await ReadRequestValuesAsync(http);

        var customerId = LegacyUtil.ParseULong(ReadAny(data, "s_id", "CustomerID", "CustomerId", "customerId"));
        var sessionId = LegacyUtil.ParseULong(ReadAny(data, "s_key", "SessionID", "SessionId", "sessionId"));

        var result = await auth.CheckLegacySessionAsync(
            customerId,
            sessionId);

        if (!result.Ok)
            return Text("WO_1");

        var touched = await auth.TouchLegacySessionAsync(
            customerId,
            sessionId);

        if (!touched)
            return Text("WO_1");

        return Text("WO_0");
    }

    private static async Task<IResult> LegacyGnaLoginAsync(
        HttpContext http,
        LegacyExternalAuthService legacyAuth)
    {
        var data = await ReadRequestValuesAsync(http);

        var userId = ReadAny(data, "userId", "UserID", "userid", "gnaUserId", "GNAUserId", "id");
        var nickname = ReadAny(data, "nickname", "nick", "username", "Username", "GNANick", "name");

        var result = await legacyAuth.LoginOrCreateExternalAsync(
            http,
            "gna",
            userId,
            nickname);

        if (!result.Ok)
            return Text("WO_00 0 0");

        return Text($"WO_0{result.CustomerId} {result.SessionId} {result.AccountStatus}");
    }

    private static async Task<IResult> LegacyGnaGetBalanceAsync(
        HttpContext http,
        AuthService auth,
        LegacyExternalAuthService legacyAuth)
    {
        var data = await ReadRequestValuesAsync(http);

        var customerId = LegacyUtil.ParseULong(ReadAny(data, "s_id", "CustomerID", "CustomerId", "customerId"));
        var sessionId = LegacyUtil.ParseULong(ReadAny(data, "s_key", "SessionID", "SessionId", "sessionId"));

        var session = await auth.CheckLegacySessionAsync(
            customerId,
            sessionId);

        if (!session.Ok)
            return Text("WO_1");

        var balance = await legacyAuth.GetGamePointsBalanceAsync(customerId);

        return Text($"WO_0{balance}");
    }

    private static async Task<IResult> LegacyGamersFirstAsync(
        HttpContext http,
        AuthService auth,
        LegacyExternalAuthService legacyAuth)
    {
        var data = await ReadRequestValuesAsync(http);

        var func = ReadAny(data, "func", "Func").Trim().ToLowerInvariant();

        if (func == "login")
        {
            var g1Id = ReadAny(data, "g1Id", "G1ID", "AccountId", "accountId", "token", "Token");

            if (string.IsNullOrWhiteSpace(g1Id))
                g1Id = ReadAny(data, "username", "Username", "email", "Email");

            var nickname = ReadAny(data, "username", "Username", "nickname", "Nickname", "name");

            var result = await legacyAuth.LoginOrCreateExternalAsync(
                http,
                "g1",
                g1Id,
                nickname);

            if (!result.Ok)
                return Text("WO_00 0 0 0 0");

            var outG1Id = string.IsNullOrWhiteSpace(g1Id) ? result.CustomerId.ToString() : g1Id;
            var outNickname = string.IsNullOrWhiteSpace(nickname) ? $"g1{result.CustomerId}" : nickname.Trim();

            return Text($"WO_0{result.CustomerId} {result.SessionId} {result.AccountStatus} {outG1Id} 0 :{outNickname}");
        }

        if (func == "create")
        {
            var username = ReadAny(data, "username", "Username");
            var password = ReadAny(data, "password", "Password");
            var email = ReadAny(data, "email", "Email");

            var result = await auth.CreateAccountAsync(
                http,
                new CreateAccountRequest(
                    username,
                    password,
                    email,
                    false));

            if (!result.Ok)
                return Text($"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}");

            return Text("WO_0");
        }

        return Text("WO_6 bad func");
    }

    private static async Task<Dictionary<string, string>> ReadRequestValuesAsync(HttpContext http)
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

    private static IResult Text(string value)
    {
        return Results.Text(value, "text/plain", Encoding.UTF8);
    }
}