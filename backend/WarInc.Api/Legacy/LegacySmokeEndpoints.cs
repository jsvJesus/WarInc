using System.Net;
using System.Text;
using System.Text.Json; // ebana v rot
using System.Text.RegularExpressions;

namespace WarInc.Api.Legacy;

public static class LegacySmokeEndpoints
{
    private static readonly string[] LegacyHttpMethods =
    [
        "GET",
        "POST",
        "PUT",
        "PATCH",
        "DELETE",
        "HEAD",
        "OPTIONS"
    ];

    private static readonly Regex LegacyApiRegex = new(
        @"^api_[A-Za-z0-9_]+\.aspx$",
        RegexOptions.Compiled | RegexOptions.IgnoreCase | RegexOptions.CultureInvariant);

    public static readonly string[] KnownLegacyApiEndpoints =
    [
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
    ];

    private static readonly HashSet<string> KnownLegacyApiSet = new(
        KnownLegacyApiEndpoints,
        StringComparer.OrdinalIgnoreCase);

    public static void MapLegacySmokeEndpoints(this WebApplication app)
    {
        app.MapGet("/legacy/smoke", SmokeInfo);
        app.MapGet("/legacy/smoke/endpoints", SmokeEndpoints);

        app.MapMethods("/{legacyEndpoint}", LegacyHttpMethods, LegacyFallbackAsync);
        app.MapMethods("/api/{legacyEndpoint}", LegacyHttpMethods, LegacyFallbackAsync);
    }

    private static IResult SmokeInfo(HttpContext http)
    {
        return Results.Json(new
        {
            ok = true,
            code = 0,
            service = "WarInc.Api",
            mode = "legacy-smoke",
            message = "Legacy api_*.aspx fallback is enabled",
            endpointsCount = KnownLegacyApiEndpoints.Length,
            testListUrl = "/legacy/smoke/endpoints"
        });
    }

    private static IResult SmokeEndpoints(HttpContext http)
    {
        return Results.Json(new
        {
            ok = true,
            code = 0,
            endpoints = KnownLegacyApiEndpoints
        });
    }

    private static async Task<IResult> LegacyFallbackAsync(HttpContext http)
    {
        var endpoint = GetLegacyEndpointName(http);

        if (!IsLegacyApiName(endpoint))
            return Results.NotFound();

        var values = await ReadRequestValuesAsync(http);
        var known = KnownLegacyApiSet.Contains(endpoint);

        http.Response.Headers["X-WarInc-Legacy-Smoke"] = "1";
        http.Response.Headers["X-WarInc-Legacy-Endpoint"] = endpoint;
        http.Response.Headers["X-WarInc-Legacy-Known"] = known ? "1" : "0";
        http.Response.Headers.CacheControl = "no-store";

        if (http.Request.Method.Equals("OPTIONS", StringComparison.OrdinalIgnoreCase))
            return Results.Text("", "text/plain", Encoding.UTF8);

        var body = BuildLegacySmokeBody(endpoint, values);

        if (WantsJson(http))
        {
            return Results.Json(new
            {
                ok = true,
                code = 0,
                legacy = true,
                smoke = true,
                known,
                endpoint,
                method = http.Request.Method,
                body
            });
        }

        return Results.Text(body, "text/plain", Encoding.UTF8);
    }

    private static string GetLegacyEndpointName(HttpContext http)
    {
        var routeValue = http.Request.RouteValues["legacyEndpoint"]?.ToString();

        if (!string.IsNullOrWhiteSpace(routeValue))
            return WebUtility.UrlDecode(routeValue).Trim();

        var rawPath = http.Request.Path.Value ?? "";
        var lastSlash = rawPath.LastIndexOf('/');

        if (lastSlash >= 0)
            rawPath = rawPath[(lastSlash + 1)..];

        rawPath = WebUtility.UrlDecode(rawPath);
        return rawPath.Trim();
    }

    private static bool IsLegacyApiName(string endpoint)
    {
        if (string.IsNullOrWhiteSpace(endpoint))
            return false;

        return LegacyApiRegex.IsMatch(endpoint);
    }

    private static bool WantsJson(HttpContext http)
    {
        if (http.Request.Query.ContainsKey("json"))
            return true;

        var accept = http.Request.Headers.Accept.ToString();
        return accept.Contains("application/json", StringComparison.OrdinalIgnoreCase);
    }

    private static async Task<Dictionary<string, string>> ReadRequestValuesAsync(HttpContext http)
    {
        var values = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

        foreach (var item in http.Request.Query)
            values[item.Key] = item.Value.ToString();

        if (http.Request.HasFormContentType)
        {
            var form = await http.Request.ReadFormAsync();

            foreach (var item in form)
                values[item.Key] = item.Value.ToString();
        }

        return values;
    }

    private static string BuildLegacySmokeBody(string endpoint, IReadOnlyDictionary<string, string> values)
    {
        var func = Get(values, "func").Trim().ToLowerInvariant();

        return endpoint.ToLowerInvariant() switch
        {
            "api_login.aspx" => "WO_00 0 0",
            "api_steamlogin.aspx" => "WO_00 0 0",
            "api_gnalogin.aspx" => "WO_00 0 0",
            "api_gamersfirst.aspx" => BuildGamersFirstSmoke(func),
            "api_steamcreateacc.aspx" => "WO_0",

            "api_checkloginsession.aspx" => "WO_0",
            "api_updateloginsession.aspx" => "WO_0",
            "api_loginsessionpoller.aspx" => "WO_0",

            "api_getprofile4.aspx" => BuildProfileSmoke(values),
            "api_getitemsinfo.aspx" => "WO_0<items><gears></gears><weapons></weapons><packages></packages><generics></generics></items>",
            "api_getshop5.aspx" => "WO_0",

            "api_buyitem3.aspx" => "WO_00",
            "api_changegamertag.aspx" => "WO_00",
            "api_changegamertag2.aspx" => "WO_00",

            "api_loadoutmodify.aspx" => BuildLoadoutModifySmoke(values),
            "api_loadoutunlock.aspx" => "WO_0",
            "api_loadoutreset.aspx" => "WO_0",

            "api_skilllearn.aspx" => "WO_00 0 0",
            "api_skillreset.aspx" => "WO_0",

            "api_weaponattach.aspx" => BuildWeaponAttachSmoke(func),
            "api_weaponattachset.aspx" => "WO_0",

            "api_welcomepackage4.aspx" => "WO_0",

            "api_friends.aspx" => BuildFriendsSmoke(func),
            "api_clanmgr.aspx" => BuildClanMgrSmoke(func),
            "api_clanapply.aspx" => "WO_0",
            "api_clancreate.aspx" => "WO_0",
            "api_clangetinfo.aspx" => "WO_0<clan id=\"0\" name=\"\" tag=\"\" tagcolor=\"0\"></clan>",
            "api_claninvites.aspx" => "WO_0<invites></invites>",

            "api_leaderboardget.aspx" => "WO_0<leaderboard pos=\"0\" size=\"0\"></leaderboard>",
            "api_mysterybox.aspx" => BuildMysteryBoxSmoke(func),
            "api_retbonus.aspx" => BuildRetentionBonusSmoke(func),

            "api_gnagetbalance.aspx" => "WO_00",
            "api_steambuygp.aspx" => BuildSteamBuyGpSmoke(func),

            "api_getcreategamekey3.aspx" => "WO_00",
            "api_setrsupdatestatus.aspx" => "WO_0",
            "api_getdatagamerewards.aspx" => "WO_0<rewards></rewards>",

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

    private static string BuildGamersFirstSmoke(string func)
    {
        if (func == "login")
            return "WO_00 0 0 0 0";

        return "WO_0";
    }

    private static string BuildWeaponAttachSmoke(string func)
    {
        if (func == "buy")
            return "WO_00";

        return "WO_0";
    }

    private static string BuildFriendsSmoke(string func)
    {
        if (func == "addreq")
            return "WO_00";

        if (func == "stats")
            return "WO_0<friends></friends>";

        return "WO_0";
    }

    private static string BuildClanMgrSmoke(string func)
    {
        if (func == "list")
            return "WO_0<clans></clans>";

        if (func == "members")
            return "WO_0<members></members>";

        return "WO_0";
    }

    private static string BuildMysteryBoxSmoke(string func)
    {
        if (func == "info")
            return "WO_0<box></box>";

        if (func == "roll")
            return "WO_00 0 0 0";

        if (func == "sell")
            return "WO_0";

        return "WO_0";
    }

    private static string BuildRetentionBonusSmoke(string func)
    {
        if (func == "info")
            return "WO_0<retbonus d=\"0\" m=\"0\"><days></days></retbonus>";

        if (func == "give")
            return "WO_00";

        return "WO_0";
    }

    private static string BuildSteamBuyGpSmoke(string func)
    {
        if (func == "shop")
            return "WO_0<SteamGPShop></SteamGPShop>";

        if (func == "fin")
            return "WO_00";

        return "WO_0";
    }

    private static string BuildLoadoutModifySmoke(IReadOnlyDictionary<string, string> values)
    {
        var items = new string[13];

        for (var i = 0; i < items.Length; i++)
        {
            var key = "i" + (i + 1);
            var value = Get(values, key);

            if (string.IsNullOrWhiteSpace(value))
                value = "0";

            items[i] = value.Trim();
        }

        return "WO_0" + string.Join(' ', items);
    }

    private static string BuildProfileSmoke(IReadOnlyDictionary<string, string> values)
    {
        var customerId = Get(values, "s_id");

        if (string.IsNullOrWhiteSpace(customerId) || customerId == "0")
            customerId = "1";

        var now = DateTime.UtcNow;

        var xml = new StringBuilder();

        xml.Append("WO_0");
        xml.Append("<account ");
        xml.Append("CustomerID=\"").Append(Xml(customerId)).Append("\" ");
        xml.Append("AccountStatus=\"100\" ");
        xml.Append("gamertag=\"SmokeTester\" ");
        xml.Append("gamepoints=\"0\" ");
        xml.Append("GameDollars=\"0\" ");
        xml.Append("HonorPoints=\"0\" ");
        xml.Append("SkillPoints=\"0\" ");
        xml.Append("Kills=\"0\" ");
        xml.Append("Deaths=\"0\" ");
        xml.Append("ShotsFired=\"0\" ");
        xml.Append("ShotsHits=\"0\" ");
        xml.Append("Headshots=\"0\" ");
        xml.Append("AssistKills=\"0\" ");
        xml.Append("Wins=\"0\" ");
        xml.Append("Losses=\"0\" ");
        xml.Append("CaptureNeutralPoints=\"0\" ");
        xml.Append("CaptureEnemyPoints=\"0\" ");
        xml.Append("TimePlayed=\"0\" ");
        xml.Append("IsDev=\"1\" ");
        xml.Append("F1S=\"0\" ");
        xml.Append("F2S=\"0\" ");
        xml.Append("F3S=\"0\" ");
        xml.Append("F4S=\"0\" ");
        xml.Append("F5S=\"0\" ");
        xml.Append("ClanID=\"0\" ");
        xml.Append("ClanRank=\"0\" ");
        xml.Append("ClanTag=\"\" ");
        xml.Append("ClanTagColor=\"0\" ");
        xml.Append("IsFPSEnabled=\"1\" ");
        xml.Append("time=\"")
            .Append(now.Year)
            .Append(' ')
            .Append(now.Month)
            .Append(' ')
            .Append(now.Day)
            .Append(' ')
            .Append(now.Hour)
            .Append(' ')
            .Append(now.Minute)
            .Append("\">");

        xml.Append("<loadouts>");
        xml.Append("<l id=\"1\" cl=\"0\" xp=\"0\" tm=\"0\" sp1=\"0\" sp2=\"0\" sp3=\"0\" sv=\"\" lo=\"0 0 0 0 0 0 0 0 0 0 0 0 0\" />");
        xml.Append("</loadouts>");

        xml.Append("<achievements></achievements>");
        xml.Append("<fpsattach></fpsattach>");
        xml.Append("<inventory></inventory>");
        xml.Append("<nis></nis>");
        xml.Append("<sday></sday>");
        xml.Append("<sweek></sweek>");
        xml.Append("</account>");

        return xml.ToString();
    }

    private static string Get(IReadOnlyDictionary<string, string> values, string key)
    {
        return values.TryGetValue(key, out var value) ? value : "";
    }

    private static string Xml(string value)
    {
        return WebUtility.HtmlEncode(value);
    }
}