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

        app.MapPost("/api_ClanInvites.aspx", LegacyClanInvitesAsync);
        app.MapPost("/api/api_ClanInvites.aspx", LegacyClanInvitesAsync);
        app.MapGet("/api_ClanInvites.aspx", LegacyClanInvitesAsync);
        app.MapGet("/api/api_ClanInvites.aspx", LegacyClanInvitesAsync);

        app.MapPost("/api_ClanApply.aspx", LegacyClanApplyAsync);
        app.MapPost("/api/api_ClanApply.aspx", LegacyClanApplyAsync);
        app.MapGet("/api_ClanApply.aspx", LegacyClanApplyAsync);
        app.MapGet("/api/api_ClanApply.aspx", LegacyClanApplyAsync);

        app.MapPost("/api_ClanCreate.aspx", LegacyClanCreateAsync);
        app.MapPost("/api/api_ClanCreate.aspx", LegacyClanCreateAsync);
        app.MapGet("/api_ClanCreate.aspx", LegacyClanCreateAsync);
        app.MapGet("/api/api_ClanCreate.aspx", LegacyClanCreateAsync);

        app.MapPost("/v1/friends/list", async (
            FriendsListRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new FriendsListResponse(false, session.Code, session.Message, Array.Empty<SocialFriendDto>()));

            var result = await social.GetFriendsAsync(request.CustomerId);
            return Results.Json(result);
        });

        app.MapPost("/v1/friends/pending", async (
            FriendsListRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new FriendsPendingResponse(false, session.Code, session.Message, Array.Empty<SocialFriendPendingDto>(), Array.Empty<SocialFriendPendingDto>()));

            var result = await social.GetFriendPendingRequestsAsync(request.CustomerId);
            return Results.Json(result);
        });

        app.MapPost("/v1/friends/add-request", async (
            FriendActionRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message });

            await social.AddFriendRequestAsync(request.CustomerId, request.FriendCustomerId, request.FriendName);

            return Results.Json(new { ok = true, code = 0, message = "OK" });
        });

        app.MapPost("/v1/friends/add-answer", async (
            FriendActionRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message });

            await social.AnswerFriendRequestAsync(request.CustomerId, request.FriendCustomerId, request.Accept);

            return Results.Json(new { ok = true, code = 0, message = "OK" });
        });

        app.MapPost("/v1/friends/remove", async (
            FriendActionRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message });

            await social.RemoveFriendAsync(request.CustomerId, request.FriendCustomerId);

            return Results.Json(new { ok = true, code = 0, message = "OK" });
        });

        app.MapPost("/v1/friends/stats", async (
            FriendsListRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
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
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
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
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ClanResponse(false, session.Code, session.Message, null));

            var result = await social.CreateClanAsync(request.CustomerId, request.Name, request.Tag);
            return Results.Json(result);
        });

        app.MapPost("/v1/clan/members", async (
            ClanRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message, members = Array.Empty<object>() });

            var members = await social.GetClanMembersAsync(request.CustomerId);

            return Results.Json(new { ok = true, code = 0, message = "OK", members });
        });

        app.MapPost("/v1/clan/invites", async (
            ClanRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message, invites = Array.Empty<object>() });

            var invites = await social.GetClanInvitesAsync(request.CustomerId);

            return Results.Json(new { ok = true, code = 0, message = "OK", invites });
        });

        app.MapPost("/v1/clan/applications", async (
            ClanRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new { ok = false, code = session.Code, message = session.Message, applications = Array.Empty<object>() });

            var applications = await social.GetClanApplicationsAsync(request.CustomerId);

            return Results.Json(new { ok = true, code = 0, message = "OK", applications });
        });

        app.MapPost("/v1/clan/invite", async (
            ClanTargetRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ClanResponse(false, session.Code, session.Message, null));

            var result = await social.InviteToClanAsync(request.CustomerId, request.TargetCustomerId, request.TargetName);
            return Results.Json(result);
        });

        app.MapPost("/v1/clan/accept-invite", async (
            ClanInviteAnswerRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ClanResponse(false, session.Code, session.Message, null));

            var result = await social.AcceptClanInviteAsync(request.CustomerId, request.ClanId, request.Accept);
            return Results.Json(result);
        });

        app.MapPost("/v1/clan/apply", async (
            ClanApplicationRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ClanResponse(false, session.Code, session.Message, null));

            var result = await social.ApplyToClanAsync(request.CustomerId, request.ClanId, request.Text);
            return Results.Json(result);
        });

        app.MapPost("/v1/clan/accept-application", async (
            ClanApplicationAnswerRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ClanResponse(false, session.Code, session.Message, null));

            var result = await social.AnswerClanApplicationAsync(
                request.CustomerId,
                request.ApplicationId,
                request.TargetCustomerId,
                request.Accept);

            return Results.Json(result);
        });

        app.MapPost("/v1/clan/leave", async (
            ClanRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ClanResponse(false, session.Code, session.Message, null));

            var result = await social.LeaveClanAsync(request.CustomerId);
            return Results.Json(result);
        });

        app.MapPost("/v1/clan/kick", async (
            ClanTargetRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ClanResponse(false, session.Code, session.Message, null));

            var result = await social.KickClanMemberAsync(request.CustomerId, request.TargetCustomerId);
            return Results.Json(result);
        });

        app.MapPost("/v1/clan/promote", async (
            ClanTargetRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ClanResponse(false, session.Code, session.Message, null));

            var result = await social.PromoteClanMemberAsync(request.CustomerId, request.TargetCustomerId);
            return Results.Json(result);
        });

        app.MapPost("/v1/clan/demote", async (
            ClanTargetRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ClanResponse(false, session.Code, session.Message, null));

            var result = await social.DemoteClanMemberAsync(request.CustomerId, request.TargetCustomerId);
            return Results.Json(result);
        });

        app.MapPost("/v1/clan/announcement", async (
            ClanAnnouncementRequest request,
            AuthService auth,
            SocialService social) =>
        {
            var session = await auth.CheckSessionAsync(request.CustomerId, request.SessionId, request.Token);
            if (!session.Ok)
                return Results.Json(new ClanResponse(false, session.Code, session.Message, null));

            var result = await social.SetClanAnnouncementAsync(request.CustomerId, request.Announcement);
            return Results.Json(result);
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

        if (string.Equals(func, "pending", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(func, "requests", StringComparison.OrdinalIgnoreCase))
        {
            var pending = await social.GetFriendPendingRequestsAsync(customerId);
            var pendingXml = social.BuildLegacyFriendsPendingXml(pending);

            return Results.Text(pendingXml, "text/xml", Encoding.UTF8);
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

        var func = Read(data, "func").Trim();

        if (string.Equals(func, "members", StringComparison.OrdinalIgnoreCase))
        {
            var members = await social.GetClanMembersAsync(customerId);
            return Results.Text(social.BuildLegacyClanMembersXml(members), "text/xml", Encoding.UTF8);
        }

        if (string.Equals(func, "invite", StringComparison.OrdinalIgnoreCase))
        {
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "FriendID", "TargetID"));
            var targetName = ReadAny(data, "Gamertag", "TargetName", "name");

            var result = await social.InviteToClanAsync(customerId, targetId, targetName);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (string.Equals(func, "leave", StringComparison.OrdinalIgnoreCase))
        {
            var result = await social.LeaveClanAsync(customerId);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (string.Equals(func, "kick", StringComparison.OrdinalIgnoreCase))
        {
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "TargetID", "MemberID"));
            var result = await social.KickClanMemberAsync(customerId, targetId);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (string.Equals(func, "promote", StringComparison.OrdinalIgnoreCase))
        {
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "TargetID", "MemberID"));
            var result = await social.PromoteClanMemberAsync(customerId, targetId);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (string.Equals(func, "demote", StringComparison.OrdinalIgnoreCase))
        {
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "TargetID", "MemberID"));
            var result = await social.DemoteClanMemberAsync(customerId, targetId);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (string.Equals(func, "announcement", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(func, "setAnnouncement", StringComparison.OrdinalIgnoreCase))
        {
            var announcement = ReadAny(data, "Announcement", "announcement", "Text", "text", "msg");
            var result = await social.SetClanAnnouncementAsync(customerId, announcement);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

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

    private static async Task<IResult> LegacyClanInvitesAsync(
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

        if (string.Equals(func, "invite", StringComparison.OrdinalIgnoreCase))
        {
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "FriendID", "TargetID"));
            var targetName = ReadAny(data, "Gamertag", "TargetName", "name");

            var result = await social.InviteToClanAsync(customerId, targetId, targetName);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (string.Equals(func, "accept", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(func, "acceptInvite", StringComparison.OrdinalIgnoreCase))
        {
            var clanId = LegacyUtil.ParseULong(ReadAny(data, "ClanID", "clanId", "cid"));
            var result = await social.AcceptClanInviteAsync(customerId, clanId, true);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (string.Equals(func, "decline", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(func, "reject", StringComparison.OrdinalIgnoreCase))
        {
            var clanId = LegacyUtil.ParseULong(ReadAny(data, "ClanID", "clanId", "cid"));
            var result = await social.AcceptClanInviteAsync(customerId, clanId, false);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        var invites = await social.GetClanInvitesAsync(customerId);
        var xml = social.BuildLegacyClanInvitesXml(invites);

        return Results.Text(xml, "text/xml", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyClanApplyAsync(
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

        if (string.Equals(func, "apply", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(func, "send", StringComparison.OrdinalIgnoreCase))
        {
            var clanId = LegacyUtil.ParseULong(ReadAny(data, "ClanID", "clanId", "cid"));
            var text = ReadAny(data, "Text", "text", "ApplicationText", "msg");

            var result = await social.ApplyToClanAsync(customerId, clanId, text);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (string.Equals(func, "accept", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(func, "acceptApplication", StringComparison.OrdinalIgnoreCase))
        {
            var appId = LegacyUtil.ParseULong(ReadAny(data, "ApplicationID", "ClanApplicationID", "appId"));
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "TargetID"));
            var result = await social.AnswerClanApplicationAsync(customerId, appId, targetId, true);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (string.Equals(func, "decline", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(func, "reject", StringComparison.OrdinalIgnoreCase))
        {
            var appId = LegacyUtil.ParseULong(ReadAny(data, "ApplicationID", "ClanApplicationID", "appId"));
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "TargetID"));
            var result = await social.AnswerClanApplicationAsync(customerId, appId, targetId, false);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        var applications = await social.GetClanApplicationsAsync(customerId);
        var xml = social.BuildLegacyClanApplicationsXml(applications);

        return Results.Text(xml, "text/xml", Encoding.UTF8);
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

    private static string ReadAny(Dictionary<string, string> data, params string[] keys)
    {
        foreach (var key in keys)
        {
            if (data.TryGetValue(key, out var value))
                return value;
        }

        return "";
    }
}