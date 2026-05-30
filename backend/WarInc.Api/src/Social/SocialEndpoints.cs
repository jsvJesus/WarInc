using System.Text;
using System.Text.Json;
using WarInc.Api.Auth;
using WarInc.Api.Common;

namespace WarInc.Api.Social;

public static class SocialEndpoints
{
    public static void MapSocialEndpoints(this WebApplication app)
    {
        MapLegacy(app, "/api_Friends.aspx", LegacyFriendsAsync);
        MapLegacy(app, "/api_FriendsList.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyFriendsAsync(http, auth, social, "list"));
        MapLegacy(app, "/api_FriendsPending.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyFriendsAsync(http, auth, social, "pending"));
        MapLegacy(app, "/api_FriendsAccept.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyFriendsAsync(http, auth, social, "accept"));
        MapLegacy(app, "/api_FriendsDecline.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyFriendsAsync(http, auth, social, "decline"));

        MapLegacy(app, "/api_ClanMgr.aspx", LegacyClanMgrAsync);
        MapLegacy(app, "/api_ClanGetInfo.aspx", LegacyClanGetInfoAsync);
        MapLegacy(app, "/api_ClanCreate.aspx", LegacyClanCreateAsync);
        MapLegacy(app, "/api_ClanMembers.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyClanMgrAsync(http, auth, social, "members"));
        MapLegacy(app, "/api_ClanInvites.aspx", LegacyClanInvitesAsync);
        MapLegacy(app, "/api_ClanInvite.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyClanInvitesAsync(http, auth, social, "invite"));
        MapLegacy(app, "/api_ClanAcceptInvite.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyClanInvitesAsync(http, auth, social, "accept"));
        MapLegacy(app, "/api_ClanDeclineInvite.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyClanInvitesAsync(http, auth, social, "decline"));
        MapLegacy(app, "/api_ClanApply.aspx", LegacyClanApplyAsync);
        MapLegacy(app, "/api_ClanAcceptApplication.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyClanApplyAsync(http, auth, social, "accept"));
        MapLegacy(app, "/api_ClanDeclineApplication.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyClanApplyAsync(http, auth, social, "decline"));
        MapLegacy(app, "/api_ClanLeave.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyClanMgrAsync(http, auth, social, "leave"));
        MapLegacy(app, "/api_ClanKick.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyClanMgrAsync(http, auth, social, "kick"));
        MapLegacy(app, "/api_ClanPromote.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyClanMgrAsync(http, auth, social, "promote"));
        MapLegacy(app, "/api_ClanDemote.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyClanMgrAsync(http, auth, social, "demote"));
        MapLegacy(app, "/api_ClanAnnouncement.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyClanMgrAsync(http, auth, social, "announcement"));
        MapLegacy(app, "/api_ClanSetAnnouncement.aspx", (HttpContext http, AuthService auth, SocialService social) => LegacyClanMgrAsync(http, auth, social, "announcement"));

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
                online = result.Friends.Count(x => x.Online != 0),
                friends = result.Friends
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
        SocialService social,
        string? forcedFunc = null)
    {
        var data = await ReadLegacyDataAsync(http);

        var customerId = LegacyUtil.ParseULong(Read(data, "s_id"));
        var sessionId = LegacyUtil.ParseULong(Read(data, "s_key"));

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);

        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var func = GetFunc(data, forcedFunc);

        if (IsFunc(func, "addReq", "add", "request", "send", "sendRequest"))
        {
            var friendId = LegacyUtil.ParseULong(ReadAny(data, "FriendID", "CustomerID", "TargetID"));
            var friendName = ReadAny(data, "Gamertag", "FriendName", "TargetName", "name");

            await social.AddFriendRequestAsync(customerId, friendId, friendName);

            return Results.Text("WO_0", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "addAns", "answer", "accept", "decline", "reject"))
        {
            var friendId = LegacyUtil.ParseULong(ReadAny(data, "FriendID", "CustomerID", "TargetID"));

            var accept = IsFunc(func, "decline", "reject")
                ? false
                : ReadLegacyBool(data, true, "Answer", "Accept", "Result");

            await social.AnswerFriendRequestAsync(customerId, friendId, accept);

            return Results.Text("WO_0", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "remove", "delete", "del", "unfriend"))
        {
            var friendId = LegacyUtil.ParseULong(ReadAny(data, "FriendID", "CustomerID", "TargetID"));

            await social.RemoveFriendAsync(customerId, friendId);

            return Results.Text("WO_0", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "pending", "requests", "getPending", "getRequests", "listPending", "pendingRequests"))
        {
            var pending = await social.GetFriendPendingRequestsAsync(customerId);
            var pendingXml = social.BuildLegacyFriendsPendingXml(pending);

            return Results.Text(pendingXml, "text/xml", Encoding.UTF8);
        }

        var friends = await social.GetFriendsAsync(customerId);

        if (IsFunc(func, "stats", "status", "online", "onlineStatus"))
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
        SocialService social,
        string? forcedFunc = null)
    {
        var data = await ReadLegacyDataAsync(http);

        var customerId = LegacyUtil.ParseULong(Read(data, "s_id"));
        var sessionId = LegacyUtil.ParseULong(Read(data, "s_key"));

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);

        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var func = GetFunc(data, forcedFunc);

        if (IsFunc(func, "create"))
        {
            var name = ReadAny(data, "name", "ClanName", "clanName");
            var tag = ReadAny(data, "tag", "ClanTag", "clanTag");

            var result = await social.CreateClanAsync(customerId, name, tag);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "members", "memberList", "getMembers", "listMembers"))
        {
            var members = await social.GetClanMembersAsync(customerId);
            return Results.Text(social.BuildLegacyClanMembersXml(members), "text/xml", Encoding.UTF8);
        }

        if (IsFunc(func, "invite", "sendInvite"))
        {
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "FriendID", "TargetID", "MemberID"));
            var targetName = ReadAny(data, "Gamertag", "TargetName", "name");

            var result = await social.InviteToClanAsync(customerId, targetId, targetName);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "acceptInvite", "inviteAccept"))
        {
            var clanOrInviteId = LegacyUtil.ParseULong(ReadAny(data, "ClanID", "clanId", "cid", "ClanInviteID", "InviteID", "id"));
            var result = await social.AcceptClanInviteAsync(customerId, clanOrInviteId, true);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "declineInvite", "rejectInvite"))
        {
            var clanOrInviteId = LegacyUtil.ParseULong(ReadAny(data, "ClanID", "clanId", "cid", "ClanInviteID", "InviteID", "id"));
            var result = await social.AcceptClanInviteAsync(customerId, clanOrInviteId, false);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "apply", "sendApplication"))
        {
            var clanId = LegacyUtil.ParseULong(ReadAny(data, "ClanID", "clanId", "cid"));
            var text = ReadAny(data, "Text", "text", "ApplicationText", "msg");

            var result = await social.ApplyToClanAsync(customerId, clanId, text);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "acceptApplication", "applicationAccept"))
        {
            var appId = LegacyUtil.ParseULong(ReadAny(data, "ApplicationID", "ClanApplicationID", "appId", "id"));
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "TargetID", "MemberID"));

            var result = await social.AnswerClanApplicationAsync(customerId, appId, targetId, true);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "declineApplication", "rejectApplication"))
        {
            var appId = LegacyUtil.ParseULong(ReadAny(data, "ApplicationID", "ClanApplicationID", "appId", "id"));
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "TargetID", "MemberID"));

            var result = await social.AnswerClanApplicationAsync(customerId, appId, targetId, false);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "leave", "quit"))
        {
            var result = await social.LeaveClanAsync(customerId);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "kick", "removeMember"))
        {
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "TargetID", "MemberID"));
            var result = await social.KickClanMemberAsync(customerId, targetId);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "promote"))
        {
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "TargetID", "MemberID"));
            var result = await social.PromoteClanMemberAsync(customerId, targetId);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "demote"))
        {
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "TargetID", "MemberID"));
            var result = await social.DemoteClanMemberAsync(customerId, targetId);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "announcement", "setAnnouncement", "setAnnounce", "message"))
        {
            var announcement = ReadAny(data, "Announcement", "announcement", "Text", "text", "msg");
            var result = await social.SetClanAnnouncementAsync(customerId, announcement);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "invites", "inviteList", "getInvites"))
        {
            var invites = await social.GetClanInvitesAsync(customerId);
            return Results.Text(social.BuildLegacyClanInvitesXml(invites), "text/xml", Encoding.UTF8);
        }

        if (IsFunc(func, "applications", "applicationList", "getApplications"))
        {
            var applications = await social.GetClanApplicationsAsync(customerId);
            return Results.Text(social.BuildLegacyClanApplicationsXml(applications), "text/xml", Encoding.UTF8);
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

        var name = ReadAny(data, "name", "ClanName", "clanName");
        var tag = ReadAny(data, "tag", "ClanTag", "clanTag");

        var result = await social.CreateClanAsync(customerId, name, tag);

        return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyClanInvitesAsync(
        HttpContext http,
        AuthService auth,
        SocialService social,
        string? forcedFunc = null)
    {
        var data = await ReadLegacyDataAsync(http);

        var customerId = LegacyUtil.ParseULong(Read(data, "s_id"));
        var sessionId = LegacyUtil.ParseULong(Read(data, "s_key"));

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);

        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var func = GetFunc(data, forcedFunc);

        if (IsFunc(func, "invite", "send", "sendInvite"))
        {
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "FriendID", "TargetID", "MemberID"));
            var targetName = ReadAny(data, "Gamertag", "TargetName", "name");

            var result = await social.InviteToClanAsync(customerId, targetId, targetName);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "accept", "acceptInvite", "inviteAccept"))
        {
            var clanOrInviteId = LegacyUtil.ParseULong(ReadAny(data, "ClanID", "clanId", "cid", "ClanInviteID", "InviteID", "id"));
            var result = await social.AcceptClanInviteAsync(customerId, clanOrInviteId, true);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "decline", "reject", "declineInvite", "rejectInvite"))
        {
            var clanOrInviteId = LegacyUtil.ParseULong(ReadAny(data, "ClanID", "clanId", "cid", "ClanInviteID", "InviteID", "id"));
            var result = await social.AcceptClanInviteAsync(customerId, clanOrInviteId, false);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        var invites = await social.GetClanInvitesAsync(customerId);
        var xml = social.BuildLegacyClanInvitesXml(invites);

        return Results.Text(xml, "text/xml", Encoding.UTF8);
    }

    private static async Task<IResult> LegacyClanApplyAsync(
        HttpContext http,
        AuthService auth,
        SocialService social,
        string? forcedFunc = null)
    {
        var data = await ReadLegacyDataAsync(http);

        var customerId = LegacyUtil.ParseULong(Read(data, "s_id"));
        var sessionId = LegacyUtil.ParseULong(Read(data, "s_key"));

        var session = await auth.CheckLegacySessionAsync(customerId, sessionId);

        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var func = GetFunc(data, forcedFunc);

        if (IsFunc(func, "apply", "send", "sendApplication"))
        {
            var clanId = LegacyUtil.ParseULong(ReadAny(data, "ClanID", "clanId", "cid"));
            var text = ReadAny(data, "Text", "text", "ApplicationText", "msg");

            var result = await social.ApplyToClanAsync(customerId, clanId, text);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "accept", "acceptApplication", "applicationAccept"))
        {
            var appId = LegacyUtil.ParseULong(ReadAny(data, "ApplicationID", "ClanApplicationID", "appId", "id"));
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "TargetID", "MemberID"));

            var result = await social.AnswerClanApplicationAsync(customerId, appId, targetId, true);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        if (IsFunc(func, "decline", "reject", "declineApplication", "rejectApplication"))
        {
            var appId = LegacyUtil.ParseULong(ReadAny(data, "ApplicationID", "ClanApplicationID", "appId", "id"));
            var targetId = LegacyUtil.ParseULong(ReadAny(data, "CustomerID", "TargetID", "MemberID"));

            var result = await social.AnswerClanApplicationAsync(customerId, appId, targetId, false);
            return Results.Text(result.Ok ? "WO_0" : "WO_5", "text/plain", Encoding.UTF8);
        }

        var applications = await social.GetClanApplicationsAsync(customerId);
        var xml = social.BuildLegacyClanApplicationsXml(applications);

        return Results.Text(xml, "text/xml", Encoding.UTF8);
    }

    private static void MapLegacy(WebApplication app, string path, Delegate handler)
    {
        app.MapGet(path, handler);
        app.MapPost(path, handler);

        if (!path.StartsWith("/api/", StringComparison.OrdinalIgnoreCase))
        {
            app.MapGet("/api" + path, handler);
            app.MapPost("/api" + path, handler);
        }
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

            return data;
        }

        if (http.Request.ContentLength.GetValueOrDefault() <= 0)
            return data;

        var contentType = http.Request.ContentType ?? "";

        if (!contentType.Contains("application/json", StringComparison.OrdinalIgnoreCase))
            return data;

        try
        {
            using var reader = new StreamReader(http.Request.Body, Encoding.UTF8);
            var body = await reader.ReadToEndAsync();

            if (string.IsNullOrWhiteSpace(body))
                return data;

            using var doc = JsonDocument.Parse(body);

            if (doc.RootElement.ValueKind != JsonValueKind.Object)
                return data;

            foreach (var prop in doc.RootElement.EnumerateObject())
            {
                data[prop.Name] = prop.Value.ValueKind switch
                {
                    JsonValueKind.String => prop.Value.GetString() ?? "",
                    JsonValueKind.Number => prop.Value.GetRawText(),
                    JsonValueKind.True => "1",
                    JsonValueKind.False => "0",
                    _ => prop.Value.GetRawText()
                };
            }
        }
        catch
        {
            return data;
        }

        return data;
    }

    private static string GetFunc(Dictionary<string, string> data, string? forcedFunc)
    {
        if (!string.IsNullOrWhiteSpace(forcedFunc))
            return forcedFunc.Trim();

        return ReadAny(data, "func", "action", "cmd", "op").Trim();
    }

    private static bool ReadLegacyBool(Dictionary<string, string> data, bool defaultValue, params string[] keys)
    {
        var value = ReadAny(data, keys);

        if (string.IsNullOrWhiteSpace(value))
            return defaultValue;

        value = value.Trim();

        if (string.Equals(value, "0", StringComparison.OrdinalIgnoreCase))
            return false;

        if (string.Equals(value, "false", StringComparison.OrdinalIgnoreCase))
            return false;

        if (string.Equals(value, "no", StringComparison.OrdinalIgnoreCase))
            return false;

        if (string.Equals(value, "decline", StringComparison.OrdinalIgnoreCase))
            return false;

        if (string.Equals(value, "reject", StringComparison.OrdinalIgnoreCase))
            return false;

        return true;
    }

    private static bool IsFunc(string func, params string[] values)
    {
        foreach (var value in values)
        {
            if (string.Equals(func, value, StringComparison.OrdinalIgnoreCase))
                return true;
        }

        return false;
    }

    private static string Read(Dictionary<string, string> data, string key)
    {
        return data.TryGetValue(key, out var value) ? value : "";
    }

    private static string ReadAny(Dictionary<string, string> data, params string[] keys)
    {
        foreach (var key in keys)
        {
            if (data.TryGetValue(key, out var value) && !string.IsNullOrWhiteSpace(value))
                return value;
        }

        return "";
    }
}