# Mini Redis — MVP Spec & File Structure

## Goal
A single-binary, in-memory key-value store in C++ that speaks a simple TCP
protocol and supports basic commands. MVP = Week 1 scope only (core store +
TCP server + basic commands). Concurrency, eviction, persistence, and pub/sub
come after MVP is working end-to-end.

## MVP Feature Scope
- TCP server accepting multiple client connections (blocking I/O is fine)
- Simple text-based protocol (RESP-lite, newline-delimited)
- Commands: `SET key value`, `GET key`, `DEL key`, `EXISTS key`, `PING`
- In-memory store backed by `std::unordered_map<std::string, std::string>`
- Graceful shutdown (Ctrl+C closes listening socket cleanly)
- Basic logging to stdout (connection open/close, commands received)

Explicitly OUT of MVP scope: TTL/expiry, LRU eviction, persistence (AOF),
pub/sub, list/set data types, multi-threading. These are Week 2+.

## File Structure

```
mini-redis/
├── README.md
├── CMakeLists.txt
├── .gitignore
├── include/
│   └── miniredis/
│       ├── server.hpp        # TCP server: accept loop, connection handling
│       ├── store.hpp         # Key-value store class (get/set/del/exists)
│       ├── command.hpp       # Command parsing + dispatch
│       └── protocol.hpp      # Wire protocol encode/decode
├── src/
│   ├── main.cpp              # Entry point: parse args, start server
│   ├── server.cpp
│   ├── store.cpp
│   ├── command.cpp
│   └── protocol.cpp
├── tests/
│   ├── test_store.cpp        # Unit tests for Store class
│   ├── test_protocol.cpp     # Unit tests for protocol parsing
│   └── test_command.cpp      # Unit tests for command dispatch
└── scripts/
    └── smoke_test.sh         # netcat/curl-style manual test script
```

## Module Responsibilities

**`store.hpp` / `store.cpp`**
- Class `Store` wrapping `std::unordered_map<std::string, std::string>`
- Methods: `set(key, value)`, `get(key) -> optional<string>`, `del(key) -> bool`,
  `exists(key) -> bool`
- No networking, no parsing — pure data structure logic. Easiest to unit test.

**`protocol.hpp` / `protocol.cpp`**
- Parses raw bytes from socket into a `Command` struct (name + args)
- Serializes responses back into wire format
- Keep this isolated so protocol can evolve (e.g., real RESP later) without
  touching store or server logic

**`command.hpp` / `command.cpp`**
- Takes a parsed `Command` + reference to `Store`, executes it, returns response
- This is the dispatch table: `SET` → `store.set(...)`, etc.
- Add new commands here without touching server code

**`server.hpp` / `server.cpp`**
- Socket setup (bind, listen, accept)
- Per-connection read loop: read bytes → protocol::parse → command::execute →
  protocol::encode → write response
- MVP: single-threaded, one connection at a time or simple sequential accept loop

**`main.cpp`**
- Parse port from argv (default 6380)
- Instantiate `Store`, `Server`, call `server.run()`

## Build Order (suggested)
1. `store.hpp/cpp` + `test_store.cpp` — get this fully working and tested first
2. `protocol.hpp/cpp` + `test_protocol.cpp` — parse `"SET foo bar\n"` style lines
3. `command.hpp/cpp` + `test_command.cpp` — wire store + protocol together
4. `server.cpp` + `main.cpp` — TCP loop, manual test via `nc localhost 6380`
5. `scripts/smoke_test.sh` — scripted end-to-end sanity check

## CMakeLists.txt notes
- C++17 minimum
- Two targets: `miniredis` (main.cpp + src/*.cpp) and `miniredis_tests`
  (tests/*.cpp + src/*.cpp, excluding main.cpp)
- Use a lightweight test framework (Catch2 header-only, or doctest) to avoid
  build complexity

## Definition of MVP Done
- `./miniredis` starts and listens on a port
- From a second terminal: `nc localhost 6380`, type `SET foo bar`, then
  `GET foo` → returns `bar`
- `DEL foo` then `GET foo` → returns nil/empty
- Unit tests pass for store, protocol, and command modules
- README documents how to build and run
