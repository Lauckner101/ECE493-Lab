# Data Model

## Entities

### UserAccount
- **id** (UUID)
- **name** (string)
- **username** (string, unique, case-insensitive)
- **email** (string, unique)
- **password_hash** (string)
- **roles** (set of enum: author, referee, editor, administrator, attendee)
- **created_at** (timestamp)
- **status** (enum: active, disabled)

### PaperSubmission
- **id** (UUID)
- **author_id** (FK -> UserAccount)
- **metadata** (JSON object: title, abstract, keywords)
- **manuscript_path** (string)
- **status** (enum: draft, submitted)
- **created_at** (timestamp)
- **updated_at** (timestamp)

### RefereeAssignment
- **id** (UUID)
- **paper_id** (FK -> PaperSubmission)
- **referee_id** (FK -> UserAccount)
- **status** (enum: invited, accepted, rejected)
- **invited_at** (timestamp)
- **responded_at** (timestamp, nullable)

### Review
- **id** (UUID)
- **paper_id** (FK -> PaperSubmission)
- **referee_id** (FK -> UserAccount)
- **review_text** (string)
- **recommendation** (enum: accept, reject)
- **submitted_at** (timestamp)

### Decision
- **id** (UUID)
- **paper_id** (FK -> PaperSubmission)
- **editor_id** (FK -> UserAccount)
- **decision** (enum: accept, reject)
- **decided_at** (timestamp)

### Conference
- **id** (UUID)
- **name** (string)
- **start_date** (date)
- **end_date** (date)
- **registration_open** (boolean)

### Room
- **id** (UUID)
- **name** (string)
- **capacity** (int)

### TimeSlot
- **id** (UUID)
- **start_time** (timestamp)
- **end_time** (timestamp)

### Schedule
- **id** (UUID)
- **conference_id** (FK -> Conference)
- **generated_at** (timestamp)
- **is_current** (boolean)

### ScheduleItem
- **id** (UUID)
- **schedule_id** (FK -> Schedule)
- **paper_id** (FK -> PaperSubmission)
- **room_id** (FK -> Room)
- **time_slot_id** (FK -> TimeSlot)

### Registration
- **id** (UUID)
- **attendee_id** (FK -> UserAccount)
- **conference_id** (FK -> Conference)
- **attendance_type** (string)
- **payment_status** (enum: pending, paid, failed)
- **created_at** (timestamp)

### PaymentRecord
- **id** (UUID)
- **registration_id** (FK -> Registration)
- **external_reference** (string)
- **status** (enum: pending, succeeded, failed)
- **processed_at** (timestamp)

### Ticket
- **id** (UUID)
- **registration_id** (FK -> Registration)
- **delivered_at** (timestamp)

## Relationships

- UserAccount 1..* PaperSubmission (author)
- UserAccount 1..* RefereeAssignment (referee)
- PaperSubmission 1..* RefereeAssignment
- PaperSubmission 1..* Review
- PaperSubmission 1..1 Decision (after reviews complete)
- Conference 1..* Schedule
- Schedule 1..* ScheduleItem
- PaperSubmission 1..* ScheduleItem
- Conference 1..* Registration
- Registration 1..1 PaymentRecord
- Registration 1..1 Ticket

## Validation Rules & Constraints

- Passwords must be at least 8 characters and meet complexity requirements.
- Email addresses must be unique.
- Usernames are unique and matched case-insensitively for login.
- Each paper must have exactly three referee assignments before reviews can be
  completed and a decision made.
- A referee may have at most five assigned papers at any time.
- Draft submissions are stored separately from submitted papers and never expire.
- If no draft information is entered, the draft is not saved.
- Each registration has exactly one attendance type.
- Schedule generation is deterministic for the same inputs.
- Data retention: user accounts, submissions, reviews, and payment records are
  retained for 3 years after conference end date.

## State Transitions

- PaperSubmission: draft -> submitted
- RefereeAssignment: invited -> accepted | rejected
- Registration: pending -> paid | failed
- PaymentRecord: pending -> succeeded | failed
