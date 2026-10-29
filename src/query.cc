#include "query.h"

#include "sql.h"

#include <stdexcept>
#include <utility>

namespace {

std::string quote_identifier(const std::string &name) {
    if (name.empty() || name.find('\0') != std::string::npos) {
        throw std::invalid_argument{"SQL identifiers must be nonempty and contain no NUL"};
    }

    std::string quoted{"\""};
    for (char character : name) {
        if (character == '"') {
            quoted += '"';
        }
        quoted += character;
    }
    return quoted + '"';
}

// Quote identifiers at the rendering boundary, then reuse the SQL renderers.
ColumnRef quoted_column(const ColumnRef &column) {
    return {quote_identifier(column.alias), quote_identifier(column.column)};
}

struct BoundColumn {
    ColumnRef column;
    ColumnType type;
};

struct ReferenceJoin {
    std::string feature;
    JoinSpec join;
};

bool matches_type(const SqlValue &value, ColumnType type) {
    return (type == ColumnType::Integer && std::holds_alternative<std::int64_t>(value)) ||
           (type == ColumnType::String && std::holds_alternative<std::string>(value));
}

} // namespace

CompiledQuery compile_query(const Query &query, const Catalog &catalog) {
    if (query.projections.empty()) {
        throw std::invalid_argument{"A query needs at least one projection"};
    }

    const std::string source_table = quote_identifier(query.source_table);
    const std::string source_alias = quote_identifier(query.source_alias);
    std::vector<ReferenceJoin> joins;

    auto bind_path = [&](const Path &path) -> BoundColumn {
        if (path.root() != query.source_alias) {
            throw std::invalid_argument{"Path alias does not match the FROM binding: " + path.root()};
        }

        const auto &features = path.features();
        if (features.size() == 1) {
            const auto &column = find_column(catalog, query.source_table, features[0]);
            return {{query.source_alias, features[0]}, column.type};
        }
        if (features.size() != 2) {
            throw std::invalid_argument{"Only direct columns and one reference hop are supported"};
        }

        const auto &mapping = find_reference(catalog.references, query.source_table, features[0]);
        const auto &source_key = find_column(catalog, mapping.source_table, mapping.source_column);
        const auto &target_key = find_column(catalog, mapping.target_table, mapping.target_column);
        if (source_key.type != target_key.type) {
            throw std::invalid_argument{"Reference key types do not match: " + features[0]};
        }
        if (!target_key.unique) {
            throw std::invalid_argument{"Reference target must be declared unique: " + features[0]};
        }

        std::string target_alias;
        for (const auto &reference : joins) {
            if (reference.feature == features[0]) {
                target_alias = reference.join.target.alias;
                break;
            }
        }
        const bool new_join = target_alias.empty();
        if (new_join) {
            // Appending to the source alias prevents collisions with it,
            // including SQLite's case-insensitive identifier comparisons.
            target_alias = query.source_alias + "_ref" + std::to_string(joins.size());
        }

        const auto resolved = resolve_path(path, query.source_table, target_alias, catalog);
        if (new_join) {
            joins.push_back({features[0], resolved.join});
        }
        const auto &column = find_column(catalog, mapping.target_table, features[1]);
        return {resolved.column, column.type};
    };

    std::string select_sql = query.distinct ? "SELECT DISTINCT " : "SELECT ";
    for (std::size_t index = 0; index < query.projections.size(); ++index) {
        const auto &projection = query.projections[index];
        const auto bound = bind_path(projection.path);
        if (index != 0) {
            select_sql += ", ";
        }
        select_sql += render_column(quoted_column(bound.column));
        if (projection.alias) {
            select_sql += " AS " + quote_identifier(*projection.alias);
        }
    }

    CompiledQuery result;
    std::string where_sql;
    if (query.where) {
        const auto bound = bind_path(query.where->path);
        if (!matches_type(query.where->value, bound.type)) {
            throw std::invalid_argument{"Predicate literal type does not match the column type"};
        }
        where_sql = "\nWHERE " + render_column(quoted_column(bound.column)) + " = ?";
        result.parameters.push_back(query.where->value);
    }

    result.sql = select_sql + "\nFROM " + source_table + " AS " + source_alias;
    for (const auto &reference : joins) {
        const auto &join = reference.join;
        result.sql += "\n" + render_join(JoinSpec{
            quote_identifier(join.target_table),
            quoted_column(join.source),
            quoted_column(join.target)
        });
    }
    result.sql += where_sql + ";";
    return result;
}
