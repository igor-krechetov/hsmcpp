# Development Guide

## Requirements Management with StrictDoc

The hsmcpp project uses [StrictDoc](https://strictdoc.readthedocs.io/) to manage system-level requirements. Requirements are written in Markdown format and stored in the `requirements/` directory.

### Directory Structure

```
requirements/
└── system.md          # System-level requirements for hsmcpp
```

### Installing StrictDoc

StrictDoc requires Python 3.7+.

```bash
pip install strictdoc
```

Or install in a virtual environment:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install strictdoc
```

### Validating Requirements

To check that the requirements document is well-formed and parseable:

```bash
strictdoc export requirements/ --output-dir build/requirements
```

If the document has structural issues, StrictDoc will report parsing errors.

### Exporting to HTML

Generate an HTML report for review:

```bash
strictdoc export requirements/ --output-dir build/requirements
```

Open `build/requirements/html/index.html` in a browser to review the rendered requirements with traceability information.

### Generating Traceability Reports

StrictDoc can produce traceability matrices showing requirement coverage:

```bash
strictdoc export requirements/ --output-dir build/requirements --view=traceability
```

### Editing Requirements

Requirements use StrictDoc's Markdown format. Each requirement is a `###` heading (node title), followed by metadata fields as `**Key**: value` lines, then the statement as body text.

Template for a new requirement:

```markdown
### Requirement Title

**UID**: SYS-HSM-NNN
**Nature**: Functional
**ASIL**: QM
**Verification**: Test
**Rationale**: Why this requirement exists.

The library SHALL ...
```

Notes:
- No `\` between meta fields — each field is its own line
- `**Security**: yes` only when applicable (default is no, omit when no)
- Rationale is kept in the metadata block (before the statement body)
- Sections use `##` headings with `**Type**: SECTION`

### Custom Fields

Each requirement carries the following metadata:

| Field | Values | Default | Description |
|-------|--------|---------|-------------|
| **Nature** | Functional, NonFunctional | — | Whether the requirement describes behavior or a quality attribute |
| **ASIL** | QM, A, B, C, D | QM | Automotive Safety Integrity Level per ISO 26262 |
| **Security** | yes, no | no | Whether the requirement has security implications (omit if no) |
| **Verification** | Test, Analysis, Inspection, Review | — | Method used to verify the requirement is met |

### Requirement ID Scheme

UIDs use a single continuous sequence within the `SYS-HSM-` namespace:

```
SYS-HSM-001, SYS-HSM-002, ..., SYS-HSM-078, ...
```

No ranges are reserved per category. New requirements simply get the next available number.

When adding new requirements:

1. Assign the next sequential UID (e.g., if last is SYS-HSM-078, use SYS-HSM-079)
2. Place the requirement in the appropriate section
3. Fill in all custom fields (Nature, ASIL, Verification; Security only if yes)
4. Write a clear, testable statement using SHALL
5. Provide a rationale explaining the "why"

## Building the Library

See [README.md](README.md) for build instructions and platform support details.
