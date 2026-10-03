#!/usr/bin/env python3
"""Render the hsmcpp TRLC requirements to a single self-contained HTML file.

Loads requirements/ (the .rsl model plus the .trlc requirement files) via the
TRLC Python API and produces a filterable, cross-linked HTML view:

  * System tab   - system requirements grouped under their document SECTIONS,
                   each row linking to the software requirements derived from it.
  * Software tab - software requirements grouped under their sections, each row
                   linking back up to its parent system requirement.
  * Glossary tab - terminology.

Sections are rendered as a TREE-TABLE: collapsible section header rows are
interleaved with the requirement rows, preserving the specification's document
order and nesting.

Usage:
    python3 scripts/requirements/render_html.py [OUTPUT_HTML]

Default output: build/requirements/hsmcpp_requirements.html
"""
import json
import sys
from pathlib import Path

from trlc.errors import Message_Handler
from trlc.trlc import Source_Manager

REPO = Path(__file__).resolve().parents[2]
REQ_DIR = REPO / "requirements"


def field(obj, name, is_ref=False):
    val = obj.to_python_dict().get(name)
    if val is None:
        return None
    if hasattr(val, "name"):
        return val.name.split(".")[-1]
    s = str(val)
    # Record references serialize as "PACKAGE.RECORD_NAME"; keep only the name.
    if is_ref and "." in s:
        return s.split(".")[-1]
    return s


def section_path(obj):
    """Return the list of section titles enclosing a record, outermost first.

    TRLC stores obj.section as a list of Section nodes (outer..inner) or None.
    """
    secs = getattr(obj, "section", None)
    if not secs:
        return []
    return [s.name for s in secs]


def load():
    mh = Message_Handler()
    sm = Source_Manager(mh)
    sm.register_directory(str(REQ_DIR))
    symbols = sm.process()
    if symbols is None:
        print("TRLC processing failed", file=sys.stderr)
        sys.exit(1)
    return symbols


def build_data(with_trace=True):
    symbols = load()
    sys_reqs, sw_reqs, terms = [], [], []
    # Preserve document order (iter_record_objects yields in file order) so the
    # section grouping matches the specification's reading order.
    for obj in symbols.iter_record_objects():
        t = obj.n_typ.name
        if t == "System_Requirement":
            sys_reqs.append({
                "id": obj.name,
                "summary": field(obj, "summary") or "",
                "description": field(obj, "description") or "",
                "nature": field(obj, "nature") or "",
                "rationale": field(obj, "rationale") or "",
                "section": section_path(obj),
                "derived": [],  # filled below
            })
        elif t == "Software_Requirement":
            sw_reqs.append({
                "id": obj.name,
                "summary": field(obj, "summary") or "",
                "description": field(obj, "description") or "",
                "nature": field(obj, "nature") or "",
                "criticality": field(obj, "criticality") or "",
                "verification": field(obj, "verification") or "",
                "rationale": field(obj, "rationale") or "",
                "parent": field(obj, "parent", is_ref=True) or "",
                "section": section_path(obj),
            })
        elif t == "Term":
            terms.append({
                "term": field(obj, "term") or "",
                "definition": field(obj, "definition") or "",
                "context": field(obj, "context") or "",
            })

    # Inverse link: each system requirement's derived software requirements.
    by_id = {s["id"]: s for s in sys_reqs}
    for w in sw_reqs:
        parent = by_id.get(w["parent"])
        if parent is not None:
            parent["derived"].append(w["id"])

    # Static traceability: attach the code entities and tests that declare they
    # cover each software requirement (from the doxygen @requirement extraction
    # and the TEST_REQUIREMENTS source scan). This is DESIGN INTENT, not execution.
    if with_trace:
        code_by_req, tests_by_req = load_static_links()
    else:
        code_by_req, tests_by_req = {}, {}
    for w in sw_reqs:
        w["code"] = sorted(code_by_req.get(w["id"], []))
        w["tests"] = sorted(tests_by_req.get(w["id"], []))

    return {"sys": sys_reqs, "sw": sw_reqs, "terms": terms}


def _req_uid_from_ref(ref):
    """A lobster ref like 'req HSMCPP.SWR_HSM_040' -> 'SWR_HSM_040'."""
    tok = ref.split()[-1] if ref else ""
    return tok.split(".")[-1]


def load_static_links():
    """Read the static code.lobster and tests.lobster (if present in the build
    dir) and return (code_by_req, tests_by_req) mapping SWR UID -> [names]."""
    build_req = REPO / "build" / "requirements"
    code_by_req, tests_by_req = {}, {}

    def ingest(path, bucket, name_key="name"):
        if not path.exists():
            return
        try:
            data = json.loads(path.read_text()).get("data", [])
        except (ValueError, OSError):
            return
        for item in data:
            label = item.get(name_key) or item.get("tag", "")
            for ref in item.get("refs", []):
                bucket.setdefault(_req_uid_from_ref(ref), []).append(label)

    ingest(build_req / "code.lobster", code_by_req)
    # Always the STATIC test links (never the runtime tests.lobster that
    # req_trace writes), so the requirements doc reflects design intent only.
    static_tests = build_req / "tests_static.lobster"
    ingest(static_tests if static_tests.exists()
           else build_req / "tests.lobster", tests_by_req)
    return code_by_req, tests_by_req


TEMPLATE = """<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8">
<title>hsmcpp Requirements</title>
<style>
:root{{--bg:#14161c;--surface:#1c1f27;--card:#242833;--accent:#4f9cf9;--text:#e6e8ec;--muted:#9aa0ac;--warn:#e0a458;}}
*{{box-sizing:border-box;margin:0;padding:0}}
body{{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',system-ui,sans-serif;background:var(--bg);color:var(--text);padding:2rem;line-height:1.5}}
h1{{color:var(--accent)}}.sub{{color:var(--muted);margin-bottom:1.5rem}}
.stats{{display:flex;gap:1rem;margin-bottom:1.5rem;flex-wrap:wrap}}
.stat{{background:var(--surface);border-radius:8px;padding:.8rem 1.4rem;border-left:4px solid var(--accent)}}
.stat .n{{font-size:1.8rem;font-weight:700;color:var(--accent)}}.stat .l{{color:var(--muted);font-size:.8rem}}
.controls{{display:flex;gap:1rem;margin-bottom:1rem;flex-wrap:wrap;align-items:center}}
input[type=search],select{{background:var(--surface);border:1px solid #333;border-radius:6px;padding:.5rem .8rem;color:var(--text)}}
input[type=search]{{width:320px}}
.controls button{{background:var(--surface);border:1px solid #333;border-radius:6px;padding:.5rem .8rem;color:var(--muted);cursor:pointer}}
.controls button:hover{{color:var(--text);border-color:var(--accent)}}
.tabs{{display:flex;margin-bottom:1.2rem}}
.tab{{padding:.6rem 1.3rem;cursor:pointer;background:var(--surface);border:1px solid #333;color:var(--muted)}}
.tab:first-child{{border-radius:6px 0 0 6px}}.tab:last-child{{border-radius:0 6px 6px 0}}
.tab.active{{background:var(--card);color:var(--text);border-color:var(--accent)}}
.sec{{display:none}}.sec.active{{display:block}}
table{{width:100%;border-collapse:collapse;background:var(--surface);border-radius:8px;overflow:hidden}}
thead th{{position:sticky;top:0;z-index:2}}
th{{background:#2b3547;padding:.85rem .7rem;text-align:left;font-size:.72rem;letter-spacing:.6px;text-transform:uppercase;color:#cdd3dd;font-weight:700;border-bottom:3px solid var(--accent);box-shadow:0 2px 6px rgba(0,0,0,.35)}}
td{{padding:.6rem .7rem;border-top:1px solid #23252c;font-size:.9rem;vertical-align:top}}
tr.req:hover{{background:rgba(79,156,249,.06)}}
tr.sec-row td{{cursor:pointer;font-weight:600;color:var(--accent);padding:.5rem .7rem;
  background:linear-gradient(90deg,rgba(79,156,249,.14),rgba(79,156,249,.03));
  border-top:1px solid #23252c;border-left:3px solid var(--accent)}}
tr.sec-row:hover td{{background:linear-gradient(90deg,rgba(79,156,249,.22),rgba(79,156,249,.06))}}
tr.sec-row[data-depth="1"] td{{color:var(--text);border-left-color:#3d6ea5;
  background:linear-gradient(90deg,rgba(79,156,249,.08),rgba(79,156,249,.02));font-size:.86rem}}
tr.sec-row[data-depth="2"] td{{color:var(--muted);border-left-color:#31415a;background:rgba(79,156,249,.04)}}
tr.sec-row .chev{{display:inline-block;width:1rem;color:var(--accent)}}
.sec-count{{color:var(--muted);font-weight:400;font-size:.8rem;margin-left:.4rem}}
.badge{{display:inline-block;padding:.15rem .5rem;border-radius:10px;font-size:.72rem;font-weight:600;background:rgba(79,156,249,.15);color:var(--accent)}}
.badge.nf{{background:rgba(224,164,88,.18);color:var(--warn)}}
.link{{color:var(--accent);cursor:pointer;text-decoration:underline}}
.derived .link{{margin-right:.35rem}}
.desc{{max-width:640px}}.rat{{color:var(--muted);font-size:.82rem;margin-top:.3rem}}
.none{{color:var(--muted)}}
.tcode,.ttest{{max-width:240px;font-size:.82rem}}
.trace-code{{color:#8bd49c;font-family:ui-monospace,Menlo,Consolas,monospace;font-size:.78rem;word-break:break-all}}
.trace-test{{color:#c39bd3;font-family:ui-monospace,Menlo,Consolas,monospace;font-size:.78rem;word-break:break-all}}
.gap{{color:var(--warn);font-size:.8rem}}
.stat.gapcard{{border-left-color:var(--warn)}}.stat.gapcard .n{{color:var(--warn)}}
#swfilters{{display:none;gap:1rem;align-items:center}}
body.tab-sw #swfilters{{display:flex}}
#swfilters label{{display:flex;align-items:center;gap:.35rem;color:var(--muted);font-size:.85rem;cursor:pointer}}
#swfilters label:hover{{color:var(--text)}}
#swfilters input[type=checkbox]{{accent-color:var(--accent);cursor:pointer}}
</style></head><body>
<h1>hsmcpp Requirements Specification</h1>
<p class="sub">Generated from TRLC (BMW format) &mdash; {n_sys} system &middot; {n_sw} software requirements.{trace_note}</p>
<div class="stats">
 <div class="stat"><div class="n">{n_sys}</div><div class="l">System Requirements</div></div>
 <div class="stat"><div class="n">{n_sw}</div><div class="l">Software Requirements</div></div>
 <div class="stat"><div class="n">{n_terms}</div><div class="l">Glossary Terms</div></div>
 <div class="stat"><div class="n">{cov}%</div><div class="l">SYS with SW child</div></div>
 {trace_cards}
</div>
<div class="controls">
 <input type="search" id="q" placeholder="Search requirements...">
 <select id="fnat"><option value="">All natures</option><option>Functional</option><option>NonFunctional</option></select>
 <button id="expand">Expand all</button><button id="collapse">Collapse all</button>
 {trace_filters}
</div>
<div class="tabs">
 <div class="tab active" data-t="sys">System</div>
 <div class="tab" data-t="sw">Software</div>
 <div class="tab" data-t="term">Glossary</div>
</div>
<div class="sec active" id="s-sys"><table><thead><tr><th>UID</th><th>Title</th><th>Nature</th><th>Statement</th><th>Derived SW</th></tr></thead><tbody id="b-sys"></tbody></table></div>
<div class="sec" id="s-sw"><table><thead><tr><th>UID</th><th>Title</th><th>Nature</th><th>Crit.</th><th>Verif.</th><th>Parent</th><th>Statement</th>{trace_th}</tr></thead><tbody id="b-sw"></tbody></table></div>
<div class="sec" id="s-term"><table><thead><tr><th>Term</th><th>Context</th><th>Definition</th></tr></thead><tbody id="b-term"></tbody></table></div>
<script>
const D={data_json};
const TRACE={trace_js};
const SYS_COLS=5, SW_COLS=TRACE?9:7;
function natBadge(n){{return `<span class="badge ${{n==='NonFunctional'?'nf':''}}">${{n}}</span>`;}}

// Build a stable section key from a path so rows can be toggled together.
function secKey(path){{return path.join(' \u203a ');}}

function rowSys(r){{
  const derived = r.derived.length
    ? `<span class="derived">${{r.derived.map(id=>`<span class="link" onclick="jump('sw','${{id}}')">${{id}}</span>`).join('')}}</span>`
    : '<span class="none">&mdash;</span>';
  return `<tr class="req" data-id="${{r.id}}" data-sec="${{secKey(r.section)}}"><td><strong>${{r.id}}</strong></td><td>${{r.summary}}</td><td>${{natBadge(r.nature)}}</td><td class="desc">${{r.description}}${{r.rationale?`<div class="rat"><em>Rationale:</em> ${{r.rationale}}</div>`:''}}</td><td>${{derived}}</td></tr>`;
}}
function rowSw(r){{
  const parent = r.parent
    ? `<span class="link" onclick="jump('sys','${{r.parent}}')">${{r.parent}}</span>`
    : '<span class="none">&mdash;</span>';
  const codeCell = (r.code&&r.code.length)
    ? r.code.map(c=>`<div class="trace-code">${{c}}</div>`).join('')
    : '<span class="gap">&#9888; none</span>';
  const testCell = (r.tests&&r.tests.length)
    ? r.tests.map(t=>`<div class="trace-test">${{t}}</div>`).join('')
    : '<span class="gap">&#9888; none</span>';
  return `<tr class="req" data-id="${{r.id}}" data-sec="${{secKey(r.section)}}"><td><strong>${{r.id}}</strong></td><td>${{r.summary}}</td><td>${{natBadge(r.nature)}}</td><td>${{r.criticality}}</td><td>${{r.verification}}</td><td>${{parent}}</td><td class="desc">${{r.description}}${{r.rationale?`<div class="rat"><em>Rationale:</em> ${{r.rationale}}</div>`:''}}</td>${{TRACE?`<td class="tcode">${{codeCell}}</td><td class="ttest">${{testCell}}</td>`:''}}</tr>`;
}}
function rowTerm(r){{return `<tr class="req"><td><strong>${{r.term}}</strong></td><td>${{r.context}}</td><td class="desc">${{r.definition}}</td></tr>`;}}

// Render a list of requirements as a tree-table: emit a collapsible header row
// whenever the section path changes, then the requirement rows beneath it.
function renderTree(items, rowFn, cols){{
  let html='', prev=[];
  items.forEach(r=>{{
    const path=r.section||[];
    // find first index where path diverges from prev
    let i=0; while(i<path.length && i<prev.length && path[i]===prev[i]) i++;
    for(let d=i; d<path.length; d++){{
      const sub=path.slice(0,d+1); const key=secKey(sub);
      html+=`<tr class="sec-row" data-key="${{key}}" data-depth="${{d}}" onclick="toggleSec(this)"><td colspan="${{cols}}"><span style="padding-left:${{d*1.2}}rem"><span class="chev">&#9662;</span>${{sub[d]}}</span></td></tr>`;
    }}
    prev=path;
    html+=rowFn(r);
  }});
  return html;
}}

function jump(t,id){{
  selectTab(t);
  const r=document.querySelector(`#s-${{t}} tr[data-id="${{id}}"]`);
  if(r){{ // make sure its section is expanded
    r.style.display=''; r.style.background='rgba(79,156,249,.2)';
    r.scrollIntoView({{block:'center'}}); setTimeout(()=>r.style.background='',2000);
  }}
}}

// Collapse/expand: hide requirement + nested section rows that belong under a
// section header until the next header at the same or shallower depth.
function toggleSec(hdr){{
  const depth=+hdr.dataset.depth;
  const collapsed=hdr.classList.toggle('collapsed');
  hdr.querySelector('.chev').innerHTML = collapsed ? '&#9656;' : '&#9662;';
  let n=hdr.nextElementSibling;
  while(n){{
    if(n.classList.contains('sec-row')){{
      if(+n.dataset.depth<=depth) break;
      n.style.display = collapsed ? 'none' : '';
      if(collapsed){{ n.classList.add('collapsed'); n.querySelector('.chev').innerHTML='&#9656;'; }}
    }} else {{
      n.style.display = collapsed ? 'none' : '';
    }}
    n=n.nextElementSibling;
  }}
}}
function setAll(collapse){{
  document.querySelectorAll('.sec.active .sec-row').forEach(h=>{{
    const isCol=h.classList.contains('collapsed');
    if(collapse!==isCol) toggleSec(h);
  }});
}}

function apply(){{
  const q=document.getElementById('q').value.toLowerCase();
  const nat=document.getElementById('fnat').value;
  const noCodeEl=document.getElementById('fnocode');
  const noTestEl=document.getElementById('fnotest');
  const noCode=noCodeEl&&noCodeEl.checked;
  const noTest=noTestEl&&noTestEl.checked;
  const base=r=>(!q||JSON.stringify(r).toLowerCase().includes(q))&&(!nat||r.nature===nat);
  const swMatch=r=>base(r)
    &&(!noCode||!(r.code&&r.code.length))
    &&(!noTest||!(r.tests&&r.tests.length));
  document.getElementById('b-sys').innerHTML=renderTree(D.sys.filter(base), rowSys, SYS_COLS);
  document.getElementById('b-sw').innerHTML=renderTree(D.sw.filter(swMatch), rowSw, SW_COLS);
  document.getElementById('b-term').innerHTML=D.terms.filter(r=>!q||JSON.stringify(r).toLowerCase().includes(q)).map(rowTerm).join('');
}}
function selectTab(t){{
  document.querySelectorAll('.tab').forEach(x=>x.classList.remove('active'));
  document.querySelectorAll('.sec').forEach(x=>x.classList.remove('active'));
  document.querySelector(`.tab[data-t="${{t}}"]`).classList.add('active');
  document.getElementById('s-'+t).classList.add('active');
  document.body.classList.toggle('tab-sw', t==='sw');
}}
document.querySelectorAll('.tab').forEach(t=>t.addEventListener('click',()=>selectTab(t.dataset.t)));
document.getElementById('q').addEventListener('input',apply);
document.getElementById('fnat').addEventListener('change',apply);
document.getElementById('expand').addEventListener('click',()=>setAll(false));
document.getElementById('collapse').addEventListener('click',()=>setAll(true));
['fnocode','fnotest'].forEach(id=>{{const el=document.getElementById(id);if(el)el.addEventListener('change',apply);}});
apply();
</script></body></html>
"""


def render(out: Path, with_trace=True):
    data = build_data(with_trace=with_trace)
    n_sys = len(data["sys"])
    n_sw = len(data["sw"])
    covered = len({w["parent"] for w in data["sw"]} & {s["id"] for s in data["sys"]})
    cov = round(covered / n_sys * 100) if n_sys else 0
    sw_with_code = sum(1 for w in data["sw"] if w.get("code"))
    sw_with_test = sum(1 for w in data["sw"] if w.get("tests"))
    trace_cards = (
        f'<div class="stat gapcard"><div class="n">{sw_with_code}/{n_sw}</div>'
        f'<div class="l">SW with Code link</div></div>\n'
        f' <div class="stat gapcard"><div class="n">{sw_with_test}/{n_sw}</div>'
        f'<div class="l">SW with test link</div></div>'
    ) if with_trace else ""
    trace_filters = (
        '<span id="swfilters">'
        '<label><input type="checkbox" id="fnocode">Not linked with code</label>'
        '<label><input type="checkbox" id="fnotest">Not linked with tests</label>'
        '</span>'
    ) if with_trace else ""
    doc = TEMPLATE.format(
        n_sys=n_sys, n_sw=n_sw, n_terms=len(data["terms"]), cov=cov,
        data_json=json.dumps(data),
        trace_note=(" Code &amp; test links are STATIC (design intent from source"
                    " tags); execution evidence lives in the LOBSTER coverage"
                    " report." if with_trace else ""),
        trace_th=("<th>Implemented by</th><th>Verified by</th>"
                  if with_trace else ""),
        trace_js=("true" if with_trace else "false"),
        trace_cards=trace_cards,
        trace_filters=trace_filters,
    )
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(doc)
    mode = "with code/test tracing" if with_trace else "requirements only"
    print(f"Wrote {out} ({n_sys} SYS, {n_sw} SW, {len(data['terms'])} terms; {mode})")


if __name__ == "__main__":
    # Usage: render_html.py [--no-trace] [OUTPUT_HTML]
    args = sys.argv[1:]
    with_trace = True
    if "--no-trace" in args:
        with_trace = False
        args = [a for a in args if a != "--no-trace"]
    target = Path(args[0]) if args \
        else REPO / "build" / "requirements" / "hsmcpp_requirements.html"
    render(target, with_trace=with_trace)
