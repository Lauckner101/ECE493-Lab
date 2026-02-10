<!--
Sync Impact Report
- Version change: 0.1.0 → 0.2.0
- Modified principles: None
- Added sections: None
- Removed sections: None
- Templates requiring updates:
  - ⚠ pending: .specify/templates/commands/*.md (directory not found)
  - ⚠ pending: runtime guidance docs (README.md/docs/* not found)
- Follow-up TODOs: TODO(RATIFICATION_DATE): original adoption date not found in repo
-->
# Conference Management System (CMS) Constitution

## Core Principles

### I. Use-Case Driven Specification
All specifications MUST be derived from the set of `UC-XX.md` files in the
repository root. Every element in each use case (actors, preconditions, flows,
extensions, and outcomes) MUST be captured in the feature specification or
explicitly marked as out of scope with justification. This guarantees the spec
reflects the agreed problem statements.

### II. Acceptance-Test Union
The project MUST maintain a user acceptance test suite that is the union of all
acceptance test suites defined inside the `UC-XX.md` files. The union MUST be
kept up to date whenever a use case changes and MUST be referenced by the
feature specification. This ensures end-to-end coverage of agreed behavior.

### III. Traceability & Independent Testability
Every requirement and user story MUST trace back to one or more use cases and
be independently testable. Each user story MUST define acceptance scenarios
that can be validated without implementing other stories. This keeps delivery
incremental and verifiable.

### IV. Security & Privacy Baseline
Any feature involving authentication, identity, payments, or personal data MUST
specify security requirements (e.g., password policy, access rules, audit needs)
and privacy handling (data retention, exposure limits). This sets a minimum
baseline for trust and compliance.

### V. Role-Constraint Enforcement
Role constraints and system invariants stated in use cases (e.g., reviewer
limits, assignment caps, role permissions) MUST be enforced in requirements and
validated through acceptance tests. This preserves critical business rules.

## Project Constraints

- Specifications and plans MUST be written in Markdown using the templates in
  `.specify/templates/`.
- Use cases live as `UC-XX.md` files in the repository root and are the source
  of truth for user-facing behavior.
- The consolidated user acceptance test suite MUST be included in the feature
  specification (or a referenced appendix) and kept consistent with use cases.
- The CMS application MUST be implemented in C++ and MUST follow the coding
  standards in `cpp_style_guide_google.md`.

## Workflow & Quality Gates

- A Constitution Check MUST be completed before research or design work begins
  for a feature and again before implementation tasks are generated.
- All changes MUST be reviewed for compliance with the Core Principles.
- If a principle is violated, the plan MUST document the violation and the
  approved justification.

## Governance

- This constitution supersedes all other guidance. Any conflict MUST be
  resolved in favor of the constitution.
- Amendments require a documented rationale, impact analysis, and version bump
  per semantic versioning rules (MAJOR/MINOR/PATCH).
- Every plan/spec/tasks artifact MUST include a Constitution Check section and
  reviewers MUST verify compliance before approval.

**Version**: 0.2.0 | **Ratified**: TODO(RATIFICATION_DATE): original adoption date not found in repo | **Last Amended**: 2026-02-10
