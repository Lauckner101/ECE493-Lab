# Quickstart

## Prerequisites

- C++20 toolchain
- PostgreSQL

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/cms-server
```

## Test

```bash
ctest --test-dir build
```
