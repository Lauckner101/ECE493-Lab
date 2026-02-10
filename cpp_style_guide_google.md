# Google C++ Style Guide (Markdown Edition)

> **Source**: Google C++ Style Guide\
> **License**: Creative Commons Attribution 3.0 (CC BY 3.0)\
> **Original**: https://google.github.io/styleguide/cppguide.html
>
> This document is an adapted Markdown version of the Google C++ Style
> Guide, intended for use as a project coding reference and as input to
> Spec Kit constitutions. Attribution retained per CC-BY requirements.

------------------------------------------------------------------------

## 1. Introduction

This style guide provides conventions for writing readable,
maintainable, and robust C++ code. The rules prioritize: - consistency
across large codebases - readability over cleverness - explicitness over
implicit behavior

These guidelines assume **modern ISO C++** and are designed to scale to
large, multi-author projects.

------------------------------------------------------------------------

## 2. File Organization

### 2.1 File Naming

-   Source files use `.cc`
-   Header files use `.h`
-   Filenames are all lowercase with underscores if needed

### 2.2 Self-Contained Headers

-   Every header **MUST be self-contained**
-   A header should compile on its own
-   Headers MUST include all dependencies they require

### 2.3 Include Guards

All headers MUST use include guards.

### 2.4 Include Order

1.  Corresponding header\
2.  C system headers\
3.  C++ standard library headers\
4.  Other project headers

------------------------------------------------------------------------

## 3. Scoping and Namespaces

### 3.1 Namespaces

-   Avoid `using namespace` at global scope
-   Never use it in headers

### 3.2 Anonymous Namespaces

-   Allowed in `.cc` files only

------------------------------------------------------------------------

## 4. Classes

-   Prefer small, focused classes
-   Use composition over inheritance
-   Mark single-argument constructors `explicit`

------------------------------------------------------------------------

## 5. Functions

-   Keep functions short and readable
-   Prefer `const T&` for large read-only parameters
-   Use pointers only for optional values

------------------------------------------------------------------------

## 6. Variables and Data

-   Declare variables in the smallest possible scope
-   Avoid global variables

------------------------------------------------------------------------

## 7. Constants

-   Prefer `constexpr` over `#define`
-   Use `const` for runtime constants

------------------------------------------------------------------------

## 8. Memory Management

-   Prefer RAII
-   Avoid raw `new` and `delete`
-   Use `std::unique_ptr` for ownership

------------------------------------------------------------------------

## 9. Exceptions

-   Prefer explicit error handling
-   Destructors must not throw

------------------------------------------------------------------------

## 10. Templates

-   Use sparingly
-   Prefer readability over cleverness

------------------------------------------------------------------------

## 11. Macros

-   Avoid macros whenever possible
-   Never define macros in headers unless unavoidable

------------------------------------------------------------------------

## 12. Comments

-   Explain *why*, not *what*
-   Document public APIs

------------------------------------------------------------------------

## 13. Formatting

-   80 character line limit
-   2-space indentation
-   Braces on the same line

------------------------------------------------------------------------

## 14. Naming

-   Types: `CamelCase`
-   Functions: `CamelCase`
-   Variables: `snake_case`
-   Constants: `kConstantName`

------------------------------------------------------------------------

## 15. Modern C++

-   Target C++17 or later
-   Use `auto` when it improves clarity
-   Use range-based `for` loops

------------------------------------------------------------------------

## 16. Authority

This document derives from the Google C++ Style Guide. Project-specific
rules may override it where explicitly stated.

------------------------------------------------------------------------
