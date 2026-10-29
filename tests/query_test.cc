#include "query.h"

#include <iostream>
#include <limits>
#include <stdexcept>

namespace {

void require(bool condition, const std::string &message) {
    if (!condition) {
        throw std::runtime_error{message};
    }
}

void require_invalid(const Query &query, const Catalog &catalog, const std::string &message) {
    try {
        compile_query(query, catalog);
    } catch (const std::invalid_argument &) {
        return;
    }
    throw std::runtime_error{message};
}

std::size_t join_count(const std::string &sql) {
    std::size_t count = 0;
    for (std::size_t position = 0; (position = sql.find("\nJOIN ", position)) != std::string::npos;
         position += 6) {
        ++count;
    }
    return count;
}

} // namespace

int main() {
    const Catalog catalog{
        {
            {"AUTHOR", {{"aid", ColumnType::Integer, true}, {"name", ColumnType::String}}},
            {"PAPER", {{"pid", ColumnType::Integer, true}, {"title", ColumnType::String},
                       {"lead_aid", ColumnType::Integer}, {"editor_aid", ColumnType::Integer}}},
            {"WROTE", {{"author_aid", ColumnType::Integer}, {"paper_pid", ColumnType::Integer}}}
        },
        {
            {"WROTE", "author", "author_aid", "AUTHOR", "aid"},
            {"WROTE", "paper", "paper_pid", "PAPER", "pid"},
            {"PAPER", "lead_author", "lead_aid", "AUTHOR", "aid"},
            {"PAPER", "editor", "editor_aid", "AUTHOR", "aid"}
        }
    };
    const Query example{
        "WROTE", "w", {{Path{"w", {"author", "name"}}, "author_name"}},
        EqualityPredicate{Path{"w", {"paper", "title"}}, std::string{"Databases"}}, true
    };

    try {
        const auto compiled = compile_query(example, catalog);
        const std::string expected =
            "SELECT DISTINCT \"w_ref0\".\"name\" AS \"author_name\"\n"
            "FROM \"WROTE\" AS \"w\"\n"
            "JOIN \"AUTHOR\" AS \"w_ref0\" ON \"w\".\"author_aid\" = \"w_ref0\".\"aid\"\n"
            "JOIN \"PAPER\" AS \"w_ref1\" ON \"w\".\"paper_pid\" = \"w_ref1\".\"pid\"\n"
            "WHERE \"w_ref1\".\"title\" = ?;";
        require(compiled.sql == expected, "Complete query SQL did not match");
        require(compiled.parameters == std::vector<SqlValue>{std::string{"Databases"}},
                "String parameter was not returned separately");

        Query direct{
            "AUTHOR", "a", {{Path{"a", {"name"}}, std::nullopt}},
            EqualityPredicate{Path{"a", {"aid"}}, std::numeric_limits<std::int64_t>::max()}
        };
        const auto direct_result = compile_query(direct, catalog);
        require(join_count(direct_result.sql) == 0, "Direct columns unexpectedly added a join");
        require(direct_result.sql.find("DISTINCT") == std::string::npos,
                "Compiler added DISTINCT without being asked");
        require(direct_result.parameters ==
                    std::vector<SqlValue>{std::numeric_limits<std::int64_t>::max()},
                "Integer parameter lost its value or type");
        direct.where.reset();
        require(compile_query(direct, catalog).parameters.empty(),
                "Query without a predicate unexpectedly returned parameters");

        Query repeated{
            "WROTE", "w", {{Path{"w", {"author", "name"}}, "name"},
                           {Path{"w", {"author", "aid"}}, "id"}},
            EqualityPredicate{Path{"w", {"author", "name"}}, std::string{"Alice"}}
        };
        require(join_count(compile_query(repeated, catalog).sql) == 1,
                "Repeated reference paths did not reuse their join");

        const Query roles{
            "PAPER", "p", {{Path{"p", {"lead_author", "name"}}, "lead"},
                           {Path{"p", {"editor", "name"}}, "editor"}}, std::nullopt
        };
        const auto role_result = compile_query(roles, catalog);
        require(join_count(role_result.sql) == 2, "Different reference roles were merged");
        require(role_result.sql.find("\"p_ref0\".\"name\" AS \"lead\"") != std::string::npos &&
                    role_result.sql.find("\"p_ref1\".\"name\" AS \"editor\"") != std::string::npos,
                "Reference roles did not use distinct aliases");

        Query collision{
            "WROTE", "W_REF0", {{Path{"W_REF0", {"author", "name"}}, std::nullopt}}, std::nullopt
        };
        require(compile_query(collision, catalog).sql.find("AS \"W_REF0_ref0\"") != std::string::npos,
                "Generated alias collided with the FROM alias");

        Query quoted = example;
        const std::string literal{"O'Reilly' OR 1=1 --"};
        quoted.where->value = literal;
        const auto quoted_result = compile_query(quoted, catalog);
        require(quoted_result.sql == compiled.sql &&
                    quoted_result.parameters == std::vector<SqlValue>{literal},
                "Literal text was interpolated into SQL");

        const Catalog reserved{{{"order", {{"select", ColumnType::String}}}}, {}};
        const Query reserved_query{
            "order", "from", {{Path{"from", {"select"}}, "display\"name"}}, std::nullopt
        };
        require(compile_query(reserved_query, reserved).sql ==
                    "SELECT \"from\".\"select\" AS \"display\"\"name\"\nFROM \"order\" AS \"from\";",
                "SQL identifiers were not quoted correctly");

        Query invalid = example;
        invalid.projections.clear();
        require_invalid(invalid, catalog, "Empty projections were accepted");
        invalid = example;
        invalid.projections[0].path = Path{"other", {"author", "name"}};
        require_invalid(invalid, catalog, "Unbound projection alias was accepted");
        invalid = example;
        invalid.where->path = Path{"other", {"paper", "title"}};
        require_invalid(invalid, catalog, "Unbound predicate alias was accepted");
        invalid = example;
        invalid.source_table = "UNKNOWN";
        require_invalid(invalid, catalog, "Unknown source table was accepted");
        invalid = example;
        invalid.projections[0].path = Path{"w", {"author", "missing"}};
        require_invalid(invalid, catalog, "Unknown column was accepted");
        invalid.projections[0].path = Path{"w", {"missing", "name"}};
        require_invalid(invalid, catalog, "Unknown reference was accepted");
        invalid.projections[0].path = Path{"w", {"paper", "editor", "name"}};
        require_invalid(invalid, catalog, "Unsupported longer path was accepted");
        invalid = example;
        invalid.where->value = std::int64_t{42};
        require_invalid(invalid, catalog, "Wrong predicate literal type was accepted");
        invalid = example;
        invalid.projections[0].alias = "";
        require_invalid(invalid, catalog, "Empty projection alias was accepted");
        invalid = example;
        invalid.source_alias.clear();
        require_invalid(invalid, catalog, "Empty FROM alias was accepted");
        invalid = example;
        invalid.projections[0].alias = std::string{"bad\0alias", 9};
        require_invalid(invalid, catalog, "NUL in an identifier was accepted");

        Catalog invalid_catalog = catalog;
        invalid_catalog.tables[0].columns[0].unique = false;
        require_invalid(example, invalid_catalog, "A non-unique reference target was accepted");
        invalid_catalog = catalog;
        invalid_catalog.tables[2].columns[0].type = ColumnType::String;
        require_invalid(example, invalid_catalog, "Mismatched reference key types were accepted");

        std::cout << "query compiler checks passed\n";
    } catch (const std::exception &error) {
        std::cerr << "query compiler check failed: " << error.what() << '\n';
        return 1;
    }
}
