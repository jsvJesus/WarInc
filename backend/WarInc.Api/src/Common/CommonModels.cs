namespace WarInc.Api.Common;

public record ApiResponse(bool Ok, int Code, string Message);

public record InternalSessionValidateResponse(
    bool Ok,
    ulong CustomerId,
    ulong SessionId,
    int AccountStatus);