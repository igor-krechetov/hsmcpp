#!/usr/bin/env python3
"""Convert doxygen XML output into a LOBSTER code trace file.

Reads the XML produced by `doxygen config/lobster/Doxyfile` and, for every
entity (class/struct via <compounddef>, function/method via <memberdef>) that
carries a `@requirement` cross-reference, emits an Implementation item linking
that entity to the requirement UIDs.

Because doxygen does the real C++ parsing, entity names, kinds, nesting and
templates are resolved correctly — unlike a regex scan of the source.

The `@requirement` command is a doxygen alias (see the Doxyfile) that renders as
an xrefsect titled "Requirement" whose body holds the comma-separated UIDs.

Usage:
    python3 scripts/requirements/doxygen_to_lobster.py <xml_dir> <out.lobster>
"""
import glob
import os
import re
import sys
import xml.etree.ElementTree as ET

try:  # LOBSTER 1.x layout
    from lobster.common.items import Implementation, Tracing_Tag
    from lobster.common.location import File_Reference
    from lobster.common.io import lobster_write
except ImportError:  # LOBSTER 0.x layout
    from lobster.items import Implementation, Tracing_Tag
    from lobster.location import File_Reference
    from lobster.io import lobster_write

UID_SPLIT = re.compile(r"[,\s]+")


def all_text(el):
    """Flatten an element's text content (para may contain nested tags)."""
    return "".join(el.itertext())


def requirement_uids(desc_el):
    """Collect UIDs from every @requirement xrefsect under a description
    element. The alias emits <xrefsect><xreftitle>Requirement</xreftitle>
    <xrefdescription><para>UID, UID</para></xrefdescription></xrefsect>."""
    uids = []
    if desc_el is None:
        return uids
    for xrefsect in desc_el.iter("xrefsect"):
        title = xrefsect.find("xreftitle")
        if title is None or (title.text or "").strip() != "Requirement":
            continue
        body = xrefsect.find("xrefdescription")
        if body is None:
            continue
        for tok in UID_SPLIT.split(all_text(body).strip()):
            if tok:
                uids.append(tok)
    return uids


def justification(desc_el):
    """Return the @no_requirement reason if present (marks the entity as
    intentionally not tied to a requirement), else None."""
    if desc_el is None:
        return None
    for xrefsect in desc_el.iter("xrefsect"):
        title = xrefsect.find("xreftitle")
        if title is not None and (title.text or "").strip() == "No Requirement":
            body = xrefsect.find("xrefdescription")
            reason = all_text(body).strip() if body is not None else ""
            return reason or "no requirement (unjustified reason)"
    return None


def location_of(el, default_file):
    loc = el.find("location")
    if loc is not None and loc.get("file"):
        line = loc.get("bodystart") or loc.get("line") or "1"
        return loc.get("file"), int(line)
    return default_file, 1


def make_impl(name, kind, uids, filename, line, just=None):
    impl = Implementation(
        tag      = Tracing_Tag(namespace="cpp", tag="%s:%s" % (name, line)),
        location = File_Reference(filename=filename, line=line),
        language = "C++",
        kind     = kind,
        name     = name,
    )
    for uid in uids:
        impl.add_tracing_target(Tracing_Tag.from_text("req", uid))
    if just:
        # A @no_requirement justification: LOBSTER marks the item "justified"
        # instead of "missing" for the required up-link.
        impl.just_up.append(just)
    return impl


def parse_compound(path, emit_all=False):
    """Extract Implementation items from one doxygen compound XML file.

    emit_all=False : only entities carrying @requirement / @no_requirement.
    emit_all=True  : EVERY function/class, so untagged code shows as unlinked
                     (red) in the report — the "no orphan code" gap analysis.

    Requirement inheritance:
      * If a class carries @requirement(s), every method of that class INHERITS
        them.
      * A method that also declares its own @requirement(s) is linked to the
        UNION of the class requirements and its own.
    """
    items = []
    root = ET.parse(path).getroot()
    for cdef in root.iter("compounddef"):
        ckind = cdef.get("kind")  # class, struct, namespace, file, ...
        cname_el = cdef.find("compoundname")
        cname = cname_el.text if cname_el is not None else "?"

        # Class/struct-level tags — these propagate to all member methods.
        class_uids = []
        if ckind in ("class", "struct"):
            class_uids = requirement_uids(cdef.find("detaileddescription"))
            class_uids += requirement_uids(cdef.find("briefdescription"))
            just = (justification(cdef.find("detaileddescription"))
                    or justification(cdef.find("briefdescription")))
            if class_uids or just or emit_all:
                fn, ln = location_of(cdef, path)
                items.append(make_impl(cname, "Class", class_uids, fn, ln, just))

        # Member-level tags (functions / methods), inheriting class requirements.
        for mdef in cdef.iter("memberdef"):
            if mdef.get("kind") not in ("function",):
                continue
            own = requirement_uids(mdef.find("detaileddescription"))
            own += requirement_uids(mdef.find("inbodydescription"))
            own += requirement_uids(mdef.find("briefdescription"))
            just = (justification(mdef.find("detaileddescription"))
                    or justification(mdef.find("inbodydescription"))
                    or justification(mdef.find("briefdescription")))
            # Effective requirements = class requirements UNION method's own,
            # de-duplicated with class reqs first, then the method's extras.
            uids = list(dict.fromkeys(class_uids + own))
            if not (uids or just or emit_all):
                continue
            qn = mdef.find("qualifiedname")
            nm = mdef.find("name")
            name = (qn.text if qn is not None and qn.text
                    else (nm.text if nm is not None else "?"))
            fn, ln = location_of(mdef, path)
            items.append(make_impl(name, "Function", uids, fn, ln, just))
    return items


def main():
    args = sys.argv[1:]
    emit_all = False
    if args and args[0] == "--all":
        emit_all = True
        args = args[1:]
    if len(args) != 2:
        sys.exit("usage: doxygen_to_lobster.py [--all] <xml_dir> <out.lobster>")
    xml_dir, out = args
    items = []
    seen = set()
    for path in sorted(glob.glob(os.path.join(xml_dir, "*.xml"))):
        base = os.path.basename(path)
        if base in ("index.xml", "Doxyfile.xml") or base.endswith(".xsd"):
            continue
        for impl in parse_compound(path, emit_all=emit_all):
            key = impl.tag.key()
            if key in seen:      # same entity can appear in file + class xml
                continue
            seen.add(key)
            items.append(impl)
    with open(out, "w", encoding="UTF-8") as fd:
        lobster_write(fd, Implementation, "doxygen_to_lobster", items)
    print("wrote %d code trace item(s) to %s" % (len(items), out),
          file=sys.stderr)


if __name__ == "__main__":
    main()
