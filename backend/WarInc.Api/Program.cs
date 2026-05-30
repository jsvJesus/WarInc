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

app.MapGet("/", () =>
{
    return Results.Json(new
    {
        ok = true,
        service = "WarInc.Api",
        message = "WarInc backend is running",
        endpoints = new[]
        {
            "/health",
            "/v1/auth/login",
            "/v1/auth/check",
            "/v1/auth/logout",
            "/v1/shop/items",
            "/v1/shop/buy",
            "/api_Login.aspx",
            "/api_CheckLoginSession.aspx",
            "/api_UpdateLoginSession.aspx",
            "/api_GetShop5.aspx",
            "/api_GetItemsInfo.aspx"
            "/api_BuyItem3.aspx",
        }
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
        sessionHours,
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

    var rows = await db.ExecuteAsync(
        """
        DELETE FROM loginsessions
        WHERE CustomerID = @CustomerId
          AND SessionID = @SessionId
          AND SessionKey = @SessionKey;
        """,
        new
        {
            request.CustomerId,
            request.SessionId,
            SessionKey = request.Token
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
        sessionHours,
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
        sessionHours,
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
        sessionHours,
        customerId,
        sessionId);

    if (!result.Ok)
        return Results.Text("WO_1", "text/plain", Encoding.UTF8);

    await using var db = new MySqlConnection(dbConnectionString);

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

    if (rows <= 0)
        return Results.Text("WO_1", "text/plain", Encoding.UTF8);

    return Results.Text("WO_0", "text/plain", Encoding.UTF8);
}

app.MapPost("/dev/create-account", async (HttpContext http, CreateAccountRequest request) =>
{
    var username = (request.Username ?? "").Trim();
    var password = request.Password ?? "";
    var ip = NormalizeLegacyIp(GetClientIp(http));

    if (username.Length < 3 || username.Length > 32)
        return Results.Json(new ApiResponse(false, 400, "BAD_USERNAME"));

    if (password.Length < 3 || password.Length > 16)
        return Results.Json(new ApiResponse(false, 400, "BAD_PASSWORD_SCHEMA_MAX_16"));

    await using var db = new MySqlConnection(dbConnectionString);

    var exists = await db.ExecuteScalarAsync<int>(
        """
        SELECT COUNT(*)
        FROM loginid
        WHERE AccountName = @Username;
        """,
        new { Username = username });

    if (exists > 0)
        return Results.Json(new CreateAccountResponse(false, 409, "ACCOUNT_ALREADY_EXISTS", 0));

    var md5Password = Md5Hex(password);

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
        });

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
        new { CustomerId = customerId });

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
        new { CustomerId = customerId });
    
    await EnsureStarterProfileAsync(db, customerId);

    return Results.Json(new CreateAccountResponse(
        true,
        0,
        "OK",
        customerId));
});

app.MapGet("/v1/shop/items", async () =>
{
    await using var db = new MySqlConnection(dbConnectionString);

    var gear = await db.QueryAsync<ShopItemRow>(
        """
        SELECT
            ItemID AS ItemId,
            1 AS ItemType,
            Category AS Category,
            1 AS IsEnabled,
            Name AS Name,
            FNAME AS FName,
            Description AS Description,
            LevelRequired AS LevelRequired,
            Weight AS Weight,
            DamagePerc AS DamagePerc,
            DamageMax AS DamageMax,
            0 AS Damage,
            0 AS DamageDecay,
            0 AS NumClips,
            0 AS ClipSize,
            60 AS RateOfFire,
            0 AS Spread,
            0 AS Recoil,
            0 AS AddGP,
            0 AS AddSP,
            0 AS Item1Id,
            0 AS Item1Exp,
            0 AS Item2Id,
            0 AS Item2Exp,
            0 AS Item3Id,
            0 AS Item3Exp,
            0 AS Item4Id,
            0 AS Item4Exp,
            0 AS Item5Id,
            0 AS Item5Exp,
            0 AS Item6Id,
            0 AS Item6Exp,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_gear
        ORDER BY ItemID;
        """);

    var weapons = await db.QueryAsync<ShopItemRow>(
        """
        SELECT
            ItemID AS ItemId,
            2 AS ItemType,
            Category AS Category,
            1 AS IsEnabled,
            Name AS Name,
            FNAME AS FName,
            Description AS Description,
            LevelRequired AS LevelRequired,
            0 AS Weight,
            0 AS DamagePerc,
            0 AS DamageMax,
            Damage AS Damage,
            DamageDecay AS DamageDecay,
            NumClips AS NumClips,
            Clipsize AS ClipSize,
            RateOfFire AS RateOfFire,
            Spread AS Spread,
            Recoil AS Recoil,
            0 AS AddGP,
            0 AS AddSP,
            0 AS Item1Id,
            0 AS Item1Exp,
            0 AS Item2Id,
            0 AS Item2Exp,
            0 AS Item3Id,
            0 AS Item3Exp,
            0 AS Item4Id,
            0 AS Item4Exp,
            0 AS Item5Id,
            0 AS Item5Exp,
            0 AS Item6Id,
            0 AS Item6Exp,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_weapons
        ORDER BY ItemID;
        """);

    var generics = await db.QueryAsync<ShopItemRow>(
        """
        SELECT
            ItemID AS ItemId,
            3 AS ItemType,
            Category AS Category,
            1 AS IsEnabled,
            Name AS Name,
            FNAME AS FName,
            Description AS Description,
            LevelRequired AS LevelRequired,
            0 AS Weight,
            0 AS DamagePerc,
            0 AS DamageMax,
            0 AS Damage,
            0 AS DamageDecay,
            0 AS NumClips,
            0 AS ClipSize,
            60 AS RateOfFire,
            0 AS Spread,
            0 AS Recoil,
            0 AS AddGP,
            0 AS AddSP,
            0 AS Item1Id,
            0 AS Item1Exp,
            0 AS Item2Id,
            0 AS Item2Exp,
            0 AS Item3Id,
            0 AS Item3Exp,
            0 AS Item4Id,
            0 AS Item4Exp,
            0 AS Item5Id,
            0 AS Item5Exp,
            0 AS Item6Id,
            0 AS Item6Exp,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_generic
        ORDER BY ItemID;
        """);

    var attachments = await db.QueryAsync<ShopItemRow>(
        """
        SELECT
            ItemID AS ItemId,
            5 AS ItemType,
            Category AS Category,
            1 AS IsEnabled,
            Name AS Name,
            FNAME AS FName,
            Description AS Description,
            LevelRequired AS LevelRequired,
            0 AS Weight,
            0 AS DamagePerc,
            0 AS DamageMax,
            Damage AS Damage,
            0 AS DamageDecay,
            0 AS NumClips,
            Clipsize AS ClipSize,
            Firerate AS RateOfFire,
            Spread AS Spread,
            Recoil AS Recoil,
            0 AS AddGP,
            0 AS AddSP,
            0 AS Item1Id,
            0 AS Item1Exp,
            0 AS Item2Id,
            0 AS Item2Exp,
            0 AS Item3Id,
            0 AS Item3Exp,
            0 AS Item4Id,
            0 AS Item4Exp,
            0 AS Item5Id,
            0 AS Item5Exp,
            0 AS Item6Id,
            0 AS Item6Exp,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_attachments
        ORDER BY ItemID;
        """);

    var packages = await db.QueryAsync<ShopItemRow>(
        """
        SELECT
            ItemID AS ItemId,
            4 AS ItemType,
            Category AS Category,
            IsEnabled AS IsEnabled,
            Name AS Name,
            FNAME AS FName,
            Description AS Description,
            LevelRequired AS LevelRequired,
            0 AS Weight,
            0 AS DamagePerc,
            0 AS DamageMax,
            0 AS Damage,
            0 AS DamageDecay,
            0 AS NumClips,
            0 AS ClipSize,
            60 AS RateOfFire,
            0 AS Spread,
            0 AS Recoil,
            AddGP AS AddGP,
            AddSP AS AddSP,
            Item1_ID AS Item1Id,
            Item1_Exp AS Item1Exp,
            Item2_ID AS Item2Id,
            Item2_Exp AS Item2Exp,
            Item3_ID AS Item3Id,
            Item3_Exp AS Item3Exp,
            Item4_ID AS Item4Id,
            Item4_Exp AS Item4Exp,
            Item5_ID AS Item5Id,
            Item5_Exp AS Item5Exp,
            Item6_ID AS Item6Id,
            Item6_Exp AS Item6Exp,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_packages
        ORDER BY ItemID;
        """);

    return Results.Json(new
    {
        ok = true,
        gear,
        weapons,
        generics,
        attachments,
        packages
    });
});

app.MapPost("/api_GetShop5.aspx", LegacyGetShop5Async);
app.MapPost("/api/api_GetShop5.aspx", LegacyGetShop5Async);

app.MapPost("/api_GetItemsInfo.aspx", LegacyGetItemsInfoAsync);
app.MapPost("/api/api_GetItemsInfo.aspx", LegacyGetItemsInfoAsync);

app.MapPost("/api_GetProfile4.aspx", LegacyGetProfile4Async);
app.MapPost("/api/api_GetProfile4.aspx", LegacyGetProfile4Async);

app.MapPost("/v1/profile/get", async (ProfileRequest request) =>
{
    var session = await CheckSessionAsync(
        dbConnectionString,
        sessionHours,
        request.CustomerId,
        request.SessionId,
        request.Token);

    if (!session.Ok)
        return Results.Json(new ApiResponse(false, session.Code, session.Message));

    var profile = await GetProfileDataAsync(
        dbConnectionString,
        request.CustomerId,
        request.JoiningGame);

    if (!profile.Ok)
        return Results.Json(new ApiResponse(false, profile.Code, profile.Message));

    return Results.Json(profile);
});

app.MapPost("/api_BuyItem3.aspx", LegacyBuyItem3Async);
app.MapPost("/api/api_BuyItem3.aspx", LegacyBuyItem3Async);

app.MapPost("/v1/shop/buy", async (BuyItemRequest request) =>
{
    var session = await CheckSessionAsync(
        dbConnectionString,
        sessionHours,
        request.CustomerId,
        request.SessionId,
        request.Token);

    if (!session.Ok)
        return Results.Json(new BuyItemResponse(
            false,
            session.Code,
            session.Message,
            0,
            false));

    var result = await BuyItemAsync(
        dbConnectionString,
        request.CustomerId,
        request.ItemId,
        request.BuyIdx,
        request.Param1);

    return Results.Json(result);
});

async Task<IResult> LegacyBuyItem3Async(HttpContext http)
{
    var form = await http.Request.ReadFormAsync();

    var customerId = ParseULong(form["s_id"].ToString());
    var sessionId = ParseULong(form["s_key"].ToString());

    var itemId = ParseInt(form["ItemID"].ToString());
    var buyIdx = ParseInt(form["BuyIdx"].ToString());
    var param1 = ParseInt(form["Param1"].ToString());

    var session = await CheckLegacySessionAsync(
        dbConnectionString,
        sessionHours,
        customerId,
        sessionId);

    if (!session.Ok)
        return Results.Text("WO_1", "text/plain", Encoding.UTF8);

    var result = await BuyItemAsync(
        dbConnectionString,
        customerId,
        itemId,
        buyIdx,
        param1);

    if (!result.Ok)
        return Results.Text(
            $"WO_{NormalizeLegacyErrorCode(result.Code)} {result.Message}",
            "text/plain",
            Encoding.UTF8);

    return Results.Text(
        $"WO_0{result.Balance}",
        "text/plain",
        Encoding.UTF8);
}

async Task<IResult> LegacyGetProfile4Async(HttpContext http)
{
    var form = await http.Request.ReadFormAsync();

    var customerId = ParseULong(form["s_id"].ToString());
    var sessionId = ParseULong(form["s_key"].ToString());

    var joiningGame = false;
    var jg = form["jg"].ToString();

    if (!string.IsNullOrWhiteSpace(jg))
        joiningGame = jg != "0";

    var session = await CheckLegacySessionAsync(
        dbConnectionString,
        sessionHours,
        customerId,
        sessionId);

    if (!session.Ok)
        return Results.Text("WO_1", "text/plain", Encoding.UTF8);

    var profile = await GetProfileDataAsync(
        dbConnectionString,
        customerId,
        joiningGame);

    if (!profile.Ok || profile.Account == null)
        return Results.Text("WO_6", "text/plain", Encoding.UTF8);

    var xml = BuildLegacyProfileXml(profile);

    return Results.Text(xml, "text/xml", Encoding.UTF8);
}

async Task<IResult> LegacyGetShop5Async()
{
    await using var db = new MySqlConnection(dbConnectionString);

    var skills = (await db.QueryAsync<SkillPriceRow>(
        """
        SELECT
            SkillID AS SkillId,
            IFNULL(Lv1, 0) AS Lv1,
            IFNULL(Lv2, 0) AS Lv2,
            IFNULL(Lv3, 0) AS Lv3,
            IFNULL(Lv4, 0) AS Lv4,
            IFNULL(Lv5, 0) AS Lv5
        FROM dataskill2price
        ORDER BY SkillID;
        """)).ToList();

    var commonItems = (await db.QueryAsync<LegacyShopPriceRow>(
        """
        SELECT
            ItemID AS ItemId,
            Category AS Category,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_gear

        UNION ALL

        SELECT
            ItemID AS ItemId,
            Category AS Category,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_weapons

        UNION ALL

        SELECT
            ItemID AS ItemId,
            Category AS Category,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_generic

        UNION ALL

        SELECT
            ItemID AS ItemId,
            Category AS Category,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_attachments

        ORDER BY ItemId;
        """)).ToList();

    var packages = (await db.QueryAsync<LegacyShopPriceRow>(
        """
        SELECT
            ItemID AS ItemId,
            Category AS Category,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_packages
        WHERE IsEnabled <> 0
        ORDER BY ItemID;
        """)).ToList();

    var shopData = new MemoryStream();
    var header = Encoding.ASCII.GetBytes("SHO3");

    shopData.Write(header, 0, header.Length);

    foreach (var skill in skills)
    {
        WriteUInt16LE(shopData, skill.SkillId);
        shopData.WriteByte((byte)ClampByte(skill.Lv1));
        shopData.WriteByte((byte)ClampByte(skill.Lv2));
        shopData.WriteByte((byte)ClampByte(skill.Lv3));
        shopData.WriteByte((byte)ClampByte(skill.Lv4));
        shopData.WriteByte((byte)ClampByte(skill.Lv5));
    }

    WriteUInt16LE(shopData, 0xFFFF);

    shopData.Write(header, 0, header.Length);

    foreach (var item in commonItems)
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

    var gears = (await db.QueryAsync<GearItemRow>(
        """
        SELECT
            ItemID AS ItemId,
            LevelRequired AS LevelRequired,
            Weight AS Weight,
            DamagePerc AS DamagePerc,
            DamageMax AS DamageMax,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_gear
        ORDER BY ItemID;
        """)).ToList();

    var weapons = (await db.QueryAsync<WeaponItemRow>(
        """
        SELECT
            ItemID AS ItemId,
            LevelRequired AS LevelRequired,
            Damage AS Damage,
            DamageDecay AS DamageDecay,
            NumClips AS NumClips,
            Clipsize AS ClipSize,
            RateOfFire AS RateOfFire,
            Spread AS Spread,
            Recoil AS Recoil,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_weapons
        ORDER BY ItemID;
        """)).ToList();

    var generics = (await db.QueryAsync<GenericItemRow>(
        """
        SELECT
            ItemID AS ItemId,
            Category AS Category,
            Name AS Name,
            FNAME AS FName,
            Description AS Description,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_generic
        ORDER BY ItemID;
        """)).ToList();

    var packages = (await db.QueryAsync<PackageItemRow>(
        """
        SELECT
            ItemID AS ItemId,
            Name AS Name,
            FNAME AS FName,
            Description AS Description,
            AddGP AS AddGP,
            AddSP AS AddSP,
            Item1_ID AS Item1Id,
            Item1_Exp AS Item1Exp,
            Item2_ID AS Item2Id,
            Item2_Exp AS Item2Exp,
            Item3_ID AS Item3Id,
            Item3_Exp AS Item3Exp,
            Item4_ID AS Item4Id,
            Item4_Exp AS Item4Exp,
            Item5_ID AS Item5Id,
            Item5_Exp AS Item5Exp,
            Item6_ID AS Item6Id,
            Item6_Exp AS Item6Exp,
            Price1 AS Price1,
            Price7 AS Price7,
            Price30 AS Price30,
            PriceP AS PricePerm,
            GPrice1 AS GPrice1,
            GPrice7 AS GPrice7,
            GPrice30 AS GPrice30,
            GPriceP AS GPricePerm
        FROM items_packages
        WHERE IsEnabled <> 0
        ORDER BY ItemID;
        """)).ToList();

    var xml = new StringBuilder();

    xml.Append("<?xml version=\"1.0\"?>\n");
    xml.Append("<items>");

    xml.Append("<gears>");
    foreach (var item in gears)
    {
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
    foreach (var item in weapons)
    {
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
    foreach (var item in generics)
    {
        if (!HasAnyPrice(item))
            continue;

        if (item.Category != 7 && item.Category != 3)
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
    foreach (var item in packages)
    {
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

    if (username.Length < 3 || username.Length > 32)
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

    if (password.Length < 1 || password.Length > 128)
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

    if (!PasswordMatches(password, account.Password, account.MD5Password))
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
    var sessionKey = CreateToken();
    var ip = NormalizeLegacyIp(GetClientIp(http));

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

static async Task<CheckSessionResponse> CheckSessionAsync(
    string dbConnectionString,
    int sessionHours,
    ulong customerId,
    ulong sessionId,
    string? tokenRaw)
{
    var token = tokenRaw ?? "";

    if (customerId == 0 || sessionId == 0 || sessionId > int.MaxValue)
        return new CheckSessionResponse(false, 400, "BAD_SESSION", 0);

    if (string.IsNullOrWhiteSpace(token))
        return new CheckSessionResponse(false, 400, "BAD_TOKEN", 0);

    await using var db = new MySqlConnection(dbConnectionString);

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

    if (session == null)
        return new CheckSessionResponse(false, 401, "SESSION_NOT_FOUND", 0);

    var minTime = DateTime.UtcNow.AddHours(-sessionHours);

    if (session.TimeUpdated < minTime)
        return new CheckSessionResponse(false, 401, "SESSION_EXPIRED", 0);

    if (session.AccountStatus >= 200)
        return new CheckSessionResponse(false, 403, "ACCOUNT_BLOCKED", session.AccountStatus);

    return new CheckSessionResponse(true, 0, "OK", session.AccountStatus);
}

static async Task<CheckSessionResponse> CheckLegacySessionAsync(
    string dbConnectionString,
    int sessionHours,
    ulong customerId,
    ulong sessionId)
{
    if (customerId == 0 || sessionId == 0 || sessionId > int.MaxValue)
        return new CheckSessionResponse(false, 400, "BAD_SESSION", 0);

    await using var db = new MySqlConnection(dbConnectionString);

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

    if (session == null)
        return new CheckSessionResponse(false, 401, "SESSION_NOT_FOUND", 0);

    var minTime = DateTime.UtcNow.AddHours(-sessionHours);

    if (session.TimeUpdated < minTime)
        return new CheckSessionResponse(false, 401, "SESSION_EXPIRED", 0);

    if (session.AccountStatus >= 200)
        return new CheckSessionResponse(false, 403, "ACCOUNT_BLOCKED", session.AccountStatus);

    return new CheckSessionResponse(true, 0, "OK", session.AccountStatus);
}

static async Task<ProfileBundle> GetProfileDataAsync(
    string dbConnectionString,
    ulong customerId,
    bool joiningGame)
{
    if (customerId == 0 || customerId > int.MaxValue)
        return ProfileBundle.Fail(400, "BAD_CUSTOMER_ID");

    await using var db = new MySqlConnection(dbConnectionString);

    if (joiningGame)
    {
        await db.ExecuteAsync(
            """
            UPDATE loginid
            SET lastjoineddate = UTC_TIMESTAMP()
            WHERE CustomerID = @CustomerId;
            """,
            new { CustomerId = customerId });
    }

    var account = await db.QueryFirstOrDefaultAsync<ProfileAccountRow>(
        """
        SELECT
            l.CustomerID AS CustomerId,
            l.AccountStatus AS AccountStatus,
            l.GamePoints AS GamePoints,
            l.GameDollars AS GameDollars,
            l.HonorPoints AS HonorPoints,
            l.SkillPoints AS SkillPoints,
            l.Gamertag AS Gamertag,

            l.Faction1Score AS Faction1Score,
            l.Faction2Score AS Faction2Score,
            l.Faction3Score AS Faction3Score,
            l.Faction4Score AS Faction4Score,
            l.Faction5Score AS Faction5Score,

            l.ClanID AS ClanId,
            l.ClanRank AS ClanRank,
            l.IsFPSEnabled AS IsFPSEnabled,
            l.IsDeveloper AS IsDeveloper,

            IFNULL(s.Kills, 0) AS Kills,
            IFNULL(s.Deaths, 0) AS Deaths,
            IFNULL(s.ShotsFired, 0) AS ShotsFired,
            IFNULL(s.ShotsHits, 0) AS ShotsHits,
            IFNULL(s.Headshots, 0) AS Headshots,
            IFNULL(s.AssistKills, 0) AS AssistKills,
            IFNULL(s.Wins, 0) AS Wins,
            IFNULL(s.Losses, 0) AS Losses,
            IFNULL(s.CaptureNeutralPoints, 0) AS CaptureNeutralPoints,
            IFNULL(s.CaptureEnemyPoints, 0) AS CaptureEnemyPoints,
            IFNULL(s.TimePlayed, 0) AS TimePlayed,

            IFNULL(pd.Skills, '') AS Skills,
            IFNULL(pd.Abilities, '') AS Abilities,

            IFNULL(cd.ClanTag, '') AS ClanTag,
            IFNULL(cd.ClanTagColor, 0) AS ClanTagColor
        FROM loginid l
        LEFT JOIN stats s ON s.CustomerID = l.CustomerID
        LEFT JOIN profiledata pd ON pd.CustomerID = l.CustomerID
        LEFT JOIN clandata cd ON cd.ClanID = l.ClanID
        WHERE l.CustomerID = @CustomerId
        LIMIT 1;
        """,
        new { CustomerId = customerId });

    if (account == null)
        return ProfileBundle.Fail(404, "ACCOUNT_NOT_FOUND");

    var loadouts = (await db.QueryAsync<ProfileLoadoutRow>(
        """
        SELECT
            LoadoutID AS LoadoutId,
            Class AS Class,
            HonorPoints AS HonorPoints,
            TimePlayed AS TimePlayed,
            Loadout AS Loadout,
            Skills AS Skills,
            SpendSP1 AS SpendSP1,
            SpendSP2 AS SpendSP2,
            SpendSP3 AS SpendSP3
        FROM profile_chars
        WHERE CustomerID = @CustomerId
        ORDER BY LoadoutID ASC;
        """,
        new { CustomerId = customerId })).ToList();

    var achievements = (await db.QueryAsync<ProfileAchievementRow>(
        """
        SELECT
            AchID AS AchId,
            Value AS Value,
            Unlocked AS Unlocked
        FROM achievements
        WHERE CustomerID = @CustomerId
        ORDER BY AchID ASC;
        """,
        new { CustomerId = customerId })).ToList();

    var fpsAttachments = (await db.QueryAsync<ProfileFpsAttachmentRow>(
        """
        SELECT
            WeaponID AS WeaponId,
            AttachmentID AS AttachmentId,
            GREATEST(TIMESTAMPDIFF(MINUTE, UTC_TIMESTAMP(), LeasedUntil), 0) AS MinutesLeft,
            IsEquipped AS IsEquipped
        FROM inventory_fps
        WHERE CustomerID = @CustomerId
          AND LeasedUntil > UTC_TIMESTAMP()
        ORDER BY WeaponID ASC, Slot ASC;
        """,
        new { CustomerId = customerId })).ToList();

    var inventory = (await db.QueryAsync<ProfileInventoryRow>(
        """
        SELECT
            ItemID AS ItemId,
            GREATEST(TIMESTAMPDIFF(MINUTE, UTC_TIMESTAMP(), LeasedUntil), 0) AS MinutesLeft,
            Quantity AS Quantity
        FROM inventory
        WHERE CustomerID = @CustomerId
          AND LeasedUntil > UTC_TIMESTAMP()
        ORDER BY ItemID ASC;
        """,
        new { CustomerId = customerId })).ToList();

    var newItems = (await db.QueryAsync<int>(
        """
        SELECT ItemID
        FROM items_weapons
        WHERE IsNew > 0
          AND (Price1 + Price7 + Price30 + PriceP + GPrice1 + GPrice7 + GPrice30 + GPriceP) > 0

        UNION

        SELECT ItemID
        FROM items_gear
        WHERE IsNew > 0
          AND (Price1 + Price7 + Price30 + PriceP + GPrice1 + GPrice7 + GPrice30 + GPriceP) > 0

        UNION

        SELECT ItemID
        FROM items_generic
        WHERE IsNew > 0
          AND (Price1 + Price7 + Price30 + PriceP + GPrice1 + GPrice7 + GPrice30 + GPriceP) > 0

        UNION

        SELECT ItemID
        FROM items_packages
        WHERE IsNew > 0
          AND (Price1 + Price7 + Price30 + PriceP + GPrice1 + GPrice7 + GPrice30 + GPriceP) > 0

        ORDER BY ItemID ASC;
        """)).ToList();

    var now = DateTime.UtcNow;
    var dayStart = now.Date;
    var dayEnd = dayStart.AddDays(1);

    var weekStart = now.Date.AddDays(-(int)now.DayOfWeek);
    var weekEnd = weekStart.AddDays(7);

    var dailyStats = await GetProfileStatsWindowAsync(db, customerId, dayStart, dayEnd);
    var weeklyStats = await GetProfileStatsWindowAsync(db, customerId, weekStart, weekEnd);

    return new ProfileBundle
    {
        Ok = true,
        Code = 0,
        Message = "OK",
        Account = account,
        Loadouts = loadouts,
        Achievements = achievements,
        FpsAttachments = fpsAttachments,
        Inventory = inventory,
        NewItems = newItems,
        DailyStats = dailyStats,
        WeeklyStats = weeklyStats
    };
}

static async Task<ProfileStatsWindowRow> GetProfileStatsWindowAsync(
    MySqlConnection db,
    ulong customerId,
    DateTime start,
    DateTime end)
{
    return await db.QueryFirstAsync<ProfileStatsWindowRow>(
        """
        SELECT
            COUNT(*) AS DailyGames,
            IFNULL(SUM(Kills), 0) AS Kills,
            IFNULL(SUM(Headshots), 0) AS Headshots,
            IFNULL(SUM(CaptureEnemyPoints), 0) AS CaptureFlags,

            IFNULL(SUM(CASE WHEN MapType = 0 THEN 1 ELSE 0 END), 0) AS MatchesCQ,
            IFNULL(SUM(CASE WHEN MapType = 1 THEN 1 ELSE 0 END), 0) AS MatchesDM,
            IFNULL(SUM(CASE WHEN MapType = 3 THEN 1 ELSE 0 END), 0) AS MatchesSB
        FROM dbg_userroundresults
        WHERE CustomerID = @CustomerId
          AND GameReportTime >= @Start
          AND GameReportTime < @End;
        """,
        new
        {
            CustomerId = customerId,
            Start = start,
            End = end
        });
}

static string BuildLegacyProfileXml(ProfileBundle profile)
{
    var account = profile.Account ?? throw new Exception("Profile account is null");

    var now = DateTime.Now;
    var curTime = $"{now.Year} {now.Month} {now.Day} {now.Hour} {now.Minute}";

    var xml = new StringBuilder();

    xml.Append("<?xml version=\"1.0\"?>\n");
    xml.Append("<account ");

    AppendXmlAttr(xml, "CustomerID", account.CustomerId);
    AppendXmlAttr(xml, "AccountStatus", account.AccountStatus);
    AppendXmlAttr(xml, "gamertag", account.Gamertag.TrimEnd());
    AppendXmlAttr(xml, "gamepoints", account.GamePoints);
    AppendXmlAttr(xml, "GameDollars", account.GameDollars);
    AppendXmlAttr(xml, "HonorPoints", account.HonorPoints);
    AppendXmlAttr(xml, "SkillPoints", account.SkillPoints);

    AppendXmlAttr(xml, "ClanID", account.ClanId);
    AppendXmlAttr(xml, "ClanRank", account.ClanRank);
    AppendXmlAttr(xml, "ClanTag", account.ClanTag);
    AppendXmlAttr(xml, "ClanTagColor", account.ClanTagColor);

    AppendXmlAttr(xml, "Kills", account.Kills);
    AppendXmlAttr(xml, "Deaths", account.Deaths);
    AppendXmlAttr(xml, "ShotsFired", account.ShotsFired);
    AppendXmlAttr(xml, "ShotsHits", account.ShotsHits);
    AppendXmlAttr(xml, "Headshots", account.Headshots);
    AppendXmlAttr(xml, "AssistKills", account.AssistKills);
    AppendXmlAttr(xml, "Wins", account.Wins);
    AppendXmlAttr(xml, "Losses", account.Losses);
    AppendXmlAttr(xml, "CaptureNeutralPoints", account.CaptureNeutralPoints);
    AppendXmlAttr(xml, "CaptureEnemyPoints", account.CaptureEnemyPoints);
    AppendXmlAttr(xml, "TimePlayed", account.TimePlayed);

    AppendXmlAttr(xml, "Abilities", account.Abilities);
    AppendXmlAttr(xml, "F1S", account.Faction1Score);
    AppendXmlAttr(xml, "F2S", account.Faction2Score);
    AppendXmlAttr(xml, "F3S", account.Faction3Score);
    AppendXmlAttr(xml, "F4S", account.Faction4Score);
    AppendXmlAttr(xml, "F5S", account.Faction5Score);
    AppendXmlAttr(xml, "time", curTime);
    AppendXmlAttr(xml, "IsFPSEnabled", account.IsFPSEnabled);

    if (account.IsDeveloper != 0)
        AppendXmlAttr(xml, "IsDev", account.IsDeveloper);

    xml.Append(">\n");

    xml.Append("<loadouts>\n");
    foreach (var loadout in profile.Loadouts)
    {
        xml.Append("<l ");
        AppendXmlAttr(xml, "id", loadout.LoadoutId);
        AppendXmlAttr(xml, "cl", loadout.Class);
        AppendXmlAttr(xml, "xp", loadout.HonorPoints);
        AppendXmlAttr(xml, "tm", loadout.TimePlayed);
        AppendXmlAttr(xml, "lo", loadout.Loadout);
        AppendXmlAttr(xml, "sp1", loadout.SpendSP1);
        AppendXmlAttr(xml, "sp2", loadout.SpendSP2);
        AppendXmlAttr(xml, "sp3", loadout.SpendSP3);
        AppendXmlAttr(xml, "sv", TrimLegacySkills(loadout.Skills));
        xml.Append("/>\n");
    }
    xml.Append("</loadouts>\n");

    xml.Append("<achievements>\n");
    foreach (var achievement in profile.Achievements)
    {
        xml.Append("<a ");
        AppendXmlAttr(xml, "id", achievement.AchId);
        AppendXmlAttr(xml, "v", achievement.Value);
        AppendXmlAttr(xml, "u", achievement.Unlocked);
        xml.Append("/>\n");
    }
    xml.Append("</achievements>\n");

    xml.Append("<fpsattach>\n");
    foreach (var item in profile.FpsAttachments)
    {
        xml.Append("<i ");
        AppendXmlAttr(xml, "wi", item.WeaponId);
        AppendXmlAttr(xml, "ai", item.AttachmentId);
        AppendXmlAttr(xml, "ml", item.MinutesLeft);

        if (item.IsEquipped > 0)
            AppendXmlAttr(xml, "eq", 1);

        xml.Append("/>\n");
    }
    xml.Append("</fpsattach>\n");

    xml.Append("<inventory>\n");
    foreach (var item in profile.Inventory)
    {
        xml.Append("<i ");
        AppendXmlAttr(xml, "id", item.ItemId);
        AppendXmlAttr(xml, "ml", item.MinutesLeft);

        if (item.Quantity > 1)
            AppendXmlAttr(xml, "qt", item.Quantity);

        xml.Append("/>\n");
    }
    xml.Append("</inventory>\n");

    xml.Append("<nis>\n");
    foreach (var itemId in profile.NewItems)
    {
        xml.Append("<i ");
        AppendXmlAttr(xml, "id", itemId);
        xml.Append("/>\n");
    }
    xml.Append("</nis>\n");

    AppendLegacyStatsXml(xml, "sday", profile.DailyStats);
    AppendLegacyStatsXml(xml, "sweek", profile.WeeklyStats);

    xml.Append("</account>");

    return xml.ToString();
}

static void AppendLegacyStatsXml(StringBuilder xml, string nodeName, ProfileStatsWindowRow stats)
{
    xml.Append("<");
    xml.Append(nodeName);
    xml.Append(" ");

    AppendXmlAttr(xml, "dg", stats.DailyGames);
    AppendXmlAttr(xml, "ki", stats.Kills);
    AppendXmlAttr(xml, "hs", stats.Headshots);
    AppendXmlAttr(xml, "cf", stats.CaptureFlags);
    AppendXmlAttr(xml, "mcq", stats.MatchesCQ);
    AppendXmlAttr(xml, "mdm", stats.MatchesDM);
    AppendXmlAttr(xml, "msb", stats.MatchesSB);

    xml.Append("/>\n");
}

static string TrimLegacySkills(string? value)
{
    return (value ?? "").TrimEnd(' ', '0');
}

static async Task EnsureStarterProfileAsync(MySqlConnection db, ulong customerId)
{
    var loadoutCount = await db.ExecuteScalarAsync<int>(
        """
        SELECT COUNT(*)
        FROM profile_chars
        WHERE CustomerID = @CustomerId;
        """,
        new { CustomerId = customerId });

    if (loadoutCount > 0)
        return;

    const string loadout = "20061 0 20019 20026 0 0 0 0 0 0 101002 101158 101004";

    await db.ExecuteAsync(
        """
        INSERT INTO profile_chars
        (
            CustomerID,
            Class,
            HonorPoints,
            TimePlayed,
            Loadout,
            Skills,
            SpendSP1,
            SpendSP2,
            SpendSP3
        )
        VALUES
        (
            @CustomerId,
            0,
            0,
            0,
            @Loadout,
            '0000000000000000000000000000000',
            0,
            0,
            0
        );
        """,
        new
        {
            CustomerId = customerId,
            Loadout = loadout
        });

    var starterItems = new[]
    {
        20061,
        20019,
        20026,
        101002,
        101158,
        101004
    };

    foreach (var itemId in starterItems)
    {
        await db.ExecuteAsync(
            """
            INSERT INTO inventory
            (
                CustomerID,
                ItemID,
                LeasedUntil,
                Quantity,
                UpSlot1,
                UpSlot2,
                UpSlot3,
                UpSlot4,
                UpSlot5
            )
            SELECT
                @CustomerId,
                @ItemId,
                DATE_ADD(UTC_TIMESTAMP(), INTERVAL 2000 DAY),
                1,
                0,
                0,
                0,
                0,
                0
            WHERE NOT EXISTS
            (
                SELECT 1
                FROM inventory
                WHERE CustomerID = @CustomerId
                  AND ItemID = @ItemId
                LIMIT 1
            );
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId
            });
    }
}

static async Task<BuyItemResponse> BuyItemAsync(
    string dbConnectionString,
    ulong customerId,
    int itemId,
    int buyIdx,
    int param1)
{
    if (customerId == 0 || customerId > int.MaxValue)
        return new BuyItemResponse(false, 6, "bad CustomerID", 0, false);

    if (itemId <= 0)
        return new BuyItemResponse(false, 6, "bad ItemID", 0, false);

    if (!TryGetBuyInfo(buyIdx, out var buyDays, out var isGameDollars))
        return new BuyItemResponse(false, 6, "bad BuyIdx", 0, false);

    await using var db = new MySqlConnection(dbConnectionString);
    await db.OpenAsync();

    await using var tx = await db.BeginTransactionAsync();

    try
    {
        var account = await db.QueryFirstOrDefaultAsync<AccountBalanceRow>(
            """
            SELECT
                CustomerID AS CustomerId,
                GamePoints AS GamePoints,
                GameDollars AS GameDollars
            FROM loginid
            WHERE CustomerID = @CustomerId
            FOR UPDATE;
            """,
            new { CustomerId = customerId },
            tx);

        if (account == null)
        {
            await tx.RollbackAsync();
            return new BuyItemResponse(false, 6, "no CustomerID", 0, isGameDollars);
        }

        var priceRow = await GetBuyItemPriceAsync(db, tx, itemId);

        if (priceRow == null)
        {
            await tx.RollbackAsync();
            return new BuyItemResponse(false, 6, "bad GetPrice", 0, isGameDollars);
        }

        var finalPrice = GetFinalPrice(priceRow, buyDays, isGameDollars);

        if (finalPrice <= 0 || priceRow.IsEnabled == 0)
        {
            await tx.RollbackAsync();
            return new BuyItemResponse(false, 6, "bad GetPrice", 0, isGameDollars);
        }

        var currentBalance = isGameDollars
            ? account.GameDollars
            : account.GamePoints;

        if (currentBalance < finalPrice)
        {
            await tx.RollbackAsync();

            return new BuyItemResponse(
                false,
                7,
                isGameDollars ? "Not Enough GD" : "Not Enough GP",
                currentBalance,
                isGameDollars);
        }

        var execResult = await ExecuteBoughtItemAsync(
            db,
            tx,
            customerId,
            priceRow,
            buyDays,
            param1);

        if (!execResult.Ok)
        {
            await tx.RollbackAsync();
            return new BuyItemResponse(false, 7, execResult.Message, currentBalance, isGameDollars);
        }

        if (isGameDollars)
        {
            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET GameDollars = GameDollars - @FinalPrice
                WHERE CustomerID = @CustomerId;
                """,
                new
                {
                    CustomerId = customerId,
                    FinalPrice = finalPrice
                },
                tx);
        }
        else
        {
            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET GamePoints = GamePoints - @FinalPrice
                WHERE CustomerID = @CustomerId;
                """,
                new
                {
                    CustomerId = customerId,
                    FinalPrice = finalPrice
                },
                tx);
        }

        await db.ExecuteAsync(
            """
            INSERT INTO financialtransactions
            (
                CustomerID,
                TransactionID,
                TransactionType,
                DateTime,
                Amount,
                ResponseCode,
                AprovalCode,
                ItemID
            )
            VALUES
            (
                @CustomerId,
                'INGAME',
                @TransactionType,
                UTC_TIMESTAMP(),
                @Amount,
                '1',
                'APPROVED',
                @ItemId
            );
            """,
            new
            {
                CustomerId = customerId,
                TransactionType = GetTransactionType(buyDays, isGameDollars),
                Amount = finalPrice,
                ItemId = itemId.ToString(CultureInfo.InvariantCulture)
            },
            tx);

        var newBalance = await db.ExecuteScalarAsync<int>(
            isGameDollars
                ? "SELECT GameDollars FROM loginid WHERE CustomerID = @CustomerId;"
                : "SELECT GamePoints FROM loginid WHERE CustomerID = @CustomerId;",
            new { CustomerId = customerId },
            tx);

        await tx.CommitAsync();

        return new BuyItemResponse(
            true,
            0,
            "OK",
            newBalance,
            isGameDollars);
    }
    catch (Exception e)
    {
        await tx.RollbackAsync();
        return new BuyItemResponse(false, 5, e.Message, 0, isGameDollars);
    }
}

static bool TryGetBuyInfo(int buyIdx, out int buyDays, out bool isGameDollars)
{
    buyDays = 0;
    isGameDollars = false;

    switch (buyIdx)
    {
        case 1:
            buyDays = 1;
            return true;

        case 2:
            buyDays = 7;
            return true;

        case 3:
            buyDays = 30;
            return true;

        case 4:
            buyDays = 2000;
            return true;

        case 5:
            buyDays = 1;
            isGameDollars = true;
            return true;

        case 6:
            buyDays = 7;
            isGameDollars = true;
            return true;

        case 7:
            buyDays = 30;
            isGameDollars = true;
            return true;

        case 8:
            buyDays = 2000;
            isGameDollars = true;
            return true;

        default:
            return false;
    }
}

static int GetFinalPrice(BuyItemPriceRow item, int buyDays, bool isGameDollars)
{
    if (!isGameDollars)
    {
        return buyDays switch
        {
            1 => item.Price1,
            7 => item.Price7,
            30 => item.Price30,
            2000 => item.PricePerm,
            _ => 0
        };
    }

    return buyDays switch
    {
        1 => item.GPrice1,
        7 => item.GPrice7,
        30 => item.GPrice30,
        2000 => item.GPricePerm,
        _ => 0
    };
}

static int GetTransactionType(int buyDays, bool isGameDollars)
{
    if (isGameDollars)
        return buyDays == 2000 ? 3001 : 2001;

    return buyDays == 2000 ? 3000 : 2000;
}

static int NormalizeLegacyErrorCode(int code)
{
    if (code < 0)
        return 5;

    if (code > 9)
        return 9;

    return code;
}

static async Task<BuyItemPriceRow?> GetBuyItemPriceAsync(
    MySqlConnection db,
    MySqlTransaction tx,
    int itemId)
{
    if (itemId >= 20000 && itemId < 99999)
    {
        return await db.QueryFirstOrDefaultAsync<BuyItemPriceRow>(
            """
            SELECT
                ItemID AS ItemId,
                1 AS ItemType,
                Category AS Category,
                1 AS IsEnabled,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_gear
            WHERE ItemID = @ItemId
            LIMIT 1;
            """,
            new { ItemId = itemId },
            tx);
    }

    if (itemId >= 100000 && itemId < 190000)
    {
        return await db.QueryFirstOrDefaultAsync<BuyItemPriceRow>(
            """
            SELECT
                ItemID AS ItemId,
                2 AS ItemType,
                Category AS Category,
                1 AS IsEnabled,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_weapons
            WHERE ItemID = @ItemId
            LIMIT 1;
            """,
            new { ItemId = itemId },
            tx);
    }

    if (itemId >= 300000 && itemId < 390000)
    {
        return await db.QueryFirstOrDefaultAsync<BuyItemPriceRow>(
            """
            SELECT
                ItemID AS ItemId,
                3 AS ItemType,
                Category AS Category,
                1 AS IsEnabled,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_generic
            WHERE ItemID = @ItemId
            LIMIT 1;
            """,
            new { ItemId = itemId },
            tx);
    }

    if (itemId >= 400000 && itemId < 490000)
    {
        return await db.QueryFirstOrDefaultAsync<BuyItemPriceRow>(
            """
            SELECT
                ItemID AS ItemId,
                5 AS ItemType,
                Category AS Category,
                1 AS IsEnabled,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_attachments
            WHERE ItemID = @ItemId
            LIMIT 1;
            """,
            new { ItemId = itemId },
            tx);
    }

    if (itemId >= 500000 && itemId < 590000)
    {
        return await db.QueryFirstOrDefaultAsync<BuyItemPriceRow>(
            """
            SELECT
                ItemID AS ItemId,
                4 AS ItemType,
                Category AS Category,
                IsEnabled AS IsEnabled,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_packages
            WHERE ItemID = @ItemId
            LIMIT 1;
            """,
            new { ItemId = itemId },
            tx);
    }

    if (itemId >= 600000 && itemId < 700000)
    {
        return await db.QueryFirstOrDefaultAsync<BuyItemPriceRow>(
            """
            SELECT
                ItemID AS ItemId,
                6 AS ItemType,
                Category AS Category,
                1 AS IsEnabled,
                Price1 AS Price1,
                Price7 AS Price7,
                Price30 AS Price30,
                PriceP AS PricePerm,
                GPrice1 AS GPrice1,
                GPrice7 AS GPrice7,
                GPrice30 AS GPrice30,
                GPriceP AS GPricePerm
            FROM items_upgradedata
            WHERE ItemID = @ItemId
            LIMIT 1;
            """,
            new { ItemId = itemId },
            tx);
    }

    return null;
}

static async Task<BuyExecResult> ExecuteBoughtItemAsync(
    MySqlConnection db,
    MySqlTransaction tx,
    ulong customerId,
    BuyItemPriceRow item,
    int buyDays,
    int param1)
{
    if (item.ItemType == 5)
    {
        await AddAttachmentToUserAsync(db, tx, customerId, param1, item.ItemId, buyDays);
        return BuyExecResult.Success();
    }

    if (item.ItemType == 4)
    {
        return await AddPackageToUserAsync(db, tx, customerId, item.ItemId);
    }

    if (item.ItemType == 6)
        return BuyExecResult.Success();

    if (item.ItemId == 301142)
        return BuyExecResult.Success();

    if (item.ItemId == 301055)
        return BuyExecResult.Success();

    if (item.ItemId == 301045)
        return BuyExecResult.Success();

    if (item.ItemId == 301046)
    {
        await db.ExecuteAsync(
            """
            UPDATE stats
            SET ShotsFired = 0,
                ShotsHits = 0
            WHERE CustomerID = @CustomerId;
            """,
            new { CustomerId = customerId },
            tx);

        return BuyExecResult.Success();
    }

    if (item.ItemId == 301047)
    {
        await db.ExecuteAsync(
            """
            UPDATE stats
            SET Kills = 0,
                Deaths = 0
            WHERE CustomerID = @CustomerId;
            """,
            new { CustomerId = customerId },
            tx);

        return BuyExecResult.Success();
    }

    if (item.ItemId == 301049)
    {
        await db.ExecuteAsync(
            """
            UPDATE stats
            SET Wins = 0,
                Losses = 0
            WHERE CustomerID = @CustomerId;
            """,
            new { CustomerId = customerId },
            tx);

        return BuyExecResult.Success();
    }

    if (item.ItemId == 301048)
    {
        await db.ExecuteAsync(
            """
            UPDATE profile_chars
            SET Skills = '0000000000000000000000000000000',
                SpendSP1 = 0,
                SpendSP2 = 0,
                SpendSP3 = 0
            WHERE CustomerID = @CustomerId;
            """,
            new { CustomerId = customerId },
            tx);

        return BuyExecResult.Success();
    }

    if (item.ItemType == 3)
    {
        if (item.Category == 3 || item.Category == 6)
            return BuyExecResult.Success();

        if (item.Category == 7)
        {
            await RemoveOneItemFromUserAsync(db, tx, customerId, item.ItemId);
            return BuyExecResult.Success();
        }
    }

    if (item.ItemId == 301107)
    {
        await db.ExecuteAsync(
            """
            UPDATE loginid
            SET GameDollars = GameDollars + 10000
            WHERE CustomerID = @CustomerId;
            """,
            new { CustomerId = customerId },
            tx);

        return BuyExecResult.Success();
    }

    await AddItemToUserAsync(db, tx, customerId, item.ItemId, buyDays);
    return BuyExecResult.Success();
}

static async Task AddItemToUserAsync(
    MySqlConnection db,
    MySqlTransaction tx,
    ulong customerId,
    int itemId,
    int expDays)
{
    if (itemId >= 100000 && itemId <= 199999)
        await AddDefaultAttachmentsToUserAsync(db, tx, customerId, itemId, expDays);

    var exists = await db.ExecuteScalarAsync<int>(
        """
        SELECT COUNT(*)
        FROM inventory
        WHERE CustomerID = @CustomerId
          AND ItemID = @ItemId
        FOR UPDATE;
        """,
        new
        {
            CustomerId = customerId,
            ItemId = itemId
        },
        tx);

    if (exists <= 0)
    {
        await db.ExecuteAsync(
            """
            INSERT INTO inventory
            (
                CustomerID,
                ItemID,
                LeasedUntil,
                Quantity,
                UpSlot1,
                UpSlot2,
                UpSlot3,
                UpSlot4,
                UpSlot5
            )
            VALUES
            (
                @CustomerId,
                @ItemId,
                DATE_ADD(UTC_TIMESTAMP(), INTERVAL @ExpDays DAY),
                1,
                0,
                0,
                0,
                0,
                0
            );
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId,
                ExpDays = expDays
            },
            tx);

        return;
    }

    await db.ExecuteAsync(
        """
        UPDATE inventory
        SET LeasedUntil =
            DATE_ADD(
                CASE
                    WHEN LeasedUntil < UTC_TIMESTAMP() THEN UTC_TIMESTAMP()
                    ELSE LeasedUntil
                END,
                INTERVAL @ExpDays DAY
            )
        WHERE CustomerID = @CustomerId
          AND ItemID = @ItemId;
        """,
        new
        {
            CustomerId = customerId,
            ItemId = itemId,
            ExpDays = expDays
        },
        tx);

    var isStackable = await db.ExecuteScalarAsync<int>(
        """
        SELECT IFNULL(IsStackable, 0)
        FROM items_generic
        WHERE ItemID = @ItemId
        LIMIT 1;
        """,
        new { ItemId = itemId },
        tx);

    if (isStackable > 0)
    {
        await db.ExecuteAsync(
            """
            UPDATE inventory
            SET Quantity = Quantity + 1
            WHERE CustomerID = @CustomerId
              AND ItemID = @ItemId;
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId
            },
            tx);
    }
}

static async Task AddDefaultAttachmentsToUserAsync(
    MySqlConnection db,
    MySqlTransaction tx,
    ulong customerId,
    int weaponId,
    int expDays)
{
    var row = await db.QueryFirstOrDefaultAsync<DefaultWeaponAttachmentsRow>(
        """
        SELECT
            IsFPS AS IsFps,
            FPSAttach0 AS FpsAttach0,
            FPSAttach1 AS FpsAttach1,
            FPSAttach2 AS FpsAttach2,
            FPSAttach3 AS FpsAttach3,
            FPSAttach4 AS FpsAttach4,
            FPSAttach5 AS FpsAttach5,
            FPSAttach6 AS FpsAttach6,
            FPSAttach7 AS FpsAttach7,
            FPSAttach8 AS FpsAttach8
        FROM items_weapons
        WHERE ItemID = @WeaponId
        LIMIT 1;
        """,
        new { WeaponId = weaponId },
        tx);

    if (row == null || row.IsFps <= 0)
        return;

    var attachments = new[]
    {
        row.FpsAttach0,
        row.FpsAttach1,
        row.FpsAttach2,
        row.FpsAttach3,
        row.FpsAttach4,
        row.FpsAttach5,
        row.FpsAttach6,
        row.FpsAttach7,
        row.FpsAttach8
    };

    foreach (var attachmentId in attachments)
    {
        if (attachmentId <= 0)
            continue;

        await AddAttachmentToUserAsync(
            db,
            tx,
            customerId,
            weaponId,
            attachmentId,
            expDays);
    }
}

static async Task AddAttachmentToUserAsync(
    MySqlConnection db,
    MySqlTransaction tx,
    ulong customerId,
    int weaponId,
    int attachmentId,
    int expDays)
{
    var slot = await db.ExecuteScalarAsync<int?>(
        """
        SELECT Type
        FROM items_attachments
        WHERE ItemID = @AttachmentId
        LIMIT 1;
        """,
        new { AttachmentId = attachmentId },
        tx);

    if (slot == null)
        return;

    var exists = await db.ExecuteScalarAsync<int>(
        """
        SELECT COUNT(*)
        FROM inventory_fps
        WHERE CustomerID = @CustomerId
          AND WeaponID = @WeaponId
          AND AttachmentID = @AttachmentId
        FOR UPDATE;
        """,
        new
        {
            CustomerId = customerId,
            WeaponId = weaponId,
            AttachmentId = attachmentId
        },
        tx);

    if (exists <= 0)
    {
        await db.ExecuteAsync(
            """
            UPDATE inventory_fps
            SET IsEquipped = 0
            WHERE CustomerID = @CustomerId
              AND WeaponID = @WeaponId
              AND Slot = @Slot;
            """,
            new
            {
                CustomerId = customerId,
                WeaponId = weaponId,
                Slot = slot.Value
            },
            tx);

        await db.ExecuteAsync(
            """
            INSERT INTO inventory_fps
            (
                CustomerID,
                WeaponID,
                AttachmentID,
                LeasedUntil,
                Slot,
                IsEquipped
            )
            VALUES
            (
                @CustomerId,
                @WeaponId,
                @AttachmentId,
                DATE_ADD(UTC_TIMESTAMP(), INTERVAL @ExpDays DAY),
                @Slot,
                1
            );
            """,
            new
            {
                CustomerId = customerId,
                WeaponId = weaponId,
                AttachmentId = attachmentId,
                ExpDays = expDays,
                Slot = slot.Value
            },
            tx);

        return;
    }

    await db.ExecuteAsync(
        """
        UPDATE inventory_fps
        SET LeasedUntil =
            DATE_ADD(
                CASE
                    WHEN LeasedUntil < UTC_TIMESTAMP() THEN UTC_TIMESTAMP()
                    ELSE LeasedUntil
                END,
                INTERVAL @ExpDays DAY
            )
        WHERE CustomerID = @CustomerId
          AND WeaponID = @WeaponId
          AND AttachmentID = @AttachmentId;
        """,
        new
        {
            CustomerId = customerId,
            WeaponId = weaponId,
            AttachmentId = attachmentId,
            ExpDays = expDays
        },
        tx);
}

static async Task<BuyExecResult> AddPackageToUserAsync(
    MySqlConnection db,
    MySqlTransaction tx,
    ulong customerId,
    int packageId)
{
    var package = await db.QueryFirstOrDefaultAsync<BuyPackageRow>(
        """
        SELECT
            IsEnabled AS IsEnabled,
            AddGP AS AddGP,
            AddSP AS AddSP,
            Item1_ID AS Item1Id,
            Item1_Exp AS Item1Exp,
            Item2_ID AS Item2Id,
            Item2_Exp AS Item2Exp,
            Item3_ID AS Item3Id,
            Item3_Exp AS Item3Exp,
            Item4_ID AS Item4Id,
            Item4_Exp AS Item4Exp,
            Item5_ID AS Item5Id,
            Item5_Exp AS Item5Exp,
            Item6_ID AS Item6Id,
            Item6_Exp AS Item6Exp
        FROM items_packages
        WHERE ItemID = @PackageId
        LIMIT 1;
        """,
        new { PackageId = packageId },
        tx);

    if (package == null)
        return BuyExecResult.Fail("bad package");

    if (package.IsEnabled == 0)
        return BuyExecResult.Fail("package disabled");

    await AddPackageItemAsync(db, tx, customerId, package.Item1Id, package.Item1Exp);
    await AddPackageItemAsync(db, tx, customerId, package.Item2Id, package.Item2Exp);
    await AddPackageItemAsync(db, tx, customerId, package.Item3Id, package.Item3Exp);
    await AddPackageItemAsync(db, tx, customerId, package.Item4Id, package.Item4Exp);
    await AddPackageItemAsync(db, tx, customerId, package.Item5Id, package.Item5Exp);
    await AddPackageItemAsync(db, tx, customerId, package.Item6Id, package.Item6Exp);

    await db.ExecuteAsync(
        """
        UPDATE loginid
        SET
            GameDollars = GameDollars + @AddGP,
            SkillPoints = SkillPoints + @AddSP
        WHERE CustomerID = @CustomerId;
        """,
        new
        {
            CustomerId = customerId,
            package.AddGP,
            package.AddSP
        },
        tx);

    return BuyExecResult.Success();
}

static async Task AddPackageItemAsync(
    MySqlConnection db,
    MySqlTransaction tx,
    ulong customerId,
    int itemId,
    int expDays)
{
    if (itemId <= 0)
        return;

    if (expDays <= 0)
        expDays = 2000;

    await AddItemToUserAsync(db, tx, customerId, itemId, expDays);
}

static async Task RemoveOneItemFromUserAsync(
    MySqlConnection db,
    MySqlTransaction tx,
    ulong customerId,
    int itemId)
{
    var quantity = await db.ExecuteScalarAsync<int?>(
        """
        SELECT Quantity
        FROM inventory
        WHERE CustomerID = @CustomerId
          AND ItemID = @ItemId
        FOR UPDATE;
        """,
        new
        {
            CustomerId = customerId,
            ItemId = itemId
        },
        tx);

    if (quantity == null)
        return;

    if (quantity.Value > 1)
    {
        await db.ExecuteAsync(
            """
            UPDATE inventory
            SET Quantity = Quantity - 1
            WHERE CustomerID = @CustomerId
              AND ItemID = @ItemId;
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId
            },
            tx);

        return;
    }

    await db.ExecuteAsync(
        """
        DELETE FROM inventory
        WHERE CustomerID = @CustomerId
          AND ItemID = @ItemId;
        """,
        new
        {
            CustomerId = customerId,
            ItemId = itemId
        },
        tx);
}

static int CreateSessionId()
{
    return RandomNumberGenerator.GetInt32(1, int.MaxValue);
}

static string CreateToken()
{
    Span<byte> bytes = stackalloc byte[32];
    RandomNumberGenerator.Fill(bytes);
    return Convert.ToBase64String(bytes);
}

static bool PasswordMatches(string password, string storedPassword, string storedMd5Password)
{
    if (!string.IsNullOrWhiteSpace(storedPassword))
    {
        if (string.Equals(password, storedPassword, StringComparison.Ordinal))
            return true;

        if (storedPassword.StartsWith("$2", StringComparison.Ordinal))
        {
            try
            {
                if (BCrypt.Net.BCrypt.Verify(password, storedPassword))
                    return true;
            }
            catch
            {
                return false;
            }
        }
    }

    if (!string.IsNullOrWhiteSpace(storedMd5Password))
    {
        var md5 = Md5Hex(password);

        if (string.Equals(md5, storedMd5Password, StringComparison.OrdinalIgnoreCase))
            return true;
    }

    return false;
}

static string Md5Hex(string value)
{
    var bytes = MD5.HashData(Encoding.UTF8.GetBytes(value));
    var sb = new StringBuilder(bytes.Length * 2);

    foreach (var b in bytes)
        sb.Append(b.ToString("x2"));

    return sb.ToString();
}

static string NormalizeLegacyIp(string ip)
{
    ip = (ip ?? "").Trim();

    if (string.IsNullOrWhiteSpace(ip))
        return "0.0.0.0";

    if (ip == "::1")
        return "127.0.0.1";

    const string ipv6MappedPrefix = "::ffff:";

    if (ip.StartsWith(ipv6MappedPrefix, StringComparison.OrdinalIgnoreCase))
        ip = ip[ipv6MappedPrefix.Length..];

    if (ip.Length > 16)
        ip = ip[..16];

    return ip;
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

static int ParseInt(string value)
{
    if (int.TryParse(value, NumberStyles.Integer, CultureInfo.InvariantCulture, out var result))
        return result;

    return 0;
}

static void WriteLegacyShopItem(MemoryStream stream, ILegacyShopPrice item)
{
    var priceBits = GetPriceBits(item);

    if (priceBits == 0)
        return;

    WriteInt32LE(stream, item.ItemId);
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

static int GetPriceBits(IHasPrices item)
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

static bool HasAnyPrice(IHasPrices item)
{
    return GetPriceBits(item) != 0;
}

static int ClampByte(int value)
{
    if (value < 0)
        return 0;

    if (value > 255)
        return 255;

    return value;
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

record ProfileRequest(ulong CustomerId, ulong SessionId, string Token, bool JoiningGame);

record BuyItemRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    int ItemId,
    int BuyIdx,
    int Param1 = 0);

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
    public string AccountName { get; set; } = "";
    public string Password { get; set; } = "";
    public string MD5Password { get; set; } = "";
    public int AccountStatus { get; set; }
    public bool IsDeveloper { get; set; }
}

class SessionRow
{
    public int SessionId { get; set; }
    public ulong CustomerId { get; set; }
    public string SessionKey { get; set; } = "";
    public DateTime TimeUpdated { get; set; }
    public int AccountStatus { get; set; }
}

class ProfileBundle
{
    public bool Ok { get; set; }
    public int Code { get; set; }
    public string Message { get; set; } = "";

    public ProfileAccountRow? Account { get; set; }

    public List<ProfileLoadoutRow> Loadouts { get; set; } = new();
    public List<ProfileAchievementRow> Achievements { get; set; } = new();
    public List<ProfileFpsAttachmentRow> FpsAttachments { get; set; } = new();
    public List<ProfileInventoryRow> Inventory { get; set; } = new();
    public List<int> NewItems { get; set; } = new();

    public ProfileStatsWindowRow DailyStats { get; set; } = new();
    public ProfileStatsWindowRow WeeklyStats { get; set; } = new();

    public static ProfileBundle Fail(int code, string message)
    {
        return new ProfileBundle
        {
            Ok = false,
            Code = code,
            Message = message
        };
    }
}

class ProfileAccountRow
{
    public ulong CustomerId { get; set; }
    public int AccountStatus { get; set; }

    public int GamePoints { get; set; }
    public int GameDollars { get; set; }
    public int HonorPoints { get; set; }
    public int SkillPoints { get; set; }

    public string Gamertag { get; set; } = "";

    public int Faction1Score { get; set; }
    public int Faction2Score { get; set; }
    public int Faction3Score { get; set; }
    public int Faction4Score { get; set; }
    public int Faction5Score { get; set; }

    public int ClanId { get; set; }
    public int ClanRank { get; set; }
    public string ClanTag { get; set; } = "";
    public int ClanTagColor { get; set; }

    public int IsFPSEnabled { get; set; }
    public int IsDeveloper { get; set; }

    public int Kills { get; set; }
    public int Deaths { get; set; }
    public int ShotsFired { get; set; }
    public int ShotsHits { get; set; }
    public int Headshots { get; set; }
    public int AssistKills { get; set; }
    public int Wins { get; set; }
    public int Losses { get; set; }
    public int CaptureNeutralPoints { get; set; }
    public int CaptureEnemyPoints { get; set; }
    public int TimePlayed { get; set; }

    public string Skills { get; set; } = "";
    public string Abilities { get; set; } = "";
}

class ProfileLoadoutRow
{
    public int LoadoutId { get; set; }
    public int Class { get; set; }
    public int HonorPoints { get; set; }
    public int TimePlayed { get; set; }
    public string Loadout { get; set; } = "";
    public string Skills { get; set; } = "";
    public int SpendSP1 { get; set; }
    public int SpendSP2 { get; set; }
    public int SpendSP3 { get; set; }
}

class ProfileAchievementRow
{
    public int AchId { get; set; }
    public int Value { get; set; }
    public int Unlocked { get; set; }
}

class ProfileFpsAttachmentRow
{
    public int WeaponId { get; set; }
    public int AttachmentId { get; set; }
    public int MinutesLeft { get; set; }
    public int IsEquipped { get; set; }
}

class ProfileInventoryRow
{
    public int ItemId { get; set; }
    public int MinutesLeft { get; set; }
    public int Quantity { get; set; }
}

class ProfileStatsWindowRow
{
    public int DailyGames { get; set; }
    public int Kills { get; set; }
    public int Headshots { get; set; }
    public int CaptureFlags { get; set; }
    public int MatchesCQ { get; set; }
    public int MatchesDM { get; set; }
    public int MatchesSB { get; set; }
}

record BuyItemResponse(
    bool Ok,
    int Code,
    string Message,
    int Balance,
    bool IsGameDollars);

class AccountBalanceRow
{
    public ulong CustomerId { get; set; }
    public int GamePoints { get; set; }
    public int GameDollars { get; set; }
}

class BuyItemPriceRow
{
    public int ItemId { get; set; }
    public int ItemType { get; set; }
    public int Category { get; set; }
    public int IsEnabled { get; set; } = 1;

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}

class BuyPackageRow
{
    public int IsEnabled { get; set; }

    public int AddGP { get; set; }
    public int AddSP { get; set; }

    public int Item1Id { get; set; }
    public int Item1Exp { get; set; }
    public int Item2Id { get; set; }
    public int Item2Exp { get; set; }
    public int Item3Id { get; set; }
    public int Item3Exp { get; set; }
    public int Item4Id { get; set; }
    public int Item4Exp { get; set; }
    public int Item5Id { get; set; }
    public int Item5Exp { get; set; }
    public int Item6Id { get; set; }
    public int Item6Exp { get; set; }
}

class DefaultWeaponAttachmentsRow
{
    public int IsFps { get; set; }

    public int FpsAttach0 { get; set; }
    public int FpsAttach1 { get; set; }
    public int FpsAttach2 { get; set; }
    public int FpsAttach3 { get; set; }
    public int FpsAttach4 { get; set; }
    public int FpsAttach5 { get; set; }
    public int FpsAttach6 { get; set; }
    public int FpsAttach7 { get; set; }
    public int FpsAttach8 { get; set; }
}

class BuyExecResult
{
    public bool Ok { get; set; }
    public string Message { get; set; } = "";

    public static BuyExecResult Success()
    {
        return new BuyExecResult
        {
            Ok = true,
            Message = "OK"
        };
    }

    public static BuyExecResult Fail(string message)
    {
        return new BuyExecResult
        {
            Ok = false,
            Message = message
        };
    }
}

interface IHasPrices
{
    int Price1 { get; }
    int Price7 { get; }
    int Price30 { get; }
    int PricePerm { get; }

    int GPrice1 { get; }
    int GPrice7 { get; }
    int GPrice30 { get; }
    int GPricePerm { get; }
}

class SkillPriceRow
{
    public int SkillId { get; set; }
    public int Lv1 { get; set; }
    public int Lv2 { get; set; }
    public int Lv3 { get; set; }
    public int Lv4 { get; set; }
    public int Lv5 { get; set; }
}

interface ILegacyShopPrice : IHasPrices
{
    int ItemId { get; }
    int Category { get; }
}

class LegacyShopPriceRow : ILegacyShopPrice
{
    public int ItemId { get; set; }
    public int Category { get; set; }

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}

class GearItemRow : IHasPrices
{
    public int ItemId { get; set; }
    public int LevelRequired { get; set; }
    public decimal Weight { get; set; }
    public decimal DamagePerc { get; set; }
    public decimal DamageMax { get; set; }

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}

class WeaponItemRow : IHasPrices
{
    public int ItemId { get; set; }
    public int LevelRequired { get; set; }

    public decimal Damage { get; set; }
    public decimal DamageDecay { get; set; }
    public int NumClips { get; set; }
    public int ClipSize { get; set; }
    public decimal RateOfFire { get; set; }
    public decimal Spread { get; set; }
    public decimal Recoil { get; set; }

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}

class GenericItemRow : IHasPrices
{
    public int ItemId { get; set; }
    public int Category { get; set; }

    public string Name { get; set; } = "";
    public string FName { get; set; } = "";
    public string Description { get; set; } = "";

    public int Price1 { get; set; }
    public int Price7 { get; set; }
    public int Price30 { get; set; }
    public int PricePerm { get; set; }

    public int GPrice1 { get; set; }
    public int GPrice7 { get; set; }
    public int GPrice30 { get; set; }
    public int GPricePerm { get; set; }
}

class PackageItemRow : IHasPrices
{
    public int ItemId { get; set; }

    public string Name { get; set; } = "";
    public string FName { get; set; } = "";
    public string Description { get; set; } = "";

    public int AddGP { get; set; }
    public int AddSP { get; set; }

    public int Item1Id { get; set; }
    public int Item1Exp { get; set; }
    public int Item2Id { get; set; }
    public int Item2Exp { get; set; }
    public int Item3Id { get; set; }
    public int Item3Exp { get; set; }
    public int Item4Id { get; set; }
    public int Item4Exp { get; set; }
    public int Item5Id { get; set; }
    public int Item5Exp { get; set; }
    public int Item6Id { get; set; }
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

class ShopItemRow : IHasPrices
{
    public int ItemId { get; set; }
    public int ItemType { get; set; }
    public int Category { get; set; }
    public int IsEnabled { get; set; }

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

    public int Item1Id { get; set; }
    public int Item1Exp { get; set; }
    public int Item2Id { get; set; }
    public int Item2Exp { get; set; }
    public int Item3Id { get; set; }
    public int Item3Exp { get; set; }
    public int Item4Id { get; set; }
    public int Item4Exp { get; set; }
    public int Item5Id { get; set; }
    public int Item5Exp { get; set; }
    public int Item6Id { get; set; }
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