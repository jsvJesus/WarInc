namespace WarInc.Api.Config;

public sealed class WarIncOptions
{
    public string Database { get; set; } = "";
    public int SessionHours { get; set; } = 24;
    public string InternalApiKey { get; set; } = "";

    public bool TrustProxyHeaders { get; set; } = false;

    public string[] GameServerAllowedIps { get; set; } = Array.Empty<string>();

    public bool RequireLegacySessionToken { get; set; } = false;

    public int LoginRateLimitPerMinute { get; set; } = 10;
    public int BuyRateLimitPerMinute { get; set; } = 20;
    public int RollRateLimitPerMinute { get; set; } = 15;
    public int GameServerRateLimitPerMinute { get; set; } = 300;

    public int MaxLegacyUploadBytes { get; set; } = 16 * 1024 * 1024;
}