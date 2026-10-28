#!/usr/bin/env python3
"""lang_tool.py -- the VR menu's translations (src/lang.h). The same tool as the ETS2 / ATS DLAA mod's (its scripts/lang_tool.py),
for this mod's UTF-8 texts.

  python scripts/lang_tool.py extract [--check]   src/*.cpp|h -> lang/template.pot (every TR / TRF / N_ / NF_ text)
                                                   --check: fail when template.pot is not up to date (CI)
  python scripts/lang_tool.py update               lang/*.po follow template.pot: new texts added empty, gone ones kept as #~
  python scripts/lang_tool.py check                every lang/*.po parses, its placeholders match, no duplicates (CI)
  python scripts/lang_tool.py pack OUT.bin OUT.rc  the packed table for the DLL (the build runs this)
  python scripts/lang_tool.py review OUT_DIR       one review file per language (paste into another AI model) + a CSV of all

The English text is the key. A c-format text (TRF / NF_) must keep the same %-conversions in the same order in its
translation; scripts and the DLL both check it, the DLL shows English when it does not match.
"""
import glob
import os
import re
import struct
import sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
LANG_DIR = os.path.join(ROOT, "lang")
TEMPLATE = os.path.join(LANG_DIR, "template.pot")
MACROS = {"TR": False, "TRF": True, "N_": False, "NF_": True}

# ---------------------------------------------------------------------------------------------------- C++ string literals
_ESC = {"n": "\n", "t": "\t", "r": "\r", "0": "\0", "\\": "\\", '"': '"', "'": "'", "?": "?", "a": "\a", "b": "\b",
        "f": "\f", "v": "\v"}


def c_unescape(body):
    """The literal's text. \\x escapes are BYTES of the UTF-8 text (the compiler's /utf-8 view, which lang.cpp hashes)."""
    out, i = bytearray(), 0
    while i < len(body):
        c = body[i]
        if c != "\\":
            out += c.encode("utf-8")
            i += 1
            continue
        n = body[i + 1]
        if n == "x":
            m = re.match(r"[0-9a-fA-F]+", body[i + 2:])
            out.append(int(m.group(0), 16) & 0xFF)
            i += 2 + len(m.group(0))
        elif n == "u":
            out += chr(int(body[i + 2:i + 6], 16)).encode("utf-8")
            i += 6
        elif n == "U":
            out += chr(int(body[i + 2:i + 10], 16)).encode("utf-8")
            i += 10
        else:
            out += _ESC.get(n, n).encode("utf-8")
            i += 2
    return out.decode("utf-8", errors="replace")


_LIT = re.compile(r'\s*(?://[^\n]*\n\s*)*(?:L|u8)?"((?:[^"\\\n]|\\.)*)"', re.S)
_CALL = re.compile(r"(?<![A-Za-z0-9_:])(TRF|TR|NF_|N_)\(")


def extract_from(text):
    """[(english, is_format, line)] in file order."""
    found = []
    for m in _CALL.finditer(text):
        pos, parts = m.end(), []
        while True:
            lm = _LIT.match(text, pos)
            if not lm:
                break
            parts.append(c_unescape(lm.group(1)))
            pos = lm.end()
        if not parts:
            continue                                  # the #define lines, or a call with a variable
        rest = text[pos:pos + 40].lstrip()
        if not rest.startswith(")"):
            continue                                  # not a plain literal argument
        found.append(("".join(parts), MACROS[m.group(1)], text.count("\n", 0, m.start()) + 1,
                      kind_at(text, m.start(), m.group(1))))
    return found


# Where a text is shown (a "#." note for translators, and the width the VR menu gives it). Most restrictive first.
KINDS = [
    ("key",    "Key on the VR keyboard (very short: about 6 Latin characters; use what keyboards in your country have printed)"),
    ("button", "Button (short: about 10 Latin characters; the font shrinks a little when needed)"),
    ("tab",    "Tab name at the top of the menu (about 11 Latin characters)"),
    ("name",   "Controller button name (about 16 Latin characters)"),
    ("value",  "Value or choice (about 18 Latin characters)"),
    ("label",  "Name of a setting row (about 30 Latin characters)"),
    ("title",  "Title bar (one line, about 40 Latin characters)"),
    ("status", "Status / hint line at the bottom (one line, about 55 Latin characters)"),
    ("help",   "Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters)"),
]
_TABLES = [("kKeys[] = {", "key"), ("kChipHelp[4] = {", "help"), ("k[VrPad::kBtnCount] = {", "name"), ("kTabs[5] = {", "tab"), ("kTabHelp[5] = {", "help"),
           ("kChip[4] = {", "button"), ("kOutNames[9] = {", "button"), ("kOutHelp[9] = {", "help"),
           ("kThemeLabel[kThemeCount] = {", "button"), ("kSm[4] = {", "button"), ("kName[4] = {", "button"),
           ("kKinds[] = {", "value")]


def kind_at(text, pos, macro):
    far = text[max(0, pos - 6000):pos]
    for name, k in _TABLES:
        i = far.rfind(name)
        if i >= 0 and "};" not in far[i:]:
            return k
    st = max(text.rfind(";", 0, pos), text.rfind("{", 0, pos), text.rfind("}", 0, pos))
    stmt = text[st + 1:pos]                           # the statement up to the text
    if "tip(" in stmt or "help" in stmt or "h += " in stmt or "h = " in stmt:
        return "help"
    if "status_" in stmt or "add(eInfo, 0," in stmt or "add(eInfo, 30" in stmt:
        return "status"
    if "add(eBar" in stmt:
        return "title"
    if "row(" in stmt or "add(eHere" in stmt:
        return "label"
    if "add(eSet" in stmt or "opts(" in stmt or "plusMinus(" in stmt or "add(ePickCancel" in stmt or "add(eKbBtn" in stmt:
        return "button"
    return "value"


def sources():
    files = sorted(glob.glob(os.path.join(ROOT, "src", "*.cpp")) + glob.glob(os.path.join(ROOT, "src", "*.h")))
    return [f for f in files if os.path.basename(f) not in ("lang.h", "kb_layouts.inc")]


# ---------------------------------------------------------------------------------------------------------- .po / .pot
def po_escape(s):
    return s.replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n").replace("\t", "\\t").replace("\r", "\\r")


def po_quote(key, s, width=100):
    """`key "..."`, long texts split into several quoted lines at spaces."""
    e = po_escape(s)
    if len(e) + len(key) + 3 <= width:
        return '%s "%s"\n' % (key, e)
    out, cur = ['%s ""\n' % key], ""
    for word in re.findall(r"\S+\s*|\s+", e):
        if cur and len(cur) + len(word) > width - 2:
            out.append('"%s"\n' % cur)
            cur = ""
        cur += word
    if cur:
        out.append('"%s"\n' % cur)
    return "".join(out)


def po_unquote(line):
    a, b = line.find('"'), line.rfind('"')
    if a < 0 or b <= a:
        raise ValueError("no quoted text")
    body, out, i = line[a + 1:b], [], 0
    while i < len(body):
        c = body[i]
        if c == "\\" and i + 1 < len(body):
            n = body[i + 1]
            out.append({"n": "\n", "t": "\t", "r": "\r"}.get(n, n))
            i += 2
        else:
            out.append(c)
            i += 1
    return "".join(out)


class Entry:
    def __init__(self):
        self.ctx, self.id, self.str = None, None, ""
        self.flags, self.comments, self.refs, self.obsolete, self.line = set(), [], [], False, 0


def parse_po(path):
    """[Entry]; the header (msgid "") is the first when present. Raises ValueError with the line on a syntax error."""
    entries, cur, part = [], None, None
    pending_flags, pending_comments, pending_refs = set(), [], []

    def flush():
        nonlocal cur, part
        if cur is not None and cur.id is not None:
            entries.append(cur)
        cur, part = None, None

    with open(path, encoding="utf-8-sig") as f:
        lines = f.read().split("\n")
    for no, raw in enumerate(lines, 1):
        line = raw.strip()
        try:
            if not line:
                if part == "str":
                    flush()
                continue
            if line.startswith("#~"):
                continue                              # obsolete entries are not read back
            if line.startswith("#"):
                if part == "str":
                    flush()
                if line.startswith("#,"):
                    pending_flags |= {x.strip() for x in line[2:].split(",") if x.strip()}
                elif line.startswith("#:"):
                    pending_refs.append(line[2:].strip())
                else:
                    pending_comments.append(raw)
                continue
            if line.startswith("msgctxt"):
                if part == "str":
                    flush()
                cur = Entry()
                cur.ctx, part = po_unquote(line), "ctx"
            elif line.startswith("msgid_plural"):
                raise ValueError("plural forms are not used")
            elif line.startswith("msgid"):
                if part == "str":
                    flush()
                if cur is None:
                    cur = Entry()
                cur.id, part, cur.line = po_unquote(line), "id", no
                cur.flags, cur.comments, cur.refs = pending_flags, pending_comments, pending_refs
                pending_flags, pending_comments, pending_refs = set(), [], []
            elif line.startswith("msgstr"):
                if cur is None or cur.id is None:
                    raise ValueError("msgstr without msgid")
                cur.str, part = po_unquote(line), "str"
            elif line.startswith('"'):
                if part == "ctx":
                    cur.ctx += po_unquote(line)
                elif part == "id":
                    cur.id += po_unquote(line)
                elif part == "str":
                    cur.str += po_unquote(line)
                else:
                    raise ValueError("a quoted line outside an entry")
            else:
                raise ValueError("not a .po line")
        except ValueError as e:
            raise ValueError("%s:%d: %s" % (os.path.relpath(path, ROOT), no, e))
    if part == "str":
        flush()
    return entries


def header_fields(entries):
    for e in entries:
        if e.id == "":
            out = {}
            for l in e.str.split("\n"):
                if ":" in l:
                    k, v = l.split(":", 1)
                    out[k.strip()] = v.strip()
            return out
    return {}


# ------------------------------------------------------------------------------------------------------ placeholders
_SPEC = re.compile(r"%(?:%|[-+ #0]*(?:\*|\d+)?(?:\.(?:\*|\d+))?(?:hh|h|ll|l|L|I64|I32|I|j|z|t|w)?([diouxXeEfFgGaAcCsSpn])?)")


def specs(s):
    """The %-conversions (and colour marks 0x01..0x03) of s in order, as lang.cpp Specs()."""
    out, i = [], 0
    while i < len(s):
        c = s[i]
        if c in "\x01\x02\x03":
            out.append(c)
            i += 1
            continue
        if c != "%":
            i += 1
            continue
        if s[i + 1:i + 2] == "%":
            i += 2
            continue
        m = _SPEC.match(s, i)
        if m and m.group(1):
            out.append(m.group(0))
            i = m.end()
        else:
            out.append("%!")
            i = max(m.end() if m else i + 1, i + 1)
    return out


# ------------------------------------------------------------------------------------------------------------ commands
def build_template():
    seen, order = {}, []
    for path in sources():
        rel = os.path.relpath(path, ROOT).replace("\\", "/")
        with open(path, encoding="utf-8") as f:
            text = f.read()
        for en, fmt, _line, kind in extract_from(text):
            if not en:
                continue
            if en not in seen:
                seen[en] = {"fmt": fmt, "refs": [], "kinds": set()}
                order.append(en)
            seen[en]["fmt"] |= fmt
            seen[en]["kinds"].add(kind)
            if rel not in seen[en]["refs"]:
                seen[en]["refs"].append(rel)
    out = ['# ETS2 ATS VR Controls -- every English text of the VR menu and keyboard.\n',
           '# Made by: python scripts/lang_tool.py extract   (do not edit by hand)\n',
           'msgid ""\n', 'msgstr ""\n', '"Content-Type: text/plain; charset=UTF-8\\n"\n', '\n']
    for en in order:
        e = seen[en]
        kind = next(k for k, _ in KINDS if k in e["kinds"])
        out.append("#. %s\n" % dict(KINDS)[kind])
        if "|" in en and en.split("|")[0].isalpha():
            out.append("#. Translate only the part after the |, and make it fit a small 2-line button (about 2 x 9 characters)\n")
        out.append("#: %s\n" % " ".join(e["refs"]))
        if e["fmt"]:
            out.append("#, c-format\n")
        out.append(po_quote("msgid", en))
        out.append('msgstr ""\n\n')
    return "".join(out), order, {en: seen[en]["fmt"] for en in order}


def template_entries():
    ents = [e for e in parse_po(TEMPLATE) if e.id]
    return ents, {e.id: ("c-format" in e.flags) for e in ents}


def cmd_extract(check):
    text, order, _ = build_template()
    old = open(TEMPLATE, encoding="utf-8").read() if os.path.exists(TEMPLATE) else ""
    if check:
        if old != text:
            print("lang/template.pot is not up to date: run  python scripts/lang_tool.py extract  and commit it")
            return 1
        print("template.pot up to date (%d texts)" % len(order))
        return 0
    os.makedirs(LANG_DIR, exist_ok=True)
    with open(TEMPLATE, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)
    print("lang/template.pot: %d texts" % len(order))
    return 0


def po_files():
    return sorted(glob.glob(os.path.join(LANG_DIR, "*.po")))


def cmd_update():
    tents, fmt = template_entries()
    for path in po_files():
        ents = parse_po(path)
        head = next((e for e in ents if e.id == ""), None)
        old = {e.id: e for e in ents if e.id}
        out = []
        if head is not None:
            out += [c + "\n" for c in head.comments]
            out.append('msgid ""\n')
            out.append(po_quote("msgstr", head.str))
            out.append("\n")
        for t in tents:
            o = old.pop(t.id, None)
            out += [c + "\n" for c in (o.comments if o else []) if not c.startswith("#.")]   # translator notes
            out += [c + "\n" for c in t.comments if c.startswith("#.")]                      # the template's notes
            out.append("#: %s\n" % " ".join(t.refs))
            flags = set(o.flags if o else ()) - {"c-format"}
            if fmt[t.id]:
                flags.add("c-format")
            if flags:
                out.append("#, %s\n" % ", ".join(sorted(flags)))
            out.append(po_quote("msgid", t.id))
            out.append(po_quote("msgstr", o.str if o else ""))
            out.append("\n")
        for o in old.values():                        # texts the code no longer has: kept for reference
            if o.str:
                out.append("#~ msgid \"%s\"\n#~ msgstr \"%s\"\n\n" % (po_escape(o.id), po_escape(o.str)))
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write("".join(out))
        print("%s updated" % os.path.relpath(path, ROOT))
    return 0


def check_po(path, fmt):
    """(errors, warnings, translated, total)"""
    errors, warnings = [], []
    rel = os.path.relpath(path, ROOT)
    try:
        ents = parse_po(path)
    except ValueError as e:
        return [str(e)], [], 0, len(fmt)
    hdr = header_fields(ents)
    code = os.path.splitext(os.path.basename(path))[0]
    if not re.fullmatch(r"[a-z]{2}_[a-z]{2}", code):
        errors.append("%s: the file name must be a game language code (de_de.po)" % rel)
    if hdr.get("Language", "").lower() != code:
        errors.append("%s: header Language: must be %s" % (rel, code))
    if not hdr.get("X-Native-Name"):
        errors.append("%s: header X-Native-Name: missing (the language's own name, e.g. Deutsch)" % rel)
    seen, done = set(), 0
    for e in ents:
        if not e.id:
            continue
        if e.id in seen:
            errors.append("%s:%d: the same msgid twice" % (rel, e.line))
        seen.add(e.id)
        if e.id not in fmt:
            if e.str:
                warnings.append("%s:%d: not in template.pot any more (run update): %r" % (rel, e.line, e.id[:60]))
            continue
        if not e.str or "fuzzy" in e.flags:
            continue
        if fmt[e.id] and specs(e.id) != specs(e.str):
            errors.append("%s:%d: placeholders differ: English %s, translation %s" % (rel, e.line, specs(e.id), specs(e.str)))
            continue
        if not fmt[e.id] and "\x01" in e.str:
            errors.append("%s:%d: control character in the translation" % (rel, e.line))
        done += 1
    return errors, warnings, done, len(fmt)


def cmd_check():
    _, fmt = template_entries()
    bad = 0
    for path in po_files():
        errors, warnings, done, total = check_po(path, fmt)
        for w in warnings:
            print("warning: " + w)
        for e in errors:
            print("ERROR: " + e)
        bad += len(errors)
        print("%s: %d / %d translated (%d %%)" % (os.path.basename(path), done, total, 100 * done // max(1, total)))
    print("%d error(s)" % bad)
    return 1 if bad else 0


def fnv64(s):
    h = 1469598103934665603
    for b in s.encode("utf-8"):
        h ^= b
        h = (h * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    return h


def cmd_pack(out_bin, out_rc):
    try:
        _, fmt = template_entries()
    except (OSError, ValueError) as e:
        print("lang pack: %s -- packing no translations" % e)
        fmt = {}
    langs = []
    for path in po_files():
        errors, _, _, _ = check_po(path, fmt)
        if errors:
            print("lang pack: %s skipped texts with errors (run check)" % os.path.basename(path))
        code = os.path.splitext(os.path.basename(path))[0]
        if not re.fullmatch(r"[a-z]{2}_[a-z]{2}", code):
            continue
        try:
            ents = parse_po(path)
        except ValueError as e:
            print("lang pack: %s skipped: %s" % (os.path.basename(path), e))
            continue
        name = header_fields(ents).get("X-Native-Name", code)
        table = {}
        for e in ents:
            if not e.id or not e.str or "fuzzy" in e.flags or e.id not in fmt:
                continue
            if fmt[e.id] and specs(e.id) != specs(e.str):
                continue
            table[fnv64(e.id)] = e.str
        langs.append((code, name, sorted(table.items())))
    blob = bytearray()
    strings = bytearray()
    head = bytearray(b"VLNG" + struct.pack("<II", 1, len(langs)))
    dir_size = 20 * len(langs)
    ent_start = len(head) + dir_size
    ent_blob = bytearray()
    str_start = ent_start + sum(12 * len(t) for _, _, t in langs)
    dirs = bytearray()
    for code, name, table in langs:
        name_off = str_start + len(strings)
        strings += name.encode("utf-8") + b"\0"
        dirs += code.encode("ascii").ljust(8, b"\0") + struct.pack("<III", name_off, ent_start + len(ent_blob), len(table))
        for h, s in table:
            ent_blob += struct.pack("<QI", h, str_start + len(strings))
            strings += s.encode("utf-8") + b"\0"
    blob = head + dirs + ent_blob + strings
    os.makedirs(os.path.dirname(os.path.abspath(out_bin)), exist_ok=True)
    with open(out_bin, "wb") as f:
        f.write(blob)
    with open(out_rc, "w", encoding="ascii", newline="\n") as f:
        f.write('LANGPACK RCDATA "%s"\n' % os.path.abspath(out_bin).replace("\\", "/"))
    print("lang pack: %d languages, %d texts, %d bytes" % (len(langs), sum(len(t) for _, _, t in langs), len(blob)))
    return 0


REVIEW_HEAD = """# Review request: {name} ({code}) translation of a VR game mod menu

You are a native {name} speaker and a PC gaming / VR expert. Please review this translation.

**Context.** This is the in-headset settings menu and on-screen keyboard of a VR controller mod for the truck simulators
Euro Truck Simulator 2 and American Truck Simulator: it maps the VR controllers' buttons, sticks and pointing to keyboard
keys, the mouse and steering. The menu floats in front of the player; its boxes are small. All {name} texts below were
written by an AI and may be wrong, unnatural or too long.

**Check every line for:**
1. Wrong meaning compared with the English.
2. Unnatural wording: use what {name} PC gamers, VR players and game options menus normally say.
3. Consistency: the same English term should get the same {name} term everywhere (grip, trigger, stick, modifier, air mouse,
   panel, see-through, dead zone, full lock).
4. Length: the "Where" column gives the room (Latin characters; a CJK character counts as 2). Too long = it gets smaller or
   cut off with "...". Key names on the keyboard: what keyboards in your country have printed (e.g. German: Strg, Entf).

**Rules your corrections must keep (or the mod falls back to English for that line):**
- Keep every placeholder exactly and in the same order: %s %d %.0f %.1f %.2f %.3g %.2g ...
- `%%` is a literal percent sign: keep it as `%%`. Never add a single `%`.
- Do not translate: VR, ETS2, ATS, config.ini, the button letters X / Y / A / B, units px, km/h.

**Answer format.** Only list the lines you would change, as a table:

| # | Current {name} text | Corrected {name} text | Reason (short, in English) |
|---|---|---|---|

Then one line: how many lines you checked and an overall quality rating (1-10). Do not repeat lines that are fine.

| # | Where | English | {name} |
|---|---|---|---|
"""


def cmd_review(out_dir):
    tents, _ = template_entries()
    notes = {e.id: next((c[3:].replace("; the font shrinks a little when needed", "").replace("; use what keyboards in your country have printed", "")
                    for c in e.comments if c.startswith("#.")), "") for e in tents}
    os.makedirs(out_dir, exist_ok=True)
    rows = ["code,language,number,where,english,translation"]
    cell = lambda t: t.replace("|", "\\|").replace("\n", " ")
    csvq = lambda t: '"' + t.replace('"', '""') + '"'
    for path in po_files():
        code = os.path.splitext(os.path.basename(path))[0]
        ents = parse_po(path)
        name = header_fields(ents).get("X-Native-Name", code)
        tr = {e.id: e.str for e in ents if e.id}
        lines = [REVIEW_HEAD.format(name=name, code=code)]
        for i, t in enumerate(tents, 1):
            lines.append("| %d | %s | %s | %s |\n" % (i, notes[t.id], cell(t.id), cell(tr.get(t.id, ""))))
            rows.append(",".join([code, csvq(name), str(i), csvq(notes[t.id]), csvq(t.id), csvq(tr.get(t.id, ""))]))
        with open(os.path.join(out_dir, code + ".md"), "w", encoding="utf-8", newline="\n") as f:
            f.write("".join(lines))
    with open(os.path.join(out_dir, "all_languages.csv"), "w", encoding="utf-8-sig", newline="\n") as f:
        f.write("\n".join(rows) + "\n")
    print("%d review files + all_languages.csv in %s" % (len(po_files()), out_dir))
    return 0


def main(argv):
    if len(argv) >= 2 and argv[1] == "extract":
        return cmd_extract("--check" in argv)
    if len(argv) >= 2 and argv[1] == "update":
        return cmd_update()
    if len(argv) >= 2 and argv[1] == "check":
        return cmd_check()
    if len(argv) == 4 and argv[1] == "pack":
        return cmd_pack(argv[2], argv[3])
    if len(argv) == 3 and argv[1] == "review":
        return cmd_review(argv[2])
    print(__doc__)
    return 2


if __name__ == "__main__":
    sys.exit(main(sys.argv))
