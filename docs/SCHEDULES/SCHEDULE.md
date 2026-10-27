# Group schedule — SQLP for AI Agents

**October 26–November 23, 2026 · Proposed commitments for group review**

**Goal:** one interactive demo and one four-page ICDE demo paper. Given the same question, schema, and approved relationships, compare an agent generating SQL with one generating SQLP. Ryan's C++ compiler translates SQLP; SQLite executes both. Measure answer correctness and total input/output tokens, including instructions and retries.

| Owner | Main responsibility | Weekly budget |
|---|---|---|
| [Ryan](Ryan_Schedule.md) | C++ SQLP-to-SQL compiler | 12–15 hours |
| [Iris](Iris_Schedule.md) | Agent runner, SQLite execution, evaluation | 8–10 hours |
| [Samridh](Sam_Schedule.md) | TypeScript website, demo walkthrough | 8–10 hours |

**First handoff, Wed October 28:** agree on one example catalog, compiler input/output, and saved run record. Choose model access and a call budget; discuss contribution expectations and provisional authorship. Iris can start with hand-written SQL; Sam can start with clearly labeled sample records.

| Due | What the group should be able to show |
|---|---|
| **Sat Oct 31** | A C++ query object compiles into SQL that runs in SQLite; Sam displays the shared record. Choose the dataset and start the shared question bank. |
| **Sat Nov 7** | Real SQLP text compiles; a paired model pilot runs; the website displays real records. Freeze tested features, guides, and evaluation questions. |
| **Mon Nov 9** | Outline and rough section material ready to review with Enamul. This is a proposed internal date; confirm his intended timing. |
| **Sat Nov 14** | Held-out comparison completed, initial analysis saved, and the question-to-answer demo connected. |
| **Wed Nov 18** | First complete four-page paper assembled for group/mentor feedback. |
| **Sat Nov 21** | Checked paper, reproducible demo, rehearsal, and video ready. |
| **Sun Nov 22** | All authors review/approve; designated submitter aims to submit. **Nov 23 is the official demo deadline and contingency day.** |

At each Saturday checkpoint, show one working example and link the code/results. Flag missed dependencies immediately. Hours include reading, writing, and integration.

Start with one model, one reviewed dataset, 12 development questions, and a target of 30 held-out questions. Iris and Sam share question preparation and independently check each other's answers. Freeze workload size before held-out outcomes; a smaller checked set beats rushed answers. NatSQL gets at most **three shared person-hours** for a feasibility check; add it only if the required pipeline stays on schedule. Extra models and broader grammar come later.

The [ICDE demo call](https://icde2027.github.io/cf-demonstrations.html) requires IEEE format and four pages including references. November 9 is **not a published outline deadline**. The separate course assignment follows Canvas's deadline. See [IDEA.md](../IDEA.md) for the direction and [AGENTS.md](../AGENTS.md) for technical/evaluation context.
