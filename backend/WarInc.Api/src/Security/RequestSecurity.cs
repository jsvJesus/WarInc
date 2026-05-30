using System.Net;
using System.Security.Cryptography;
using System.Text;
using WarInc.Api.Config;
using WarInc.Api.Common;

namespace WarInc.Api.Security;

public static class RequestSecurity
{
    public static string GetClientIp(HttpContext http, WarIncOptions options)
    {
        if (options.TrustProxyHeaders)
        {
            var forwarded = http.Request.Headers["X-Forwarded-For"].ToString();

            if (!string.IsNullOrWhiteSpace(forwarded))
                return SecurityUtil.NormalizeLegacyIp(forwarded.Split(',')[0].Trim());

            var realIp = http.Request.Headers["X-Real-IP"].ToString();

            if (!string.IsNullOrWhiteSpace(realIp))
                return SecurityUtil.NormalizeLegacyIp(realIp);
        }

        return SecurityUtil.NormalizeLegacyIp(http.Connection.RemoteIpAddress?.ToString() ?? "0.0.0.0");
    }

    public static string RateLimitKey(HttpContext http, WarIncOptions options)
    {
        var ip = GetClientIp(http, options);
        var path = http.Request.Path.ToString().ToLowerInvariant();

        return ip + ":" + path;
    }

    public static bool HasInternalKey(
        HttpContext http,
        WarIncOptions options,
        IReadOnlyDictionary<string, string>? data = null)
    {
        var expected = options.InternalApiKey;

        if (string.IsNullOrWhiteSpace(expected))
            return false;

        var actual = http.Request.Headers["X-WarInc-Internal-Key"].ToString();

        if (string.IsNullOrWhiteSpace(actual) && data != null)
        {
            actual = ReadAny(
                data,
                "skey1",
                "SKey1",
                "serverKey",
                "ServerKey",
                "key",
                "Key",
                "internalKey",
                "InternalKey");
        }

        if (string.IsNullOrWhiteSpace(actual))
            return false;

        return FixedEquals(actual, expected);
    }

    public static bool IsGameServerAllowed(
        HttpContext http,
        WarIncOptions options,
        IReadOnlyDictionary<string, string>? data,
        out string reason)
    {
        if (!HasInternalKey(http, options, data))
        {
            reason = "BAD_INTERNAL_KEY";
            return false;
        }

        if (options.GameServerAllowedIps.Length == 0)
        {
            reason = "";
            return true;
        }

        var ip = GetClientIp(http, options);

        foreach (var rule in options.GameServerAllowedIps)
        {
            if (IpMatches(ip, rule))
            {
                reason = "";
                return true;
            }
        }

        reason = "BAD_SERVER_IP";
        return false;
    }

    public static string ReadAny(
        IReadOnlyDictionary<string, string> data,
        params string[] keys)
    {
        foreach (var key in keys)
        {
            if (data.TryGetValue(key, out var value))
                return value;
        }

        return "";
    }

    private static bool FixedEquals(string actual, string expected)
    {
        var a = Encoding.UTF8.GetBytes(actual);
        var b = Encoding.UTF8.GetBytes(expected);

        return a.Length == b.Length && CryptographicOperations.FixedTimeEquals(a, b);
    }

    private static bool IpMatches(string ipRaw, string ruleRaw)
    {
        var rule = (ruleRaw ?? "").Trim();

        if (string.IsNullOrWhiteSpace(rule))
            return false;

        if (rule == "*")
            return true;

        if (!IPAddress.TryParse(ipRaw, out var ip))
            return false;

        if (ip.IsIPv4MappedToIPv6)
            ip = ip.MapToIPv4();

        if (!rule.Contains('/'))
        {
            if (!IPAddress.TryParse(rule, out var exact))
                return false;

            if (exact.IsIPv4MappedToIPv6)
                exact = exact.MapToIPv4();

            return ip.Equals(exact);
        }

        var parts = rule.Split('/', 2);

        if (!IPAddress.TryParse(parts[0], out var network))
            return false;

        if (!int.TryParse(parts[1], out var prefix))
            return false;

        if (network.IsIPv4MappedToIPv6)
            network = network.MapToIPv4();

        if (ip.AddressFamily != network.AddressFamily)
            return false;

        var ipBytes = ip.GetAddressBytes();
        var networkBytes = network.GetAddressBytes();

        var maxPrefix = ipBytes.Length * 8;

        if (prefix < 0 || prefix > maxPrefix)
            return false;

        var fullBytes = prefix / 8;
        var remainingBits = prefix % 8;

        for (var i = 0; i < fullBytes; i++)
        {
            if (ipBytes[i] != networkBytes[i])
                return false;
        }

        if (remainingBits == 0)
            return true;

        var mask = (byte)(0xFF << (8 - remainingBits));

        return (ipBytes[fullBytes] & mask) == (networkBytes[fullBytes] & mask);
    }
}