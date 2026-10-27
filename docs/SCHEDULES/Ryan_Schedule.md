# Ryan's schedule — C++ compiler

**October 26–November 23, 2026 · 12–15 hours/week · Proposed for group review**

Your component accepts SQLP text plus the approved relationship catalog and returns SQL, parameter values, and useful errors. **SQLite supplies execution.** Current work provides path/catalog helpers and one-hop SQL rendering; the complete-query compiler and parser still need building.

| Due | Deliverable |
|---|---|
| **Wed Oct 28** | Agree with Iris on compiler input/output and catalog format. Trace the existing complete-query test; sketch the small query AST. |
| **Sat Oct 31** | Define the AST and implement `compile_query` from C++ objects. Return complete SQL and separate parameters. Test a filter, repeated reference, and invalid name; execute one example with Iris. Help check the chosen dataset's reference assumptions. Text parsing is a stretch this week. |
| **Sat Nov 7** | Add the tokenizer/Bison parser and a command-line interface Iris can call. Support multi-hop references, generated aliases, shared joins, and clear diagnostics. Give Iris a working text-query example early in the week and Sam a minimal path-to-join record. Freeze and document the tested subset. |
| **Mon Nov 9** | Supply the architecture diagram, supported-feature list, and a short explanation of SQLP → AST → SQL for the internal outline review. |
| **Sat Nov 14** | Fix correctness defects exposed by integration and evaluation. Check catalog assumptions and parameter/type handling. Avoid adding syntax during the main comparison. |
| **Wed Nov 18** | Finish the compiler/architecture section and review every implementation claim in the first complete paper. |
| **Sat Nov 21** | Verify a clean build, regression tests, compiler instructions, and the exact demo queries. Rerun affected experiments with Iris after any correctness fix. |
| **Sun Nov 22** | Review the final PDF and artifact, confirm the final author list/order with the group, and coordinate submission. Keep Nov 23 available for submission issues. |

**Required first release:** projections, one FROM binding/alias, multi-hop paths, literal equality and AND, DISTINCT, basic name/type checks, and parameterized SQL. Correlated EXISTS, UNION, and broader syntax remain later learning goals. After November 7, preserve the tested profile through submission. This is the evaluated subset, not a claim to implement the full language.

**Learning check:** explain one query through every compiler stage without relying on generated code you cannot explain. Use AI for small implementations, explanations, and counterexamples; review each increment yourself.

**Handoff:** Iris needs callable text compilation by November 7; Sam needs the path-to-join mapping for display. If that date is at risk, tell them early and cut optional grammar. [Shared schedule](SCHEDULE.md) · [Agent context](../AGENTS.md)
