namespace WarInc.Api.Database;

public sealed record DatabaseCheckResponse(
    bool Ok,
    IReadOnlyList<string> MissingTables,
    IReadOnlyList<DatabaseMissingColumnDto> MissingColumns,
    IReadOnlyList<string> Warnings
);

public sealed record DatabaseMissingColumnDto(
    string Table,
    string Column,
    string ExpectedType
);

public sealed record DatabaseSchemaResponse(
    bool Ok,
    IReadOnlyList<DatabaseTableSchemaDto> Tables,
    IReadOnlyList<string> Warnings
);

public sealed record DatabaseTableSchemaDto(
    string Table,
    IReadOnlyList<DatabaseColumnSchemaDto> Columns
);

public sealed record DatabaseColumnSchemaDto(
    string Name,
    string Type,
    bool Nullable,
    string Key,
    string DefaultValue,
    string Extra
);

public sealed record DatabaseRequiredColumn(
    string Table,
    string Column,
    string ExpectedType
);