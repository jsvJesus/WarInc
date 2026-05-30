using MySqlConnector;

namespace WarInc.Api.Database;

public sealed class WarIncDb
{
    private readonly string _connectionString;

    public WarIncDb(string connectionString)
    {
        _connectionString = connectionString;
    }

    public MySqlConnection CreateConnection()
    {
        return new MySqlConnection(_connectionString);
    }
}