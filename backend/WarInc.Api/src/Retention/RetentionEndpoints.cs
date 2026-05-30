using System.Text;
using System.Text.Json;
using WarInc.Api.Auth;
using WarInc.Api.Common;

namespace WarInc.Api.Retention;

public static class RetentionEndpoints
{
    public static void MapRetentionEndpoints(this WebApplication app)
    {
        app.MapGet("/api_RetBonus.aspx", LegacyRetBonusAsync);
        app.MapGet("/api/api_RetBonus.aspx", LegacyRetBonusAsync);

        app.MapPost("/api_RetBonus.aspx", LegacyRetBonusAsync);
        app.MapPost("/api/api_RetBonus.aspx", LegacyRetBonusAsync);

        app.MapGet("/v1/retention/info", JsonInfoAsync);
        app.MapPost("/v1/retention/info", JsonInfoAsync);

        app.MapGet("/v1/retention/claim", JsonClaimAsync);
        app.MapPost("/v1/retention/claim", JsonClaimAsync);
    }

    private static async Task<IResult> LegacyRetBonusAsync(
        HttpContext http,
        AuthService auth,
        RetentionService retention)
    {
        var data = await ReadRequestDataAsync(http);

        var customerId = LegacyUtil.ParseULong(ReadAny(
            data,
            "s_id",
            "CustomerID",
            "CustomerId",
            "customerId"));

        var sessionId = LegacyUtil.ParseULong(ReadAny(
            data,
            "s_key",
            "SessionID",
            "SessionId",
            "sessionId"));

        var session = await auth.CheckLegacySessionAsync(
            customerId,
            sessionId);

        if (!session.Ok)
            return Results.Text("WO_1", "text/plain", Encoding.UTF8);

        var func = ReadAny(data, "func").Trim();

        if (string.Equals(func, "give", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(func, "claim", StringComparison.OrdinalIgnoreCase))
        {
            var result = await retention.ClaimAsync(customerId);
            var text = retention.BuildLegacyClaimText(result);

            return Results.Text(text, "text/plain", Encoding.UTF8);
        }

        if (!string.Equals(func, "info", StringComparison.OrdinalIgnoreCase) &&
            !string.IsNullOrWhiteSpace(func))
        {
            return Results.Text("WO_5", "text/plain", Encoding.UTF8);
        }

        var info = await retention.GetInfoAsync(customerId);
        var xml = retention.BuildLegacyInfoXml(info);

        return Results.Text(xml, "text/xml", Encoding.UTF8);
    }

    private static async Task<IResult> JsonInfoAsync(
        HttpContext http,
        AuthService auth,
        RetentionService retention)
    {
        var request = await ReadJsonRequestAsync(http);

        var session = await auth.CheckSessionAsync(
            request.CustomerId,
            request.SessionId,
            request.Token);

        if (!session.Ok)
        {
            return Results.Json(new RetentionInfoResponse(
                false,
                session.Code,
                session.Message,
                0,
                0,
                0,
                Array.Empty<RetentionDayDto>()));
        }

        var info = await retention.GetInfoAsync(request.CustomerId);

        return Results.Json(info);
    }

    private static async Task<IResult> JsonClaimAsync(
        HttpContext http,
        AuthService auth,
        RetentionService retention)
    {
        var request = await ReadJsonRequestAsync(http);

        var session = await auth.CheckSessionAsync(
            request.CustomerId,
            request.SessionId,
            request.Token);

        if (!session.Ok)
        {
            return Results.Json(new RetentionClaimResponse(
                false,
                session.Code,
                session.Message,
                0,
                0,
                0,
                Array.Empty<RetentionDayDto>()));
        }

        var result = await retention.ClaimAsync(request.CustomerId);

        return Results.Json(result);
    }

    private static async Task<RetentionRequest> ReadJsonRequestAsync(HttpContext http)
    {
        var data = await ReadRequestDataAsync(http);

        var customerId = LegacyUtil.ParseULong(ReadAny(
            data,
            "customerId",
            "CustomerId",
            "CustomerID",
            "s_id"));

        var sessionId = LegacyUtil.ParseULong(ReadAny(
            data,
            "sessionId",
            "SessionId",
            "SessionID",
            "s_key"));

        var token = ReadAny(
            data,
            "token",
            "Token",
            "s_token");

        return new RetentionRequest(
            customerId,
            sessionId,
            token);
    }

    private static async Task<Dictionary<string, string>> ReadRequestDataAsync(HttpContext http)
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
        else if (http.Request.ContentLength > 0)
        {
            try
            {
                var body = await JsonSerializer.DeserializeAsync<Dictionary<string, JsonElement>>(
                    http.Request.Body,
                    new JsonSerializerOptions
                    {
                        PropertyNameCaseInsensitive = true
                    });

                if (body != null)
                {
                    foreach (var item in body)
                    {
                        if (item.Value.ValueKind == JsonValueKind.String)
                        {
                            data[item.Key] = item.Value.GetString() ?? "";
                        }
                        else if (item.Value.ValueKind == JsonValueKind.Number)
                        {
                            data[item.Key] = item.Value.GetRawText();
                        }
                        else if (item.Value.ValueKind == JsonValueKind.True)
                        {
                            data[item.Key] = "1";
                        }
                        else if (item.Value.ValueKind == JsonValueKind.False)
                        {
                            data[item.Key] = "0";
                        }
                    }
                }
            }
            catch
            {
            }
        }

        return data;
    }

    private static string ReadAny(
        Dictionary<string, string> data,
        params string[] keys)
    {
        foreach (var key in keys)
        {
            if (data.TryGetValue(key, out var value))
                return value;
        }

        return "";
    }
}