using WarInc.Api.Auth;
using WarInc.Api.Config;
using WarInc.Api.Database;
using WarInc.Api.Profile;
using WarInc.Api.Purchase;
using WarInc.Api.Shop;

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

            "/api_Login.aspx",
            "/api_CheckLoginSession.aspx",
            "/api_UpdateLoginSession.aspx",
            "/api_GetShop5.aspx",
            "/api_GetItemsInfo.aspx",
            "/api_GetProfile4.aspx",
            "/api_BuyItem3.aspx"
        }
    });
});

app.MapAuthEndpoints();
app.MapShopEndpoints();
app.MapProfileEndpoints();
app.MapPurchaseEndpoints();

app.Run();