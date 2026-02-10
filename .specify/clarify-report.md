# Clarify Report

**Date**: 2026-02-10
**Feature**: Use Case Flow Specification
**Spec**: .specify/spec.md

## Questions Asked and Answers

1. Q: What password policy should the CMS enforce?  
   A: Minimum 8 chars, complexity required.
2. Q: For paper submissions, do you want drafts to have an expiration?  
   A: Drafts never expire.
3. Q: Does conference registration allow multiple attendance types per attendee?  
   A: Exactly one type per attendee.
4. Q: Should schedule generation be deterministic (same inputs always yield the same schedule)?  
   A: Yes, deterministic for same inputs.
5. Q: When an author clicks Save with no information entered, which behavior should apply?  
   A: Show warning, do not save.
6. Q: What data retention policy should the CMS use for user accounts, submissions, reviews, and payment records?  
   A: Retain for 3 years after conference end.

## Necessity Statement

These clarifications are necessary for the CMS. They define security requirements,
workflow boundaries, privacy handling, and behavioral guarantees that affect user
experience, validation logic, compliance expectations, and acceptance testing.
