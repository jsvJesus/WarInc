using System.Text;
using WarInc.Api.Auth;
using WarInc.Api.Common;

namespace WarInc.Api.Profile;

public static class ProfileEndpoints
{
    public static void MapProfileEndpoints(this WebApplication app)
    {
        app.MapPost("/api_GetProfile4.aspx", LegacyGetProfile4Async);
        app.MapPost("/api/api_GetProfile4.aspx", LegacyGetProfile4Async);

        app.MapPost("/v1/profile/get", async (
            ProfileRequest request,
            AuthService auth,
            ProfileService profileService) =>
        {
            var session = await auth.CheckSessionAsync(
                request.CustomerId,
                request.SessionId,
                request.Token);

            if (!session.Ok)
                return Results.Json(new ApiResponse(false, session.Code, session.Message));

            var profile = await profileService.GetProfileDataAsync(
                request.CustomerId,
                request.JoiningGame);

            if (!profile.Ok)
                return Results.Json(new ApiResponse(false, profile.Code, profile.Message));

            return Results.Json(profile);
        });
    }

    private static async Task<IResult> LegacyGetProfile4Async(
        HttpContext http,
        AuthService auth,
        ProfileService profileService)
    {
        var form = await http.Request.ReadFormAsync();

        var customerId = LegacyUtil.ParseULong(form["s_id"].ToString());
        var sessionId = LegacyUtil.ParseULong(form["s_key"].ToString());

        var joiningGame = false;
        var jg = form["jg"].ToString();

        if (!string.IsNullOrWhiteSpace(jg))
            joiningGame = jg != "0";

        var session = await auth.CheckLegacySessionAsync(
            customerId,
            sessionId);

        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var profile = await profileService.GetProfileDataAsync(
            customerId,
            joiningGame);

        if (!profile.Ok || profile.Account == null)
            return Results.Text("WO_6", "text/plain", Encoding.UTF8);

        var xml = profileService.BuildLegacyProfileXml(profile);

        return Results.Text(xml, "text/xml", Encoding.UTF8);
    }
}