# Analysis Remediation Report

**Date**: 2026-02-10
**Scope**: .specify/spec.md, .specify/plan.md, .specify/data_model.md, contracts/openapi.yaml, .specify/tasks.md

## Actions Taken

### A1 Registration requires username (spec/UAT mismatch)
- Updated spec FR-001/FR-004 to require name, username, email, password.
- Updated UAT-UC01 steps to include username in registration inputs.

### A2 Required paper metadata not specified
- Updated spec FR-015 to explicitly require title, abstract, keywords.
- Updated data_model.md PaperSubmission metadata description to list required keys.
- Updated contracts/openapi.yaml PaperMetadata schema with required fields.

### A3 Required review fields not specified
- Updated data_model.md Review fields to review_text + recommendation.
- Updated contracts/openapi.yaml ReviewSubmissionRequest schema to require review_text + recommendation.

### A4 File format/size limits missing
- Updated spec FR-016 to accept PDF only, max 10 MB.
- Updated tasks US4 to enforce PDF-only and 10 MB limits in paper submission service.

### A5 Retention requirement partially covered
- Updated spec FR-040 to require delete/anonymize after 3 years.
- Updated tasks to add retention cleanup job and retention policy unit tests.

### A6 Path inconsistency between plan and .specify
- Updated .specify/plan.md to reference .specify paths.

### A7 Password complexity ambiguous
- Updated spec FR-003/FR-012 to define complexity (uppercase, lowercase, digit, symbol).

### A8 Notification requirements not specified
- Updated spec FR-025/FR-033/FR-038 to require email + in-app notifications.

### A9 FR list marker formatting
- Fixed malformed list markers for FR-003 and FR-012.

### A10 “Standard conference size” ambiguous
- Updated spec SC-004 to quantify size (<=1000 submissions, 50 rooms, 200 time slots).

## Files Updated

- .specify/spec.md
- .specify/plan.md
- .specify/data_model.md
- contracts/openapi.yaml
- .specify/tasks.md
