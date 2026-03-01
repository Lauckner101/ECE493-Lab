# Research

## Decision 1: C++ Web Framework
- **Decision**: Use Drogon (C++ web framework) for HTTP routing and middleware.
- **Rationale**: Mature C++ web framework with routing, middleware, and async support.
- **Alternatives considered**: Crow, Pistache.

## Decision 2: JSON and Serialization
- **Decision**: Use nlohmann/json for JSON parsing and serialization.
- **Rationale**: Widely used C++ JSON library with straightforward API.
- **Alternatives considered**: RapidJSON.

## Decision 3: Database
- **Decision**: PostgreSQL with libpqxx driver.
- **Rationale**: Relational schema fits CMS entities and constraints; strong SQL support.
- **Alternatives considered**: MySQL, SQLite.

## Decision 4: Testing
- **Decision**: GoogleTest for unit tests and HTTP-based integration tests.
- **Rationale**: Standard C++ testing framework with good tooling support.
- **Alternatives considered**: Catch2.

## Decision 5: Deployment Target
- **Decision**: Linux server deployment.
- **Rationale**: Common target for C++ web services and aligns with CMS hosting.
- **Alternatives considered**: Windows server.
