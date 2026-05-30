using WarInc.Api.Auth;
using WarInc.Api.Config;
using WarInc.Api.Database;
using WarInc.Api.GameServer;
using WarInc.Api.Profile;
using WarInc.Api.Purchase;
using WarInc.Api.Shop;
using WarInc.Api.Legacy;
using System.Text;
using System.Text.RegularExpressions;

var builder = WebApplication.CreateBuilder(args);

builder.Services.Configure<WarIncOptions>(
    builder.Configuration.GetSection("WarInc"));

var warIncOptions = builder.Configuration
    .GetSection("WarInc")
    .Get<WarIncOptions>() ?? new WarIncOptions();

if (string.IsNullOrWhiteSpace(warIncOptions.Database))
    throw new Exception("WarInc:Database is empty");

builder.Services.AddSingleton(new WarIncDb(warIncOptions.Database));

builder.Services.AddScoped<AuthService>();
builder.Services.AddScoped<ShopService>();
builder.Services.AddScoped<ProfileService>();
builder.Services.AddScoped<PurchaseService>();
builder.Services.AddScoped<ProfileWriteService>();

builder.Services.AddSingleton<GameServerService>();

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

var legacyApiRegex = new Regex(
    @"^/((api/)?api_[A-Za-z0-9_]+\.aspx)$",
    RegexOptions.Compiled | RegexOptions.IgnoreCase | RegexOptions.CultureInvariant);

app.Use(async (context, next) =>
{
    var path = context.Request.Path.Value ?? "";

    if (path.Equals("/legacy/smoke", StringComparison.OrdinalIgnoreCase))
    {
        context.Response.StatusCode = 200;
        context.Response.ContentType = "application/json; charset=utf-8";

        await context.Response.WriteAsync("""
        {
          "ok": true,
          "code": 0,
          "service": "WarInc.Api",
          "mode": "legacy-smoke",
          "message": "Legacy smoke middleware is enabled"
        }
        """);

        return;
    }

    if (path.Equals("/legacy/smoke/endpoints", StringComparison.OrdinalIgnoreCase))
    {
        context.Response.StatusCode = 200;
        context.Response.ContentType = "application/json; charset=utf-8";

        await context.Response.WriteAsync("""
        {
          "ok": true,
          "code": 0,
          "endpoints": [
            "api_BuyItem3.aspx",
            "api_ChangeGamerTag.aspx",
            "api_ChangeGamerTag2.aspx",
            "api_CheckLoginSession.aspx",
            "api_ClientUpdateAchievements.aspx",
            "api_ClanApply.aspx",
            "api_ClanCreate.aspx",
            "api_ClanGetInfo.aspx",
            "api_ClanInvites.aspx",
            "api_ClanMgr.aspx",
            "api_Friends.aspx",
            "api_Gamersfirst.aspx",
            "api_GetCreateGameKey3.aspx",
            "api_GetDataGameRewards.aspx",
            "api_GetItemsInfo.aspx",
            "api_GetProfile4.aspx",
            "api_GetShop5.aspx",
            "api_GNAGetBalance.aspx",
            "api_GNALogin.aspx",
            "api_LeaderboardGet.aspx",
            "api_LoadoutModify.aspx",
            "api_LoadoutReset.aspx",
            "api_LoadoutUnlock.aspx",
            "api_Login.aspx",
            "api_LoginSessionPoller.aspx",
            "api_ModifySlot.aspx",
            "api_MysteryBox.aspx",
            "api_ReportHWInfo.aspx",
            "api_ReportHWInfo_Customer.aspx",
            "api_RetBonus.aspx",
            "api_SetRSUpdateStatus.aspx",
            "api_SkillLearn.aspx",
            "api_SkillReset.aspx",
            "api_SrvAddCheatAttempts.aspx",
            "api_SrvAddLogInfo.aspx",
            "api_SrvAddUserRoundResult4.aspx",
            "api_SrvAddWeaponStats.aspx",
            "api_SrvGiveItem.aspx",
            "api_SrvGiveItemInMinutes.aspx",
            "api_SrvRemoveItem.aspx",
            "api_SrvSetCreateGameKey2.aspx",
            "api_SrvUpdateAchievements.aspx",
            "api_SrvUploadLogFile.aspx",
            "api_SteamBuyGP.aspx",
            "api_SteamCreateAcc.aspx",
            "api_SteamLogin.aspx",
            "api_UpdateLoginSession.aspx",
            "api_WeaponAttach.aspx",
            "api_WeaponAttachSet.aspx",
            "api_WelcomePackage4.aspx"
          ]
        }
        """);

        return;
    }

    if (!legacyApiRegex.IsMatch(path))
    {
        await next();
        return;
    }

    var endpoint = path.Split('/', StringSplitOptions.RemoveEmptyEntries).Last();

    IFormCollection? form = null;

    if (context.Request.HasFormContentType)
        form = await context.Request.ReadFormAsync();

    var func = form?["func"].ToString() ?? context.Request.Query["func"].ToString();
    var body = BuildLegacySmokeResponse(endpoint, func, form);

    context.Response.StatusCode = 200;
    context.Response.ContentType = "text/plain; charset=utf-8";
    context.Response.Headers["X-WarInc-Legacy-Smoke"] = "1";
    context.Response.Headers["X-WarInc-Legacy-Endpoint"] = endpoint;

    await context.Response.WriteAsync(body, Encoding.UTF8);
});

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
            "/internal/session/validate",

            "/v1/shop/items",
            "/v1/shop/buy",
            "/v1/profile/get",

            "/v1/loadout/modify",
            "/v1/loadout/unlock",
            "/v1/loadout/reset",

            "/v1/skill/learn",
            "/v1/skill/reset",

            "/v1/weapon/attach",

            "/v1/profile/change-gamertag",
            "/v1/profile/welcome-package",

            "/v1/friends/list",
            "/v1/clan/get",

            "/internal/gameserver/register",
            "/internal/gameserver/heartbeat",
            "/internal/gameserver/unregister",
            "/internal/gameserver/player/join",
            "/internal/gameserver/player/leave",
            "/internal/gameserver/report",
            "/internal/gameserver/list",

            "/api_Login.aspx",
            "/api_CheckLoginSession.aspx",
            "/api_UpdateLoginSession.aspx",
            "/api_GetShop5.aspx",
            "/api_GetItemsInfo.aspx",
            "/api_GetProfile4.aspx",
            "/api_BuyItem3.aspx",

            "/api_LoadoutModify.aspx",
            "/api_LoadoutUnlock.aspx",
            "/api_LoadoutReset.aspx",
            "/api_SkillLearn.aspx",
            "/api_SkillReset.aspx",
            "/api_WeaponAttach.aspx",
            "/api_WeaponAttachSet.aspx",
            "/api_ChangeGamerTag2.aspx",
            "/api_WelcomePackage4.aspx"
        }
    });
});

app.MapAuthEndpoints();
app.MapShopEndpoints();
app.MapProfileEndpoints();
app.MapPurchaseEndpoints();
app.MapProfileAdditionalEndpoints();
app.MapGameServerEndpoints();
app.MapLegacySmokeEndpoints();

static string BuildLegacySmokeResponse(string endpoint, string func, IFormCollection? form)
{
    endpoint = endpoint.ToLowerInvariant();
    func = func.Trim().ToLowerInvariant();

    return endpoint switch
    {
        "api_login.aspx" => "WO_00 0 0",
        "api_steamlogin.aspx" => "WO_00 0 0",
        "api_gnalogin.aspx" => "WO_00 0 0",
        "api_gamersfirst.aspx" => func == "login" ? "WO_00 0 0 0 0" : "WO_0",

        "api_checkloginsession.aspx" => "WO_0",
        "api_updateloginsession.aspx" => "WO_0",
        "api_loginsessionpoller.aspx" => "WO_0",

        "api_getprofile4.aspx" => BuildLegacyProfileSmoke(),
        "api_getitemsinfo.aspx" => "WO_0<items><gears></gears><weapons></weapons><packages></packages><generics></generics></items>",
        "api_getshop5.aspx" => "WO_0",

        "api_buyitem3.aspx" => "WO_00",
        "api_changegamertag.aspx" => "WO_00",
        "api_changegamertag2.aspx" => "WO_00",

        "api_loadoutmodify.aspx" => BuildLegacyLoadoutModifySmoke(form),
        "api_loadoutunlock.aspx" => "WO_0",
        "api_loadoutreset.aspx" => "WO_0",

        "api_skilllearn.aspx" => "WO_00 0 0",
        "api_skillreset.aspx" => "WO_0",

        "api_weaponattach.aspx" => func == "buy" ? "WO_00" : "WO_0",
        "api_weaponattachset.aspx" => "WO_0",

        "api_welcomepackage4.aspx" => "WO_0",

        "api_friends.aspx" => func == "stats" ? "WO_0<friends></friends>" : "WO_0",
        "api_clanmgr.aspx" => "WO_0",
        "api_clanapply.aspx" => "WO_0",
        "api_clancreate.aspx" => "WO_0",
        "api_clangetinfo.aspx" => "WO_0<clan id=\"0\" name=\"\" tag=\"\" tagcolor=\"0\"></clan>",
        "api_claninvites.aspx" => "WO_0<invites></invites>",

        "api_leaderboardget.aspx" => "WO_0<leaderboard pos=\"0\" size=\"0\"></leaderboard>",
        "api_mysterybox.aspx" => func == "info" ? "WO_0<box></box>" : "WO_00 0 0 0",
        "api_retbonus.aspx" => func == "info" ? "WO_0<retbonus d=\"0\" m=\"0\"><days></days></retbonus>" : "WO_00",

        "api_gnagetbalance.aspx" => "WO_00",
        "api_steambuygp.aspx" => func == "shop" ? "WO_0<SteamGPShop></SteamGPShop>" : "WO_00",

        "api_getcreategamekey3.aspx" => "WO_00",
        "api_getdatagamerewards.aspx" => "WO_0<rewards></rewards>",
        "api_setrsupdatestatus.aspx" => "WO_0",

        "api_modifyslot.aspx" => "WO_0",
        "api_clientupdateachievements.aspx" => "WO_0",

        "api_reporthwinfo.aspx" => "WO_0",
        "api_reporthwinfo_customer.aspx" => "WO_0",

        "api_srvaddcheatattempts.aspx" => "WO_0",
        "api_srvaddloginfo.aspx" => "WO_0",
        "api_srvadduserroundresult4.aspx" => "WO_0",
        "api_srvaddweaponstats.aspx" => "WO_0",
        "api_srvgiveitem.aspx" => "WO_0",
        "api_srvgiveiteminminutes.aspx" => "WO_0",
        "api_srvremoveitem.aspx" => "WO_0",
        "api_srvsetcreategamekey2.aspx" => "WO_0",
        "api_srvupdateachievements.aspx" => "WO_0",
        "api_srvuploadlogfile.aspx" => "WO_0",

        _ => "WO_0"
    };
}

static string BuildLegacyLoadoutModifySmoke(IFormCollection? form)
{
    var result = new string[13];

    for (var i = 0; i < 13; i++)
    {
        var key = "i" + (i + 1);
        var value = form?[key].ToString();

        if (string.IsNullOrWhiteSpace(value))
            value = "0";

        result[i] = value;
    }

    return "WO_0" + string.Join(' ', result);
}

static string BuildLegacyProfileSmoke()
{
    var now = DateTime.UtcNow;

    return "WO_0" +
           "<account CustomerID=\"1\" AccountStatus=\"100\" gamertag=\"SmokeTester\" " +
           "gamepoints=\"0\" GameDollars=\"0\" HonorPoints=\"0\" SkillPoints=\"0\" " +
           "Kills=\"0\" Deaths=\"0\" ShotsFired=\"0\" ShotsHits=\"0\" Headshots=\"0\" AssistKills=\"0\" " +
           "Wins=\"0\" Losses=\"0\" CaptureNeutralPoints=\"0\" CaptureEnemyPoints=\"0\" TimePlayed=\"0\" " +
           "IsDev=\"1\" F1S=\"0\" F2S=\"0\" F3S=\"0\" F4S=\"0\" F5S=\"0\" " +
           "ClanID=\"0\" ClanRank=\"0\" ClanTag=\"\" ClanTagColor=\"0\" IsFPSEnabled=\"1\" " +
           $"time=\"{now.Year} {now.Month} {now.Day} {now.Hour} {now.Minute}\">" +
           "<loadouts><l id=\"1\" cl=\"0\" xp=\"0\" tm=\"0\" sp1=\"0\" sp2=\"0\" sp3=\"0\" sv=\"\" lo=\"0 0 0 0 0 0 0 0 0 0 0 0 0\" /></loadouts>" +
           "<achievements></achievements>" +
           "<fpsattach></fpsattach>" +
           "<inventory></inventory>" +
           "<nis></nis>" +
           "<sday></sday>" +
           "<sweek></sweek>" +
           "</account>";
}

app.Run();