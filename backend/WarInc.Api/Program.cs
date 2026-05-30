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
builder.Services.AddScoped<SocialService>();
builder.Services.AddScoped<LeaderboardService>();

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
            "/v1/leaderboard/get",

            "/internal/gameserver/register",
            "/internal/gameserver/heartbeat",
            "/internal/gameserver/unregister",
            "/internal/gameserver/player/join",
            "/internal/gameserver/player/leave",
            "/internal/gameserver/report",
            "/internal/gameserver/list",

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
app.MapGameServerEndpoints();
app.MapLegacySmokeEndpoints();

app.Run();