namespace WarInc.Api.Auth;

public record LoginRequest(string Username, string Password);

public record CheckSessionRequest(ulong CustomerId, ulong SessionId, string Token);

public record CreateAccountRequest(string Username, string Password, string? Email, bool IsDeveloper);

public record LoginResponse(
    bool Ok,
    int Code,
    string Message,
    ulong CustomerId,
    ulong SessionId,
    int AccountStatus,
    string Token);

public record CheckSessionResponse(
    bool Ok,
    int Code,
    string Message,
    int AccountStatus);

public record CreateAccountResponse(
    bool Ok,
    int Code,
    string Message,
    ulong CustomerId);

public sealed class AccountRow
{
    public ulong CustomerId { get; set; }
    public string AccountName { get; set; } = "";
    public string Password { get; set; } = "";
    public string MD5Password { get; set; } = "";
    public int AccountStatus { get; set; }
    public bool IsDeveloper { get; set; }
}

public sealed class SessionRow
{
    public int SessionId { get; set; }
    public ulong CustomerId { get; set; }
    public string SessionKey { get; set; } = "";
    public DateTime TimeUpdated { get; set; }
    public int AccountStatus { get; set; }
}