# Samridh's schedule — website and demo

**October 26–November 23, 2026 · 8–10 hours/week · Proposed for group review**

Build the TypeScript website that makes the comparison understandable: the question, generated SQL and SQLP, compiled SQL, answers, correctness labels, and tokens. Iris owns experiment execution; Ryan owns SQLP compilation.

| Due | Deliverable |
|---|---|
| **Wed Oct 28** | Agree with Iris on one saved run record. Sketch a simple comparison page and the first demo question. |
| **Sat Oct 31** | Build the page using clearly labeled sample records. Show both conditions, answers, errors, and token fields. Help choose the dataset, start drafting questions/reference SQL, and check initial answers. Spend at most one hour helping Iris assess the existing NatSQL converter. |
| **Sat Nov 7** | Load real saved pilot records from Iris. Show SQLP alongside its compiled SQL. Add simple relationship inspection: clicking a path reveals its catalog mapping. Draft half the held-out questions/reference SQL and cross-check Iris's half before the workload freezes. |
| **Mon Nov 9** | Supply a screenshot and a short audience walkthrough for the internal outline review. Explain what a visitor can change and inspect. |
| **Sat Nov 14** | Connect the question form to Iris's local runner. Show running/error states and completed records. Finish path highlighting if time permits; help inspect surprising answers. |
| **Wed Nov 18** | Finish the demonstration-scenarios section, architecture screenshot, and walkthrough for the first complete paper. |
| **Sat Nov 21** | Rehearse a reliable demonstration. Record the encouraged video, at most five minutes and 50 MB, and prepare a clearly labeled replay of real runs as fallback. Verify the interface against Iris's records. |
| **Sun Nov 22** | Check the final PDF's screenshots, video, and interaction claims; review/approve with the group and support submission. Keep Nov 23 available for submission issues. |

**Minimum interaction:** choose a prepared question, inspect both queries/answers/tokens, and inspect a reference path. For example, show that `author.university` follows `AUTHOR.university_id = UNIVERSITY.id`. A simple highlighted list is enough before a graph animation.

Prepared questions can show independently checked correctness. Arbitrary questions should say **“not independently benchmark-scored.”** Clearly distinguish live runs, recorded real runs, and development fixtures.

**Scope:** prioritize a clear local demo. Authentication, a Notion-style editor, automatic relationship discovery, deployment infrastructure, and another compiler are optional future work. If time is tight, simplify animation and styling before cutting answer visibility or the walkthrough. [Shared schedule](SCHEDULE.md) · [Agent context](../AGENTS.md)
