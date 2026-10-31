"""Exercise the real compiler process and independently check SQLite answers."""

import json
import os
from pathlib import Path
import sqlite3
import subprocess
import unittest


ROOT = Path(__file__).resolve().parents[1]
COMPILER = ROOT / "build" / "sqlpc"
CATALOG = [
    "--column", "AUTHOR", "aid", "integer", "unique",
    "--column", "AUTHOR", "name", "string", "nonunique",
    "--column", "PAPER", "pid", "integer", "unique",
    "--column", "PAPER", "title", "string", "nonunique",
    "--column", "WROTE", "author_aid", "integer", "nonunique",
    "--column", "WROTE", "paper_pid", "integer", "nonunique",
    "--reference", "WROTE", "author", "author_aid", "AUTHOR", "aid",
    "--reference", "WROTE", "paper", "paper_pid", "PAPER", "pid",
]


def identifier(value):
    return '"' + value.replace('"', '""') + '"'


class CompilerCliTests(unittest.TestCase):
    def invoke(self, query, args=None, code=0, stage=None):
        result = subprocess.run(
            [str(COMPILER), *(CATALOG if args is None else args)],
            input=query.encode("utf-8") if isinstance(query, str) else query,
            capture_output=True, timeout=5,
        )
        self.assertEqual(result.returncode, code, result.stdout)
        self.assertEqual(result.stderr, b"")
        text = result.stdout.decode("utf-8")
        self.assertEqual(text.count("\n"), 1)
        record = json.loads(text)
        self.assertEqual(record["version"], 1)
        self.assertEqual(record["ok"], code == 0)
        if stage:
            self.assertEqual(record["error"]["stage"], stage)
            self.assertTrue(record["error"]["message"])
        return record

    @staticmethod
    def parameters(record):
        return [int(p["value"]) if p["type"] == "int64" else p["value"]
                for p in record["parameters"]]

    def test_sqlite_answers_preserve_duplicates_and_literal_values(self):
        with sqlite3.connect(":memory:") as database:
            database.executescript("""
                CREATE TABLE AUTHOR(aid INTEGER PRIMARY KEY, name TEXT NOT NULL);
                CREATE TABLE PAPER(pid INTEGER PRIMARY KEY, title TEXT NOT NULL);
                CREATE TABLE WROTE(author_aid INTEGER NOT NULL, paper_pid INTEGER NOT NULL);
                INSERT INTO AUTHOR VALUES (1, 'Alice'), (2, 'Bob');
                INSERT INTO PAPER VALUES (10, 'Databases');
                INSERT INTO WROTE VALUES (1, 10), (1, 10), (2, 10);
            """)
            for distinct, expected in [("", [("Alice",), ("Alice",), ("Bob",)]),
                                       ("DISTINCT ", [("Alice",), ("Bob",)])]:
                query = (f"SELECT {distinct}w.author.name AS name FROM WROTE w "
                         "WHERE w.paper.title = 'Databases';")
                record = self.invoke(query)
                self.assertEqual(record["parameters"], [{"type": "string", "value": "Databases"}])
                rows = database.execute(record["sql"], self.parameters(record)).fetchall()
                self.assertCountEqual(rows, expected)

            record = self.invoke("SELECT a.name FROM AUTHOR a WHERE a.name = 'x'' OR 1=1 --';")
            self.assertNotIn("OR 1=1", record["sql"])
            self.assertEqual(database.execute(record["sql"], self.parameters(record)).fetchall(), [])

    def test_integer_boundaries_are_strings_and_execute_exactly(self):
        with sqlite3.connect(":memory:") as database:
            database.execute("CREATE TABLE AUTHOR(aid INTEGER PRIMARY KEY, name TEXT)")
            for number in [-9223372036854775808, 9223372036854775807]:
                database.execute("INSERT INTO AUTHOR VALUES (?, ?)", (number, str(number)))
                record = self.invoke(f"SELECT a.name FROM AUTHOR a WHERE a.aid = {number};")
                self.assertEqual(record["parameters"], [{"type": "int64", "value": str(number)}])
                self.assertEqual(database.execute(record["sql"], self.parameters(record)).fetchall(), [(str(number),)])

    def test_quoted_names_unicode_and_control_characters_round_trip(self):
        table, column, alias = 'table" café', 'field\t\n"', 'from'
        value = 'O\'Brien "\\\t\n\x01 café 🧠'
        args = ["--column", table, column, "string", "nonunique"]
        query = (f"SELECT {identifier(alias)}.{identifier(column)} "
                 f"FROM {identifier(table)} {identifier(alias)} "
                 f"WHERE {identifier(alias)}.{identifier(column)} = '{value.replace(chr(39), chr(39) * 2)}';")
        record = self.invoke(query, args)
        self.assertEqual(record["parameters"], [{"type": "string", "value": value}])
        with sqlite3.connect(":memory:") as database:
            database.execute(f"CREATE TABLE {identifier(table)}({identifier(column)} TEXT)")
            database.execute(f"INSERT INTO {identifier(table)} VALUES (?)", (value,))
            self.assertEqual(database.execute(record["sql"], self.parameters(record)).fetchall(), [(value,)])

    def test_structured_query_errors(self):
        query = "SELECT a.name\nFROM AUTHOR a\nWHERE a.aid = ;"
        record = self.invoke(query, code=1, stage="parser")
        self.assertEqual(record["error"]["location"], {"offset": query.index(';'), "line": 3, "column": 15})
        self.invoke("SELECT a.name FROM AUTHOR a WHERE a.aid = 9223372036854775808;", code=1, stage="lexer")
        self.invoke("SELECT a.missing FROM AUTHOR a;", code=1, stage="compiler")
        self.invoke("SELECT a.name FROM AUTHOR a WHERE a.aid = 'one';", code=1, stage="compiler")
        self.invoke("SELECT a.name FROM AUTHOR a; SELECT a.aid FROM AUTHOR a;", code=1, stage="parser")
        self.invoke("", code=1, stage="parser")
        self.invoke("SELECT a.name FROM AUTHOR a;\0", code=1, stage="lexer")

    def test_argument_errors(self):
        for args in [["--unknown"], ["--column", "AUTHOR"], ["--reference", "AUTHOR"],
                     ["--column", "T", "id", "float", "unique"],
                     ["--column", "T", "id", "integer", "yes"], CATALOG + ["--help"]]:
            with self.subTest(args=args):
                self.invoke("", args, code=2, stage="arguments")

    def test_catalog_errors_including_unused_references(self):
        for args in [[], ["--column", "", "id", "integer", "unique"],
                     ["--column", "T", "", "integer", "unique"],
                     CATALOG + ["--column", "author", "other", "integer", "nonunique"],
                     CATALOG + ["--column", "AUTHOR", "AID", "integer", "unique"],
                     CATALOG + ["--reference", "WROTE", "author", "author_aid", "AUTHOR", "aid"],
                     CATALOG + ["--reference", "WROTE", "AUTHOR", "author_aid", "AUTHOR", "aid"],
                     CATALOG + ["--reference", "WROTE", "missing", "unknown", "AUTHOR", "aid"],
                     CATALOG + ["--reference", "WROTE", "bad_type", "author_aid", "AUTHOR", "name"],
                     CATALOG + ["--reference", "AUTHOR", "bad_unique", "name", "PAPER", "title"]]:
            with self.subTest(args=args):
                self.invoke("SELECT a.name FROM AUTHOR a;", args, code=1, stage="catalog")

    def test_invalid_utf8_produces_valid_json(self):
        for source in [b"\x80", b"\xc0\xaf", b"\xed\xa0\x80", b"\xf4\x90\x80\x80", b"\xe2(\xa1", b"\xf0\x90"]:
            with self.subTest(source=source):
                self.invoke(source, code=1, stage="input")

    def test_help(self):
        result = subprocess.run([str(COMPILER), "--help"], capture_output=True, timeout=5)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stderr, b"")
        self.assertIn(b"Usage: sqlpc", result.stdout)

    def test_stdin_read_failure_is_an_input_error(self):
        with open(os.devnull, "wb") as write_only:
            result = subprocess.run([str(COMPILER), *CATALOG], stdin=write_only,
                                    capture_output=True, timeout=5)
        self.assertEqual(result.returncode, 1)
        self.assertEqual(json.loads(result.stdout)["error"]["stage"], "input")

    @unittest.skipUnless(Path("/dev/full").exists(), "Output-failure check requires /dev/full")
    def test_output_failure_does_not_report_success(self):
        with open("/dev/full", "wb") as full:
            result = subprocess.run([str(COMPILER), "--help"], stdout=full,
                                    stderr=subprocess.PIPE, timeout=5)
        self.assertEqual(result.returncode, 3)


if __name__ == "__main__":
    unittest.main()
