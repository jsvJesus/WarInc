using System.Security.Cryptography;
using System.Text;
using Microsoft.AspNetCore.Http;

namespace WarInc.Api.Common;

public static class SecurityUtil
{
    public static string CreateToken()
    {
        Span<byte> bytes = stackalloc byte[32];
        RandomNumberGenerator.Fill(bytes);
        return Convert.ToBase64String(bytes);
    }

    public static int CreateSessionId()
    {
        return RandomNumberGenerator.GetInt32(1, int.MaxValue);
    }

    public static string Md5Hex(string value)
    {
        var bytes = MD5.HashData(Encoding.UTF8.GetBytes(value));
        var sb = new StringBuilder(bytes.Length * 2);

        foreach (var b in bytes)
            sb.Append(b.ToString("x2"));

        return sb.ToString();
    }

    public static string Sha256Hex(string value)
    {
        var bytes = SHA256.HashData(Encoding.UTF8.GetBytes(value));
        var sb = new StringBuilder(bytes.Length * 2);

        foreach (var b in bytes)
            sb.Append(b.ToString("x2"));

        return sb.ToString();
    }

    public static string GetClientIp(HttpContext http)
    {
        var forwarded = http.Request.Headers["X-Forwarded-For"].ToString();

        if (!string.IsNullOrWhiteSpace(forwarded))
        {
            var comma = forwarded.IndexOf(',');

            if (comma > 0)
                return forwarded[..comma].Trim();

            return forwarded.Trim();
        }

        return http.Connection.RemoteIpAddress?.ToString() ?? "0.0.0.0";
    }

    public static string NormalizeLegacyIp(string ip)
    {
        ip = (ip ?? "").Trim();

        if (string.IsNullOrWhiteSpace(ip))
            return "0.0.0.0";

        if (ip == "::1")
            return "127.0.0.1";

        const string ipv6MappedPrefix = "::ffff:";

        if (ip.StartsWith(ipv6MappedPrefix, StringComparison.OrdinalIgnoreCase))
            ip = ip[ipv6MappedPrefix.Length..];

        if (ip.Length > 16)
            ip = ip[..16];

        return ip;
    }

    public static bool PasswordMatches(string password, string storedPassword, string storedMd5Password)
    {
        if (!string.IsNullOrWhiteSpace(storedPassword))
        {
            if (string.Equals(password, storedPassword, StringComparison.Ordinal))
                return true;

            if (storedPassword.StartsWith("$2", StringComparison.Ordinal))
            {
                try
                {
                    if (BCrypt.Net.BCrypt.Verify(password, storedPassword))
                        return true;
                }
                catch
                {
                    return false;
                }
            }
        }

        if (!string.IsNullOrWhiteSpace(storedMd5Password))
        {
            var md5 = Md5Hex(password);

            if (string.Equals(md5, storedMd5Password, StringComparison.OrdinalIgnoreCase))
                return true;
        }

        return false;
    }
}