using WarInc.Api.Auth;
using WarInc.Api.Config;
using WarInc.Api.Database;
using WarInc.Api.GameServer;
using WarInc.Api.Legacy;
using WarInc.Api.Profile;
using WarInc.Api.Purchase;
using WarInc.Api.Shop;
using WarInc.Api.Social;
using WarInc.Api.Leaderboard;
using WarInc.Api.Retention;
using WarInc.Api.MysteryBox;
using WarInc.Api.GameRewards;
using WarInc.Api.ClientTelemetry;

var builder = WebApplication.CreateBuilder(args);

builder.Services.Configure<WarIncOptions>(
    builder.Configuration.GetSection("WarInc"));

var warIncOptions = builder.Configuration
    .GetSection("WarInc")
    .Get<WarIncOptions>() ?? new WarIncOptions();

if (string.IsNullOrWhiteSpace(warIncOptions.Database))
    throw new Exception("WarInc:Database is empty");

// Database
builder.Services.AddSingleton(new WarIncDb(warIncOptions.Database));

// Client
builder.Services.AddScoped<AuthService>();
builder.Services.AddScoped<LegacyExternalAuthService>();
builder.Services.AddScoped<ShopService>();
builder.Services.AddScoped<ProfileService>();
builder.Services.AddScoped<PurchaseService>();
builder.Services.AddScoped<ProfileWriteService>();
builder.Services.AddScoped<SocialService>();
builder.Services.AddScoped<LeaderboardService>();
builder.Services.AddScoped<RetentionService>();
builder.Services.AddScoped<MysteryBoxService>();
builder.Services.AddScoped<DatabaseCheckService>();
builder.Services.AddScoped<GameRewardsService>();
builder.Services.AddScoped<ClientTelemetryService>();

// Server
builder.Services.AddSingleton<GameServerService>();
builder.Services.AddScoped<GameServerInventoryService>();

// Agent
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
            "/v1/leaderboard/get",
            "/v1/retention/info",
            "/v1/retention/claim",
            "/v1/mystery-box/info",
            "/v1/mystery-box/roll",
            "/v1/mystery-box/sell",
            "/v1/game-rewards/get",
            "/v1/session/poll",
            "/v1/client/status",
            "/v1/hardware/report",
            "/v1/achievements/update",
            "/v1/gameserver/give-item",
            "/v1/gameserver/remove-item",
            
            "/v1/clan/invite",
            "/v1/clan/accept-invite",
            "/v1/clan/apply",
            "/v1/clan/accept-application",
            "/v1/clan/leave",
            "/v1/clan/kick",
            "/v1/clan/promote",
            "/v1/clan/demote",
            "/v1/clan/announcement",
            "/v1/gameserver/result",
            "/v1/gameserver/weapon-stats",

            "/internal/session/validate",
            "/internal/gameserver/register",
            "/internal/gameserver/heartbeat",
            "/internal/gameserver/unregister",
            "/internal/gameserver/player/join",
            "/internal/gameserver/player/leave",
            "/internal/gameserver/report",
            "/internal/gameserver/list",
            "/internal/database/check",
            "/internal/database/schema",

            "/legacy/smoke",
            "/legacy/smoke/endpoints",

            "/api_Login.aspx",
            "/api_CheckLoginSession.aspx",
            "/api_UpdateLoginSession.aspx",
            "/api_GetShop5.aspx",
            "/api_GetItemsInfo.aspx",
            "/api_GetProfile4.aspx",
            "/api_BuyItem3.aspx",
            "/api_LeaderboardGet.aspx",
            "/api_RetBonus.aspx",
            "/api_MysteryBox.aspx",
            "/api_GetDataGameRewards.aspx",
            "/api_LoginSessionPoller.aspx",
            "/api_SetRSUpdateStatus.aspx",
            "/api_ReportHWInfo.aspx",
            "/api_ReportHWInfo_Customer.aspx",
            "/api_ClientUpdateAchievements.aspx",
            "/api_SrvSetCreateGameKey2.aspx",
            "/api_GetCreateGameKey3.aspx",
            "/api_SrvAddUserRoundResult4.aspx",
            "/api_SrvAddWeaponStats.aspx",
            "/api_SrvUpdateAchievements.aspx",
            "/api_SrvAddLogInfo.aspx",
            "/api_SrvUploadLogFile.aspx",
            "/api_SrvAddCheatAttempts.aspx",
            "/api_SrvGiveItem.aspx",
            "/api_SrvGiveItemInMinutes.aspx",
            "/api_SrvRemoveItem.aspx",
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
app.MapSocialEndpoints();
app.MapLeaderboardEndpoints();
app.MapRetentionEndpoints();
app.MapMysteryBoxEndpoints();
app.MapGameRewardsEndpoints();
app.MapClientTelemetryEndpoints();
app.MapGameServerEndpoints();
app.MapGameServerInventoryEndpoints();
app.MapDatabaseCheckEndpoints();
app.MapGameServerLegacyEndpoints();
app.MapLegacySmokeEndpoints();

app.Run();