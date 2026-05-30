using System.Text;
using System.Text.RegularExpressions;
using Dapper;
using MySqlConnector;
using WarInc.Api.Auth;
using WarInc.Api.Common;
using WarInc.Api.Database;
using WarInc.Api.Purchase;

namespace WarInc.Api.Profile;

public static class ProfileAdditionalEndpoints
{
    public static void MapProfileAdditionalEndpoints(this WebApplication app)
    {
        app.MapPost("/api_LoadoutModify.aspx", LegacyLoadoutModifyAsync);
        app.MapPost("/api/api_LoadoutModify.aspx", LegacyLoadoutModifyAsync);

        app.MapPost("/api_LoadoutUnlock.aspx", LegacyLoadoutUnlockAsync);
        app.MapPost("/api/api_LoadoutUnlock.aspx", LegacyLoadoutUnlockAsync);

        app.MapPost("/api_LoadoutReset.aspx", LegacyLoadoutResetAsync);
        app.MapPost("/api/api_LoadoutReset.aspx", LegacyLoadoutResetAsync);

        app.MapPost("/api_SkillLearn.aspx", LegacySkillLearnAsync);
        app.MapPost("/api/api_SkillLearn.aspx", LegacySkillLearnAsync);

        app.MapPost("/api_SkillReset.aspx", LegacySkillResetAsync);
        app.MapPost("/api/api_SkillReset.aspx", LegacySkillResetAsync);

        app.MapPost("/api_WeaponAttach.aspx", LegacyWeaponAttachAsync);
        app.MapPost("/api/api_WeaponAttach.aspx", LegacyWeaponAttachAsync);

        app.MapPost("/api_WeaponAttachSet.aspx", LegacyWeaponAttachSetAsync);
        app.MapPost("/api/api_WeaponAttachSet.aspx", LegacyWeaponAttachSetAsync);

        app.MapPost("/api_ChangeGamerTag2.aspx", LegacyChangeGamerTagAsync);
        app.MapPost("/api/api_ChangeGamerTag2.aspx", LegacyChangeGamerTagAsync);
        app.MapPost("/api_ChangeGamerTag.aspx", LegacyChangeGamerTagAsync);
        app.MapPost("/api/api_ChangeGamerTag.aspx", LegacyChangeGamerTagAsync);

        app.MapPost("/api_WelcomePackage4.aspx", LegacyWelcomePackageAsync);
        app.MapPost("/api/api_WelcomePackage4.aspx", LegacyWelcomePackageAsync);

        app.MapPost("/v1/loadout/modify", async (
            LoadoutModifyRequest request,
            AuthService auth,
            ProfileWriteService service) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ApiResponse(false, session.Code, session.Message));

            var result = await service.ModifyLoadoutAsync(
                request.CustomerId,
                request.LoadoutId,
                new[]
                {
                    request.I1, request.I2, request.I3, request.I4, request.I5, request.I6,
                    request.I7, request.I8, request.I9, request.I10, request.I11, request.I12, request.I13
                });

            return Results.Json(result);
        });

        app.MapPost("/v1/loadout/unlock", async (
            LoadoutUnlockRequest request,
            AuthService auth,
            ProfileWriteService service) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ApiResponse(false, session.Code, session.Message));

            var result = await service.UnlockLoadoutAsync(request.CustomerId, request.Class, request.BuyIdx);
            return Results.Json(result);
        });

        app.MapPost("/v1/loadout/reset", async (
            LoadoutResetRequest request,
            AuthService auth,
            ProfileWriteService service) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ApiResponse(false, session.Code, session.Message));

            var result = await service.ResetLoadoutAsync(request.CustomerId, request.LoadoutId, request.Class, request.BuyIdx);
            return Results.Json(result);
        });

        app.MapPost("/v1/skill/learn", async (
            SkillLearnRequest request,
            AuthService auth,
            ProfileWriteService service) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ApiResponse(false, session.Code, session.Message));

            var result = await service.LearnSkillAsync(request.CustomerId, request.LoadoutId, request.SkillId, request.SkillLevel);
            return Results.Json(result);
        });

        app.MapPost("/v1/skill/reset", async (
            SkillResetRequest request,
            AuthService auth,
            ProfileWriteService service) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ApiResponse(false, session.Code, session.Message));

            var result = await service.ResetSkillAsync(request.CustomerId, request.LoadoutId);
            return Results.Json(result);
        });

        app.MapPost("/v1/weapon/attach", async (
            WeaponAttachRequest request,
            AuthService auth,
            ProfileWriteService service) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ApiResponse(false, session.Code, session.Message));

            var result = await service.WeaponAttachAsync(
                request.CustomerId,
                request.Func,
                request.WeaponId,
                request.AttachId,
                request.Slot,
                request.BuyIdx);

            return Results.Json(result);
        });

        app.MapPost("/v1/profile/change-gamertag", async (
            ChangeGamerTagRequest request,
            AuthService auth,
            ProfileWriteService service) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ApiResponse(false, session.Code, session.Message));

            var result = await service.ChangeGamerTagAsync(
                request.CustomerId,
                request.ItemId,
                request.BuyIdx,
                request.Gametag);

            return Results.Json(result);
        });

        app.MapPost("/v1/profile/welcome-package", async (
            WelcomePackageRequest request,
            AuthService auth,
            ProfileWriteService service) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ApiResponse(false, session.Code, session.Message));

            var result = await service.ApplyWelcomePackageAsync(request.CustomerId, request.SpecId);
            return Results.Json(result);
        });

        app.MapPost("/v1/friends/list", async (
            BasicProfileRequest request,
            AuthService auth) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ApiResponse(false, session.Code, session.Message));

            return Results.Json(new
            {
                ok = true,
                code = 0,
                message = "OK",
                friends = Array.Empty<object>(),
                incoming = Array.Empty<object>(),
                outgoing = Array.Empty<object>()
            });
        });

        app.MapPost("/v1/clan/get", async (
            BasicProfileRequest request,
            AuthService auth,
            ProfileWriteService service) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ApiResponse(false, session.Code, session.Message));

            var result = await service.GetClanStubAsync(request.CustomerId);
            return Results.Json(result);
        });
    }

    private static async Task<IResult> LegacyLoadoutModifyAsync(
        HttpContext http,
        AuthService auth,
        ProfileWriteService service)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);
        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var loadoutId = LegacyUtil.ParseInt(form["LoadoutID"].ToString());

        var items = new[]
        {
            LegacyUtil.ParseInt(form["i1"].ToString()),
            LegacyUtil.ParseInt(form["i2"].ToString()),
            LegacyUtil.ParseInt(form["i3"].ToString()),
            LegacyUtil.ParseInt(form["i4"].ToString()),
            LegacyUtil.ParseInt(form["i5"].ToString()),
            LegacyUtil.ParseInt(form["i6"].ToString()),
            LegacyUtil.ParseInt(form["i7"].ToString()),
            LegacyUtil.ParseInt(form["i8"].ToString()),
            LegacyUtil.ParseInt(form["i9"].ToString()),
            LegacyUtil.ParseInt(form["i10"].ToString()),
            LegacyUtil.ParseInt(form["i11"].ToString()),
            LegacyUtil.ParseInt(form["i12"].ToString()),
            LegacyUtil.ParseInt(form["i13"].ToString())
        };

        var result = await service.ModifyLoadoutAsync(customerId, loadoutId, items);

        if (!result.Ok)
            return Results.Text($"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}", "text/plain", Encoding.UTF8);

        return Results.Text($"WO_0{result.Data}", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyLoadoutUnlockAsync(
        HttpContext http,
        AuthService auth,
        ProfileWriteService service)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);
        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var cls = LegacyUtil.ParseInt(form["Class"].ToString());

        var buyIdx = LegacyUtil.ParseInt(form["BuyIdx"].ToString());
        if (buyIdx == 0)
            buyIdx = 4;

        var result = await service.UnlockLoadoutAsync(customerId, cls, buyIdx);

        if (!result.Ok)
            return Results.Text($"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}", "text/plain", Encoding.UTF8);

        return Results.Text("WO_0", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyLoadoutResetAsync(
        HttpContext http,
        AuthService auth,
        ProfileWriteService service)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);
        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var loadoutId = LegacyUtil.ParseInt(form["LoadoutID"].ToString());
        var cls = LegacyUtil.ParseInt(form["Class"].ToString());

        var buyIdx = LegacyUtil.ParseInt(form["BuyIdx"].ToString());
        if (buyIdx == 0)
            buyIdx = 4;

        var result = await service.ResetLoadoutAsync(customerId, loadoutId, cls, buyIdx);

        if (!result.Ok)
            return Results.Text($"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}", "text/plain", Encoding.UTF8);

        return Results.Text("WO_0", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> LegacySkillLearnAsync(
        HttpContext http,
        AuthService auth,
        ProfileWriteService service)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);
        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var loadoutId = LegacyUtil.ParseInt(form["LoadoutID"].ToString());
        var skillId = LegacyUtil.ParseInt(form["SkillID"].ToString());
        var skillLevel = LegacyUtil.ParseInt(form["SkillLevel"].ToString());

        var result = await service.LearnSkillAsync(customerId, loadoutId, skillId, skillLevel);

        if (!result.Ok)
            return Results.Text($"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}", "text/plain", Encoding.UTF8);

        return Results.Text($"WO_0{result.Data}", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> LegacySkillResetAsync(
        HttpContext http,
        AuthService auth,
        ProfileWriteService service)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);
        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var loadoutId = LegacyUtil.ParseInt(form["LoadoutID"].ToString());

        var result = await service.ResetSkillAsync(customerId, loadoutId);

        if (!result.Ok)
            return Results.Text($"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}", "text/plain", Encoding.UTF8);

        return Results.Text("WO_0", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyWeaponAttachAsync(
        HttpContext http,
        AuthService auth,
        ProfileWriteService service)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);
        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var func = form["func"].ToString();
        var weaponId = LegacyUtil.ParseInt(form["WeaponID"].ToString());
        var attachId = LegacyUtil.ParseInt(form["AttachID"].ToString());
        var slot = LegacyUtil.ParseInt(form["Slot"].ToString());
        var buyIdx = LegacyUtil.ParseInt(form["BuyIdx"].ToString());

        if (buyIdx == 0)
            buyIdx = 4;

        var result = await service.WeaponAttachAsync(customerId, func, weaponId, attachId, slot, buyIdx);

        if (!result.Ok)
            return Results.Text($"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}", "text/plain", Encoding.UTF8);

        if (func == "buy")
            return Results.Text($"WO_0{result.Balance}", "text/plain", Encoding.UTF8);

        return Results.Text("WO_0", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyWeaponAttachSetAsync(
        HttpContext http,
        AuthService auth,
        ProfileWriteService service)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);
        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var weaponId = LegacyUtil.ParseInt(form["WeaponID"].ToString());
        var attachId = LegacyUtil.ParseInt(form["AttachID"].ToString());
        var slot = LegacyUtil.ParseInt(form["Slot"].ToString());

        var result = await service.WeaponAttachAsync(customerId, "equip", weaponId, attachId, slot, 0);

        if (!result.Ok)
            return Results.Text($"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}", "text/plain", Encoding.UTF8);

        return Results.Text("WO_0", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyChangeGamerTagAsync(
        HttpContext http,
        AuthService auth,
        ProfileWriteService service)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);
        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var itemId = LegacyUtil.ParseInt(form["ItemID"].ToString());
        var buyIdx = LegacyUtil.ParseInt(form["BuyIdx"].ToString());
        var gamertag = form["gametag"].ToString();

        var result = await service.ChangeGamerTagAsync(customerId, itemId, buyIdx, gamertag);

        if (!result.Ok)
            return Results.Text($"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}", "text/plain", Encoding.UTF8);

        return Results.Text($"WO_0{result.Balance}", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyWelcomePackageAsync(
        HttpContext http,
        AuthService auth,
        ProfileWriteService service)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);
        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var specId = LegacyUtil.ParseInt(form["specID"].ToString());

        var result = await service.ApplyWelcomePackageAsync(customerId, specId);

        if (!result.Ok)
            return Results.Text($"WO_{LegacyUtil.NormalizeLegacyErrorCode(result.Code)} {result.Message}", "text/plain", Encoding.UTF8);

        return Results.Text("WO_0", "text/plain", Encoding.UTF8);
    }
}

public sealed class ProfileWriteService
{
    private const int LoadoutUnlockItemId = 301142;
    private const int LoadoutResetItemId = 301150;

    private const int NumTiers = 3;
    private const int NumRanks = 5;
    private const int NumSkillsPerTier = 10;
    private const int SkillClassMult = 100;
    private const int Tier2Unlock = 20;
    private const int Tier3Unlock = 20;
    private const int SkillSlots = NumTiers * NumSkillsPerTier;

    private static readonly Regex GamertagRegex = new("^[A-Za-z0-9]{1,16}$", RegexOptions.Compiled);

    private readonly WarIncDb _db;
    private readonly PurchaseService _purchase;

    public ProfileWriteService(WarIncDb db, PurchaseService purchase)
    {
        _db = db;
        _purchase = purchase;
    }

    public async Task<ProfileWriteResponse> ModifyLoadoutAsync(
        ulong customerId,
        int loadoutId,
        int[] items)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return ProfileWriteResponse.Fail(6, "bad CustomerID");

        if (loadoutId <= 0)
            return ProfileWriteResponse.Fail(6, "bad LoadoutID");

        if (items.Length != 13)
            return ProfileWriteResponse.Fail(6, "bad loadout items");

        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var owner = await db.ExecuteScalarAsync<ulong?>(
                """
                SELECT CustomerID
                FROM profile_chars
                WHERE LoadoutID = @LoadoutId
                LIMIT 1
                FOR UPDATE;
                """,
                new { LoadoutId = loadoutId },
                tx);

            if (owner == null)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "no loadout id");
            }

            if (owner.Value != customerId)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "bad customerid");
            }

            for (var i = 0; i < items.Length; i++)
                items[i] = await ValidateLoadoutItemAsync(db, tx, customerId, items[i]);

            var loadout = string.Join(' ', items);

            await db.ExecuteAsync(
                """
                UPDATE profile_chars
                SET Loadout = @Loadout
                WHERE LoadoutID = @LoadoutId;
                """,
                new
                {
                    LoadoutId = loadoutId,
                    Loadout = loadout
                },
                tx);

            await tx.CommitAsync();

            return ProfileWriteResponse.Success(loadout);
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return ProfileWriteResponse.Fail(5, e.Message);
        }
    }

    public async Task<ProfileWriteResponse> UnlockLoadoutAsync(
        ulong customerId,
        int cls,
        int buyIdx)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return ProfileWriteResponse.Fail(6, "bad CustomerID");

        var buy = await _purchase.BuyItemAsync(customerId, LoadoutUnlockItemId, buyIdx, 0);
        if (!buy.Ok)
            return ProfileWriteResponse.Fail(buy.Code, buy.Message, balance: buy.Balance);

        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var preset = GetClassPreset(cls);

            foreach (var item in preset.Items)
                await AddItemToUserAsync(db, tx, customerId, item, 2000);

            var loadoutId = await db.ExecuteScalarAsync<int>(
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
                    @Class,
                    0,
                    0,
                    @Loadout,
                    '',
                    0,
                    0,
                    0
                );

                SELECT LAST_INSERT_ID();
                """,
                new
                {
                    CustomerId = customerId,
                    Class = preset.Class,
                    Loadout = preset.Loadout
                },
                tx);

            await tx.CommitAsync();

            return ProfileWriteResponse.Success(preset.Loadout, buy.Balance, new
            {
                loadoutId,
                loadout = preset.Loadout
            });
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return ProfileWriteResponse.Fail(5, e.Message, balance: buy.Balance);
        }
    }

    public async Task<ProfileWriteResponse> ResetLoadoutAsync(
        ulong customerId,
        int loadoutId,
        int cls,
        int buyIdx)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return ProfileWriteResponse.Fail(6, "bad CustomerID");

        if (loadoutId <= 0)
            return ProfileWriteResponse.Fail(6, "bad LoadoutID");

        var buy = await _purchase.BuyItemAsync(customerId, LoadoutResetItemId, buyIdx, 0);
        if (!buy.Ok)
            return ProfileWriteResponse.Fail(buy.Code, buy.Message, balance: buy.Balance);

        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var owner = await db.ExecuteScalarAsync<ulong?>(
                """
                SELECT CustomerID
                FROM profile_chars
                WHERE LoadoutID = @LoadoutId
                LIMIT 1
                FOR UPDATE;
                """,
                new { LoadoutId = loadoutId },
                tx);

            if (owner == null)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "no loadout id", balance: buy.Balance);
            }

            if (owner.Value != customerId)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "bad customerid", balance: buy.Balance);
            }

            var preset = GetClassPreset(cls);

            foreach (var item in preset.Items)
                await AddItemToUserAsync(db, tx, customerId, item, 2000);

            await db.ExecuteAsync(
                """
                UPDATE profile_chars
                SET
                    Class = @Class,
                    HonorPoints = 0,
                    TimePlayed = 0,
                    Loadout = @Loadout,
                    Skills = '',
                    SpendSP1 = 0,
                    SpendSP2 = 0,
                    SpendSP3 = 0
                WHERE LoadoutID = @LoadoutId;
                """,
                new
                {
                    LoadoutId = loadoutId,
                    Class = preset.Class,
                    Loadout = preset.Loadout
                },
                tx);

            await tx.CommitAsync();

            return ProfileWriteResponse.Success(preset.Loadout, buy.Balance, new
            {
                loadoutId,
                loadout = preset.Loadout
            });
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return ProfileWriteResponse.Fail(5, e.Message, balance: buy.Balance);
        }
    }

    public async Task<ProfileWriteResponse> LearnSkillAsync(
        ulong customerId,
        int loadoutId,
        int skillId,
        int skillLevel)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return ProfileWriteResponse.Fail(6, "bad CustomerID");

        if (loadoutId <= 0)
            return ProfileWriteResponse.Fail(6, "bad LoadoutID");

        var skillClass = skillId / SkillClassMult;
        var skillOff = skillId % SkillClassMult;
        var skillTier = skillOff / NumSkillsPerTier;

        if (skillOff < 0 || skillOff >= SkillSlots)
            return ProfileWriteResponse.Fail(6, "bad skillId");

        if (skillLevel < 1 || skillLevel > NumRanks)
            return ProfileWriteResponse.Fail(6, "bad skill level");

        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var playerSp = await db.ExecuteScalarAsync<int?>(
                """
                SELECT SkillPoints
                FROM loginid
                WHERE CustomerID = @CustomerId
                LIMIT 1
                FOR UPDATE;
                """,
                new { CustomerId = customerId },
                tx);

            if (playerSp == null)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "no customer");
            }

            var loadout = await db.QueryFirstOrDefaultAsync<SkillLoadoutRow>(
                """
                SELECT
                    CustomerID AS CustomerId,
                    Class AS Class,
                    IFNULL(Skills, '') AS Skills,
                    IFNULL(SpendSP1, 0) AS SpendSP1,
                    IFNULL(SpendSP2, 0) AS SpendSP2,
                    IFNULL(SpendSP3, 0) AS SpendSP3
                FROM profile_chars
                WHERE LoadoutID = @LoadoutId
                LIMIT 1
                FOR UPDATE;
                """,
                new { LoadoutId = loadoutId },
                tx);

            if (loadout == null || loadout.CustomerId != customerId)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "bad loadout");
            }

            if (loadout.Class != skillClass)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "wrong skill class");
            }

            var prices = await db.QueryFirstOrDefaultAsync<SkillPriceRow>(
                """
                SELECT
                    SkillID AS SkillId,
                    ABS(IFNULL(Lv1, 0)) AS Lv1,
                    ABS(IFNULL(Lv2, 0)) AS Lv2,
                    ABS(IFNULL(Lv3, 0)) AS Lv3,
                    ABS(IFNULL(Lv4, 0)) AS Lv4,
                    ABS(IFNULL(Lv5, 0)) AS Lv5
                FROM dataskill2price
                WHERE SkillID = @SkillId
                LIMIT 1;
                """,
                new { SkillId = skillId },
                tx);

            if (prices == null)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "bad skillid data");
            }

            var skillData = ParseSkills(loadout.Skills);

            if (skillData[skillOff] + 1 != skillLevel)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "bad skill advance");
            }

            var price = skillLevel switch
            {
                1 => prices.Lv1,
                2 => prices.Lv2,
                3 => prices.Lv3,
                4 => prices.Lv4,
                5 => prices.Lv5,
                _ => 0
            };

            if (price <= 0)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "skill level price is not set");
            }

            var spendSp1 = loadout.SpendSP1;
            var spendSp2 = loadout.SpendSP2;
            var spendSp3 = loadout.SpendSP3;

            switch (skillTier)
            {
                case 0:
                    spendSp1 += price;
                    break;

                case 1:
                    if (spendSp1 < Tier2Unlock)
                    {
                        await tx.RollbackAsync();
                        return ProfileWriteResponse.Fail(6, "not enough sp for tier2");
                    }

                    spendSp2 += price;
                    break;

                case 2:
                    if (spendSp2 < Tier3Unlock)
                    {
                        await tx.RollbackAsync();
                        return ProfileWriteResponse.Fail(6, "not enough sp for tier3");
                    }

                    spendSp3 += price;
                    break;

                default:
                    await tx.RollbackAsync();
                    return ProfileWriteResponse.Fail(6, "bad skill tier");
            }

            if (spendSp1 + spendSp2 + spendSp3 > playerSp.Value)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(7, "not enough SP");
            }

            skillData[skillOff]++;

            var skillString = BuildSkillString(skillData);

            await db.ExecuteAsync(
                """
                UPDATE profile_chars
                SET
                    SpendSP1 = @SpendSP1,
                    SpendSP2 = @SpendSP2,
                    SpendSP3 = @SpendSP3,
                    Skills = @Skills
                WHERE LoadoutID = @LoadoutId;
                """,
                new
                {
                    LoadoutId = loadoutId,
                    SpendSP1 = spendSp1,
                    SpendSP2 = spendSp2,
                    SpendSP3 = spendSp3,
                    Skills = skillString
                },
                tx);

            await tx.CommitAsync();

            return ProfileWriteResponse.Success($"{spendSp1} {spendSp2} {spendSp3}", 0, new
            {
                spendSp1,
                spendSp2,
                spendSp3,
                skills = skillString
            });
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return ProfileWriteResponse.Fail(5, e.Message);
        }
    }

    public async Task<ProfileWriteResponse> ResetSkillAsync(
        ulong customerId,
        int loadoutId)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return ProfileWriteResponse.Fail(6, "bad CustomerID");

        await using var db = _db.CreateConnection();

        var rows = await db.ExecuteAsync(
            """
            UPDATE profile_chars
            SET
                SpendSP1 = 0,
                SpendSP2 = 0,
                SpendSP3 = 0,
                Skills = ''
            WHERE CustomerID = @CustomerId
              AND LoadoutID = @LoadoutId;
            """,
            new
            {
                CustomerId = customerId,
                LoadoutId = loadoutId
            });

        if (rows <= 0)
            return ProfileWriteResponse.Fail(6, "bad loadout");

        return ProfileWriteResponse.Success();
    }

    public async Task<ProfileWriteResponse> WeaponAttachAsync(
        ulong customerId,
        string func,
        int weaponId,
        int attachId,
        int slot,
        int buyIdx)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return ProfileWriteResponse.Fail(6, "bad CustomerID");

        if (weaponId <= 0)
            return ProfileWriteResponse.Fail(6, "bad WeaponID");

        if (func == "buy")
        {
            var check = await CheckWeaponAttachmentBuyAsync(customerId, weaponId, attachId, slot);
            if (!check.Ok)
                return check;

            var buy = await _purchase.BuyItemAsync(customerId, attachId, buyIdx, weaponId);
            if (!buy.Ok)
                return ProfileWriteResponse.Fail(buy.Code, buy.Message, balance: buy.Balance);

            return ProfileWriteResponse.Success(balance: buy.Balance);
        }

        if (func == "equip")
            return await EquipWeaponAttachmentAsync(customerId, weaponId, attachId, slot);

        if (func == "fix")
        {
            await using var db = _db.CreateConnection();
            await db.OpenAsync();

            await using var tx = await db.BeginTransactionAsync();

            try
            {
                await AddDefaultAttachmentsToUserAsync(db, tx, customerId, weaponId, 2000);
                await tx.CommitAsync();
                return ProfileWriteResponse.Success();
            }
            catch (Exception e)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(5, e.Message);
            }
        }

        return ProfileWriteResponse.Fail(6, "bad func");
    }

    public async Task<ProfileWriteResponse> ChangeGamerTagAsync(
        ulong customerId,
        int itemId,
        int buyIdx,
        string gamertag)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return ProfileWriteResponse.Fail(6, "bad CustomerID");

        gamertag = (gamertag ?? "").Trim();

        if (!GamertagRegex.IsMatch(gamertag))
            return ProfileWriteResponse.Fail(4, "bad gamertag");

        await using (var db = _db.CreateConnection())
        {
            var exists = await db.ExecuteScalarAsync<int>(
                """
                SELECT COUNT(*)
                FROM loginid
                WHERE Gamertag = @Gamertag;
                """,
                new { Gamertag = gamertag });

            if (exists > 0)
                return ProfileWriteResponse.Fail(9, "Gamertag already exists");
        }

        var buy = await _purchase.BuyItemAsync(customerId, itemId, buyIdx, 0);
        if (!buy.Ok)
            return ProfileWriteResponse.Fail(buy.Code, buy.Message, balance: buy.Balance);

        await using (var db = _db.CreateConnection())
        {
            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET Gamertag = @Gamertag
                WHERE CustomerID = @CustomerId;
                """,
                new
                {
                    CustomerId = customerId,
                    Gamertag = gamertag
                });
        }

        return ProfileWriteResponse.Success(balance: buy.Balance, payload: new
        {
            gamertag
        });
    }

    public async Task<ProfileWriteResponse> ApplyWelcomePackageAsync(
        ulong customerId,
        int cls)
    {
        if (customerId == 0 || customerId > int.MaxValue)
            return ProfileWriteResponse.Fail(6, "bad CustomerID");

        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            var accountStatus = await db.ExecuteScalarAsync<int?>(
                """
                SELECT AccountStatus
                FROM loginid
                WHERE CustomerID = @CustomerId
                LIMIT 1
                FOR UPDATE;
                """,
                new { CustomerId = customerId },
                tx);

            if (accountStatus == null)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "no CustomerID");
            }

            if (accountStatus.Value != 100)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(7, "bad AccountStatus");
            }

            foreach (var item in GetWelcomeItems())
                await AddItemToUserAsync(db, tx, customerId, item.ItemId, item.ExpDays);

            await db.ExecuteAsync(
                """
                UPDATE loginid
                SET
                    AccountStatus = 101,
                    GameDollars = GameDollars + 5000
                WHERE CustomerID = @CustomerId;
                """,
                new { CustomerId = customerId },
                tx);

            var preset = GetClassPreset(cls);

            foreach (var item in preset.Items)
                await AddItemToUserAsync(db, tx, customerId, item, 2000);

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
                    @Class,
                    0,
                    0,
                    @Loadout,
                    '',
                    0,
                    0,
                    0
                );
                """,
                new
                {
                    CustomerId = customerId,
                    Class = preset.Class,
                    Loadout = preset.Loadout
                },
                tx);

            await tx.CommitAsync();

            return ProfileWriteResponse.Success();
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return ProfileWriteResponse.Fail(5, e.Message);
        }
    }

    public async Task<object> GetClanStubAsync(ulong customerId)
    {
        await using var db = _db.CreateConnection();

        var row = await db.QueryFirstOrDefaultAsync<ClanAccountRow>(
            """
            SELECT
                l.ClanID AS ClanId,
                l.ClanRank AS ClanRank,
                IFNULL(c.ClanName, '') AS ClanName,
                IFNULL(c.ClanTag, '') AS ClanTag,
                IFNULL(c.ClanTagColor, 0) AS ClanTagColor,
                IFNULL(c.ClanLevel, 0) AS ClanLevel,
                IFNULL(c.ClanXP, 0) AS ClanXp
            FROM loginid l
            LEFT JOIN clandata c ON c.ClanID = l.ClanID
            WHERE l.CustomerID = @CustomerId
            LIMIT 1;
            """,
            new { CustomerId = customerId });

        if (row == null || row.ClanId <= 0)
        {
            return new
            {
                ok = true,
                code = 0,
                message = "OK",
                clan = (object?)null,
                members = Array.Empty<object>(),
                invites = Array.Empty<object>(),
                applications = Array.Empty<object>()
            };
        }

        return new
        {
            ok = true,
            code = 0,
            message = "OK",
            clan = row,
            members = Array.Empty<object>(),
            invites = Array.Empty<object>(),
            applications = Array.Empty<object>()
        };
    }

    private static async Task<int> ValidateLoadoutItemAsync(
        MySqlConnection db,
        MySqlTransaction tx,
        ulong customerId,
        int itemId)
    {
        if (itemId <= 0)
            return 0;

        var exists = await db.ExecuteScalarAsync<int>(
            """
            SELECT COUNT(*)
            FROM inventory
            WHERE CustomerID = @CustomerId
              AND ItemID = @ItemId
              AND LeasedUntil > UTC_TIMESTAMP();
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId
            },
            tx);

        return exists > 0 ? itemId : 0;
    }

    private async Task<ProfileWriteResponse> CheckWeaponAttachmentBuyAsync(
        ulong customerId,
        int weaponId,
        int attachId,
        int slot)
    {
        if (attachId <= 0)
            return ProfileWriteResponse.Fail(6, "bad AttachmentID");

        await using var db = _db.CreateConnection();

        var attachment = await db.QueryFirstOrDefaultAsync<AttachmentSpecRow>(
            """
            SELECT
                ItemID AS ItemId,
                Type AS Slot,
                SpecID AS SpecId
            FROM items_attachments
            WHERE ItemID = @AttachmentId
            LIMIT 1;
            """,
            new { AttachmentId = attachId });

        if (attachment == null)
            return ProfileWriteResponse.Fail(6, "no such attachment");

        if (attachment.Slot != slot)
            return ProfileWriteResponse.Fail(6, "bad attachment slot");

        var weapon = await db.QueryFirstOrDefaultAsync<WeaponSpecRow>(
            """
            SELECT
                ItemID AS ItemId,
                FPSSpec0 AS FpsSpec0,
                FPSSpec1 AS FpsSpec1,
                FPSSpec2 AS FpsSpec2,
                FPSSpec3 AS FpsSpec3,
                FPSSpec4 AS FpsSpec4,
                FPSSpec5 AS FpsSpec5,
                FPSSpec6 AS FpsSpec6,
                FPSSpec7 AS FpsSpec7,
                FPSSpec8 AS FpsSpec8
            FROM items_weapons
            WHERE ItemID = @WeaponId
            LIMIT 1;
            """,
            new { WeaponId = weaponId });

        if (weapon == null)
            return ProfileWriteResponse.Fail(6, "no such weapon");

        var allowedSpec = slot switch
        {
            0 => weapon.FpsSpec0,
            1 => weapon.FpsSpec1,
            2 => weapon.FpsSpec2,
            3 => weapon.FpsSpec3,
            4 => weapon.FpsSpec4,
            5 => weapon.FpsSpec5,
            6 => weapon.FpsSpec6,
            7 => weapon.FpsSpec7,
            8 => weapon.FpsSpec8,
            _ => -1
        };

        if (allowedSpec != attachment.SpecId)
            return ProfileWriteResponse.Fail(6, "SpecID mismatch");

        return ProfileWriteResponse.Success();
    }

    private async Task<ProfileWriteResponse> EquipWeaponAttachmentAsync(
        ulong customerId,
        int weaponId,
        int attachId,
        int slot)
    {
        await using var db = _db.CreateConnection();
        await db.OpenAsync();

        await using var tx = await db.BeginTransactionAsync();

        try
        {
            if (attachId == 0)
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
                        Slot = slot
                    },
                    tx);

                await tx.CommitAsync();
                return ProfileWriteResponse.Success();
            }

            var current = await db.QueryFirstOrDefaultAsync<FpsAttachmentRow>(
                """
                SELECT
                    Slot AS Slot,
                    IsEquipped AS IsEquipped
                FROM inventory_fps
                WHERE CustomerID = @CustomerId
                  AND WeaponID = @WeaponId
                  AND AttachmentID = @AttachmentId
                  AND LeasedUntil > UTC_TIMESTAMP()
                LIMIT 1
                FOR UPDATE;
                """,
                new
                {
                    CustomerId = customerId,
                    WeaponId = weaponId,
                    AttachmentId = attachId
                },
                tx);

            if (current == null)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "attachment not exists");
            }

            if (current.Slot != slot)
            {
                await tx.RollbackAsync();
                return ProfileWriteResponse.Fail(6, "wrong slot");
            }

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
                    Slot = slot
                },
                tx);

            await db.ExecuteAsync(
                """
                UPDATE inventory_fps
                SET IsEquipped = 1
                WHERE CustomerID = @CustomerId
                  AND WeaponID = @WeaponId
                  AND AttachmentID = @AttachmentId;
                """,
                new
                {
                    CustomerId = customerId,
                    WeaponId = weaponId,
                    AttachmentId = attachId
                },
                tx);

            await tx.CommitAsync();

            return ProfileWriteResponse.Success();
        }
        catch (Exception e)
        {
            await tx.RollbackAsync();
            return ProfileWriteResponse.Fail(5, e.Message);
        }
    }

    private static async Task AddItemToUserAsync(
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

        if (itemId >= 100000 && itemId <= 199999)
            await AddDefaultAttachmentsToUserAsync(db, tx, customerId, itemId, expDays);

        var current = await db.QueryFirstOrDefaultAsync<InventoryLeaseRow>(
            """
            SELECT
                LeasedUntil AS LeasedUntil,
                Quantity AS Quantity
            FROM inventory
            WHERE CustomerID = @CustomerId
              AND ItemID = @ItemId
            LIMIT 1
            FOR UPDATE;
            """,
            new
            {
                CustomerId = customerId,
                ItemId = itemId
            },
            tx);

        if (current == null)
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

    private static async Task AddDefaultAttachmentsToUserAsync(
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

            await AddAttachmentToUserAsync(db, tx, customerId, weaponId, attachmentId, expDays);
        }
    }

    private static async Task AddAttachmentToUserAsync(
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

        var current = await db.QueryFirstOrDefaultAsync<FpsAttachmentLeaseRow>(
            """
            SELECT LeasedUntil AS LeasedUntil
            FROM inventory_fps
            WHERE CustomerID = @CustomerId
              AND WeaponID = @WeaponId
              AND AttachmentID = @AttachmentId
            LIMIT 1
            FOR UPDATE;
            """,
            new
            {
                CustomerId = customerId,
                WeaponId = weaponId,
                AttachmentId = attachmentId
            },
            tx);

        if (current == null)
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

    private static int[] ParseSkills(string? value)
    {
        var result = new int[SkillSlots];
        var text = value ?? "";

        var len = Math.Min(text.Length, SkillSlots);

        for (var i = 0; i < len; i++)
        {
            var c = text[i];
            result[i] = c >= '0' && c <= '9' ? c - '0' : 0;
        }

        return result;
    }

    private static string BuildSkillString(int[] skills)
    {
        var sb = new StringBuilder();

        for (var i = 0; i < SkillSlots; i++)
            sb.Append((char)('0' + Math.Clamp(skills[i], 0, 9)));

        return sb.ToString().TrimEnd(' ', '0');
    }

    private static LoadoutPreset GetClassPreset(int cls)
    {
        if (cls == 0)
        {
            return new LoadoutPreset(
                0,
                "20061 0 20019 20026 0 0 0 0 0 0 101002 101158 101004",
                new[] { 20026, 20019, 20061, 101002, 101158, 101004 });
        }

        if (cls == 1)
        {
            return new LoadoutPreset(
                1,
                "20056 0 20020 20092 0 0 0 0 0 0 101093 101003 101004",
                new[] { 20092, 20020, 20056, 101093, 101003, 101004 });
        }

        if (cls == 2)
        {
            return new LoadoutPreset(
                2,
                "20014 0 20018 20000 0 0 0 0 0 0 101068 101103 101004",
                new[] { 20000, 20018, 20014, 101068, 101103, 101004 });
        }

        return new LoadoutPreset(
            3,
            "20001 0 20021 20007 0 0 0 0 0 0 101002 101103 101004",
            new[] { 20007, 20021, 20001, 20043, 101002, 101103, 101004 });
    }

    private static IEnumerable<WelcomeItem> GetWelcomeItems()
    {
        yield return new WelcomeItem(101004, 2000);
        yield return new WelcomeItem(101008, 2000);
        yield return new WelcomeItem(101149, 2000);
        yield return new WelcomeItem(101032, 2000);

        yield return new WelcomeItem(20000, 2000);
        yield return new WelcomeItem(20001, 2000);
        yield return new WelcomeItem(20019, 2000);
        yield return new WelcomeItem(20047, 2000);
        yield return new WelcomeItem(20014, 2000);

        yield return new WelcomeItem(101193, 5);
        yield return new WelcomeItem(101109, 5);
        yield return new WelcomeItem(101247, 5);
        yield return new WelcomeItem(101183, 5);
        yield return new WelcomeItem(101060, 5);
        yield return new WelcomeItem(101202, 5);
        yield return new WelcomeItem(101107, 5);
        yield return new WelcomeItem(101180, 5);

        yield return new WelcomeItem(20022, 5);
        yield return new WelcomeItem(20023, 5);
        yield return new WelcomeItem(20024, 5);
        yield return new WelcomeItem(20025, 5);
        yield return new WelcomeItem(20035, 5);
        yield return new WelcomeItem(20036, 5);
        yield return new WelcomeItem(20043, 5);
        yield return new WelcomeItem(20046, 5);
        yield return new WelcomeItem(20052, 5);
        yield return new WelcomeItem(20067, 5);

        yield return new WelcomeItem(20018, 5);
        yield return new WelcomeItem(20019, 5);
        yield return new WelcomeItem(20020, 5);
        yield return new WelcomeItem(20021, 5);
        yield return new WelcomeItem(20004, 5);
        yield return new WelcomeItem(20005, 5);
        yield return new WelcomeItem(20064, 5);

        yield return new WelcomeItem(20003, 5);
        yield return new WelcomeItem(20015, 5);
        yield return new WelcomeItem(20016, 5);
        yield return new WelcomeItem(20054, 5);
        yield return new WelcomeItem(20056, 5);
        yield return new WelcomeItem(20061, 5);
        yield return new WelcomeItem(20065, 5);

        yield return new WelcomeItem(20007, 5);
        yield return new WelcomeItem(20012, 5);
        yield return new WelcomeItem(20026, 5);
        yield return new WelcomeItem(20038, 5);
        yield return new WelcomeItem(20039, 5);
        yield return new WelcomeItem(20070, 5);
        yield return new WelcomeItem(20071, 5);
    }
}

public record BasicProfileRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token);

public record LoadoutModifyRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    int LoadoutId,
    int I1,
    int I2,
    int I3,
    int I4,
    int I5,
    int I6,
    int I7,
    int I8,
    int I9,
    int I10,
    int I11,
    int I12,
    int I13);

public record LoadoutUnlockRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    int Class,
    int BuyIdx = 4);

public record LoadoutResetRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    int LoadoutId,
    int Class,
    int BuyIdx = 4);

public record SkillLearnRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    int LoadoutId,
    int SkillId,
    int SkillLevel);

public record SkillResetRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    int LoadoutId);

public record WeaponAttachRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    string Func,
    int WeaponId,
    int AttachId,
    int Slot,
    int BuyIdx = 4);

public record ChangeGamerTagRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    int ItemId,
    int BuyIdx,
    string Gametag);

public record WelcomePackageRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    int SpecId);

public sealed class ProfileWriteResponse
{
    public bool Ok { get; set; }
    public int Code { get; set; }
    public string Message { get; set; } = "";
    public string Data { get; set; } = "";
    public int Balance { get; set; }
    public object? Payload { get; set; }

    public static ProfileWriteResponse Success(
        string data = "",
        int balance = 0,
        object? payload = null)
    {
        return new ProfileWriteResponse
        {
            Ok = true,
            Code = 0,
            Message = "OK",
            Data = data,
            Balance = balance,
            Payload = payload
        };
    }

    public static ProfileWriteResponse Fail(
        int code,
        string message,
        int balance = 0)
    {
        return new ProfileWriteResponse
        {
            Ok = false,
            Code = code,
            Message = message,
            Balance = balance
        };
    }
}

public sealed class SkillLoadoutRow
{
    public ulong CustomerId { get; set; }
    public int Class { get; set; }
    public string Skills { get; set; } = "";
    public int SpendSP1 { get; set; }
    public int SpendSP2 { get; set; }
    public int SpendSP3 { get; set; }
}

public sealed class SkillPriceRow
{
    public int SkillId { get; set; }
    public int Lv1 { get; set; }
    public int Lv2 { get; set; }
    public int Lv3 { get; set; }
    public int Lv4 { get; set; }
    public int Lv5 { get; set; }
}

public sealed class InventoryLeaseRow
{
    public DateTime LeasedUntil { get; set; }
    public int Quantity { get; set; }
}

public sealed class FpsAttachmentLeaseRow
{
    public DateTime LeasedUntil { get; set; }
}

public sealed class FpsAttachmentRow
{
    public int Slot { get; set; }
    public int IsEquipped { get; set; }
}

public sealed class AttachmentSpecRow
{
    public int ItemId { get; set; }
    public int Slot { get; set; }
    public int SpecId { get; set; }
}

public sealed class WeaponSpecRow
{
    public int ItemId { get; set; }
    public int FpsSpec0 { get; set; }
    public int FpsSpec1 { get; set; }
    public int FpsSpec2 { get; set; }
    public int FpsSpec3 { get; set; }
    public int FpsSpec4 { get; set; }
    public int FpsSpec5 { get; set; }
    public int FpsSpec6 { get; set; }
    public int FpsSpec7 { get; set; }
    public int FpsSpec8 { get; set; }
}

public sealed class DefaultWeaponAttachmentsRow
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

public sealed class ClanAccountRow
{
    public int ClanId { get; set; }
    public int ClanRank { get; set; }
    public string ClanName { get; set; } = "";
    public string ClanTag { get; set; } = "";
    public int ClanTagColor { get; set; }
    public int ClanLevel { get; set; }
    public int ClanXp { get; set; }
}

public record LoadoutPreset(
    int Class,
    string Loadout,
    int[] Items);

public record WelcomeItem(
    int ItemId,
    int ExpDays);