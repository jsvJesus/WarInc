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
    string? FriendName = null,
    bool Accept = true);

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

public sealed record ClanTargetRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    ulong TargetCustomerId = 0,
    string? TargetName = null);

public sealed record ClanInviteAnswerRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    ulong ClanId,
    bool Accept = true);

public sealed record ClanApplicationRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    ulong ClanId,
    string? Text = null);

public sealed record ClanApplicationAnswerRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    ulong ApplicationId = 0,
    ulong TargetCustomerId = 0,
    bool Accept = true);

public sealed record ClanAnnouncementRequest(
    ulong CustomerId,
    ulong SessionId,
    string Token,
    string Announcement);

public sealed record SocialFriendDto(
    ulong CustomerId,
    string Gamertag,
    int Status,
    int Online,
    int Level,
    int XP);

public sealed record SocialFriendPendingDto(
    ulong CustomerId,
    string Gamertag,
    int Incoming,
    DateTime DateAdded);

public sealed record FriendsListResponse(
    bool Ok,
    int Code,
    string Message,
    IReadOnlyList<SocialFriendDto> Friends);

public sealed record FriendsPendingResponse(
    bool Ok,
    int Code,
    string Message,
    IReadOnlyList<SocialFriendPendingDto> Incoming,
    IReadOnlyList<SocialFriendPendingDto> Outgoing);

public sealed record SocialClanDto(
    ulong ClanId,
    string Name,
    string Tag,
    ulong OwnerCustomerId,
    int Level,
    int XP,
    int MemberCount,
    string Announcement);

public sealed record SocialClanMemberDto(
    ulong CustomerId,
    string Gamertag,
    int Rank,
    int Online,
    int XP,
    int ContributedXP,
    int ContributedGP);

public sealed record SocialClanInviteDto(
    ulong InviteId,
    ulong ClanId,
    string ClanName,
    string ClanTag,
    ulong InviterCustomerId,
    string InviterGamertag,
    ulong TargetCustomerId,
    string TargetGamertag,
    int Incoming,
    DateTime ExpireTime);

public sealed record SocialClanApplicationDto(
    ulong ApplicationId,
    ulong ClanId,
    string ClanName,
    string ClanTag,
    ulong CustomerId,
    string Gamertag,
    string Text,
    DateTime ExpireTime);

public sealed record ClanResponse(
    bool Ok,
    int Code,
    string Message,
    SocialClanDto? Clan);