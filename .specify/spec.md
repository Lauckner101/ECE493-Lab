# Feature Specification: Use Case Flow Specification

**Feature Branch**: `001-use-case-spec`  
**Created**: 2026-02-10  
**Status**: Draft  
**Input**: User description: "Use the use case flows in the use case files UC-XX.md as the source of truth. Copy the flows directly into spec.md with only style/grammer edits. Then extract functional requirements from those flows. Do not invent new flows"

## Clarifications

### Session 2026-02-10

- Q: What password policy should the CMS enforce? → A: Minimum 8 chars, complexity required.
- Q: For paper submissions, do you want drafts to have an expiration? → A: Drafts never expire.
- Q: Does conference registration allow multiple attendance types per attendee? → A: Exactly one type per attendee.
- Q: Should schedule generation be deterministic (same inputs always yield the same schedule)? → A: Yes, deterministic for same inputs.
- Q: When an author clicks Save with no information entered, which behavior should apply? → A: Show warning, do not save.
- Q: What data retention policy should the CMS use for user accounts, submissions, reviews, and payment records? → A: Retain for 3 years after conference end.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Register User Account (Priority: P1)

A guest registers a new account to access CMS features.

**Why this priority**: Account creation is a prerequisite for all authenticated workflows.

**Independent Test**: Can be tested by completing registration and verifying account creation and redirect.

**Flow (From UC-01)**

**Main Success Scenario**
1. The guest selects the **Register** option.
2. The system displays the user registration form.
3. The guest enters all required registration information.
4. The guest submits the registration form.
5. The system validates the email address and password.
6. The system stores the new user account information.
7. The system confirms successful registration and redirects the user to the login page.

**Extensions**
5a: The email address is already registered. The system displays an error message and
prompts the guest to enter a different email.
5b: The password does not meet security requirements. The system displays password
rules and prompts the guest to enter a valid password.
3a: Required fields are missing. The system highlights missing fields and requests
completion.

**Acceptance Scenarios**:
1. **Given** a guest on the registration form, **When** they submit valid registration
   information, **Then** a new account is created and they are redirected to login.
2. **Given** an email already registered, **When** the guest submits the form, **Then**
   registration is blocked and an error message is shown.
3. **Given** a password that fails security rules, **When** the guest submits the form,
   **Then** the system displays password rules and blocks registration.
4. **Given** missing required fields, **When** the guest submits the form, **Then**
   missing fields are highlighted and registration is blocked.

---

### User Story 2 - Log In User (Priority: P1)

A registered user logs in to access their CMS home page.

**Why this priority**: Authentication is required to access all role-specific features.

**Independent Test**: Can be tested by logging in with valid credentials and accessing
protected pages.

**Flow (From UC-02)**

**Main Success Scenario**
1. The user selects the **Log In** option.
2. The system displays the login form.
3. The user enters their username and password.
4. The user submits the login form.
5. The system validates the credentials against stored account data.
6. The system creates an authenticated user session.
7. The system redirects the user to their home page.

**Extensions**
5a: The username does not exist. The system displays an error message indicating
invalid credentials.
5b: The password is incorrect. The system displays an error message indicating
invalid credentials.
3a: Required fields are missing. The system highlights missing fields and requests
completion.

**Acceptance Scenarios**:
1. **Given** a registered user with valid credentials, **When** they submit the login
   form, **Then** they are authenticated and redirected to their home page.
2. **Given** a non-existent username, **When** the user submits the login form, **Then**
   login is rejected and an invalid credentials message is shown.
3. **Given** an incorrect password, **When** the user submits the login form, **Then**
   login is rejected and an invalid credentials message is shown.
4. **Given** missing username or password, **When** the user submits the login form,
   **Then** missing fields are highlighted and login is blocked.

---

### User Story 3 - Change User Password (Priority: P1)

A logged-in user changes their password to maintain account security.

**Why this priority**: Users must be able to maintain account security after login.

**Independent Test**: Can be tested by changing the password and verifying login
with the new password.

**Flow (From UC-03)**

**Main Success Scenario**
1. The user navigates to account settings.
2. The user selects the **Change Password** option.
3. The system displays the change password form.
4. The user enters the current password.
5. The user enters a new password and confirms it.
6. The user submits the password change request.
7. The system verifies the current password.
8. The system validates the new password against security requirements.
9. The system updates the user’s password.
10. The system confirms the password change to the user.

**Extensions**
7a: The current password is incorrect. The system displays an error message and
prompts re-entry.
8a: The new password does not meet security requirements. The system displays
password rules and requests a valid password.
5a: The new password and confirmation do not match. The system displays a mismatch
error message.

**Acceptance Scenarios**:
1. **Given** a logged-in user with a valid current password, **When** they submit a
   compliant new password, **Then** the password is updated and confirmed.
2. **Given** an incorrect current password, **When** the user submits the change,
   **Then** the system displays an error and the password remains unchanged.
3. **Given** a weak new password, **When** the user submits the change, **Then** the
   system displays password rules and blocks the update.
4. **Given** a mismatch between new password and confirmation, **When** the user
   submits the change, **Then** a mismatch error is shown and the password remains
   unchanged.

---

### User Story 4 - Submit Paper Manuscript (Priority: P2)

An author submits a paper manuscript and metadata for review.

**Why this priority**: Paper submission is a core author workflow and feeds review.

**Independent Test**: Can be tested by submitting a manuscript and confirming it is
stored and marked as submitted.

**Flow (From UC-04)**

**Main Success Scenario**
1. The author selects the **Submit Paper** option.
2. The system displays the paper submission form.
3. The author enters all required paper metadata.
4. The author uploads the manuscript file.
5. The author submits the paper.
6. The system validates the metadata and manuscript file.
7. The system stores the paper and manuscript.
8. The system confirms successful submission to the author.

**Extensions**
6a: Required metadata fields are missing. The system highlights missing fields and
prompts completion.
7a: The uploaded file format is invalid. The system displays supported file format
requirements.
7b: The uploaded file exceeds the maximum allowed size. The system displays a file
size error message.
4a: The author chooses to save the submission as a draft. The system saves the
current information without submitting the paper.

**Acceptance Scenarios**:
1. **Given** a logged-in author with valid metadata and manuscript, **When** they
   submit the paper, **Then** it is stored and marked as submitted.
2. **Given** missing required metadata, **When** the author submits the paper, **Then**
   missing fields are highlighted and submission is blocked.
3. **Given** an invalid file format, **When** the author submits the paper, **Then**
   the system displays file format requirements and blocks submission.
4. **Given** an oversized file, **When** the author submits the paper, **Then** the
   system displays a file size error and blocks submission.
5. **Given** an author choosing to save as draft, **When** they select **Save**, **Then**
   the submission is saved without being marked as submitted.

---

### User Story 5 - Save Paper Submission Draft (Priority: P2)

An author saves a partially completed paper submission for later completion.

**Why this priority**: Draft saving supports incomplete submissions and reduces
abandonment.

**Independent Test**: Can be tested by saving a draft and verifying it is retrievable
and not marked as submitted.

**Flow (From UC-05)**

**Main Success Scenario**
1. The author enters partial paper information.
2. The author selects the **Save** option.
3. The system validates entered information.
4. The system stores the submission as a draft.
5. The system confirms that the draft has been saved.

**Extensions**
3a: Entered information is invalid. The system displays a validation error and does
not save the draft.
2a: No information is entered. The system either saves an empty draft or informs the
author that there is no information to save.
4a: A system error occurs while saving. The system notifies the author that the draft
could not be saved.

**Acceptance Scenarios**:
1. **Given** partial submission data, **When** the author saves, **Then** the draft is
   stored and confirmation is shown.
2. **Given** invalid submission data, **When** the author saves, **Then** validation
   errors are shown and the draft is not saved.
3. **Given** no submission data, **When** the author saves, **Then** the system either
   saves an empty draft or informs the author that there is nothing to save.
4. **Given** a system error during save, **When** the author saves, **Then** an error
   message is shown and the draft is not saved.

---

### User Story 6 - Assign Referees to Submitted Paper (Priority: P2)

An editor assigns exactly three referees to a submitted paper.

**Why this priority**: Referee assignment is required before reviews can start.

**Independent Test**: Can be tested by assigning three eligible referees and
confirming invitations are sent and the paper is fully assigned.

**Flow (From UC-06)**

**Main Success Scenario**
1. The editor selects a submitted paper.
2. The system displays the referee assignment interface.
3. The editor enters the email address of a referee.
4. The system checks the referee’s current workload.
5. The system assigns the referee to the paper.
6. The system sends a review invitation to the referee.
7. Steps 3–6 are repeated until three referees are assigned.
8. The system confirms that the paper has three assigned referees.

**Extensions**
4a: The referee already has five assigned papers. The system blocks the assignment
and notifies the editor.
6a: The referee is already assigned to the paper. The system displays a duplicate
assignment error.
8a: Fewer than three referees are assigned when the editor exits. The system marks
the paper as incompletely assigned.

**Acceptance Scenarios**:
1. **Given** an eligible referee, **When** the editor assigns the referee, **Then** the
   assignment is recorded and an invitation is sent.
2. **Given** a referee with five assigned papers, **When** the editor attempts the
   assignment, **Then** the system blocks the assignment and notifies the editor.
3. **Given** a duplicate referee assignment, **When** the editor attempts the
   assignment, **Then** the system reports a duplicate error and does not assign.
4. **Given** fewer than three referees assigned, **When** the editor exits, **Then** the
   paper is marked incompletely assigned.

---

### User Story 7 - Accept or Reject Review Invitation (Priority: P2)

A referee accepts or rejects a review invitation.

**Why this priority**: Referee confirmation is required to begin reviews.

**Independent Test**: Can be tested by accepting and rejecting invitations and
verifying assignment and notifications.

**Flow (From UC-07)**

**Main Success Scenario**
1. The referee opens the review invitation.
2. The system displays paper information and response options.
3. The referee selects **Accept**.
4. The system verifies referee workload limits.
5. The system assigns the paper to the referee.
6. The system updates the paper’s referee count.
7. The system confirms acceptance to the referee.
8. The system notifies the editor of the acceptance.

**Extensions**
3a: The referee selects **Reject**. The system records the rejection and notifies the
editor.
4a: The paper already has three assigned referees. The system blocks acceptance and
notifies the referee and editor.
4b: The referee has reached the maximum workload. The system blocks acceptance and
notifies the editor.

**Acceptance Scenarios**:
1. **Given** a valid invitation and available capacity, **When** the referee accepts,
   **Then** the paper is assigned and the editor is notified.
2. **Given** a valid invitation, **When** the referee rejects, **Then** the rejection is
   recorded and the editor is notified.
3. **Given** the paper already has three referees, **When** the referee accepts,
   **Then** acceptance is blocked and both parties are notified.
4. **Given** the referee is at workload limit, **When** the referee accepts, **Then**
   acceptance is blocked and the editor is notified.

---

### User Story 8 - Review Assigned Paper (Priority: P2)

A referee submits a completed review for an assigned paper.

**Why this priority**: Reviews are required before editorial decisions can be made.

**Independent Test**: Can be tested by submitting a review and verifying storage and
editor notification.

**Flow (From UC-08)**

**Main Success Scenario**
1. The referee logs into the CMS.
2. The referee navigates to their list of assigned papers.
3. The referee selects a paper to review.
4. The system displays the paper details and review form.
5. The referee completes all required review fields.
6. The referee submits the review.
7. The system validates the review input.
8. The system stores the review.
9. The system confirms submission to the referee.
10. The system notifies the editor of the completed review.

**Extensions**
5a: Required review fields are missing. The system highlights missing fields and
blocks submission.
7a: Review contains invalid data. The system displays a validation error message.
8a: A system error occurs while saving the review. The system notifies the referee
that the review could not be saved.

**Acceptance Scenarios**:
1. **Given** an assigned paper and valid review data, **When** the referee submits the
   review, **Then** the review is stored and the editor is notified.
2. **Given** missing required review fields, **When** the referee submits, **Then** the
   system highlights missing fields and blocks submission.
3. **Given** invalid review data, **When** the referee submits, **Then** the system
   displays a validation error and does not save the review.
4. **Given** a system error during save, **When** the referee submits, **Then** an
   error is shown and the review is not stored.

---

### User Story 9 - Make Final Paper Decision (Priority: P2)

An editor records a final accept or reject decision after reviews are complete.

**Why this priority**: Final decisions close the review process and notify authors.

**Independent Test**: Can be tested by submitting a decision and verifying storage
and author notification.

**Flow (From UC-09)**

**Main Success Scenario**
1. The editor logs into the CMS.
2. The editor navigates to the list of papers ready for decision.
3. The editor selects a paper.
4. The system displays the paper details and completed reviews.
5. The editor selects a final decision (Accept or Reject).
6. The editor submits the decision.
7. The system validates that all required reviews are present.
8. The system stores the final decision.
9. The system updates the paper’s status.
10. The system notifies the author of the decision.

**Extensions**
7a: Fewer than three reviews are completed. The system blocks decision submission and
informs the editor.
5a: No decision is selected. The system displays an error message requesting a
decision.
8a: A system error occurs while saving the decision. The system notifies the editor
that the decision could not be saved.

**Acceptance Scenarios**:
1. **Given** three completed reviews, **When** the editor submits an accept decision,
   **Then** the decision is stored, the status is updated, and the author is notified.
2. **Given** fewer than three reviews, **When** the editor submits a decision, **Then**
   submission is blocked and the editor is informed.
3. **Given** no decision selected, **When** the editor submits, **Then** the system
   displays an error and no decision is stored.
4. **Given** a system error during save, **When** the editor submits, **Then** an error
   is shown and the decision is not stored.

---

### User Story 10 - Generate Conference Schedule (Priority: P3)

An administrator generates a schedule for accepted papers.

**Why this priority**: Scheduling depends on accepted papers and concludes program
organization.

**Independent Test**: Can be tested by generating and viewing a saved schedule.

**Flow (From UC-10)**

**Main Success Scenario**
1. The administrator navigates to schedule management.
2. The administrator selects **Generate Schedule**.
3. The system retrieves accepted papers, rooms, and time slots.
4. The system executes the scheduling algorithm.
5. The system generates a valid schedule.
6. The system stores the schedule.
7. The system displays the generated schedule in HTML format.

**Extensions**
2a: No accepted papers exist. The system blocks generation and informs the
administrator.
4a: Scheduling constraints cannot be satisfied. The system reports conflicts and
does not save an invalid schedule.
6a: A system error occurs while saving the schedule. The system notifies the
administrator that the schedule could not be saved.

**Acceptance Scenarios**:
1. **Given** accepted papers and defined rooms/time slots, **When** the administrator
   generates a schedule, **Then** a valid schedule is stored and displayed.
2. **Given** no accepted papers, **When** the administrator generates a schedule,
   **Then** generation is blocked and an informational message is shown.
3. **Given** unsatisfiable scheduling constraints, **When** the administrator
   generates a schedule, **Then** conflicts are reported and no schedule is saved.
4. **Given** a system error during save, **When** the administrator generates a
   schedule, **Then** an error is shown and the schedule is not saved.

---

### User Story 11 - Edit Conference Schedule (Priority: P3)

An editor edits an existing schedule to resolve conflicts or adjust assignments.

**Why this priority**: Schedule edits happen after generation and are refinements.

**Independent Test**: Can be tested by editing a schedule and verifying changes
persist.

**Flow (From UC-11)**

**Main Success Scenario**
1. The editor navigates to the schedule management page.
2. The editor selects **Edit Schedule**.
3. The system displays the current schedule in editable form.
4. The editor modifies room and/or time assignments.
5. The editor submits the updated schedule.
6. The system validates the updated schedule for conflicts.
7. The system stores the updated schedule.
8. The system confirms the successful update to the editor.

**Extensions**
6a: The updated schedule contains conflicts. The system displays conflict errors and
blocks saving.
5a: No changes are made to the schedule. The system informs the editor that no
updates were detected.
7a: A system error occurs while saving the schedule. The system notifies the editor
that the schedule could not be saved.

**Acceptance Scenarios**:
1. **Given** an existing schedule, **When** the editor saves valid changes, **Then**
   the schedule is updated and confirmed.
2. **Given** conflicting edits, **When** the editor saves, **Then** conflict errors are
   shown and the schedule is not saved.
3. **Given** no changes, **When** the editor saves, **Then** the system informs the
   editor that no updates were detected.
4. **Given** a system error during save, **When** the editor saves, **Then** an error
   is shown and the schedule remains unchanged.

---

### User Story 12 - Register for Conference Attendance (Priority: P3)

An attendee registers for the conference and completes payment.

**Why this priority**: Registration is a core attendee workflow and depends on
authentication and open registration.

**Independent Test**: Can be tested by completing payment and verifying registration
and ticket delivery.

**Flow (From UC-12)**

**Main Success Scenario**
1. The attendee logs into the CMS.
2. The attendee navigates to conference registration.
3. The system displays attendance options and pricing.
4. The attendee selects an attendance type.
5. The system redirects the attendee to the payment interface.
6. The attendee enters valid payment information.
7. The attendee submits the payment.
8. The system processes the payment successfully.
9. The system records the registration.
10. The system generates a confirmation/ticket.
11. The system notifies the attendee of successful registration.

**Extensions**
8a: Payment is declined. The system displays a payment failure message and does not
register the attendee.
6a: Invalid payment information is entered. The system displays validation errors
and requests correction.
9a: A system error occurs while recording the registration. The system notifies the
attendee that registration could not be completed.

**Acceptance Scenarios**:
1. **Given** registration is open and payment succeeds, **When** the attendee submits
   payment, **Then** registration is recorded and a confirmation/ticket is delivered.
2. **Given** a declined payment, **When** the attendee submits payment, **Then** a
   failure message is shown and registration is not completed.
3. **Given** invalid payment information, **When** the attendee submits payment, **Then**
   validation errors are shown and payment is not processed.
4. **Given** a system error during registration recording, **When** the attendee
   submits payment, **Then** an error is shown and registration is not completed.

### Edge Cases

- Missing required fields during registration, login, password change, submission,
  and review flows.
- Invalid credentials or invalid payment data.
- Duplicate submissions (paper submission, review submission, decision submission).
- Referee workload limit reached or paper already has three referees.
- Scheduling constraints unsatisfied or schedule conflicts detected.
- System errors during save operations (drafts, reviews, decisions, schedules,
  registrations).

### Assumptions & Dependencies

- CMS roles (guest, registered user, author, referee, editor, administrator,
  attendee) exist and are authenticated according to the preconditions in each use
  case.
- Conference registration status (open/closed) and accepted papers exist where
  required by preconditions.
- Rooms and time slots are defined before schedule generation.

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: System MUST display a registration form when a guest selects **Register** and require name, username, email, and password.
- **FR-002**: System MUST validate email uniqueness during registration.
- **FR-003**: System MUST validate passwords during registration to enforce a
  minimum of 8 characters including at least 1 uppercase, 1 lowercase, 1 digit,
  and 1 symbol.
- **FR-004**: System MUST create and store a new user account (name, username, email, password) upon successful
  registration.
- **FR-005**: System MUST redirect newly registered users to the login page.
- **FR-006**: System MUST display a login form when a user selects **Log In**.
- **FR-007**: System MUST validate submitted credentials against stored account data.
- **FR-008**: System MUST create an authenticated user session on successful login.
- **FR-009**: System MUST redirect authenticated users to their home page.
- **FR-010**: System MUST display a change password form from account settings.
- **FR-011**: System MUST verify the current password before allowing a change.
- **FR-012**: System MUST validate new passwords to enforce a minimum of 8
  characters including at least 1 uppercase, 1 lowercase, 1 digit, and 1 symbol.
- **FR-013**: System MUST update the stored password after successful validation.
- **FR-014**: System MUST display the paper submission form for authors.
- **FR-015**: System MUST validate required paper metadata on submission: title,
  abstract, and keywords.
- **FR-016**: System MUST validate manuscript files to accept PDF only with a
  maximum size of 10 MB.
- **FR-017**: System MUST store submitted papers and manuscripts on success.
- **FR-018**: System MUST allow authors to save paper submissions as drafts.
- **FR-019**: System MUST validate draft information before saving.
- **FR-020**: System MUST store drafts separately from submitted papers.
- **FR-020a**: Draft submissions MUST not expire automatically.
- **FR-020b**: If no information is entered, the system MUST warn the author and
  NOT save an empty draft.
- **FR-021**: System MUST allow editors to assign referees to submitted papers.
- **FR-022**: System MUST enforce a maximum of three referees per paper.
- **FR-023**: System MUST enforce a referee workload limit of five assigned papers.
- **FR-024**: System MUST prevent duplicate referee assignments on a paper.
- **FR-025**: System MUST send review invitations via email and in-app
  notification on successful assignment.
- **FR-026**: System MUST record referee acceptance or rejection of invitations.
- **FR-027**: System MUST block acceptance when referee workload or paper capacity
  limits are exceeded.
- **FR-028**: System MUST display assigned papers and review forms to referees.
- **FR-029**: System MUST validate required review fields before submission.
- **FR-030**: System MUST store submitted reviews and notify the editor.
- **FR-031**: System MUST allow editors to submit accept or reject decisions once
  three reviews are complete.
- **FR-032**: System MUST store final decisions and update paper status.
- **FR-033**: System MUST notify authors of final decisions via email and in-app
  notification.
- **FR-034**: System MUST generate a schedule from accepted papers, rooms, and time
  slots when requested by an administrator.
- **FR-035**: System MUST store generated schedules and display them to users.
- **FR-036**: System MUST allow editors to edit schedules and validate conflicts
  before saving.
- **FR-036a**: Schedule generation MUST be deterministic for the same inputs.
- **FR-037**: System MUST record attendee registrations after successful payment.
- **FR-038**: System MUST generate and deliver a confirmation/ticket via email
  and in-app notification on successful registration.
- **FR-039**: System MUST allow exactly one attendance type selection per attendee
  registration.
- **FR-040**: System MUST retain user accounts, submissions, reviews, and payment
  records for 3 years after the conference end date and then delete or
  anonymize them.

### Key Entities *(include if feature involves data)*

- **User Account**: Registered identity with credentials and role assignments.
- **Paper Submission**: Manuscript and metadata, with status (draft/submitted).
- **Referee Assignment**: Mapping between paper and referee, with workload tracking.
- **Review**: Referee evaluation associated with a paper.
- **Decision**: Final accept or reject outcome for a paper.
- **Schedule**: Room and time assignments for accepted papers.
- **Registration**: Attendee registration status and payment outcome.
- **Ticket/Confirmation**: Proof of successful attendee registration.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: At least 95% of users can complete registration and login flows on
  their first attempt without administrator intervention.
- **SC-002**: 95% of paper submissions with valid metadata and files are successfully
  stored without retries.
- **SC-003**: 100% of papers with three completed reviews can receive a final
  decision and author notification without manual workaround.
- **SC-004**: A generated schedule is viewable by administrators within 1 minute
  of initiating generation for conferences with up to 1000 submissions, 50 rooms,
  and 200 time slots.
- **SC-005**: 95% of attendee registrations with valid payment details complete
  successfully and deliver a confirmation/ticket.

## Consolidated User Acceptance Test Suite *(mandatory)*

- **UAT-UC01-001 — Successful Registration**
  **Preconditions:** Guest not logged in; email does not exist in system.
  **Steps:** Open registration page; enter valid name, username, email, and
  password; submit.
  **Expected Results:** Account is created; user redirected to login page.
- **UAT-UC01-002 — Duplicate Email Address**
  **Preconditions:** Email already exists in database.
  **Steps:** Enter existing email and username; enter valid password; submit form.
  **Expected Results:** Error message displayed; no account created.
- **UAT-UC01-003 — Invalid Password**
  **Preconditions:** Email does not exist.
  **Steps:** Enter valid name, username, email; enter weak password; submit form.
  **Expected Results:** Password error displayed; no account created.
- **UAT-UC01-004 — Missing Required Fields**
  **Preconditions:** Guest on registration form.
  **Steps:** Leave one or more required fields blank; submit form.
  **Expected Results:** Missing fields highlighted; registration blocked.

- **UAT-UC02-001 — Successful Login**
  **Preconditions:** User account exists; user is not logged in.
  **Steps:** Open login page; enter valid username and password; submit login form.
  **Expected Results:** User authenticated; redirected to home page; active session
  created.
- **UAT-UC02-002 — Username Does Not Exist**
  **Preconditions:** Username is not registered in the system.
  **Steps:** Enter non-existent username; enter any password; submit login form.
  **Expected Results:** Login rejected; error message displayed; no session created.
- **UAT-UC02-003 — Incorrect Password**
  **Preconditions:** Username exists; password is incorrect.
  **Steps:** Enter valid username; enter incorrect password; submit login form.
  **Expected Results:** Login rejected; error message displayed; no session created.
- **UAT-UC02-004 — Missing Username or Password**
  **Preconditions:** User on login page.
  **Steps:** Leave username or password blank; submit login form.
  **Expected Results:** Missing fields highlighted; login blocked.
- **UAT-UC02-005 — Session Persistence After Login**
  **Preconditions:** Successful login completed.
  **Steps:** Navigate to another protected page; refresh the page.
  **Expected Results:** User remains authenticated; page remains accessible.
- **UAT-UC02-006 — Access Login Page While Already Logged In**
  **Preconditions:** User already logged in.
  **Steps:** Navigate to login page.
  **Expected Results:** User redirected to home page or informed they are already
  logged in.
- **UAT-UC02-007 — Case-Insensitive Username Matching**
  **Preconditions:** Username is stored in lowercase.
  **Steps:** Enter same username with different letter casing; enter correct
  password; submit login form.
  **Expected Results:** Login succeeds; user redirected to home page.

- **UAT-UC03-001 — Successful Password Change**
  **Preconditions:** User is logged in; current password is valid.
  **Steps:** Open change password page; enter correct current password; enter valid
  new password and confirmation; submit request.
  **Expected Results:** Password updated; confirmation message displayed.
- **UAT-UC03-002 — Incorrect Current Password**
  **Preconditions:** User is logged in.
  **Steps:** Enter incorrect current password; enter valid new password; submit
  request.
  **Expected Results:** Error message displayed; password unchanged.
- **UAT-UC03-003 — Weak New Password**
  **Preconditions:** User is logged in.
  **Steps:** Enter correct current password; enter weak new password; submit
  request.
  **Expected Results:** Password validation error displayed; password unchanged.
- **UAT-UC03-004 — Password Confirmation Mismatch**
  **Preconditions:** User is logged in.
  **Steps:** Enter correct current password; enter mismatched new password and
  confirmation; submit request.
  **Expected Results:** Mismatch error displayed; password unchanged.
- **UAT-UC03-005 — Missing Required Fields**
  **Preconditions:** User on change password form.
  **Steps:** Leave one or more fields blank; submit request.
  **Expected Results:** Missing fields highlighted; password unchanged.
- **UAT-UC03-006 — Old Password No Longer Valid After Change**
  **Preconditions:** Successful password change completed.
  **Steps:** Log out; attempt login with old password; attempt login with new
  password.
  **Expected Results:** Login with old password fails; login with new password
  succeeds.
- **UAT-UC03-007 — Prevent Reusing Same Password**
  **Preconditions:** User is logged in.
  **Steps:** Enter current password as new password; submit request.
  **Expected Results:** Password reuse rejected; error message displayed.

- **UAT-UC04-001 — Successful Paper Submission**
  **Preconditions:** Author is logged in; valid manuscript file available.
  **Steps:** Open submission form; enter valid metadata; upload valid manuscript;
  submit paper.
  **Expected Results:** Paper stored; confirmation displayed; paper marked as
  submitted.
- **UAT-UC04-002 — Missing Required Metadata**
  **Preconditions:** Author on submission form.
  **Steps:** Leave one or more required fields blank; upload valid manuscript;
  submit paper.
  **Expected Results:** Missing fields highlighted; submission blocked.
- **UAT-UC04-003 — Invalid File Format**
  **Preconditions:** Author on submission form.
  **Steps:** Enter valid metadata; upload unsupported file type; submit paper.
  **Expected Results:** File format error displayed; submission blocked.
- **UAT-UC04-004 — File Size Exceeds Limit**
  **Preconditions:** Author on submission form.
  **Steps:** Enter valid metadata; upload oversized file; submit paper.
  **Expected Results:** File size error displayed; submission blocked.
- **UAT-UC04-005 — Save Draft Instead of Submit**
  **Preconditions:** Author on submission form.
  **Steps:** Enter partial metadata; click **Save**.
  **Expected Results:** Draft saved; paper not marked as submitted.
- **UAT-UC04-006 — Submit Saved Draft**
  **Preconditions:** Draft exists.
  **Steps:** Open saved draft; complete metadata and upload file; submit paper.
  **Expected Results:** Draft converted to submitted paper; confirmation displayed.
- **UAT-UC04-007 — Prevent Duplicate Submission**
  **Preconditions:** Author submits paper.
  **Steps:** Double-click submit button.
  **Expected Results:** Only one submission created.

- **UAT-UC05-001 — Save Draft with Partial Information**
  **Preconditions:** Author is logged in and on submission form.
  **Steps:** Enter partial metadata; click **Save**.
  **Expected Results:** Draft saved; confirmation message displayed.
- **UAT-UC05-002 — Update Existing Draft**
  **Preconditions:** Draft already exists.
  **Steps:** Open saved draft; modify information; click **Save**.
  **Expected Results:** Draft updated; no duplicate draft created.
- **UAT-UC05-003 — Invalid Information Blocks Save**
  **Preconditions:** Author on submission form.
  **Steps:** Enter invalid data; click **Save**.
  **Expected Results:** Validation error displayed; draft not saved.
- **UAT-UC05-004 — Save with No Information Entered**
  **Preconditions:** Author on new submission form.
  **Steps:** Enter no data; click **Save**.
  **Expected Results:** System behavior follows defined rule (empty draft saved or
  warning shown).
- **UAT-UC05-005 — Draft Is Not Marked as Submitted**
  **Preconditions:** Draft exists.
  **Steps:** View submissions list.
  **Expected Results:** Draft labeled as **Draft**; not included in submitted
  papers.
- **UAT-UC05-006 — System Error During Save**
  **Preconditions:** Backend failure simulated.
  **Steps:** Enter partial data; click **Save**.
  **Expected Results:** Error message displayed; draft not saved.
- **UAT-UC05-007 — Retry Save After Failure**
  **Preconditions:** Previous save failed.
  **Steps:** Retry **Save** after system recovery.
  **Expected Results:** Draft saved successfully.

- **UAT-UC06-001 — Assign Three Referees Successfully**
  **Preconditions:** Editor logged in; eligible referees available.
  **Steps:** Select submitted paper; assign three eligible referees.
  **Expected Results:** Three referees assigned; invitations sent; paper marked as
  fully assigned.
- **UAT-UC06-002 — Referee Workload Limit Exceeded**
  **Preconditions:** Referee already has five assigned papers.
  **Steps:** Attempt to assign overloaded referee.
  **Expected Results:** Assignment blocked; error message displayed.
- **UAT-UC06-003 — Duplicate Referee Assignment**
  **Preconditions:** Referee already assigned to paper.
  **Steps:** Attempt to assign same referee again.
  **Expected Results:** Duplicate assignment error; no duplicate record created.
- **UAT-UC06-004 — Fewer Than Three Referees Assigned**
  **Preconditions:** Paper has fewer than three referees assigned.
  **Steps:** Exit assignment interface.
  **Expected Results:** Paper marked as incompletely assigned.
- **UAT-UC06-005 — Referee Workload Increment**
  **Preconditions:** Referee eligible before assignment.
  **Steps:** Assign referee to paper.
  **Expected Results:** Referee workload count increases by one.
- **UAT-UC06-006 — Invitation Sent on Successful Assignment**
  **Preconditions:** Valid referee assignment.
  **Steps:** Assign referee.
  **Expected Results:** Review invitation sent.
- **UAT-UC06-007 — Prevent Over-Assignment Beyond Three Referees**
  **Preconditions:** Two referees already assigned.
  **Steps:** Attempt to assign two additional referees simultaneously.
  **Expected Results:** Only one additional referee assigned; total referees does
  not exceed three.

- **UAT-UC07-001 — Accept Invitation Successfully**
  **Preconditions:** Valid invitation exists; referee workload < limit; paper has
  fewer than three referees.
  **Steps:** Open invitation; select **Accept**.
  **Expected Results:** Invitation accepted; paper assigned to referee; editor
  notified.
- **UAT-UC07-002 — Reject Invitation**
  **Preconditions:** Valid invitation exists.
  **Steps:** Open invitation; select **Reject**.
  **Expected Results:** Rejection recorded; paper not assigned; editor notified.
- **UAT-UC07-003 — Paper Already Has Three Referees**
  **Preconditions:** Paper already has three referees assigned.
  **Steps:** Attempt to accept invitation.
  **Expected Results:** Acceptance blocked; error message displayed; editor notified.
- **UAT-UC07-004 — Referee Workload Limit Exceeded**
  **Preconditions:** Referee has reached maximum workload.
  **Steps:** Attempt to accept invitation.
  **Expected Results:** Acceptance blocked; workload error displayed; editor
  notified.
- **UAT-UC07-005 — Invalid or Expired Invitation**
  **Preconditions:** Invitation token invalid or expired.
  **Steps:** Open invitation link.
  **Expected Results:** Error message displayed; no response recorded.
- **UAT-UC07-006 — Prevent Duplicate Acceptance**
  **Preconditions:** Invitation already accepted.
  **Steps:** Attempt to accept invitation again.
  **Expected Results:** No duplicate assignment; informational message displayed.
- **UAT-UC07-007 — Paper Visible in Referee Account After Acceptance**
  **Preconditions:** Invitation accepted successfully.
  **Steps:** Log into CMS; view assigned papers.
  **Expected Results:** Paper appears in referee’s assigned list.

- **UAT-UC08-001 — Submit Review Successfully**
  **Preconditions:** Referee logged in; paper assigned and not yet reviewed.
  **Steps:** Open assigned paper; complete review form with valid data; submit
  review.
  **Expected Results:** Review saved; confirmation displayed; editor notified.
- **UAT-UC08-002 — Missing Required Review Fields**
  **Preconditions:** Referee on review form.
  **Steps:** Leave one or more required fields blank; submit review.
  **Expected Results:** Missing fields highlighted; submission blocked.
- **UAT-UC08-003 — Invalid Review Data**
  **Preconditions:** Referee on review form.
  **Steps:** Enter invalid data; submit review.
  **Expected Results:** Validation error displayed; review not saved.
- **UAT-UC08-004 — System Error During Review Submission**
  **Preconditions:** Backend failure simulated.
  **Steps:** Complete review form; submit review.
  **Expected Results:** Error message displayed; review not saved.
- **UAT-UC08-005 — Prevent Duplicate Review Submission**
  **Preconditions:** Review already submitted.
  **Steps:** Attempt to resubmit review.
  **Expected Results:** Duplicate submission blocked.
- **UAT-UC08-006 — Review Visible to Editor**
  **Preconditions:** Review successfully submitted.
  **Steps:** Editor views paper reviews.
  **Expected Results:** Review visible to editor.
- **UAT-UC08-007 — Retry Review Submission After Failure**
  **Preconditions:** Previous submission failed.
  **Steps:** Retry submission.
  **Expected Results:** Review submitted successfully.

- **UAT-UC09-001 — Accept Paper Successfully**
  **Preconditions:** Paper has three completed reviews; editor logged in.
  **Steps:** Open paper ready for decision; select **Accept**; submit decision.
  **Expected Results:** Decision saved; paper status set to Accepted; author
  notified.
- **UAT-UC09-002 — Reject Paper Successfully**
  **Preconditions:** Paper has three completed reviews; editor logged in.
  **Steps:** Open paper ready for decision; select **Reject**; submit decision.
  **Expected Results:** Decision saved; paper status set to Rejected; author
  notified.
- **UAT-UC09-003 — Insufficient Reviews**
  **Preconditions:** Paper has fewer than three reviews.
  **Steps:** Attempt to submit decision.
  **Expected Results:** Submission blocked; informational error displayed.
- **UAT-UC09-004 — No Decision Selected**
  **Preconditions:** Paper has three reviews.
  **Steps:** Submit decision form without selecting decision.
  **Expected Results:** Error message displayed; no decision saved.
- **UAT-UC09-005 — System Error While Saving Decision**
  **Preconditions:** Backend failure simulated.
  **Steps:** Submit valid decision.
  **Expected Results:** Error message displayed; decision not saved.
- **UAT-UC09-006 — Author Notification Content**
  **Preconditions:** Decision successfully saved.
  **Steps:** Check author notification.
  **Expected Results:** Correct decision communicated; notification sent once.
- **UAT-UC09-007 — Prevent Duplicate Decision Submission**
  **Preconditions:** Decision already recorded.
  **Steps:** Attempt to resubmit decision.
  **Expected Results:** Duplicate submission blocked.
- **UAT-UC09-008 — Decision Persistence**
  **Preconditions:** Decision recorded.
  **Steps:** Refresh page or reopen paper.
  **Expected Results:** Decision remains stored.

- **UAT-UC10-001 — Generate Schedule Successfully**
  **Preconditions:** Accepted papers exist; admin logged in.
  **Steps:** Open schedule management; click **Generate Schedule**.
  **Expected Results:** Schedule generated; schedule saved; HTML schedule displayed.
- **UAT-UC10-002 — View Persisted Schedule**
  **Preconditions:** Schedule already generated.
  **Steps:** Refresh or revisit schedule page.
  **Expected Results:** Same schedule displayed; no regeneration required.
- **UAT-UC10-003 — No Accepted Papers**
  **Preconditions:** No papers accepted.
  **Steps:** Attempt to generate schedule.
  **Expected Results:** Generation blocked; informational message displayed.
- **UAT-UC10-004 — Unsatisfiable Constraints**
  **Preconditions:** Accepted papers exceed available slots.
  **Steps:** Attempt to generate schedule.
  **Expected Results:** Conflict reported; no schedule saved.
- **UAT-UC10-005 — System Error While Saving Schedule**
  **Preconditions:** Backend failure simulated.
  **Steps:** Generate schedule.
  **Expected Results:** Error message displayed; schedule not saved.
- **UAT-UC10-006 — Deterministic Scheduling Behavior**
  **Preconditions:** Inputs unchanged.
  **Steps:** Generate schedule twice.
  **Expected Results:** Output consistent or valid per algorithm definition.
- **UAT-UC10-007 — HTML Output Validation**
  **Preconditions:** Schedule generated.
  **Steps:** Inspect rendered schedule.
  **Expected Results:** Schedule rendered in HTML format.

- **UAT-UC11-001 — Edit Schedule Successfully**
  **Preconditions:** Schedule exists; editor logged in.
  **Steps:** Open schedule editor; make valid changes; save schedule.
  **Expected Results:** Schedule updated; confirmation displayed.
- **UAT-UC11-002 — Conflict Detected During Edit**
  **Preconditions:** Schedule exists.
  **Steps:** Create conflicting assignments; attempt to save.
  **Expected Results:** Conflict error displayed; schedule not saved.
- **UAT-UC11-003 — No Changes Made**
  **Preconditions:** Schedule exists.
  **Steps:** Open editor; save without changes.
  **Expected Results:** Informational message displayed; schedule unchanged.
- **UAT-UC11-004 — System Error While Saving**
  **Preconditions:** Backend failure simulated.
  **Steps:** Make valid changes; save schedule.
  **Expected Results:** Error message displayed; schedule unchanged.
- **UAT-UC11-005 — Persistence of Edited Schedule**
  **Preconditions:** Schedule updated successfully.
  **Steps:** Refresh or reopen schedule.
  **Expected Results:** Updated schedule persists.
- **UAT-UC11-006 — Replace Previous Schedule Version**
  **Preconditions:** Schedule already edited once.
  **Steps:** Edit schedule again; save changes.
  **Expected Results:** Latest version becomes current schedule.
- **UAT-UC11-007 — Unauthorized User Cannot Edit Schedule**
  **Preconditions:** Non-editor user logged in.
  **Steps:** Attempt to access edit schedule.
  **Expected Results:** Access denied.

- **UAT-UC12-001 — Successful Registration and Payment**
  **Preconditions:** Attendee logged in; registration open.
  **Steps:** Select attendance type; enter valid payment details; submit payment.
  **Expected Results:** Payment processed; registration recorded;
  confirmation/ticket generated.
- **UAT-UC12-002 — Payment Declined**
  **Preconditions:** Attendee logged in.
  **Steps:** Enter declined payment details; submit payment.
  **Expected Results:** Payment failure message displayed; registration not
  completed.
- **UAT-UC12-003 — Invalid Payment Information**
  **Preconditions:** Attendee on payment page.
  **Steps:** Enter invalid card details; submit payment.
  **Expected Results:** Validation error displayed; payment not processed.
- **UAT-UC12-004 — System Error While Recording Registration**
  **Preconditions:** Backend failure simulated.
  **Steps:** Submit valid payment.
  **Expected Results:** Error message displayed; registration not completed.
- **UAT-UC12-005 — Confirmation Delivery**
  **Preconditions:** Successful registration.
  **Steps:** Check attendee email or notifications.
  **Expected Results:** Confirmation/ticket delivered.
- **UAT-UC12-006 — Registration Status Reflected in Account**
  **Preconditions:** Registration completed.
  **Steps:** View attendee dashboard.
  **Expected Results:** Status shows registered/paid.
- **UAT-UC12-007 — Prevent Duplicate Payment**
  **Preconditions:** Registration already completed.
  **Steps:** Attempt to pay again.
  **Expected Results:** Duplicate payment blocked.
- **UAT-UC12-008 — Unauthorized User Cannot Register**
  **Preconditions:** User not logged in.
  **Steps:** Attempt to access registration.
  **Expected Results:** Redirected to login.
