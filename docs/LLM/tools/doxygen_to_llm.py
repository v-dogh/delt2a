#!/usr/bin/env python3
"""Convert Doxygen's XML symbol inventory to compact Markdown for coding agents.

Requires only the Python standard library. No Doxygen installation is needed
for conversion, only for producing the source XML.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import shutil
import xml.etree.ElementTree as ET
from collections import defaultdict
from pathlib import Path

COMPOUND_KINDS = {"class", "struct", "union", "namespace", "concept", "group"}
PLURAL = {"class": "classes", "struct": "structs", "union": "unions", "namespace": "namespaces", "concept": "concepts", "group": "groups"}
MEMBER_KINDS = {"function", "variable", "typedef", "enum", "property", "event", "friend", "signal", "slot"}


def flatten(element: ET.Element | None) -> str:
    if element is None:
        return ""
    return re.sub(r"\s+", " ", "".join(element.itertext())).strip()


def child_text(element: ET.Element, path: str) -> str:
    return flatten(element.find(path))


def md_escape(text: str) -> str:
    return text.replace("|", r"\|").replace("`", "'").replace("\n", " ")


def slug(name: str, refid: str) -> str:
    stem = re.sub(r"[^A-Za-z0-9_.-]+", "_", name).strip("._")[:96] or "symbol"
    digest = hashlib.sha256(refid.encode()).hexdigest()[:8]
    return f"{stem}-{digest}"


def source_location(node: ET.Element) -> tuple[str, str]:
    location = node.find("location")
    if location is None:
        return "", ""
    src = location.get("file", "")
    line = location.get("line", "")
    if src:
        src = src.replace("\\", "/")
        # Doxygen may return an absolute path. Keep enough path to identify the file.
        for marker in ("/core/", "/elements/"):
            if marker in src:
                src = src[src.index(marker) + 1:]
                break
        if src.endswith("/d2_std.hpp"):
            src = "d2_std.hpp"
    return src, line


def template_text(node: ET.Element) -> str:
    params = node.find("templateparamlist")
    if params is None:
        return ""
    items = []
    for param in params.findall("param"):
        value = " ".join(p for p in (
            child_text(param, "type"),
            child_text(param, "declname"),
        ) if p)
        default = child_text(param, "defval")
        items.append(value + (f" = {default}" if default else ""))
    return "template<" + ", ".join(items) + ">\n" if items else ""


def signature(node: ET.Element, name: str) -> str:
    kind = node.get("kind", "")
    args = child_text(node, "argsstring")
    definition = child_text(node, "definition")
    typ = child_text(node, "type")
    if kind == "enum":
        return f"enum {name}"
    if kind in {"define", "macro"}:
        return f"#define {name}{args}"
    if definition:
        s = definition
        if kind == "function" and args:
            s += args
        return template_text(node) + s
    if kind == "function":
        return template_text(node) + (f"{typ} {name}{args}" if typ else f"{name}{args}")
    if kind in {"typedef", "variable", "property", "event"}:
        return template_text(node) + (f"{typ} {name}" if typ else name)
    return template_text(node) + name


def member_records(compound: ET.Element):
    seen = set()
    for section in compound.findall("sectiondef"):
        for member in section.findall("memberdef"):
            if member.get("prot", "public") == "private":
                continue
            kind = member.get("kind", "")
            if kind not in MEMBER_KINDS:
                continue
            refid = member.get("id", "")
            if refid and refid in seen:
                continue
            seen.add(refid)
            yield member


def document(node: ET.Element, outpath: Path, kind: str, name: str) -> list[dict]:
    src, line = source_location(node)
    brief = child_text(node, "briefdescription")
    details = child_text(node, "detaileddescription")
    result = [f"# `{name}`", "", f"**Kind:** {kind}"]
    if src:
        result.append(f"**Declared in:** `{src}{(':' + line) if line else ''}`")
    if brief:
        result.extend(["", brief])
    if details and details != brief:
        result.extend(["", details])
    parents = [flatten(x) for x in node.findall("basecompoundref") if flatten(x)]
    if parents:
        result.extend(["", "**Bases:** " + ", ".join(f"`{p}`" for p in parents)])
    records = []
    sections: dict[str, list[ET.Element]] = defaultdict(list)
    for m in member_records(node):
        sections[m.get("kind", "other")].append(m)
    for mkind, members in sorted(sections.items()):
        result.extend(["", f"## {mkind.title()}s", ""])
        for member in members:
            mname = child_text(member, "name") or "<unnamed>"
            sig = signature(member, mname)
            msrc, mline = source_location(member)
            short = child_text(member, "briefdescription")
            detail = child_text(member, "detaileddescription")
            result.extend([f"### `{mname}`", "", "```cpp", sig, "```"])
            if short:
                result.extend(["", short])
            if detail and detail != short:
                result.extend(["", detail])
            if msrc:
                result.extend(["", f"Source: `{msrc}{(':' + mline) if mline else ''}`"])
            if mkind == "enum":
                values = [child_text(v, "name") for v in member.findall("enumvalue")]
                if values:
                    result.extend(["", "Values: " + ", ".join(f"`{v}`" for v in values)])
            result.append("")
            records.append({
                "kind": mkind, "name": f"{name}::{mname}", "signature": sig,
                "file": msrc or src, "line": mline or line,
                "doc": outpath.as_posix(),
            })
    outpath.parent.mkdir(parents=True, exist_ok=True)
    outpath.write_text("\n".join(result).rstrip() + "\n", encoding="utf-8")
    return records


def generate(xml_dir: Path, dest: Path):
    if not (xml_dir / "index.xml").is_file():
        raise SystemExit(f"No Doxygen XML index at {xml_dir / 'index.xml'}; run doxygen first")
    tree = ET.parse(xml_dir / "index.xml")
    compounds = []
    records = []
    macros = {}
    # Index file selects only compounds known to Doxygen.
    for entry in tree.findall("./compound"):
        kind, refid = entry.get("kind", ""), entry.get("refid", "")
        name = child_text(entry, "name")
        path = xml_dir / f"{refid}.xml"
        if not path.exists():
            continue
        root = ET.parse(path).getroot()
        compound = root.find("compounddef")
        if compound is None:
            continue
        if kind == "file":
            for section in compound.findall("sectiondef"):
                for m in section.findall("memberdef"):
                    if m.get("kind") != "define":
                        continue
                    macro_name = child_text(m, "name")
                    macro_src, macro_line = source_location(m)
                    if macro_name and macro_name not in macros:
                        macros[macro_name] = {
                            "name": macro_name, "signature": signature(m, macro_name),
                            "file": macro_src, "line": macro_line,
                            "description": child_text(m, "briefdescription"),
                        }
            continue
        if kind not in COMPOUND_KINDS:
            continue
        outfile = Path(PLURAL[kind]) / (slug(name, refid) + ".md")
        records.append({"kind": kind, "name": name, "file": source_location(compound)[0], "doc": outfile.as_posix()})
        records.extend(document(compound, dest / outfile, kind, name))
        compounds.append({"kind": kind, "name": name, "file": outfile})
    dest.mkdir(parents=True, exist_ok=True)
    groups = defaultdict(list)
    for c in compounds:
        groups[c["kind"]].append(c)
    index = ["# delt2a generated API index", "", "Generated from Doxygen XML. For idiomatic application code and lifecycle rules, **read `AGENTS.md` and the curated handbook first**.", "", "This is a declaration inventory, not proof that an API is safe or suitable in every context.", ""]
    for kind, entries in sorted(groups.items()):
        index.extend([f"## {PLURAL[kind].title()} ({len(entries)})", ""])
        for entry in sorted(entries, key=lambda e: e["name"].casefold()):
            index.append(f"- [`{md_escape(entry['name'])}`]({entry['file'].as_posix()})")
        index.append("")
    index.extend(["## Other indexes", "", "- [Macros](MACROS.md)", "- `SYMBOLS.jsonl` for ripgrep/automated symbol lookup", ""])
    (dest / "INDEX.md").write_text("\n".join(index), encoding="utf-8")
    macro_doc = ["# Preprocessor macros", "", "Source declarations only. Check the curated handbook for intended invocation rules.", ""]
    for key, m in sorted(macros.items()):
        macro_doc.extend([f"## `{key}`", "", "```cpp", m["signature"], "```"])
        if m["file"]:
            macro_doc.append(f"Source: `{m['file']}{(':' + m['line']) if m['line'] else ''}`")
        if m["description"]:
            macro_doc.extend(["", m["description"]])
        macro_doc.append("")
        records.append({"kind": "macro", "name": key, "signature": m["signature"], "file": m["file"], "line": m["line"], "doc": "MACROS.md"})
    (dest / "MACROS.md").write_text("\n".join(macro_doc), encoding="utf-8")
    with (dest / "SYMBOLS.jsonl").open("w", encoding="utf-8") as f:
        for record in sorted(records, key=lambda r: (r["name"].casefold(), r["kind"])):
            # Relative paths are relative to docs/api; intentionally no source duplication.
            if record["doc"].startswith(dest.as_posix() + "/"):
                record["doc"] = record["doc"][len(dest.as_posix()) + 1:]
            f.write(json.dumps(record, ensure_ascii=False) + "\n")
    print(f"Indexed {len(compounds)} named compounds, {len(macros)} macros, {len(records)} symbols")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("xml_dir", type=Path, help="Doxygen XML directory containing index.xml")
    parser.add_argument("dest", type=Path, help="Output docs/api directory")
    args = parser.parse_args()
    generate(args.xml_dir, args.dest)
