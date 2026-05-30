namespace WarInc.Api.Social;

public sealed record FriendsListRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token);

public sealed record FriendActionRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    ulong FriendCustomerId,
    string? FriendName);

public sealed record ClanRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token);

public sealed record ClanCreateRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    string Name,
    string Tag);

public sealed record SocialFriendDto(
    ulong CustomerId,
    string Gamertag,
    int Status,
    int Online,
    int Level,
    int XP);

public sealed record FriendsListResponse(
    bool Ok,
    int Code,
    string Message,
    IReadOnlyList<SocialFriendDto> Friends);

public sealed record SocialClanDto(
    ulong ClanId,
    string Name,
    string Tag,
    ulong OwnerCustomerId,
    int Level,
    int XP,
    int MemberCount);

public sealed record ClanResponse(
    bool Ok,
    int Code,
    string Message,
    SocialClanDto? Clan);