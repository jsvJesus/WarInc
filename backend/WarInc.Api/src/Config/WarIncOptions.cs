namespace WarInc.Api.Config;

public sealed class WarIncOptions
{
    public string Database { get; set; } = "";
    public int SessionHours { get; set; } = 24;
    public string InternalApiKey { get; set; } = "";
}