# AI Usage Report — city_manager filter feature

## Tool Used

**Claude** (claude.ai, Sonnet model) — used as an interactive assistant during a
single conversation session.

## Context Provided to the AI

Before asking for code I described the project in full:

> "I have a C program that manages city infrastructure reports. Each report is a
> struct stored as a raw binary record in a file called `reports.dat`.
> The struct looks like this:
>
> ```c
> typedef struct {
>     int   id;
>     float X;
>     float Y;
>     char  name[20];       /* inspector username */
>     char  category[20];
>     int   severity;
>     char  description[256];
>     char  timestamp[26];
> } Report;
> ```
>
> The `name` field stores the username of the inspector who filed the report.
> I need two helper functions for a filter command that supports the fields:
> severity, category, inspector, and timestamp."

---

## Prompt 1 — `parse_condition`

**Prompt given:**

> "Please generate a C function with this signature:
> `int parse_condition(const char *input, char *field, char *op, char *value);`
> It should split a string of the form `field:operator:value` into the three
> parts. The operator can be `==`, `!=`, `<`, `<=`, `>`, or `>=`
> (one or two characters). Return 1 on success and 0 if the string is malformed."

**What was generated:**

The AI produced a function that:
1. Finds the first colon with `strchr` to isolate the field name.
2. Finds the second colon with another `strchr` call to isolate the operator.
3. Copies the remainder as the value.
4. Validates that no part is empty and that the operator is 1–2 characters.

**Changes I made:**

- Added an upper-bound length check on `field_len` (`< 32`) to avoid
  writing past the caller-supplied buffer.
- Changed the value buffer copy to use `strncpy` with a hard cap of 63
  characters (matching the `values[16][64]` array declared in
  `filter_reports`) instead of the unbounded copy the AI originally wrote.
- Added a check that `*val_start != '\0'` so an empty value is rejected.

**Why I made those changes:**

The AI's version was correct in logic but assumed the caller always passed
large enough buffers. In `filter_reports` I have fixed-size arrays, so I
needed explicit length limits to avoid buffer overflows.

---

## Prompt 2 — `match_condition`

**Prompt given:**

> "Now generate a C function:
> `int match_condition(Report *r, const char *field, const char *op, const char *value);`
> Supported fields:
> - `severity` — stored as `int`, use numeric comparison.
> - `category` — stored as `char category[20]`, use `strcmp`.
> - `inspector` — stored as `char name[20]` (the username of the person who
>   filed the report), use `strcmp`. The filter keyword is `inspector` even
>   though the struct field is called `name`.
> - `timestamp` — stored as `char timestamp[26]` in ctime format, use
>   `strncmp` with the length of the value so partial prefixes work.
> Return 1 if the record satisfies the condition, 0 otherwise."

**What was generated:**

The AI produced a function that handles each field in its own `if` block,
using a chain of `strcmp(op, ...)` calls for each operator, and falling
through to `return 0` for unknown fields or operators. It correctly mapped
the `inspector` keyword to `r->name`.

**Changes I made:**

- For `timestamp` I confirmed the AI used `strncmp(r->timestamp, value, strlen(value))`
  rather than a full `strcmp`. This lets a user write a partial prefix like
  `"Mon Apr"` without having to supply the full 26-character ctime string.
- Reordered the field checks to put `severity` first since it is the most
  common filter target (micro-optimisation, but also improves readability).

**Why I made those changes:**

The `strncmp` on timestamp was a deliberate design decision to make the
filter more ergonomic at the command line. A full ctime timestamp contains
spaces, which would require quoting on most shells.

---

## Filter Logic (written by me)

The `filter_reports` function is entirely my own work:

1. Build the file path the same way `list_reports` does.
2. Open the file with `open(..., O_RDONLY)`.
3. Pre-parse all conditions **once** (before the read loop) into parallel
   arrays of `fields`, `ops`, `values` so `parse_condition` is not called
   repeatedly inside the inner loop.
4. Loop: call `read(fd, &report, sizeof(Report))` until it returns fewer
   bytes than a full record.
5. For each record, iterate over every condition and call `match_condition`.
   Break early on the first mismatch (implicit AND short-circuit).
6. Print matching records with the `Inspector:` line (reading from
   `report.name`) consistent with the rest of the program.
7. After the loop, report how many records matched; print a message if none
   did.
8. `close(fd)`.

---

## Symbolic Links (written by me)

All symlink logic is original:

- `create_symlink(district)` constructs `active_reports-<district>` as the
  link name and `<district>/reports.dat` as the target, then calls
  `symlink()`. Before creating, it calls `lstat()` to check whether a link
  already exists and `unlink()`s it to avoid `EEXIST`.
- `print_permissions` was changed from `stat()` to `lstat()` so that
  symlinks appear as `l` entries rather than being silently followed.
- Dangling-link detection: if `lstat()` succeeds (the link file exists) but
  `stat()` (following the link) fails, a `WARNING:` line is printed instead
  of crashing.
- `create_symlink` is called at the end of `add_directory` (both on fresh
  creation and when the directory already existed) so the link is always
  refreshed.

---

## What I Learned

- The filter keyword (`inspector`) does not have to match the struct field
  name (`name`). `match_condition` acts as the translation layer — the
  keyword is what the user types, and the function accesses the correct
  struct member.
- `strchr` scanning twice for the two colons is the cleanest way to parse a
  delimited string in C without heap allocation.
- `lstat` vs `stat` is a meaningful choice: following symlinks silently can
  hide structural information and cause crashes on dangling links.
- Pre-parsing conditions outside the read loop is a small but real
  optimisation: avoid redundant work inside a tight I/O loop.
- AI-generated C code often omits buffer-size guards. Reviewing every
  `strcpy` / `strncpy` / `snprintf` call for potential overflow is
  essential before accepting generated code.
