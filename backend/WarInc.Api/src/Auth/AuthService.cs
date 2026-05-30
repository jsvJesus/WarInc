using Dapper;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Options;
using MySqlConnector;
using WarInc.Api.Common;
using WarInc.Api.Config;
using WarInc.Api.Database;
using WarInc.Api.Profile;

namespace WarInc.Api.Auth;

public sealed class AuthService
{
    private readonly WarIncDb _db;
    private readonly WarIncOptions _options;
    private readonly ProfileService _profileService;

    public AuthService(
        WarIncDb db,
        IOptions<WarIncOptions> options,
        ProfileService profileService)
    {
        _db = db;
        _options = options.Value;
        _profileService = profileService;
    }

    public async Task<LoginResponse> LoginAsync(
        HttpContext http,
        string? usernameRaw,
        string? passwordRaw)
    {
        var username = (usernameRaw ?? "").Trim();
        var password = passwordRaw ?? "";

        if (username.Length < 3 || username.Length > 32)
            return new LoginResponse(false, 400, "BAD_LOGIN_OR_PASSWORD", 0, 0, 0, "");

        if (password.Length < 1 || password.Length > 128)
            return new LoginResponse(false, 400, "BAD_LOGIN_OR_PASSWORD", 0, 0, 0, "");

        await using var db = _db.CreateConnection();

        var account = await db.QueryFirstOrDefaultAsync<AccountRow>(
            """
            SELECT
                CustomerID AS CustomerId,
                AccountName AS AccountName,
                Password AS Password,
                MD5Password AS MD5Password,
                AccountStatus AS AccountStatus,
                IsDeveloper AS IsDeveloper
            FROM loginid
            WHERE AccountName = @Username
            LIMIT 1;
            """,
            new { Username = username });

        if (account == null)
            return new LoginResponse(false, 401, "BAD_LOGIN_OR_PASSWORD", 0, 0, 0, "");

        if (!SecurityUtil.PasswordMatches(password, account.Password, account.MD5Password))
            return new LoginResponse(false, 401, "BAD_LOGIN_OR_PASSWORD", 0, 0, 0, "");

        if (account.AccountStatus >= 200)
            return new LoginResponse(false, 403, "ACCOUNT_BLOCKED", account.CustomerId, 0, account.AccountStatus, "");

        var sessionId = SecurityUtil.CreateSessionId();
        var sessionKey = SecurityUtil.CreateToken();
        var ip = SecurityUtil.NormalizeLegacyIp(SecurityUtil.GetClientIp(http));

        await db.ExecuteAsync(
            """
            DELETE FROM loginsessions
            WHERE CustomerID = @CustomerId;
            """,
            new { account.CustomerId });

        await db.ExecuteAsync(
            """
            INSERT INTO loginsessions
            (
                CustomerID,
                SessionKey,
                SessionID,
                LoginIP,
                TimeLogged,
                TimeUpdated,
                GameSessionID
            )
            VALUES
            (
                @CustomerId,
                @SessionKey,
                @SessionId,
                @Ip,
                UTC_TIMESTAMP(),
                UTC_TIMESTAMP(),
                0
            );
            """,
            new
            {
                account.CustomerId,
                SessionKey = sessionKey,
                SessionId = sessionId,
                Ip = ip
            });

        await db.ExecuteAsync(
            """
            UPDATE loginid
            SET
                lastlogindate = UTC_TIMESTAMP(),
                lastloginIP = @Ip
            WHERE CustomerID = @CustomerId;
            """,
            new
            {
                account.CustomerId,
                Ip = ip
            });

        await db.ExecuteAsync(
            """
            INSERT INTO logins
            (
                CustomerID,
                LoginTime,
                IP,
                LoginSource
            )
            VALUES
            (
                @CustomerId,
                UTC_TIMESTAMP(),
                @Ip,
                0
            );
            """,
            new
            {
                account.CustomerId,
                Ip = ip
            });

        return new LoginResponse(
            true,
            0,
            "OK",
            account.CustomerId,
            (ulong)sessionId,
            account.AccountStatus,
            sessionKey);
    }

    public async Task<CheckSessionResponse> CheckSessionAsync(
        ulong customerId,
        ulong sessionId,
        string? tokenRaw)
    {
        var token = tokenRaw ?? "";

        if (customerId == 0 || sessionId == 0 || sessionId > int.MaxValue)
            return new CheckSessionResponse(false, 400, "BAD_SESSION", 0);

        if (string.IsNullOrWhiteSpace(token))
            return new CheckSessionResponse(false, 400, "BAD_TOKEN", 0);

        await using var db = _db.CreateConnection();

        var session = await db.QueryFirstOrDefaultAsync<SessionRow>(
            """
            SELECT
                s.SessionID AS SessionId,
                s.CustomerID AS CustomerId,
                s.SessionKey AS SessionKey,
                s.TimeUpdated AS TimeUpdated,
                a.AccountStatus AS AccountStatus
            FROM loginsessions s
            INNER JOIN loginid a ON a.CustomerID = s.CustomerID
            WHERE s.CustomerID = @CustomerId
              AND s.SessionID = @SessionId
              AND s.SessionKey = @SessionKey
            LIMIT 1;
            """,
            new
            {
                CustomerId = customerId,
                SessionId = (int)sessionId,
                SessionKey = token
            });

        return ValidateSessionRow(session);
    }

    public async Task<CheckSessionResponse> CheckLegacySessionAsync(
        ulong customerId,
        ulong sessionId)
    {
        if (customerId == 0 || sessionId == 0 || sessionId > int.MaxValue)
            return new CheckSessionResponse(false, 400, "BAD_SESSION", 0);

        await using var db = _db.CreateConnection();

        var session = await db.QueryFirstOrDefaultAsync<SessionRow>(
            """
            SELECT
                s.SessionID AS SessionId,
                s.CustomerID AS CustomerId,
                s.SessionKey AS SessionKey,
                s.TimeUpdated AS TimeUpdated,
                a.AccountStatus AS AccountStatus
            FROM loginsessions s
            INNER JOIN loginid a ON a.CustomerID = s.CustomerID
            WHERE s.CustomerID = @CustomerId
              AND s.SessionID = @SessionId
            LIMIT 1;
            """,
            new
            {
                CustomerId = customerId,
                SessionId = (int)sessionId
            });

        return ValidateSessionRow(session);
    }

    public async Task<bool> TouchLegacySessionAsync(
        ulong customerId,
        ulong sessionId)
    {
        await using var db = _db.CreateConnection();

        var rows = await db.ExecuteAsync(
            """
            UPDATE loginsessions
            SET TimeUpdated = UTC_TIMESTAMP()
            WHERE CustomerID = @CustomerId
              AND SessionID = @SessionId;
            """,
            new
            {
                CustomerId = customerId,
                SessionId = sessionId
            });

        return rows > 0;
    }
    
    public async Task<CheckSessionResponse> CheckLegacySessionWithTokenModeAsync(
        ulong customerId,
        ulong sessionId,
        string? tokenRaw,
        bool requireToken)
    {
        var token = tokenRaw ?? "";

        if (requireToken || !string.IsNullOrWhiteSpace(token))
            return await CheckSessionAsync(customerId, sessionId, token);

        return await CheckLegacySessionAsync(customerId, sessionId);
    }

    public async Task<bool> LogoutAsync(
        ulong customerId,
        ulong sessionId,
        string token)
    {
        await using var db = _db.CreateConnection();

        var rows = await db.ExecuteAsync(
            """
            DELETE FROM loginsessions
            WHERE CustomerID = @CustomerId
              AND SessionID = @SessionId
              AND SessionKey = @SessionKey;
            """,
            new
            {
                CustomerId = customerId,
                SessionId = sessionId,
                SessionKey = token
            });

        return rows > 0;
    }

    public async Task<CreateAccountResponse> CreateAccountAsync(
        HttpContext http,
        CreateAccountRequest request)
    {
        var username = (request.Username ?? "").Trim();
        var password = request.Password ?? "";
        var ip = SecurityUtil.NormalizeLegacyIp(SecurityUtil.GetClientIp(http));

        if (username.Length < 3 || username.Length > 32)
            return new CreateAccountResponse(false, 400, "BAD_USERNAME", 0);

        if (password.Length < 3 || password.Length > 16)
            return new CreateAccountResponse(false, 400, "BAD_PASSWORD_SCHEMA_MAX_16", 0);

        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var exists = await db.ExecuteScalarAsync<int>(
                """
                SELECT COUNT(*)
                FROM loginid
                WHERE AccountName = @Username;
                """,
                new { Username = username },
                tx);

            if (exists > 0)
            {
                await tx.RollbackAsync();
                return new CreateAccountResponse(false, 409, "ACCOUNT_ALREADY_EXISTS", 0);
            }

            var md5Password = SecurityUtil.Md5Hex(password);

            var customerId = await db.ExecuteScalarAsync<ulong>(
                """
                INSERT INTO loginid
                (
                    AccountName,
                    Password,
                    AccountStatus,
                    GamePoints,
                    HonorPoints,
                    SkillPoints,
                    Gamertag,
                    dateregistered,
                    lastlogindate,
                    lastloginIP,
                    lastgamedate,
                    ReferralID,
                    lastjoineddate,
                    MD5Password,
                    ClanID,
                    GameDollars,
                    Faction1Score,
                    Faction2Score,
                    Faction3Score,
                    Faction4Score,
                    Faction5Score,
                    ClanRank,
                    lastRetBonusDate,
                    IsFPSEnabled,
                    reg_sid,
                    ClanContributedXP,
                    ClanContributedGP,
                    IsDeveloper
                )
                VALUES
                (
                    @Username,
                    @Password,
                    100,
                    0,
                    0,
                    0,
                    @Gamertag,
                    UTC_TIMESTAMP(),
                    UTC_TIMESTAMP(),
                    @Ip,
                    UTC_TIMESTAMP(),
                    0,
                    UTC_TIMESTAMP(),
                    @MD5Password,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    NULL,
                    1,
                    NULL,
                    0,
                    0,
                    @IsDeveloper
                );

                SELECT LAST_INSERT_ID();
                """,
                new
                {
                    Username = username,
                    Password = password,
                    Gamertag = username,
                    MD5Password = md5Password,
                    Ip = ip,
                    IsDeveloper = request.IsDeveloper ? 1 : 0
                },
                tx);

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
                    @CustomerId,
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
                    0
                );
                """,
                new { CustomerId = customerId },
                tx);

            await db.ExecuteAsync(
                """
                INSERT INTO profiledata
                (
                    CustomerID,
                    Skills,
                    Achievements,
                    Abilities
                )
                VALUES
                (
                    @CustomerId,
                    '0000000000',
                    '',
                    ''
                );
                """,
                new { CustomerId = customerId },
                tx);

            await _profileService.EnsureStarterProfileAsync(db, tx, customerId);

            await tx.CommitAsync();

            return new CreateAccountResponse(true, 0, "OK", customerId);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return new CreateAccountResponse(false, 500, e.Message, 0);
        }
    }

    private CheckSessionResponse ValidateSessionRow(SessionRow? session)
    {
        if (session == null)
            return new CheckSessionResponse(false, 401, "SESSION_NOT_FOUND", 0);

        var minTime = DateTime.UtcNow.AddHours(-_options.SessionHours);

        if (session.TimeUpdated < minTime)
            return new CheckSessionResponse(false, 401, "SESSION_EXPIRED", 0);

        if (session.AccountStatus >= 200)
            return new CheckSessionResponse(false, 403, "ACCOUNT_BLOCKED", session.AccountStatus);

        return new CheckSessionResponse(true, 0, "OK", session.AccountStatus);
    }
}