# hsmcpp Requirements Grammar

## Element: SECTION

**Composite**: True

### Field: TITLE
**Type**: String
**Required**: True

## Element: REQUIREMENT

### Field: UID
**Type**: String
**Required**: True

### Field: TITLE
**Type**: String
**Required**: False

### Field: Nature
**Type**: SingleChoice(Functional, NonFunctional)
**Required**: True
**Human title**: Nature

### Field: Criticality
**Type**: SingleChoice(NonSafety, SafetyRelevant, ContextDependent)
**Required**: True
**Human title**: Criticality

### Field: Security
**Type**: SingleChoice(Yes, No)
**Required**: False
**Human title**: Security

### Field: Verification
**Type**: String
**Required**: True
**Human title**: Verification

### Field: Rationale
**Type**: String
**Required**: False
**Human title**: Rationale

### Field: STATEMENT
**Type**: String
**Required**: True

### Relations

#### Relation: Parent
