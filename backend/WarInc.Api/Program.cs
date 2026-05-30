using System.Security.Cryptography;
using System.Text;
using Dapper;
using MySqlConnector;

var builder = WebApplication.CreateBuilder(args);

var dbConnectionString = builder.Configuration["WarInc:Database"] ?? "";
var sessionHours = int.TryParse(builder.Configuration["WarInc:SessionHours"], out var h) ? h : 24;
var internalApiKey = builder.Configuration["WarInc:InternalApiKey"] ?? "";

if (string.IsNullOrWhiteSpace(dbConnectionString))
    throw new Exception("WarInc:Database is empty");

if (string.IsNullOrWhiteSpace(internalApiKey) || internalApiKey == "CHANGE_ME_LONG_RANDOM_KEY")
    Console.WriteLine("WARNING: WarInc:InternalApiKey is not configured");

builder.Services.AddCors(options =>
{
    options.AddPolicy("WarIncCors", policy =>
    {
        policy
            .AllowAnyHeader()
            .AllowAnyMethod()
            .AllowAnyOrigin();
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
    var username = (request.Username ?? "").Trim();
    var password = request.Password ?? "";

    if (username.Length < 3 || password.Length < 3)
    {
        return Results.Json(new ApiResponse(false, 400, "BAD_LOGIN_OR_PASSWORD"));
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
        return Results.Json(new LoginResponse(
            false,
            401,
            "BAD_LOGIN_OR_PASSWORD",
            0,
            0,
            0,
            ""));
    }

    if (!BCrypt.Net.BCrypt.Verify(password, account.PasswordHash))
    {
        return Results.Json(new LoginResponse(
            false,
            401,
            "BAD_LOGIN_OR_PASSWORD",
            0,
            0,
            0,
            ""));
    }

    if (account.AccountStatus >= 200)
    {
        return Results.Json(new LoginResponse(
            false,
            403,
            "ACCOUNT_BLOCKED",
            account.CustomerId,
            0,
            account.AccountStatus,
            ""));
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

    return Results.Json(new LoginResponse(
        true,
        0,
        "OK",
        account.CustomerId,
        sessionId,
        account.AccountStatus,
        token));
});

app.MapPost("/v1/auth/check", async (HttpContext http, CheckSessionRequest request) =>
{
    var customerId = request.CustomerId;
    var sessionId = request.SessionId;
    var token = request.Token ?? "";

    if (customerId <= 0 || sessionId <= 0 || string.IsNullOrWhiteSpace(token))
    {
        return Results.Json(new ApiResponse(false, 400, "BAD_SESSION"));
    }

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
    {
        return Results.Json(new ApiResponse(false, 401, "SESSION_NOT_FOUND"));
    }

    if (session.RevokedAt != null)
    {
        return Results.Json(new ApiResponse(false, 401, "SESSION_REVOKED"));
    }

    if (session.ExpiresAt <= DateTime.UtcNow)
    {
        return Results.Json(new ApiResponse(false, 401, "SESSION_EXPIRED"));
    }

    if (session.AccountStatus >= 200)
    {
        return Results.Json(new ApiResponse(false, 403, "ACCOUNT_BLOCKED"));
    }

    return Results.Json(new ApiResponse(true, 0, "OK"));
});

app.MapPost("/internal/session/validate", async (HttpContext http, CheckSessionRequest request) =>
{
    var key = http.Request.Headers["X-WarInc-Internal-Key"].ToString();

    if (key != internalApiKey)
    {
        return Results.Json(new ApiResponse(false, 403, "FORBIDDEN"));
    }

    var customerId = request.CustomerId;
    var sessionId = request.SessionId;
    var token = request.Token ?? "";

    if (customerId <= 0 || sessionId <= 0 || string.IsNullOrWhiteSpace(token))
    {
        return Results.Json(new ApiResponse(false, 400, "BAD_SESSION"));
    }

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

    if (session == null || session.RevokedAt != null || session.ExpiresAt <= DateTime.UtcNow || session.AccountStatus >= 200)
    {
        return Results.Json(new InternalSessionValidateResponse(
            false,
            customerId,
            sessionId,
            0));
    }

    return Results.Json(new InternalSessionValidateResponse(
        true,
        session.CustomerId,
        session.SessionId,
        session.AccountStatus));
});

app.MapPost("/api_Login.aspx", async (HttpContext http) =>
{
    var form = await http.Request.ReadFormAsync();

    var username = form["username"].ToString();
    var password = form["password"].ToString();

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

    if (account == null || !BCrypt.Net.BCrypt.Verify(password, account.PasswordHash))
    {
        return Results.Text("WO_00 0 0", "text/plain", Encoding.UTF8);
    }

    if (account.AccountStatus >= 200)
    {
        return Results.Text($"WO_0{account.CustomerId} 0 {account.AccountStatus}", "text/plain", Encoding.UTF8);
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

    return Results.Text($"WO_0{account.CustomerId} {sessionId} {account.AccountStatus}", "text/plain", Encoding.UTF8);
});

app.MapPost("/dev/create-account", async (CreateAccountRequest request) =>
{
    var username = (request.Username ?? "").Trim();
    var password = request.Password ?? "";

    if (username.Length < 3 || password.Length < 6)
    {
        return Results.Json(new ApiResponse(false, 400, "BAD_USERNAME_OR_PASSWORD"));
    }

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
                Email = string.IsNullOrWhiteSpace(request.Email) ? null : request.Email.Trim(),
                PasswordHash = hash,
                IsDeveloper = request.IsDeveloper ? 1 : 0
            });

        return Results.Json(new
        {
            ok = true,
            customerId
        });
    }
    catch (MySqlException e) when (e.Number == 1062)
    {
        return Results.Json(new ApiResponse(false, 409, "ACCOUNT_ALREADY_EXISTS"));
    }
});

app.Run();

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