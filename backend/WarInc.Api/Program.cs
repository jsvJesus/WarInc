using System.Security.Cryptography;
using System.Text;
using System.Globalization;
using System.Net;
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

app.MapGet("/v1/shop/items", async () =>
{
    await using var db = new MySqlConnection(dbConnectionString);

    var items = await db.QueryAsync<ShopItemRow>(
        """
        SELECT
            i.item_id AS ItemId,
            i.item_type AS ItemType,
            i.category AS Category,
            i.is_enabled AS IsEnabled,

            i.name AS Name,
            i.fname AS FName,
            i.description AS Description,

            i.level_required AS LevelRequired,

            i.weight AS Weight,
            i.damage_perc AS DamagePerc,
            i.damage_max AS DamageMax,

            i.damage AS Damage,
            i.damage_decay AS DamageDecay,
            i.num_clips AS NumClips,
            i.clip_size AS ClipSize,
            i.rate_of_fire AS RateOfFire,
            i.spread AS Spread,
            i.recoil AS Recoil,

            i.add_gp AS AddGP,
            i.add_sp AS AddSP,

            i.item1_id AS Item1Id,
            i.item1_exp AS Item1Exp,
            i.item2_id AS Item2Id,
            i.item2_exp AS Item2Exp,
            i.item3_id AS Item3Id,
            i.item3_exp AS Item3Exp,
            i.item4_id AS Item4Id,
            i.item4_exp AS Item4Exp,
            i.item5_id AS Item5Id,
            i.item5_exp AS Item5Exp,
            i.item6_id AS Item6Id,
            i.item6_exp AS Item6Exp,

            p.price1 AS Price1,
            p.price7 AS Price7,
            p.price30 AS Price30,
            p.price_perm AS PricePerm,
            p.gprice1 AS GPrice1,
            p.gprice7 AS GPrice7,
            p.gprice30 AS GPrice30,
            p.gprice_perm AS GPricePerm
        FROM game_items i
        LEFT JOIN shop_prices p ON p.item_id = i.item_id
        ORDER BY i.category, i.item_id;
        """);

    return Results.Json(new
    {
        ok = true,
        items
    });
});

app.MapPost("/api_GetShop5.aspx", LegacyGetShop5Async);
app.MapPost("/api/api_GetShop5.aspx", LegacyGetShop5Async);

app.MapPost("/api_GetItemsInfo.aspx", LegacyGetItemsInfoAsync);
app.MapPost("/api/api_GetItemsInfo.aspx", LegacyGetItemsInfoAsync);

async Task<IResult> LegacyGetShop5Async()
{
    await using var db = new MySqlConnection(dbConnectionString);

    var skills = (await db.QueryAsync<SkillPriceRow>(
        """
        SELECT
            skill_id AS SkillId,
            lv1 AS Lv1,
            lv2 AS Lv2,
            lv3 AS Lv3,
            lv4 AS Lv4,
            lv5 AS Lv5
        FROM shop_skill_prices
        WHERE is_enabled = 1
        ORDER BY skill_id;
        """)).ToList();

    var items = (await db.QueryAsync<ShopItemRow>(
        """
        SELECT
            i.item_id AS ItemId,
            i.item_type AS ItemType,
            i.category AS Category,
            i.is_enabled AS IsEnabled,

            i.name AS Name,
            i.fname AS FName,
            i.description AS Description,

            i.level_required AS LevelRequired,

            i.weight AS Weight,
            i.damage_perc AS DamagePerc,
            i.damage_max AS DamageMax,

            i.damage AS Damage,
            i.damage_decay AS DamageDecay,
            i.num_clips AS NumClips,
            i.clip_size AS ClipSize,
            i.rate_of_fire AS RateOfFire,
            i.spread AS Spread,
            i.recoil AS Recoil,

            i.add_gp AS AddGP,
            i.add_sp AS AddSP,

            i.item1_id AS Item1Id,
            i.item1_exp AS Item1Exp,
            i.item2_id AS Item2Id,
            i.item2_exp AS Item2Exp,
            i.item3_id AS Item3Id,
            i.item3_exp AS Item3Exp,
            i.item4_id AS Item4Id,
            i.item4_exp AS Item4Exp,
            i.item5_id AS Item5Id,
            i.item5_exp AS Item5Exp,
            i.item6_id AS Item6Id,
            i.item6_exp AS Item6Exp,

            p.price1 AS Price1,
            p.price7 AS Price7,
            p.price30 AS Price30,
            p.price_perm AS PricePerm,
            p.gprice1 AS GPrice1,
            p.gprice7 AS GPrice7,
            p.gprice30 AS GPrice30,
            p.gprice_perm AS GPricePerm
        FROM game_items i
        INNER JOIN shop_prices p ON p.item_id = i.item_id
        WHERE i.is_enabled = 1
          AND p.is_enabled = 1
          AND i.item_type <> 4
        ORDER BY i.category, i.item_id;
        """)).ToList();

    var packages = (await db.QueryAsync<ShopItemRow>(
        """
        SELECT
            i.item_id AS ItemId,
            i.item_type AS ItemType,
            i.category AS Category,
            i.is_enabled AS IsEnabled,

            i.name AS Name,
            i.fname AS FName,
            i.description AS Description,

            i.level_required AS LevelRequired,

            i.weight AS Weight,
            i.damage_perc AS DamagePerc,
            i.damage_max AS DamageMax,

            i.damage AS Damage,
            i.damage_decay AS DamageDecay,
            i.num_clips AS NumClips,
            i.clip_size AS ClipSize,
            i.rate_of_fire AS RateOfFire,
            i.spread AS Spread,
            i.recoil AS Recoil,

            i.add_gp AS AddGP,
            i.add_sp AS AddSP,

            i.item1_id AS Item1Id,
            i.item1_exp AS Item1Exp,
            i.item2_id AS Item2Id,
            i.item2_exp AS Item2Exp,
            i.item3_id AS Item3Id,
            i.item3_exp AS Item3Exp,
            i.item4_id AS Item4Id,
            i.item4_exp AS Item4Exp,
            i.item5_id AS Item5Id,
            i.item5_exp AS Item5Exp,
            i.item6_id AS Item6Id,
            i.item6_exp AS Item6Exp,

            p.price1 AS Price1,
            p.price7 AS Price7,
            p.price30 AS Price30,
            p.price_perm AS PricePerm,
            p.gprice1 AS GPrice1,
            p.gprice7 AS GPrice7,
            p.gprice30 AS GPrice30,
            p.gprice_perm AS GPricePerm
        FROM game_items i
        INNER JOIN shop_prices p ON p.item_id = i.item_id
        WHERE i.is_enabled = 1
          AND p.is_enabled = 1
          AND i.item_type = 4
        ORDER BY i.category, i.item_id;
        """)).ToList();

    var shopData = new MemoryStream();
    var header = Encoding.ASCII.GetBytes("SHO3");

    shopData.Write(header, 0, header.Length);

    foreach (var skill in skills)
    {
        WriteUInt16LE(shopData, skill.SkillId);
        shopData.WriteByte(skill.Lv1);
        shopData.WriteByte(skill.Lv2);
        shopData.WriteByte(skill.Lv3);
        shopData.WriteByte(skill.Lv4);
        shopData.WriteByte(skill.Lv5);
    }

    WriteUInt16LE(shopData, 0xFFFF);

    shopData.Write(header, 0, header.Length);

    foreach (var item in items)
        WriteLegacyShopItem(shopData, item);

    foreach (var item in packages)
        WriteLegacyShopItem(shopData, item);

    shopData.Write(header, 0, header.Length);

    var prefix = Encoding.ASCII.GetBytes("WO_0");
    var payload = shopData.ToArray();

    var result = new byte[prefix.Length + payload.Length];

    Buffer.BlockCopy(prefix, 0, result, 0, prefix.Length);
    Buffer.BlockCopy(payload, 0, result, prefix.Length, payload.Length);

    return Results.Bytes(result, "application/octet-stream");
}

async Task<IResult> LegacyGetItemsInfoAsync()
{
    await using var db = new MySqlConnection(dbConnectionString);

    var rows = (await db.QueryAsync<ShopItemRow>(
        """
        SELECT
            i.item_id AS ItemId,
            i.item_type AS ItemType,
            i.category AS Category,
            i.is_enabled AS IsEnabled,

            i.name AS Name,
            i.fname AS FName,
            i.description AS Description,

            i.level_required AS LevelRequired,

            i.weight AS Weight,
            i.damage_perc AS DamagePerc,
            i.damage_max AS DamageMax,

            i.damage AS Damage,
            i.damage_decay AS DamageDecay,
            i.num_clips AS NumClips,
            i.clip_size AS ClipSize,
            i.rate_of_fire AS RateOfFire,
            i.spread AS Spread,
            i.recoil AS Recoil,

            i.add_gp AS AddGP,
            i.add_sp AS AddSP,

            i.item1_id AS Item1Id,
            i.item1_exp AS Item1Exp,
            i.item2_id AS Item2Id,
            i.item2_exp AS Item2Exp,
            i.item3_id AS Item3Id,
            i.item3_exp AS Item3Exp,
            i.item4_id AS Item4Id,
            i.item4_exp AS Item4Exp,
            i.item5_id AS Item5Id,
            i.item5_exp AS Item5Exp,
            i.item6_id AS Item6Id,
            i.item6_exp AS Item6Exp,

            p.price1 AS Price1,
            p.price7 AS Price7,
            p.price30 AS Price30,
            p.price_perm AS PricePerm,
            p.gprice1 AS GPrice1,
            p.gprice7 AS GPrice7,
            p.gprice30 AS GPrice30,
            p.gprice_perm AS GPricePerm
        FROM game_items i
        LEFT JOIN shop_prices p ON p.item_id = i.item_id
        WHERE i.is_enabled = 1
        ORDER BY i.category, i.item_id;
        """)).ToList();

    var xml = new StringBuilder();

    xml.Append("<?xml version=\"1.0\"?>\n");
    xml.Append("<items>");

    xml.Append("<gears>");
    foreach (var item in rows)
    {
        if (item.ItemType != 1)
            continue;

        if (!HasAnyPrice(item))
            continue;

        xml.Append("<g ");
        AppendXmlAttr(xml, "ID", item.ItemId);
        AppendXmlAttr(xml, "lv", item.LevelRequired);
        AppendXmlAttr(xml, "wg", item.Weight);
        AppendXmlAttr(xml, "dp", item.DamagePerc);
        AppendXmlAttr(xml, "dm", item.DamageMax);
        xml.Append("/>\n");
    }
    xml.Append("</gears>");

    xml.Append("<weapons>");
    foreach (var item in rows)
    {
        if (item.ItemType != 2)
            continue;

        if (!HasAnyPrice(item))
            continue;

        xml.Append("<w ");
        AppendXmlAttr(xml, "ID", item.ItemId);
        AppendXmlAttr(xml, "lv", item.LevelRequired);
        AppendXmlAttr(xml, "d1", item.Damage);
        AppendXmlAttr(xml, "d2", item.DamageDecay);
        AppendXmlAttr(xml, "c1", item.NumClips);
        AppendXmlAttr(xml, "c2", item.ClipSize);
        AppendXmlAttr(xml, "rf", item.RateOfFire);
        AppendXmlAttr(xml, "sp", item.Spread);
        AppendXmlAttr(xml, "rc", item.Recoil);
        xml.Append("/>\n");
    }
    xml.Append("</weapons>");

    xml.Append("<generics>");
    foreach (var item in rows)
    {
        if (item.ItemType != 3)
            continue;

        if (!HasAnyPrice(item))
            continue;

        if (item.Category != 3 && item.Category != 7)
            continue;

        xml.Append("<b ");
        AppendXmlAttr(xml, "ID", item.ItemId);
        AppendXmlAttr(xml, "name", item.Name);
        AppendXmlAttr(xml, "fname", item.FName);
        AppendXmlAttr(xml, "desc", item.Description);
        xml.Append("/>\n");
    }
    xml.Append("</generics>");

    xml.Append("<packages>");
    foreach (var item in rows)
    {
        if (item.ItemType != 4)
            continue;

        if (!HasAnyPrice(item))
            continue;

        xml.Append("<p ");
        AppendXmlAttr(xml, "ID", item.ItemId);
        AppendXmlAttr(xml, "name", item.Name);
        AppendXmlAttr(xml, "fname", item.FName);
        AppendXmlAttr(xml, "desc", item.Description);
        AppendXmlAttr(xml, "gd", item.AddGP);
        AppendXmlAttr(xml, "sp", item.AddSP);
        AppendXmlAttr(xml, "i1i", item.Item1Id);
        AppendXmlAttr(xml, "i1e", item.Item1Exp);
        AppendXmlAttr(xml, "i2i", item.Item2Id);
        AppendXmlAttr(xml, "i2e", item.Item2Exp);
        AppendXmlAttr(xml, "i3i", item.Item3Id);
        AppendXmlAttr(xml, "i3e", item.Item3Exp);
        AppendXmlAttr(xml, "i4i", item.Item4Id);
        AppendXmlAttr(xml, "i4e", item.Item4Exp);
        AppendXmlAttr(xml, "i5i", item.Item5Id);
        AppendXmlAttr(xml, "i5e", item.Item5Exp);
        AppendXmlAttr(xml, "i6i", item.Item6Id);
        AppendXmlAttr(xml, "i6e", item.Item6Exp);
        xml.Append("/>\n");
    }
    xml.Append("</packages>");

    xml.Append("</items>");

    return Results.Text(xml.ToString(), "text/xml", Encoding.UTF8);
}

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

static void WriteLegacyShopItem(MemoryStream stream, ShopItemRow item)
{
    var priceBits = GetPriceBits(item);

    if (priceBits == 0)
        return;

    WriteInt32LE(stream, (int)item.ItemId);
    stream.WriteByte((byte)priceBits);
    stream.WriteByte((byte)item.Category);

    if ((priceBits & 1) != 0)
        WriteInt32LE(stream, item.Price1);

    if ((priceBits & 2) != 0)
        WriteInt32LE(stream, item.Price7);

    if ((priceBits & 4) != 0)
        WriteInt32LE(stream, item.Price30);

    if ((priceBits & 8) != 0)
        WriteInt32LE(stream, item.PricePerm);

    if ((priceBits & 16) != 0)
        WriteInt32LE(stream, item.GPrice1);

    if ((priceBits & 32) != 0)
        WriteInt32LE(stream, item.GPrice7);

    if ((priceBits & 64) != 0)
        WriteInt32LE(stream, item.GPrice30);

    if ((priceBits & 128) != 0)
        WriteInt32LE(stream, item.GPricePerm);
}

static int GetPriceBits(ShopItemRow item)
{
    var bits = 0;

    if (item.Price1 > 0)
        bits |= 1;

    if (item.Price7 > 0)
        bits |= 2;

    if (item.Price30 > 0)
        bits |= 4;

    if (item.PricePerm > 0)
        bits |= 8;

    if (item.GPrice1 > 0)
        bits |= 16;

    if (item.GPrice7 > 0)
        bits |= 32;

    if (item.GPrice30 > 0)
        bits |= 64;

    if (item.GPricePerm > 0)
        bits |= 128;

    return bits;
}

static bool HasAnyPrice(ShopItemRow item)
{
    return GetPriceBits(item) != 0;
}

static void WriteInt32LE(MemoryStream stream, int value)
{
    stream.WriteByte((byte)((value >> 0) & 0xFF));
    stream.WriteByte((byte)((value >> 8) & 0xFF));
    stream.WriteByte((byte)((value >> 16) & 0xFF));
    stream.WriteByte((byte)((value >> 24) & 0xFF));
}

static void WriteUInt16LE(MemoryStream stream, int value)
{
    stream.WriteByte((byte)((value >> 0) & 0xFF));
    stream.WriteByte((byte)((value >> 8) & 0xFF));
}

static void AppendXmlAttr(StringBuilder xml, string name, object? value)
{
    var text = Convert.ToString(value, CultureInfo.InvariantCulture) ?? "";
    xml.Append(name);
    xml.Append("=\"");
    xml.Append(WebUtility.HtmlEncode(text));
    xml.Append("\" ");
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

class SkillPriceRow
{
    public int SkillId { get; set; }
    public byte Lv1 { get; set; }
    public byte Lv2 { get; set; }
    public byte Lv3 { get; set; }
    public byte Lv4 { get; set; }
    public byte Lv5 { get; set; }
}

class ShopItemRow
{
    public uint ItemId { get; set; }
    public int ItemType { get; set; }
    public int Category { get; set; }
    public bool IsEnabled { get; set; }

    public string Name { get; set; } = "";
    public string FName { get; set; } = "";
    public string Description { get; set; } = "";

    public int LevelRequired { get; set; }

    public decimal Weight { get; set; }
    public decimal DamagePerc { get; set; }
    public decimal DamageMax { get; set; }

    public decimal Damage { get; set; }
    public decimal DamageDecay { get; set; }
    public int NumClips { get; set; }
    public int ClipSize { get; set; }
    public decimal RateOfFire { get; set; }
    public decimal Spread { get; set; }
    public decimal Recoil { get; set; }

    public int AddGP { get; set; }
    public int AddSP { get; set; }

    public uint Item1Id { get; set; }
    public int Item1Exp { get; set; }
    public uint Item2Id { get; set; }
    public int Item2Exp { get; set; }
    public uint Item3Id { get; set; }
    public int Item3Exp { get; set; }
    public uint Item4Id { get; set; }
    public int Item4Exp { get; set; }
    public uint Item5Id { get; set; }
    public int Item5Exp { get; set; }
    public uint Item6Id { get; set; }
    public int Item6Exp { get; set; }

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}

class SessionRow
{
    public ulong SessionId { get; set; }
    public ulong CustomerId { get; set; }
    public DateTime ExpiresAt { get; set; }
    public DateTime? RevokedAt { get; set; }
    public int AccountStatus { get; set; }
}