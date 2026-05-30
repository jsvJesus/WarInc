using System.Security.Cryptography;
using System.Text;
using Dapper;
using MySqlConnector;

var builder = WebApplication.CreateBuilder(args);

var dbConnectionString = builder.Configuration["WarInc:Database"] ?? "";
var sessionHours = int.TryParse(builder.Configuration["WarInc:SessionHours"], out var h) ? h : 24;
var internalApiKey = builder.Configuration["WarInc:InternalApiKey"] ?? "";

if (string.IsNullOrWhiteSpace(dbConnectionString))
    throw new Exception("WarInc:Database is empty"); // ebana rot

builder.Services.AddCors(options =>
{
    options.AddPolicy("WarIncCors", policy =>
    {
        policy.AllowAnyHeader();
        policy.AllowAnyMethod();
        policy.AllowAnyOrigin();
    });
});

var app = builder.Build();

app.UseCors("WarIncCors");

app.MapGet("/health", () =>
{
    return Results.Json(new
    {
        ok = true,
        service = "WarInc.Api",
        time = DateTimeOffset.UtcNow
    });
});

app.MapPost("/v1/auth/login", async (HttpContext http, LoginRequest request) =>
{
    var result = await LoginAsync(
        dbConnectionString,
        sessionHours,
        http,
        request.Username,
        request.Password);

    return Results.Json(result);
});

app.MapPost("/v1/auth/check", async (CheckSessionRequest request) =>
{
    var result = await CheckSessionAsync(
        dbConnectionString,
        request.CustomerId,
        request.SessionId,
        request.Token);

    return Results.Json(result);
});

app.MapPost("/v1/auth/logout", async (CheckSessionRequest request) =>
{
    if (request.CustomerId == 0 || request.SessionId == 0 || string.IsNullOrWhiteSpace(request.Token))
        return Results.Json(new ApiResponse(false, 400, "BAD_SESSION"));

    await using var db = new MySqlConnection(dbConnectionString);

    var tokenHash = Sha256Hex(request.Token);

    var rows = await db.ExecuteAsync(
        """
        UPDATE login_sessions
        SET revoked_at = UTC_TIMESTAMP()
        WHERE customer_id = @CustomerId
          AND session_id = @SessionId
          AND token_hash = @TokenHash
          AND revoked_at IS NULL;
        """,
        new
        {
            request.CustomerId,
            request.SessionId,
            TokenHash = tokenHash
        });

    if (rows <= 0)
        return Results.Json(new ApiResponse(false, 404, "SESSION_NOT_FOUND"));

    return Results.Json(new ApiResponse(true, 0, "OK"));
});

app.MapPost("/internal/session/validate", async (HttpContext http, CheckSessionRequest request) =>
{
    var key = http.Request.Headers["X-WarInc-Internal-Key"].ToString();

    if (string.IsNullOrWhiteSpace(internalApiKey) || key != internalApiKey)
        return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));

    var result = await CheckSessionAsync(
        dbConnectionString,
        request.CustomerId,
        request.SessionId,
        request.Token);

    return Results.Json(new InternalSessionValidateResponse(
        result.Ok,
        request.CustomerId,
        request.SessionId,
        result.Ok ? result.AccountStatus : 0));
});

app.MapPost("/api_Login.aspx", async (HttpContext http) =>
{
    var form = await http.Request.ReadFormAsync();

    var username = form["username"].ToString();
    var password = form["password"].ToString();

    var result = await LoginAsync(
        dbConnectionString,
        sessionHours,
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

async Task<IResult> LegacyCheckLoginSessionAsync(HttpContext http)
{
    var form = await http.Request.ReadFormAsync();

    var customerId = ParseULong(form["s_id"].ToString());
    var sessionId = ParseULong(form["s_key"].ToString());

    var result = await CheckLegacySessionAsync(
        dbConnectionString,
        customerId,
        sessionId);

    if (!result.Ok)
        return Results.Text("WO_1", "text/plain", Encoding.UTF8);

    return Results.Text("WO_0", "text/plain", Encoding.UTF8);
}

async Task<IResult> LegacyUpdateLoginSessionAsync(HttpContext http)
{
    var form = await http.Request.ReadFormAsync();

    var customerId = ParseULong(form["s_id"].ToString());
    var sessionId = ParseULong(form["s_key"].ToString());

    var result = await CheckLegacySessionAsync(
        dbConnectionString,
        customerId,
        sessionId);

    if (!result.Ok)
        return Results.Text("WO_1", "text/plain", Encoding.UTF8);

    await using var db = new MySqlConnection(dbConnectionString);

    await db.ExecuteAsync(
        """
        UPDATE login_sessions
        SET expires_at = DATE_ADD(UTC_TIMESTAMP(), INTERVAL @SessionHours HOUR)
        WHERE customer_id = @CustomerId
          AND session_id = @SessionId
          AND revoked_at IS NULL;
        """,
        new
        {
            SessionHours = sessionHours,
            CustomerId = customerId,
            SessionId = sessionId
        });

    return Results.Text("WO_0", "text/plain", Encoding.UTF8);
}

app.MapPost("/dev/create-account", async (CreateAccountRequest request) =>
{
    var username = (request.Username ?? "").Trim();
    var password = request.Password ?? "";
    var email = string.IsNullOrWhiteSpace(request.Email) ? null : request.Email.Trim();

    if (username.Length < 3 || username.Length > 64)
        return Results.Json(new ApiResponse(false, 400, "BAD_USERNAME"));

    if (password.Length < 6 || password.Length > 128)
        return Results.Json(new ApiResponse(false, 400, "BAD_PASSWORD"));

    var hash = BCrypt.Net.BCrypt.HashPassword(password);

    await using var db = new MySqlConnection(dbConnectionString);

    try
    {
        var customerId = await db.ExecuteScalarAsync<ulong>(
            """
            INSERT INTO accounts
                (username, email, password_hash, account_status, is_developer)
            VALUES
                (@Username, @Email, @PasswordHash, 100, @IsDeveloper);

            SELECT LAST_INSERT_ID();
            """,
            new
            {
                Username = username,
                Email = email,
                PasswordHash = hash,
                IsDeveloper = request.IsDeveloper ? 1 : 0
            });

        return Results.Json(new CreateAccountResponse(
            true,
            0,
            "OK",
            customerId));
    }
    catch (MySqlException e) when (e.Number == 1062)
    {
        return Results.Json(new CreateAccountResponse(
            false,
            409,
            "ACCOUNT_ALREADY_EXISTS",
            0));
    }
});

app.Run();

static async Task<LoginResponse> LoginAsync(
    string dbConnectionString,
    int sessionHours,
    HttpContext http,
    string? usernameRaw,
    string? passwordRaw)
{
    var username = (usernameRaw ?? "").Trim();
    var password = passwordRaw ?? "";

    if (username.Length < 3 || username.Length > 64)
    {
        return new LoginResponse(
            false,
            400,
            "BAD_LOGIN_OR_PASSWORD",
            0,
            0,
            0,
            "");
    }

    if (password.Length < 3 || password.Length > 128)
    {
        return new LoginResponse(
            false,
            400,
            "BAD_LOGIN_OR_PASSWORD",
            0,
            0,
            0,
            "");
    }

    await using var db = new MySqlConnection(dbConnectionString);

    var account = await db.QueryFirstOrDefaultAsync<AccountRow>(
        """
        SELECT
            customer_id AS CustomerId,
            username AS Username,
            password_hash AS PasswordHash,
            account_status AS AccountStatus,
            is_developer AS IsDeveloper
        FROM accounts
        WHERE username = @Username
        LIMIT 1;
        """,
        new { Username = username });

    if (account == null)
    {
        return new LoginResponse(
            false,
            401,
            "BAD_LOGIN_OR_PASSWORD",
            0,
            0,
            0,
            "");
    }

    if (!BCrypt.Net.BCrypt.Verify(password, account.PasswordHash))
    {
        return new LoginResponse(
            false,
            401,
            "BAD_LOGIN_OR_PASSWORD",
            0,
            0,
            0,
            "");
    }

    if (account.AccountStatus >= 200)
    {
        return new LoginResponse(
            false,
            403,
            "ACCOUNT_BLOCKED",
            account.CustomerId,
            0,
            account.AccountStatus,
            "");
    }

    var sessionId = CreateSessionId();
    var token = CreateToken();
    var tokenHash = Sha256Hex(token);
    var ip = GetClientIp(http);
    var userAgent = http.Request.Headers.UserAgent.ToString();
    var expiresAt = DateTime.UtcNow.AddHours(sessionHours);

    await db.ExecuteAsync(
        """
        INSERT INTO login_sessions
            (session_id, customer_id, token_hash, ip_address, user_agent, expires_at)
        VALUES
            (@SessionId, @CustomerId, @TokenHash, @IpAddress, @UserAgent, @ExpiresAt);
        """,
        new
        {
            SessionId = sessionId,
            CustomerId = account.CustomerId,
            TokenHash = tokenHash,
            IpAddress = ip,
            UserAgent = userAgent,
            ExpiresAt = expiresAt
        });

    await db.ExecuteAsync(
        """
        UPDATE accounts
        SET last_login_at = UTC_TIMESTAMP()
        WHERE customer_id = @CustomerId;
        """,
        new { account.CustomerId });

    return new LoginResponse(
        true,
        0,
        "OK",
        account.CustomerId,
        sessionId,
        account.AccountStatus,
        token);
}

static async Task<CheckSessionResponse> CheckSessionAsync(
    string dbConnectionString,
    ulong customerId,
    ulong sessionId,
    string? tokenRaw)
{
    var token = tokenRaw ?? "";

    if (customerId == 0 || sessionId == 0)
        return new CheckSessionResponse(false, 400, "BAD_SESSION", 0);

    if (string.IsNullOrWhiteSpace(token))
        return new CheckSessionResponse(false, 400, "BAD_TOKEN", 0);

    await using var db = new MySqlConnection(dbConnectionString);

    var tokenHash = Sha256Hex(token);

    var session = await db.QueryFirstOrDefaultAsync<SessionRow>(
        """
        SELECT
            s.session_id AS SessionId,
            s.customer_id AS CustomerId,
            s.expires_at AS ExpiresAt,
            s.revoked_at AS RevokedAt,
            a.account_status AS AccountStatus
        FROM login_sessions s
        INNER JOIN accounts a ON a.customer_id = s.customer_id
        WHERE s.customer_id = @CustomerId
          AND s.session_id = @SessionId
          AND s.token_hash = @TokenHash
        LIMIT 1;
        """,
        new
        {
            CustomerId = customerId,
            SessionId = sessionId,
            TokenHash = tokenHash
        });

    if (session == null)
        return new CheckSessionResponse(false, 401, "SESSION_NOT_FOUND", 0);

    if (session.RevokedAt != null)
        return new CheckSessionResponse(false, 401, "SESSION_REVOKED", 0);

    if (session.ExpiresAt <= DateTime.UtcNow)
        return new CheckSessionResponse(false, 401, "SESSION_EXPIRED", 0);

    if (session.AccountStatus >= 200)
        return new CheckSessionResponse(false, 403, "ACCOUNT_BLOCKED", session.AccountStatus);

    return new CheckSessionResponse(true, 0, "OK", session.AccountStatus);
}

static async Task<CheckSessionResponse> CheckLegacySessionAsync(
    string dbConnectionString,
    ulong customerId,
    ulong sessionId)
{
    if (customerId == 0 || sessionId == 0)
        return new CheckSessionResponse(false, 400, "BAD_SESSION", 0);

    await using var db = new MySqlConnection(dbConnectionString);

    var session = await db.QueryFirstOrDefaultAsync<SessionRow>(
        """
        SELECT
            s.session_id AS SessionId,
            s.customer_id AS CustomerId,
            s.expires_at AS ExpiresAt,
            s.revoked_at AS RevokedAt,
            a.account_status AS AccountStatus
        FROM login_sessions s
        INNER JOIN accounts a ON a.customer_id = s.customer_id
        WHERE s.customer_id = @CustomerId
          AND s.session_id = @SessionId
        LIMIT 1;
        """,
        new
        {
            CustomerId = customerId,
            SessionId = sessionId
        });

    if (session == null)
        return new CheckSessionResponse(false, 401, "SESSION_NOT_FOUND", 0);

    if (session.RevokedAt != null)
        return new CheckSessionResponse(false, 401, "SESSION_REVOKED", 0);

    if (session.ExpiresAt <= DateTime.UtcNow)
        return new CheckSessionResponse(false, 401, "SESSION_EXPIRED", 0);

    if (session.AccountStatus >= 200)
        return new CheckSessionResponse(false, 403, "ACCOUNT_BLOCKED", session.AccountStatus);

    return new CheckSessionResponse(true, 0, "OK", session.AccountStatus);
}

static ulong CreateSessionId()
{
    Span<byte> bytes = stackalloc byte[8];
    RandomNumberGenerator.Fill(bytes);

    var value = BitConverter.ToUInt64(bytes);

    if (value == 0)
        value = 1;

    return value;
}

static string CreateToken()
{
    Span<byte> bytes = stackalloc byte[32];
    RandomNumberGenerator.Fill(bytes);
    return Convert.ToBase64String(bytes);
}

static string Sha256Hex(string value)
{
    var bytes = SHA256.HashData(Encoding.UTF8.GetBytes(value));
    var sb = new StringBuilder(bytes.Length * 2);

    foreach (var b in bytes)
        sb.Append(b.ToString("x2"));

    return sb.ToString();
}

static string GetClientIp(HttpContext http)
{
    var forwarded = http.Request.Headers["X-Forwarded-For"].ToString();

    if (!string.IsNullOrWhiteSpace(forwarded))
    {
        var comma = forwarded.IndexOf(',');

        if (comma > 0)
            return forwarded[..comma].Trim();

        return forwarded.Trim();
    }

    return http.Connection.RemoteIpAddress?.ToString() ?? "0.0.0.0";
}

static ulong ParseULong(string value)
{
    if (ulong.TryParse(value, out var result))
        return result;

    return 0;
}

record LoginRequest(string Username, string Password);

record CheckSessionRequest(ulong CustomerId, ulong SessionId, string Token);

record CreateAccountRequest(string Username, string Password, string? Email, bool IsDeveloper);

record ApiResponse(bool Ok, int Code, string Message);

record LoginResponse(
    bool Ok,
    int Code,
    string Message,
    ulong CustomerId,
    ulong SessionId,
    int AccountStatus,
    string Token);

record CheckSessionResponse(
    bool Ok,
    int Code,
    string Message,
    int AccountStatus);

record CreateAccountResponse(
    bool Ok,
    int Code,
    string Message,
    ulong CustomerId);

record InternalSessionValidateResponse(
    bool Ok,
    ulong CustomerId,
    ulong SessionId,
    int AccountStatus);

class AccountRow
{
    public ulong CustomerId { get; set; }
    public string Username { get; set; } = "";
    public string PasswordHash { get; set; } = "";
    public int AccountStatus { get; set; }
    public bool IsDeveloper { get; set; }
}

class SessionRow
{
    public ulong SessionId { get; set; }
    public ulong CustomerId { get; set; }
    public DateTime ExpiresAt { get; set; }
    public DateTime? RevokedAt { get; set; }
    public int AccountStatus { get; set; }
}