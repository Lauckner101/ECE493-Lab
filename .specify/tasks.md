---

description: "Task list for CMS implementation"
---

# Tasks: Use Case Flow Specification

**Input**: Design documents from `.specify/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), data_model.md, contracts/

**Tests**: Tests are REQUIRED because the specification includes acceptance tests.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [X] T001 Create project structure per plan in `src/` and `tests/`
- [X] T002 Initialize CMake build with dependencies in `CMakeLists.txt`
- [X] T003 [P] Add Google C++ style config in `.clang-format`
- [X] T004 [P] Add test runner configuration in `tests/CMakeLists.txt`
- [X] T005 [P] Add environment config template in `config/app.yaml`

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**⚠️ CRITICAL**: No user story work can begin until this phase is complete

- [X] T006 Create database schema and migrations in `db/schema.sql`
- [X] T007 Implement DB connection and migration runner in `src/services/db.cpp`
- [X] T008 Create base UserAccount model in `src/models/user_account.cpp`
- [X] T009 Create Conference, Room, TimeSlot models in `src/models/conference.cpp`
- [X] T010 Implement request validation helpers in `src/lib/validation.cpp`
- [X] T011 Implement auth/session middleware in `src/api/auth_middleware.cpp`
- [X] T012 Implement error response helpers in `src/api/http_errors.cpp`
- [X] T013 Implement logging setup in `src/lib/logging.cpp`
- [X] T014 Implement notification service interface in `src/services/notification_service.cpp`

**Checkpoint**: Foundation ready - user story implementation can now begin in parallel

---

## Phase 3: User Story 1 - Register User Account (Priority: P1) 🎯 MVP

**Goal**: Allow guests to register a new account with password validation and unique email

**Independent Test**: A guest can register and is redirected to login; duplicates are blocked

### Tests for User Story 1

- [X] T015 [P] [US1] Add UAT registration tests in `tests/acceptance/uat_uc01_registration.cpp`
- [X] T016 [P] [US1] Add contract test for POST /auth/register in `tests/contract/auth_register_contract.cpp`
- [X] T017 [P] [US1] Add integration test for registration flow in `tests/integration/registration_flow.cpp`

### Implementation for User Story 1

- [X] T018 [P] [US1] Add user repository in `src/services/user_repository.cpp`
- [X] T019 [US1] Implement registration service in `src/services/auth_service.cpp`
- [X] T020 [US1] Implement register endpoint in `src/api/auth_controller.cpp`
- [X] T021 [US1] Wire route for POST /auth/register in `src/api/routes.cpp`
- [X] T022 [US1] Add password complexity validation in `src/lib/validation.cpp`

**Checkpoint**: User Story 1 is independently testable

---

## Phase 4: User Story 2 - Log In User (Priority: P1)

**Goal**: Allow registered users to log in and receive an authenticated session

**Independent Test**: User can log in with valid credentials and access protected pages

### Tests for User Story 2

- [X] T023 [P] [US2] Add UAT login tests in `tests/acceptance/uat_uc02_login.cpp`
- [X] T024 [P] [US2] Add contract test for POST /auth/login in `tests/contract/auth_login_contract.cpp`
- [X] T025 [P] [US2] Add integration test for login flow in `tests/integration/login_flow.cpp`

### Implementation for User Story 2

- [X] T026 [P] [US2] Implement session store in `src/services/session_service.cpp`
- [X] T027 [US2] Implement login service in `src/services/auth_service.cpp`
- [X] T028 [US2] Implement login endpoint in `src/api/auth_controller.cpp`
- [X] T029 [US2] Wire route for POST /auth/login in `src/api/routes.cpp`

**Checkpoint**: User Story 2 is independently testable

---

## Phase 5: User Story 3 - Change User Password (Priority: P1)

**Goal**: Allow logged-in users to change their password with validation

**Independent Test**: User can change password and old password is rejected

### Tests for User Story 3

- [X] T030 [P] [US3] Add UAT change password tests in `tests/acceptance/uat_uc03_password_change.cpp`
- [X] T031 [P] [US3] Add contract test for POST /auth/password/change in `tests/contract/auth_password_change_contract.cpp`
- [X] T032 [P] [US3] Add integration test for password change flow in `tests/integration/password_change_flow.cpp`

### Implementation for User Story 3

- [X] T033 [US3] Implement password change service in `src/services/auth_service.cpp`
- [X] T034 [US3] Implement password change endpoint in `src/api/auth_controller.cpp`
- [X] T035 [US3] Wire route for POST /auth/password/change in `src/api/routes.cpp`

**Checkpoint**: User Story 3 is independently testable

---

## Phase 6: User Story 4 - Submit Paper Manuscript (Priority: P2)

**Goal**: Allow authors to submit papers with metadata and manuscript files

**Independent Test**: Valid submissions are stored; invalid metadata or files are blocked

### Tests for User Story 4

- [X] T036 [P] [US4] Add UAT submission tests in `tests/acceptance/uat_uc04_submit_paper.cpp`
- [X] T037 [P] [US4] Add contract test for POST /papers in `tests/contract/papers_submit_contract.cpp`
- [X] T038 [P] [US4] Add integration test for submission flow in `tests/integration/paper_submit_flow.cpp`

### Implementation for User Story 4

- [X] T039 [P] [US4] Create PaperSubmission model in `src/models/paper_submission.cpp`
- [X] T040 [US4] Implement paper submission service with PDF-only and 10 MB limits in `src/services/paper_service.cpp`
- [X] T041 [US4] Implement submit endpoint in `src/api/paper_controller.cpp`
- [X] T042 [US4] Wire route for POST /papers in `src/api/routes.cpp`

**Checkpoint**: User Story 4 is independently testable

---

## Phase 7: User Story 5 - Save Paper Submission Draft (Priority: P2)

**Goal**: Allow authors to save drafts and submit them later

**Independent Test**: Drafts are saved, not submitted, and empty drafts are rejected

### Tests for User Story 5

- [X] T043 [P] [US5] Add UAT draft tests in `tests/acceptance/uat_uc05_save_draft.cpp`
- [X] T044 [P] [US5] Add contract test for POST /papers/drafts in `tests/contract/papers_draft_contract.cpp`
- [X] T045 [P] [US5] Add contract test for PUT /papers/{paperId}/submit in `tests/contract/papers_submit_draft_contract.cpp`

### Implementation for User Story 5

- [X] T046 [US5] Implement draft save service in `src/services/paper_service.cpp`
- [X] T047 [US5] Implement draft submit service in `src/services/paper_service.cpp`
- [X] T048 [US5] Implement draft endpoints in `src/api/paper_controller.cpp`
- [X] T049 [US5] Wire routes for draft endpoints in `src/api/routes.cpp`

**Checkpoint**: User Story 5 is independently testable

---

## Phase 8: User Story 6 - Assign Referees to Submitted Paper (Priority: P2)

**Goal**: Allow editors to assign exactly three referees and send invitations

**Independent Test**: Three unique referees can be assigned and invitations sent

### Tests for User Story 6

- [X] T050 [P] [US6] Add UAT referee assignment tests in `tests/acceptance/uat_uc06_assign_referees.cpp`
- [X] T051 [P] [US6] Add contract test for POST /papers/{paperId}/referees in `tests/contract/referee_assign_contract.cpp`
- [X] T052 [P] [US6] Add integration test for assignment flow in `tests/integration/referee_assign_flow.cpp`

### Implementation for User Story 6

- [X] T053 [P] [US6] Create RefereeAssignment model in `src/models/referee_assignment.cpp`
- [X] T054 [US6] Implement referee assignment service in `src/services/referee_service.cpp`
- [X] T055 [US6] Implement assignment endpoint in `src/api/referee_controller.cpp`
- [X] T056 [US6] Wire route for POST /papers/{paperId}/referees in `src/api/routes.cpp`

**Checkpoint**: User Story 6 is independently testable

---

## Phase 9: User Story 7 - Accept or Reject Review Invitation (Priority: P2)

**Goal**: Allow referees to accept or reject invitations with workload checks

**Independent Test**: Invitations can be accepted or rejected with constraints enforced

### Tests for User Story 7

- [X] T057 [P] [US7] Add UAT invitation response tests in `tests/acceptance/uat_uc07_invitation_response.cpp`
- [X] T058 [P] [US7] Add contract test for POST /invitations/{id}/accept in `tests/contract/invitation_accept_contract.cpp`
- [X] T059 [P] [US7] Add contract test for POST /invitations/{id}/reject in `tests/contract/invitation_reject_contract.cpp`

### Implementation for User Story 7

- [X] T060 [US7] Implement invitation response service in `src/services/referee_service.cpp`
- [X] T061 [US7] Implement invitation endpoints in `src/api/referee_controller.cpp`
- [X] T062 [US7] Wire routes for invitation endpoints in `src/api/routes.cpp`

**Checkpoint**: User Story 7 is independently testable

---

## Phase 10: User Story 8 - Review Assigned Paper (Priority: P2)

**Goal**: Allow referees to submit reviews for assigned papers

**Independent Test**: Review submissions are validated, stored, and visible to editors

### Tests for User Story 8

- [X] T063 [P] [US8] Add UAT review submission tests in `tests/acceptance/uat_uc08_submit_review.cpp`
- [X] T064 [P] [US8] Add contract test for POST /papers/{paperId}/reviews in `tests/contract/review_submit_contract.cpp`
- [X] T065 [P] [US8] Add integration test for review flow in `tests/integration/review_submit_flow.cpp`

### Implementation for User Story 8

- [X] T066 [P] [US8] Create Review model in `src/models/review.cpp`
- [X] T067 [US8] Implement review submission service in `src/services/review_service.cpp`
- [X] T068 [US8] Implement review endpoint in `src/api/review_controller.cpp`
- [X] T069 [US8] Wire route for POST /papers/{paperId}/reviews in `src/api/routes.cpp`

**Checkpoint**: User Story 8 is independently testable

---

## Phase 11: User Story 9 - Make Final Paper Decision (Priority: P2)

**Goal**: Allow editors to record final decisions after reviews are complete

**Independent Test**: Accept/reject decisions are stored and authors are notified

### Tests for User Story 9

- [X] T070 [P] [US9] Add UAT decision tests in `tests/acceptance/uat_uc09_decision.cpp`
- [X] T071 [P] [US9] Add contract test for POST /papers/{paperId}/decision in `tests/contract/decision_contract.cpp`
- [X] T072 [P] [US9] Add integration test for decision flow in `tests/integration/decision_flow.cpp`

### Implementation for User Story 9

- [X] T073 [P] [US9] Create Decision model in `src/models/decision.cpp`
- [X] T074 [US9] Implement decision service in `src/services/decision_service.cpp`
- [X] T075 [US9] Implement decision endpoint in `src/api/decision_controller.cpp`
- [X] T076 [US9] Wire route for POST /papers/{paperId}/decision in `src/api/routes.cpp`

**Checkpoint**: User Story 9 is independently testable

---

## Phase 12: User Story 10 - Generate Conference Schedule (Priority: P3)

**Goal**: Allow administrators to generate a deterministic schedule

**Independent Test**: Generated schedule is stored and viewable

### Tests for User Story 10

- [X] T077 [P] [US10] Add UAT schedule generation tests in `tests/acceptance/uat_uc10_generate_schedule.cpp`
- [X] T078 [P] [US10] Add contract test for POST /schedule/generate in `tests/contract/schedule_generate_contract.cpp`

### Implementation for User Story 10

- [X] T079 [P] [US10] Create Schedule and ScheduleItem models in `src/models/schedule.cpp`
- [X] T080 [US10] Implement scheduling service in `src/services/schedule_service.cpp`
- [X] T081 [US10] Implement schedule generation endpoint in `src/api/schedule_controller.cpp`
- [X] T082 [US10] Wire route for POST /schedule/generate in `src/api/routes.cpp`

**Checkpoint**: User Story 10 is independently testable

---

## Phase 13: User Story 11 - Edit Conference Schedule (Priority: P3)

**Goal**: Allow editors to edit the current schedule with conflict validation

**Independent Test**: Schedule edits persist and conflicts are blocked

### Tests for User Story 11

- [X] T083 [P] [US11] Add UAT schedule edit tests in `tests/acceptance/uat_uc11_edit_schedule.cpp`
- [X] T084 [P] [US11] Add contract test for PUT /schedule/current in `tests/contract/schedule_edit_contract.cpp`

### Implementation for User Story 11

- [X] T085 [US11] Implement schedule edit service in `src/services/schedule_service.cpp`
- [X] T086 [US11] Implement schedule edit endpoint in `src/api/schedule_controller.cpp`
- [X] T087 [US11] Wire route for PUT /schedule/current in `src/api/routes.cpp`

**Checkpoint**: User Story 11 is independently testable

---

## Phase 14: User Story 12 - Register for Conference Attendance (Priority: P3)

**Goal**: Allow attendees to register and complete payment

**Independent Test**: Registration and payment records are created; ticket delivered

### Tests for User Story 12

- [X] T088 [P] [US12] Add UAT registration/payment tests in `tests/acceptance/uat_uc12_registration.cpp`
- [X] T089 [P] [US12] Add contract test for POST /conference/registration in `tests/contract/registration_contract.cpp`
- [X] T090 [P] [US12] Add contract test for POST /conference/registration/{id}/pay in `tests/contract/payment_contract.cpp`
- [X] T091 [P] [US12] Add integration test for registration/payment flow in `tests/integration/registration_payment_flow.cpp`

### Implementation for User Story 12

- [X] T092 [P] [US12] Create Registration, PaymentRecord, Ticket models in `src/models/registration.cpp`
- [X] T093 [US12] Implement registration service in `src/services/registration_service.cpp`
- [X] T094 [US12] Implement payment service in `src/services/payment_service.cpp`
- [X] T095 [US12] Implement registration endpoints in `src/api/registration_controller.cpp`
- [X] T096 [US12] Wire routes for registration/payment in `src/api/routes.cpp`

**Checkpoint**: User Story 12 is independently testable

---

## Phase 15: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [ ] T097 [P] Update API docs in `contracts/openapi.yaml`
- [ ] T098 [P] Update quickstart documentation in `.specify/quickstart.md`
- [ ] T099 Add data retention cleanup job in `src/services/retention_service.cpp`
- [ ] T100 [P] Add retention policy unit tests in `tests/unit/retention_policy_tests.cpp`
- [ ] T101 [P] Add additional unit tests for shared validation in `tests/unit/validation_tests.cpp`

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3+)**: All depend on Foundational phase completion
- **Polish (Final Phase)**: Depends on all desired user stories being complete

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Foundational (Phase 2)
- **User Story 2 (P1)**: Can start after Foundational (Phase 2)
- **User Story 3 (P1)**: Can start after Foundational (Phase 2)
- **User Story 4 (P2)**: Can start after Foundational (Phase 2)
- **User Story 5 (P2)**: Depends on User Story 4 model/service components
- **User Story 6 (P2)**: Can start after Foundational (Phase 2)
- **User Story 7 (P2)**: Depends on User Story 6 assignments
- **User Story 8 (P2)**: Depends on User Story 7 accepted invitations
- **User Story 9 (P2)**: Depends on User Story 8 completed reviews
- **User Story 10 (P3)**: Depends on User Story 9 accepted papers
- **User Story 11 (P3)**: Depends on User Story 10 generated schedule
- **User Story 12 (P3)**: Can start after Foundational (Phase 2)

### Within Each User Story

- Tests MUST be written and FAIL before implementation
- Models before services
- Services before endpoints
- Core implementation before integration
- Story complete before moving to next priority

### Parallel Opportunities

- All Setup tasks marked [P] can run in parallel
- Foundational tasks marked [P] can run in parallel
- Tests for each story can run in parallel
- Different user stories can be worked on in parallel after Phase 2 (subject to dependencies)

---

## Parallel Example: User Story 1

```bash
Task: "Add UAT registration tests in tests/acceptance/uat_uc01_registration.cpp"
Task: "Add contract test for POST /auth/register in tests/contract/auth_register_contract.cpp"
Task: "Add integration test for registration flow in tests/integration/registration_flow.cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational
3. Complete Phase 3: User Story 1
4. Stop and validate User Story 1 independently

### Incremental Delivery

1. Complete Setup + Foundational
2. Add User Stories 1–3 (auth)
3. Add User Stories 4–6 (submission + referee assignment)
4. Add User Stories 7–9 (review + decision)
5. Add User Stories 10–12 (schedule + registration)
6. Complete Polish phase
