using System.Text.RegularExpressions;
using Dapper;
using Microsoft.AspNetCore.Http;
using MySqlConnector;
using WarInc.Api.Common;
using WarInc.Api.Database;
using WarInc.Api.Profile;

namespace WarInc.Api.Auth;

public sealed class LegacyExternalAuthService
{
    private static readonly Regex SafeNameRegex = new("[^A-Za-z0-9]", RegexOptions.Compiled);

    private readonly WarIncDb _db;
    private readonly ProfileService _profileService;

    public LegacyExternalAuthService(
        WarIncDb db,
        ProfileService profileService)
    {
        _db = db;
        _profileService = profileService;
    }

    public async Task<LoginResponse> LoginOrCreateExternalAsync(
        HttpContext http,
        string provider,
        string externalIdRaw,
        string? nicknameRaw)
    {
        provider = NormalizeProvider(provider);
        var externalId = (externalIdRaw ?? "").Trim();

        if (string.IsNullOrWhiteSpace(provider))
            return new LoginResponse(false, 400, "BAD_PROVIDER", 0, 0, 0, "");

        if (string.IsNullOrWhiteSpace(externalId))
            return new LoginResponse(false, 400, "BAD_EXTERNAL_ID", 0, 0, 0, "");

        var accountName = BuildAccountName(provider, externalId);
        var gamertag = BuildGamertag(provider, externalId, nicknameRaw);
        var password = BuildPassword(provider, externalId);
        var md5Password = SecurityUtil.Md5Hex(password);
        var ip = SecurityUtil.NormalizeLegacyIp(SecurityUtil.GetClientIp(http));

        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var account = await db.QueryFirstOrDefaultAsync<LegacyExternalAccountRow>(
                """
                SELECT
                    CustomerID AS CustomerId,
                    AccountStatus AS AccountStatus
                FROM loginid
                WHERE AccountName = @AccountName
                LIMIT 1
                FOR UPDATE;
                """,
                new { AccountName = accountName },
                tx);

            ulong customerId;
            int accountStatus;

            if (account == null)
            {
                customerId = await db.ExecuteScalarAsync<ulong>(
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
                        @AccountName,
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
                        @RegSid,
                        0,
                        0,
                        0
                    );

                    SELECT LAST_INSERT_ID();
                    """,
                    new
                    {
                        AccountName = accountName,
                        Password = password,
                        Gamertag = gamertag,
                        Ip = ip,
                        MD5Password = md5Password,
                        RegSid = $"legacy_external:{provider}:{externalId}"
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

                accountStatus = 100;
            }
            else
            {
                customerId = account.CustomerId;
                accountStatus = account.AccountStatus;
            }

            if (accountStatus >= 200)
            {
                await tx.RollbackAsync();
                return new LoginResponse(false, 403, "ACCOUNT_BLOCKED", customerId, 0, accountStatus, "");
            }

            var sessionId = SecurityUtil.CreateSessionId();
            var sessionKey = SecurityUtil.CreateToken();

            await db.ExecuteAsync(
                """
                DELETE FROM loginsessions
                WHERE CustomerID = @CustomerId;
                """,
                new { CustomerId = customerId },
                tx);

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
                    CustomerId = customerId,
                    SessionKey = sessionKey,
                    SessionId = sessionId,
                    Ip = ip
                },
                tx);

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
                    CustomerId = customerId,
                    Ip = ip
                },
                tx);

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
                    CustomerId = customerId,
                    Ip = ip
                },
                tx);

            await tx.CommitAsync();

            return new LoginResponse(
                true,
                0,
                "OK",
                customerId,
                (ulong)sessionId,
                accountStatus,
                sessionKey);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return new LoginResponse(false, 500, e.Message, 0, 0, 0, "");
        }
    }

    public async Task<int> GetGamePointsBalanceAsync(ulong customerId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return 0;

        await using var db = _db.CreateConnection();

        return await db.ExecuteScalarAsync<int>(
            """
            SELECT IFNULL(GamePoints, 0)
            FROM loginid
            WHERE CustomerID = @CustomerId
            LIMIT 1;
            """,
            new { CustomerId = customerId });
    }

    private static string NormalizeProvider(string provider)
    {
        provider = SafeNameRegex.Replace((provider ?? "").Trim().ToLowerInvariant(), "");

        if (provider.Length > 8)
            provider = provider[..8];

        return provider;
    }

    private static string BuildAccountName(string provider, string externalId)
    {
        var safeId = SafeNameRegex.Replace(externalId, "");

        if (string.IsNullOrWhiteSpace(safeId))
            safeId = SecurityUtil.Sha256Hex(externalId)[..16];

        var value = $"{provider}_{safeId}";

        if (value.Length <= 32)
            return value;

        var hash = SecurityUtil.Sha256Hex(externalId)[..24];
        value = $"{provider}_{hash}";

        if (value.Length <= 32)
            return value;

        return value[..32];
    }

    private static string BuildGamertag(
        string provider,
        string externalId,
        string? nicknameRaw)
    {
        var nickname = SafeNameRegex.Replace((nicknameRaw ?? "").Trim(), "");

        if (string.IsNullOrWhiteSpace(nickname))
        {
            var safeId = SafeNameRegex.Replace(externalId, "");

            if (safeId.Length > 12)
                safeId = safeId[^12..];

            nickname = provider + safeId;
        }

        if (string.IsNullOrWhiteSpace(nickname))
            nickname = provider + SecurityUtil.Sha256Hex(externalId)[..8];

        if (nickname.Length > 31)
            nickname = nickname[..31];

        return nickname;
    }

    private static string BuildPassword(string provider, string externalId)
    {
        return SecurityUtil.Sha256Hex($"{provider}:{externalId}")[..16];
    }

    private sealed class LegacyExternalAccountRow
    {
        public ulong CustomerId { get; set; }
        public int AccountStatus { get; set; }
    }
}