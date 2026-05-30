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
            "/api_Login.aspx",
            "/api_CheckLoginSession.aspx",
            "/api_UpdateLoginSession.aspx",
            "/api_GetShop5.aspx",
            "/api_GetItemsInfo.aspx"
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