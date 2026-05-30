using System.Text.Json;
using Dapper;
using WarInc.Api.Common;
using WarInc.Api.Database;

namespace WarInc.Api.Security;

public sealed class SecurityAuditService
{
    private readonly WarIncDb _db;

    public SecurityAuditService(WarIncDb db)
    {
        _db = db;
    }

    public async Task EnsureSchemaAsync()
    {
        await using var db = _db.CreateConnection();

        await db.ExecuteAsync(
            """
            CREATE TABLE IF NOT EXISTS securityaudit (
                Id BIGINT NOT NULL AUTO_INCREMENT,
                CreatedAt DATETIME NOT NULL,
                CustomerID BIGINT NULL,
                ServerID VARCHAR(128) NULL,
                IP VARCHAR(64) NOT NULL,
                Path VARCHAR(255) NOT NULL,
                Method VARCHAR(16) NOT NULL,
                Action VARCHAR(128) NOT NULL,
                Reason VARCHAR(255) NOT NULL,
                Payload TEXT NULL,
                PRIMARY KEY (Id),
                KEY IX_securityaudit_CreatedAt (CreatedAt),
                KEY IX_securityaudit_CustomerID (CustomerID),
                KEY IX_securityaudit_Action (Action)
            ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
            """);
    }

    public async Task LogAsync(
        HttpContext http,
        string action,
        string reason,
        ulong customerId = 0,
        string serverId = "",
        object? payload = null)
    {
        try
        {
            await EnsureSchemaAsync();

            var payloadText = "";

            if (payload != null)
            {
                payloadText = JsonSerializer.Serialize(payload);

                if (payloadText.Length > 16000)
                    payloadText = payloadText[..16000];
            }

            await using var db = _db.CreateConnection();

            await db.ExecuteAsync(
                """
                INSERT INTO securityaudit
                (
                    CreatedAt,
                    CustomerID,
                    ServerID,
                    IP,
                    Path,
                    Method,
                    Action,
                    Reason,
                    Payload
                )
                VALUES
                (
                    UTC_TIMESTAMP(),
                    @CustomerId,
                    @ServerId,
                    @Ip,
                    @Path,
                    @Method,
                    @Action,
                    @Reason,
                    @Payload
                );
                """,
                new
                {
                    CustomerId = customerId == 0 ? (ulong?)null : customerId,
                    ServerId = string.IsNullOrWhiteSpace(serverId) ? null : serverId,
                    Ip = SecurityUtil.NormalizeLegacyIp(SecurityUtil.GetClientIp(http)),
                    Path = http.Request.Path.ToString(),
                    Method = http.Request.Method,
                    Action = action,
                    Reason = reason,
                    Payload = payloadText
                });
        }
        catch
        {
        }
    }
}