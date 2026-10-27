# Iris's schedule — agent runner and evaluation

**October 26–November 23, 2026 · 8–10 hours/week · Proposed for group review**

Your runner calls the model in separate SQL and SQLP conditions, sends SQLP through Ryan's compiler, executes SQL in SQLite, checks answers, and saves queries, errors, results, and token usage for Sam's website.

| Due | Deliverable |
|---|---|
| **Wed Oct 28** | Agree on the catalog, compiler interface, and saved run record. Choose model access and a call budget with the group. |
| **Sat Oct 31** | Create a tiny SQLite fixture, hand-written reference SQL, checked expected answers, and a SQL execution/scoring script. With Ryan/Sam, select the evaluation dataset and start the question bank. Give Sam one sample record and run Ryan's first compiled query. Cap the shared NatSQL check at two hours of your time. |
| **Sat Nov 7** | Connect one model and Ryan's text compiler. Run the 12-question development pilot; save prompts, attempts, answers, errors, and raw usage. Finalize corresponding SQL/SQLP guides. With Sam, prepare/check roughly 30 held-out questions and freeze them before seeing comparative outcomes. |
| **Mon Nov 9** | Supply the methods outline, evaluation rules, and clearly labeled pilot evidence for the internal outline review. |
| **Sat Nov 14** | Run the frozen held-out comparison on one model; retain failures. Produce correctness and token summaries plus a few inspected error cases. Provide a minimal local interface so Sam can trigger a run. |
| **Wed Nov 18** | Finish evaluation/limitations text and a result table for the first complete paper. Report mixed or negative outcomes accurately. |
| **Sat Nov 21** | Verify every number against saved records. Rerun cases affected by compiler fixes; save model/settings/data versions and reproduction instructions. Supply real recorded runs for the demo fallback. |
| **Sun Nov 22** | Check the final PDF's numbers and claims, review/approve with the group, and support submission. Keep Nov 23 available for submission issues. |

**Fair comparison:** same model/settings, questions, relationship facts, and repair limits. Start with first-attempt generation; equal-budget repairs are optional. Count all input/output tokens, including guides and retries, without double-counting cache/reasoning fields. Keep expected answers private from the model.

**Question preparation is shared:** draft about half the held-out questions/reference SQL; Sam drafts the other half. Cross-check each other's expected answers, including duplicate-sensitive and wrong-relationship cases. Ryan checks reference assumptions. The 30-question target may shrink for capacity, but never remove questions because SQLP performed poorly.

**Handoff:** Sam can display saved records before a live interface exists. If Ryan's parser is late, finish SQL execution, scoring, logging, and guides while he resolves it. NatSQL, extra models, and extra repetitions must not delay the main study. [Shared schedule](SCHEDULE.md) · [Agent context](../AGENTS.md)
