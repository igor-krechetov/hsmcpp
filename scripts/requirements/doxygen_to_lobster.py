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


def wrapper_target(desc_el):
    """Return the sibling method name from a @requirement_wrapper tag, else None.
    A method tagged `@requirement_wrapper <siblingName>` is a thin helper that
    delegates to <siblingName> IN THE SAME CLASS, and inherits that sibling's
    effective @requirement UIDs. Resolution is same-class only."""
    if desc_el is None:
        return None
    for xrefsect in desc_el.iter("xrefsect"):
        title = xrefsect.find("xreftitle")
        if title is not None and (title.text or "").strip() == "Requirement Wrapper":
            body = xrefsect.find("xrefdescription")
            name = all_text(body).strip() if body is not None else ""
            return name or None
    return None


def is_container(desc_el):
    """True if the class/struct carries @requirements_container. A requirements
    container is a pure grouping scope whose MEMBERS each carry their own
    @requirement: the class entity itself is suppressed from the trace report
    (so a bare grouping class is not flagged as orphan code), but — unlike
    @no_requirement — NOTHING is propagated to its members. An untagged member
    of a container still surfaces as a real orphan, so a genuinely-missing
    requirement is never masked."""
    if desc_el is None:
        return False
    for xrefsect in desc_el.iter("xrefsect"):
        title = xrefsect.find("xreftitle")
        if title is not None and (title.text or "").strip() == "Requirements Container":
            return True
    return False


def implements_target(desc_el):
    """Return the qualified interface name from an @implements xrefsect, else
    None. A class tagged `@implements ns::Interface` declares that its methods
    implement the same requirements as the same-named methods of that interface,
    so each method inherits the interface method's @requirement UIDs by name."""
    if desc_el is None:
        return None
    for xrefsect in desc_el.iter("xrefsect"):
        title = xrefsect.find("xreftitle")
        if title is not None and (title.text or "").strip() == "Implements":
            body = xrefsect.find("xrefdescription")
            name = all_text(body).strip() if body is not None else ""
            return name or None
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


def collect_class_tags(paths):
    """Prepass: map every class/struct qualified name to the @requirement UIDs
    and @no_requirement justification it declares on its own brief/detailed
    description. Used so members and nested types can inherit their enclosing
    class's tags (a nested type lives in its own XML file, so it has no access
    to the parent's tags at parse time).

    Also collects, for the @implements interface/impl linking:
      * method_uids: UIDs of each tagged method, keyed "ClassQualifiedName::method"
      * class_implements: for each class, the interface it declares it @implements
      * container_classes: set of qualified names tagged @requirements_container,
        so a class that @implements a container interface inherits container-ness
        (its class entity is suppressed too, methods still traced individually).

    Returns (name_to_uids, name_to_just, method_uids, class_implements,
             container_classes)."""
    name_to_uids = {}
    name_to_just = {}
    method_uids = {}
    class_implements = {}
    container_classes = set()
    for path in paths:
        root = ET.parse(path).getroot()
        for cdef in root.iter("compounddef"):
            if cdef.get("kind") not in ("class", "struct"):
                continue
            cname_el = cdef.find("compoundname")
            if cname_el is None or not cname_el.text:
                continue
            cname = cname_el.text
            uids = requirement_uids(cdef.find("detaileddescription"))
            uids += requirement_uids(cdef.find("briefdescription"))
            if uids:
                name_to_uids[cname] = list(dict.fromkeys(uids))
            just = (justification(cdef.find("detaileddescription"))
                    or justification(cdef.find("briefdescription")))
            if just:
                name_to_just[cname] = just
            if (is_container(cdef.find("detaileddescription"))
                    or is_container(cdef.find("briefdescription"))):
                container_classes.add(cname)
            iface = (implements_target(cdef.find("detaileddescription"))
                     or implements_target(cdef.find("briefdescription")))
            if iface:
                class_implements[cname] = iface
            # Per-method UIDs so an @implements impl class can inherit by name.
            for mdef in cdef.iter("memberdef"):
                if mdef.get("kind") != "function":
                    continue
                mu = requirement_uids(mdef.find("detaileddescription"))
                mu += requirement_uids(mdef.find("inbodydescription"))
                mu += requirement_uids(mdef.find("briefdescription"))
                if not mu:
                    continue
                nm = mdef.find("name")
                if nm is None or not nm.text:
                    continue
                key = "%s::%s" % (cname, nm.text)
                method_uids.setdefault(key, [])
                for u in mu:
                    if u not in method_uids[key]:
                        method_uids[key].append(u)
    return name_to_uids, name_to_just, method_uids, class_implements, \
        container_classes


def parse_compound(path, emit_all=False, enclosing_uids=None,
                   enclosing_just=None, method_uids=None, class_implements=None,
                   container_classes=None):
    """Extract Implementation items from one doxygen compound XML file.

    emit_all=False : only entities carrying @requirement / @no_requirement.
    emit_all=True  : EVERY function/class, so untagged code shows as unlinked
                     (red) in the report — the "no orphan code" gap analysis.

    Tag inheritance:
      * If a class carries @requirement(s), every method of that class INHERITS
        them. A method that also declares its own @requirement(s) is linked to
        the UNION of the class requirements and its own.
      * If a class carries @no_requirement, every method and NESTED type INHERITS
        that justification — UNLESS the member/nested type has its own tag
        (a real @requirement or its own @no_requirement wins over an inherited
        justification, and a justification never applies to an entity that has
        requirement UIDs).
      * A NESTED type (struct/class) INHERITS its enclosing class's @requirement
        and @no_requirement. Doxygen emits a nested type as its own <compounddef>
        with empty descriptions, so without this it would wrongly show as orphan
        code. `enclosing_uids` / `enclosing_just` carry the parent's tags, keyed
        by qualified name in the prepass (see collect_class_tags / main).
      * A class that declares @implements <ns::Interface> IMPLEMENTS that
        interface: each of its methods inherits the @requirement UIDs of the
        same-named interface method. This links an implementation class (e.g.
        HierarchicalStateMachine::Impl) to the requirements already tagged on the
        public interface (HierarchicalStateMachine) without re-tagging each
        method. `method_uids` maps "Class::method" -> UIDs; `class_implements`
        maps an impl class to the interface it implements. If the implemented
        interface is a @requirements_container, the impl class INHERITS
        container-ness: its own class entity is suppressed too (its methods are
        already traced individually via the mirror), so it is not flagged as
        orphan code.
    """
    enclosing_uids = enclosing_uids or {}
    enclosing_just = enclosing_just or {}
    method_uids = method_uids or {}
    class_implements = class_implements or {}
    container_classes = container_classes or set()
    items = []
    root = ET.parse(path).getroot()
    for cdef in root.iter("compounddef"):
        ckind = cdef.get("kind")  # class, struct, namespace, file, ...
        cname_el = cdef.find("compoundname")
        cname = cname_el.text if cname_el is not None else "?"

        # Interface this class implements (for per-method name-based inheritance).
        impl_iface = class_implements.get(cname)

        # Class/struct-level tags — these propagate to all member methods.
        class_uids = []
        class_just = None
        if ckind in ("class", "struct"):
            class_uids = requirement_uids(cdef.find("detaileddescription"))
            class_uids += requirement_uids(cdef.find("briefdescription"))
            own_just = (justification(cdef.find("detaileddescription"))
                        or justification(cdef.find("briefdescription")))
            container = (is_container(cdef.find("detaileddescription"))
                         or is_container(cdef.find("briefdescription")))
            # A class that @implements a @requirements_container interface is
            # itself a container (its methods are mirror-traced individually).
            if impl_iface in container_classes:
                container = True
            # A nested type inherits the enclosing class's tags. The enclosing
            # scope is the qualified name minus the final "::segment".
            inherited_uids = []
            inherited_just = None
            if "::" in cname:
                parent = cname.rsplit("::", 1)[0]
                inherited_uids = enclosing_uids.get(parent, [])
                inherited_just = enclosing_just.get(parent)
            class_uids = list(dict.fromkeys(inherited_uids + class_uids))
            # Own tag wins over an inherited justification; a justification never
            # applies to an entity that already carries requirement UIDs.
            class_just = own_just or (None if class_uids else inherited_just)
            just = class_just if not class_uids else None
            # @requirements_container: suppress the class entity from the report
            # and propagate NOTHING to members (each member is scored on its own
            # tag; an untagged member still surfaces as a real orphan). Ignored
            # if the class also carries real @requirement/@no_requirement.
            if container and not class_uids and not class_just:
                class_uids = []
                class_just = None
            elif class_uids or just or emit_all:
                fn, ln = location_of(cdef, path)
                items.append(make_impl(cname, "Class", class_uids, fn, ln, just))

        # Member-level tags (functions / methods), inheriting class tags.
        for mdef in cdef.iter("memberdef"):
            if mdef.get("kind") not in ("function",):
                continue
            own = requirement_uids(mdef.find("detaileddescription"))
            own += requirement_uids(mdef.find("inbodydescription"))
            own += requirement_uids(mdef.find("briefdescription"))
            own_just = (justification(mdef.find("detaileddescription"))
                        or justification(mdef.find("inbodydescription"))
                        or justification(mdef.find("briefdescription")))
            # Implemented-interface requirements: if this class @implements an
            # interface, inherit the same-named interface method's UIDs.
            # Constructors and destructors are named after their own class, so
            # map the impl class's ctor/dtor name to the interface's ctor/dtor
            # name before looking up (e.g. Impl -> HierarchicalStateMachine,
            # ~Impl -> ~HierarchicalStateMachine).
            inherited_from_iface = []
            mname_el = mdef.find("name")
            if impl_iface and mname_el is not None and mname_el.text:
                mname = mname_el.text
                impl_short = cname.rsplit("::", 1)[-1]
                iface_short = impl_iface.rsplit("::", 1)[-1]
                if mname == impl_short:               # constructor
                    lookup = iface_short
                elif mname == "~" + impl_short:       # destructor
                    lookup = "~" + iface_short
                else:
                    lookup = mname
                inherited_from_iface = method_uids.get(
                    "%s::%s" % (impl_iface, lookup), [])
            # @requirement_wrapper <sibling>: a thin helper that delegates to a
            # SAME-CLASS sibling and inherits that sibling's effective UIDs. The
            # sibling's effective UIDs are its own @requirement in this class
            # (method_uids[cname::sibling]) plus, if this class @implements an
            # interface, the sibling's mirrored interface UIDs
            # (method_uids[impl_iface::sibling]). Resolution is same-class only.
            inherited_from_wrapper = []
            wrap = (wrapper_target(mdef.find("detaileddescription"))
                    or wrapper_target(mdef.find("inbodydescription"))
                    or wrapper_target(mdef.find("briefdescription")))
            if wrap:
                inherited_from_wrapper = list(method_uids.get(
                    "%s::%s" % (cname, wrap), []))
                if impl_iface:
                    for u in method_uids.get("%s::%s" % (impl_iface, wrap), []):
                        if u not in inherited_from_wrapper:
                            inherited_from_wrapper.append(u)
            # Effective requirements = class requirements UNION interface-method
            # UIDs UNION wrapper-sibling UIDs UNION method's own, de-duplicated.
            uids = list(dict.fromkeys(
                class_uids + inherited_from_iface
                + inherited_from_wrapper + own))
            # Own justification wins over an inherited class justification; a
            # justification never applies when the entity has requirement UIDs.
            just = own_just or (None if uids else class_just)
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
    paths = [p for p in sorted(glob.glob(os.path.join(xml_dir, "*.xml")))
             if os.path.basename(p) not in ("index.xml", "Doxyfile.xml")
             and not p.endswith(".xsd")]
    # Prepass: collect class/struct @requirement UIDs, @no_requirement
    # justifications, per-method UIDs and @implements targets so members, nested
    # types and implementing classes can inherit the right tags.
    enclosing_uids, enclosing_just, method_uids, class_implements, \
        container_classes = collect_class_tags(paths)
    for path in paths:
        for impl in parse_compound(path, emit_all=emit_all,
                                   enclosing_uids=enclosing_uids,
                                   enclosing_just=enclosing_just,
                                   method_uids=method_uids,
                                   class_implements=class_implements,
                                   container_classes=container_classes):
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
