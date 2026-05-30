using System.Text;
using WarInc.Api.Auth;
using WarInc.Api.Common;

namespace WarInc.Api.Social;

public static class SocialEndpoints
{
    public static void MapSocialEndpoints(this WebApplication app)
    {
        app.MapPost("/api_Friends.aspx", LegacyFriendsAsync);
        app.MapPost("/api/api_Friends.aspx", LegacyFriendsAsync);

        app.MapGet("/api_Friends.aspx", LegacyFriendsAsync);
        app.MapGet("/api/api_Friends.aspx", LegacyFriendsAsync);

        app.MapPost("/api_ClanMgr.aspx", LegacyClanMgrAsync);
        app.MapPost("/api/api_ClanMgr.aspx", LegacyClanMgrAsync);

        app.MapGet("/api_ClanMgr.aspx", LegacyClanMgrAsync);
        app.MapGet("/api/api_ClanMgr.aspx", LegacyClanMgrAsync);

        app.MapPost("/api_ClanGetInfo.aspx", LegacyClanGetInfoAsync);
        app.MapPost("/api/api_ClanGetInfo.aspx", LegacyClanGetInfoAsync);

        app.MapGet("/api_ClanGetInfo.aspx", LegacyClanGetInfoAsync);
        app.MapGet("/api/api_ClanGetInfo.aspx", LegacyClanGetInfoAsync);

        app.MapPost("/api_ClanInvites.aspx", LegacyClanOkAsync);
        app.MapPost("/api/api_ClanInvites.aspx", LegacyClanOkAsync);

        app.MapGet("/api_ClanInvites.aspx", LegacyClanOkAsync);
        app.MapGet("/api/api_ClanInvites.aspx", LegacyClanOkAsync);

        app.MapPost("/api_ClanApply.aspx", LegacyClanOkAsync);
        app.MapPost("/api/api_ClanApply.aspx", LegacyClanOkAsync);

        app.MapGet("/api_ClanApply.aspx", LegacyClanOkAsync);
        app.MapGet("/api/api_ClanApply.aspx", LegacyClanOkAsync);

        app.MapPost("/api_ClanCreate.aspx", LegacyClanCreateAsync);
        app.MapPost("/api/api_ClanCreate.aspx", LegacyClanCreateAsync);

        app.MapGet("/api_ClanCreate.aspx", LegacyClanCreateAsync);
        app.MapGet("/api/api_ClanCreate.aspx", LegacyClanCreateAsync);

        app.MapPost("/v1/friends/list", async (
            FriendsListRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
                return Results.Json(new FriendsListResponse(false, session.Code, session.Message, Array.Empty<SocialFriendDto>()));

            var result = await social.GetFriendsAsync(request.CustomerId);

            return Results.Json(result);
        });

        app.MapPost("/v1/friends/add-request", async (
            FriendActionRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message });

            await social.AddFriendRequestAsync(
                request.CustomerId,
                request.FriendCustomerId,
                request.FriendName);

            return Results.Json(new { ok = true, code = 0, message = "OK" });
        });

        app.MapPost("/v1/friends/add-answer", async (
            FriendActionRequest request,
            AuthService auth,
            SocialService social,
            HttpContext http) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message });

            var accept = ReadBool(http, "accept", true);

            await social.AnswerFriendRequestAsync(
                request.CustomerId,
                request.FriendCustomerId,
                accept);

            return Results.Json(new { ok = true, code = 0, message = "OK" });
        });

        app.MapPost("/v1/friends/remove", async (
            FriendActionRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message });

            await social.RemoveFriendAsync(
                request.CustomerId,
                request.FriendCustomerId);

            return Results.Json(new { ok = true, code = 0, message = "OK" });
        });

        app.MapPost("/v1/friends/stats", async (
            FriendsListRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message });

            var result = await social.GetFriendsAsync(request.CustomerId);

            return Results.Json(new
            {
                ok = true,
                code = 0,
                message = "OK",
                count = result.Friends.Count,
                online = result.Friends.Count(x => x.Online != 0)
            });
        });

        app.MapPost("/v1/clan/get", async (
            ClanRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
                return Results.Json(new ClanResponse(false, session.Code, session.Message, null));

            var result = await social.GetClanAsync(request.CustomerId);

            return Results.Json(result);
        });

        app.MapPost("/v1/clan/create", async (
            ClanCreateRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
                return Results.Json(new ClanResponse(false, session.Code, session.Message, null));

            var result = await social.CreateClanAsync(
                request.CustomerId,
                request.Name,
                request.Tag);

            return Results.Json(result);
        });

        app.MapPost("/v1/clan/members", async (
            ClanRequest request,
            AuthService auth) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message, members = Array.Empty<object>() });

            return Results.Json(new { ok = true, code = 0, message = "OK", members = Array.Empty<object>() });
        });

        app.MapPost("/v1/clan/invites", async (
            ClanRequest request,
            AuthService auth) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message, invites = Array.Empty<object>() });

            return Results.Json(new { ok = true, code = 0, message = "OK", invites = Array.Empty<object>() });
        });

        app.MapPost("/v1/clan/apply", async (
            ClanRequest request,
            AuthService auth) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message });

            return Results.Json(new { ok = true, code = 0, message = "OK" });
        });
    }

    private static async Task<IResult> LegacyFriendsAsync(
        HttpContext http,
        AuthService auth,
        SocialService social)
    {
        var data = await ReadLegacyDataAsync(http);

        var customerId = LegacyUtil.ParseULong(Read(data, "s_id"));
        var sessionId = LegacyUtil.ParseULong(Read(data, "s_key"));

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);

        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var func = Read(data, "func").Trim();

        if (string.Equals(func, "addReq", StringComparison.OrdinalIgnoreCase))
        {
            var friendId = LegacyUtil.ParseULong(Read(data, "FriendID"));
            var friendName = Read(data, "Gamertag");

            await social.AddFriendRequestAsync(customerId, friendId, friendName);

            return Results.Text("WO_0", "text/plain", Encoding.UTF8);
        }

        if (string.Equals(func, "addAns", StringComparison.OrdinalIgnoreCase))
        {
            var friendId = LegacyUtil.ParseULong(Read(data, "FriendID"));
            var accept = Read(data, "Answer") != "0";

            await social.AnswerFriendRequestAsync(customerId, friendId, accept);

            return Results.Text("WO_0", "text/plain", Encoding.UTF8);
        }

        if (string.Equals(func, "remove", StringComparison.OrdinalIgnoreCase))
        {
            var friendId = LegacyUtil.ParseULong(Read(data, "FriendID"));

            await social.RemoveFriendAsync(customerId, friendId);

            return Results.Text("WO_0", "text/plain", Encoding.UTF8);
        }

        var friends = await social.GetFriendsAsync(customerId);

        if (string.Equals(func, "stats", StringComparison.OrdinalIgnoreCase))
        {
            var statsXml = social.BuildLegacyFriendsStatsXml(friends.Friends);

            return Results.Text(statsXml, "text/xml", Encoding.UTF8);
        }

        var xml = social.BuildLegacyFriendsXml(friends.Friends);

        return Results.Text(xml, "text/xml", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyClanMgrAsync(
        HttpContext http,
        AuthService auth,
        SocialService social)
    {
        var data = await ReadLegacyDataAsync(http);

        var customerId = LegacyUtil.ParseULong(Read(data, "s_id"));
        var sessionId = LegacyUtil.ParseULong(Read(data, "s_key"));

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);

        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var clan = await social.GetClanAsync(customerId);
        var xml = social.BuildLegacyClanXml(clan.Clan);

        return Results.Text(xml, "text/xml", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyClanGetInfoAsync(
        HttpContext http,
        AuthService auth,
        SocialService social)
    {
        var data = await ReadLegacyDataAsync(http);

        var customerId = LegacyUtil.ParseULong(Read(data, "s_id"));
        var sessionId = LegacyUtil.ParseULong(Read(data, "s_key"));

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);

        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var clan = await social.GetClanAsync(customerId);
        var xml = social.BuildLegacyClanXml(clan.Clan);

        return Results.Text(xml, "text/xml", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyClanCreateAsync(
        HttpContext http,
        AuthService auth,
        SocialService social)
    {
        var data = await ReadLegacyDataAsync(http);

        var customerId = LegacyUtil.ParseULong(Read(data, "s_id"));
        var sessionId = LegacyUtil.ParseULong(Read(data, "s_key"));

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);

        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var name = Read(data, "name");

        if (string.IsNullOrWhiteSpace(name))
            name = Read(data, "ClanName");

        var tag = Read(data, "tag");

        if (string.IsNullOrWhiteSpace(tag))
            tag = Read(data, "ClanTag");

        var result = await social.CreateClanAsync(customerId, name, tag);

        if (!result.Ok)
            return Results.Text("WO_5", "text/plain", Encoding.UTF8);

        return Results.Text("WO_0", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyClanOkAsync(
        HttpContext http,
        AuthService auth)
    {
        var data = await ReadLegacyDataAsync(http);

        var customerId = LegacyUtil.ParseULong(Read(data, "s_id"));
        var sessionId = LegacyUtil.ParseULong(Read(data, "s_key"));

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);

        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        return Results.Text("WO_0", "text/plain", Encoding.UTF8);
    }

    private static async Task<Dictionary<string, string>> ReadLegacyDataAsync(HttpContext http)
    {
        var data = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);

        foreach (var item in http.Request.Query)
            data[item.Key] = item.Value.ToString();

        if (http.Request.HasFormContentType)
        {
            var form = await http.Request.ReadFormAsync();

            foreach (var item in form)
                data[item.Key] = item.Value.ToString();
        }

        return data;
    }

    private static string Read(Dictionary<string, string> data, string key)
    {
        return data.TryGetValue(key, out var value) ? value : "";
    }

    private static bool ReadBool(HttpContext http, string key, bool defaultValue)
    {
        if (!http.Request.Query.TryGetValue(key, out var value))
            return defaultValue;

        var text = value.ToString();

        if (string.IsNullOrWhiteSpace(text))
            return defaultValue;

        if (text == "1")
            return true;

        if (text == "0")
            return false;

        return bool.TryParse(text, out var result) ? result : defaultValue;
    }
}