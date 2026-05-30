namespace WarInc.Api.Database;

public sealed record DatabaseCheckResponse(
    bool Ok,
    IReadOnlyList<string> MissingTables,
    IReadOnlyList<DatabaseMissingColumnDto> MissingColumns,
    IReadOnlyList<DatabaseColumnTypeMismatchDto> TypeMismatches,
    IReadOnlyList<DatabaseTableStatDto> TableStats,
    IReadOnlyList<string> Warnings
);

public sealed record DatabaseMissingColumnDto(
    string Table,
    string Column,
    string ExpectedType
);

public sealed record DatabaseColumnTypeMismatchDto(
    string Table,
    string Column,
    string ExpectedType,
    string ActualType
);

public sealed record DatabaseTableStatDto(
    string Table,
    long Rows,
    bool AllowedEmpty
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

public sealed record DatabaseSmokeResponse(
    bool Ok,
    DatabaseCheckResponse Schema,
    IReadOnlyList<DatabaseSmokeStepDto> Steps,
    IReadOnlyList<string> Warnings
);

public sealed record DatabaseSmokeStepDto(
    string Name,
    bool Ok,
    string Message,
    long? Count
);