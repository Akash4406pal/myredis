# MiniRedis: In-Memory Key-Value Data Store

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=for-the-badge&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/17)
[![CMake](https://img.shields.io/badge/CMake-3.16%2B-064F8C.svg?style=for-the-badge&logo=cmake)](https://cmake.org/)
[![Testing](https://img.shields.io/badge/Tests-Catch2-brightgreen.svg?style=for-the-badge)](https://github.com/catchorg/Catch2)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey.svg?style=for-the-badge)]()
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](LICENSE)

> A high-performance, single-binary, in-memory key-value database engine implemented in modern **C++17**. Features cross-platform TCP networking, custom stream buffering, clean protocol serialization (RESP-lite), modular command dispatching, and comprehensive automated test coverage.

---

## 📌 Executive Summary

**MiniRedis** is a systems programming project demonstrating core database and network server internals from first principles. It delivers an isolated, low-latency key-value storage engine accessible over standard TCP sockets using a newline-delimited wire protocol compatible with Redis-style semantics.

The codebase prioritizes **SOLID design principles**, complete decoupling of network I/O from storage logic, zero memory leaks, modern C++ idioms (`std::optional`, RAII, smart memory layout), and seamless cross-platform execution on both Windows (`Winsock2`) and POSIX/Linux systems.

---

## ⚡ Key Engineering Highlights

- **Decoupled 4-Tier Architecture**: Clean separation between Socket I/O (`Server`), Byte-Stream Serialization (`Protocol`), Dispatch Routing (`CommandDispatcher`), and Storage Mechanics (`Store`).
- **$\mathcal{O}(1)$ Key-Value Storage**: In-memory hashtable engine powered by `std::unordered_map` with null-safe lookup semantics via `std::optional`.
- **Custom TCP Stream Buffering**: Transparently accumulates socket chunks and handles partial reads, preventing framing bugs across network packet boundaries.
- **Cross-Platform Networking**: Unified socket abstraction supporting Windows (`ws2_32`) and Linux/macOS POSIX sockets with zero third-party networking bloat.
- **Defensive Design & Unified Error Handling**: Case-insensitive command parsing, strict token validation, and unified exception barriers preventing server crashes from malformed client inputs.
- **Graceful Lifecycle Management**: Clean signal interception (`SIGINT`, `SIGTERM`) ensuring proper socket closure, memory flushing, and port deallocation.
- **Test-Driven & Verifiable**: Automated test suite with **Catch2** (fetched automatically via CMake `FetchContent`) covering edge cases, concurrent state safety, and serialization boundaries.
- **Turnkey Client Tooling**: Includes zero-dependency Python and PowerShell interactive REPL clients for immediate manual inspection and integration testing.

---

## 🏗️ Architecture & Request Lifecycle

MiniRedis strictly decouples data structures from network transport:

```
+-------------------------------------------------------------------+
|                        TCP Client Layer                           |
|       (Python REPL / PowerShell Client / Netcat / Raw TCP)        |
+---------------------------------+---------------------------------+
                                  |
                           TCP Stream (Byte Chunks)
                                  |
                                  v
+-------------------------------------------------------------------+
|                        Network Layer                              |
|                    miniredis::Server                              |
|  - Manages socket lifecycle (bind, listen, accept, close)         |
|  - Cross-platform abstraction (Winsock2 / POSIX)                  |
|  - Byte buffer splits stream into lines on '\n' / '\r\n'          |
+---------------------------------+---------------------------------+
                                  |
                             Raw Command String
                                  |
                                  v
+-------------------------------------------------------------------+
|                        Protocol Layer                             |
|                   miniredis::Protocol                             |
|  - Tokenizes raw socket payload into Command { name, args }       |
|  - Serializes responses: Simple String, Bulk, Integer, Err, Nil   |
+---------------------------------+---------------------------------+
                                  |
                             miniredis::Command
                                  |
                                  v
+-------------------------------------------------------------------+
|                        Dispatch Layer                             |
|               miniredis::CommandDispatcher                        |
|  - Validates argument count and parameter constraints             |
|  - Normalizes command casing (SET == set == Set)                  |
|  - Wraps execution in safe exception barriers                     |
+---------------------------------+---------------------------------+
                                  |
                             C++ Method Invocation
                                  |
                                  v
+-------------------------------------------------------------------+
|                        Storage Engine                             |
|                             Store                                 |
|  - Pure data structure logic: set(), get(), del(), exists()       |
|  - std::unordered_map backed O(1) hash storage                    |
|  - Completely isolated from networking and wire protocols         |
+-------------------------------------------------------------------+
```

---

## 📂 Project Organization

```text
miniredis/
├── CMakeLists.txt              # CMake build manifest (C++17, Catch2 FetchContent)
├── README.md                   # Technical documentation & project portfolio
├── mini-redis-mvp_1.md         # MVP architecture design specification
├── client.py                   # Cross-platform interactive Python REPL client
├── client.ps1                  # Native PowerShell interactive client for Windows
├── include/
│   └── miniredis/
│       ├── command.hpp         # Command routing & parameter validation interface
│       ├── protocol.hpp        # RESP-lite wire parser & serializer interface
│       ├── server.hpp          # Cross-platform TCP server (Winsock2 / POSIX)
│       └── store.hpp           # Encapsulated in-memory key-value store class
├── src/
│   ├── command.cpp             # Command execution handlers & dispatch table
│   ├── main.cpp                # Server entry point, CLI arguments & signal wiring
│   ├── protocol.cpp            # Wire protocol tokenization & response formatting
│   ├── server.cpp              # Socket listener, connection loop & stream buffer
│   └── store.cpp               # In-memory hash-map storage implementation
├── tests/
│   ├── test_command.cpp        # Unit tests for command dispatch & validation
│   ├── test_protocol.cpp       # Unit tests for protocol encoding/decoding
│   └── test_store.cpp          # Unit tests for core hash store operations
└── scripts/
    └── smoke_test.sh           # End-to-end integration smoke test
```

---

## 💻 Supported Commands & Complexity

All commands are **case-insensitive** (`set`, `SET`, and `Set` behave identically).

| Command | Syntax | Time Complexity | Description | Wire Response |
| :--- | :--- | :---: | :--- | :--- |
| **`PING`** | `PING [message]` | $\mathcal{O}(1)$ | Tests connection vitality or echoes custom message. | `PONG\r\n` or `<message>\r\n` |
| **`SET`** | `SET <key> <value>` | $\mathcal{O}(1)$ avg | Sets the string value of a key in the database. | `OK\r\n` |
| **`GET`** | `GET <key>` | $\mathcal{O}(1)$ avg | Retrieves the value of a key. Returns null if missing. | `<value>\r\n` or `(nil)\r\n` |
| **`EXISTS`**| `EXISTS <key> [keys...]` | $\mathcal{O}(N)$ | Checks how many of the specified keys exist in memory. | `<count>\r\n` (e.g. `2\r\n`) |
| **`DEL`** | `DEL <key> [keys...]` | $\mathcal{O}(N)$ | Removes one or more keys from the database. | `<count>\r\n` (e.g. `1\r\n`) |

---

## 🚀 Getting Started

### Prerequisites

Ensure you have the following installed on your machine:
- **C++ Compiler**: GCC (MinGW for Windows, GCC 8+ on Linux), Clang 7+, or MSVC supporting **C++17**.
- **CMake**: Version **3.16** or higher.
- **Git**: Required by CMake's `FetchContent` to download Catch2 automatically during build configuration.

### 1. Clone Repository

```bash
git clone https://github.com/Akash4406pal/myredis.git
cd myredis
```

### 2. Configure & Build

```bash
# Generate build files
cmake -B build -S .

# Compile the server and test suite
cmake --build build
```

> **Note for Windows MinGW users**: If using MinGW directly in terminal:
> ```bash
> cmake -B build -S . -G "MinGW Makefiles"
> cmake --build build
> ```

This compiles two binaries inside `build/`:
1. `run_server` (`run_server.exe`): The MiniRedis database server daemon.
2. `run_tests` (`run_tests.exe`): Automated unit test runner.

---

## 🕹️ Running the Server

Start the server on the default port (**6380**):

```bash
# Windows
.\build\run_server.exe

# Linux / macOS
./build/run_server
```

Or pass a custom port as a command-line argument:

```bash
# Windows
.\build\run_server.exe 9999

# Linux / macOS
./build/run_server 9999
```

**Graceful Termination**: Press `Ctrl + C` in the server terminal to cleanly disconnect active sessions, release OS sockets, and shut down.

---

## 🧪 Running Unit Tests

MiniRedis uses [Catch2](https://github.com/catchorg/Catch2) for unit testing. All tests are decoupled and can be executed via:

```bash
# Windows
.\build\run_tests.exe

# Linux / macOS
./build/run_tests
```

### Target Specific Test Tags

Catch2 test tags allow you to isolate testing across specific layers:

```bash
.\build\run_tests.exe [store]     # Test core data structure mechanics
.\build\run_tests.exe [protocol]  # Test serialization & parsing edge cases
.\build\run_tests.exe [command]   # Test dispatcher validation & execution
```

---

## 🔌 Connecting Clients

MiniRedis can be accessed with standard TCP utilities or the included interactive clients:

### 1. Interactive Python Client

A zero-dependency REPL client is provided for rapid interaction:

```bash
python client.py 6380
```

**Sample Interactive Session:**
```text
Connected to MiniRedis on port 6380!
Type commands (e.g. PING, SET key val, GET key, DEL key, EXISTS key) or 'exit'/'quit':

127.0.0.1:6380> PING
PONG
127.0.0.1:6380> SET user:1001 Akash
OK
127.0.0.1:6380> GET user:1001
Akash
127.0.0.1:6380> EXISTS user:1001
1
127.0.0.1:6380> DEL user:1001
1
127.0.0.1:6380> GET user:1001
(nil)
127.0.0.1:6380> quit
Disconnecting...
```

### 2. Interactive PowerShell Client (Windows)

For native Windows environments without Python:

```powershell
.\client.ps1 -Port 6380
```

### 3. Netcat / Telnet

You can connect directly using standard networking tools:

```bash
# Netcat
nc localhost 6380

# Telnet
telnet localhost 6380
```

---

## 📡 Wire Protocol Specification (RESP-lite)

MiniRedis implements a human-readable, newline-delimited wire protocol inspired by Redis RESP:

1. **Client Request Framing**: Single-line text commands delimited by `\n` or `\r\n`. Arguments are space-separated.
2. **Server Response Framing**:
   - **Simple String**: `<text>\r\n` (e.g., `OK\r\n`, `PONG\r\n`)
   - **Bulk String**: `<text>\r\n`
   - **Null Value**: `(nil)\r\n`
   - **Integer**: `<value>\r\n` (e.g., `1\r\n`, `0\r\n`)
   - **Error**: `ERR <message>\r\n`

---

## 🔍 Technical Deep Dive & Design Decisions

### 1. Why `std::optional<std::string>` for Lookups?
In C++, distinguishing between a key that exists with an empty string value `""` versus a key that does not exist at all is critical for database correctness. Rather than returning magic sentinel strings (like `"NIL"`), `Store::get()` returns `std::optional<std::string>`, enforcing compile-time null safety.

### 2. Stream Buffering vs. Packet Boundaries
TCP is a byte-stream protocol, not a message-based protocol. A single command may arrive fragmented across multiple `recv()` calls, or multiple commands may arrive in a single packet. MiniRedis implements a per-client buffer accumulation algorithm that scans for newline delimiters (`\n`), processes complete frames, and retains trailing fragments for subsequent reads.

### 3. Safe Dispatch & Exception Boundaries
Commands are routed through a static dispatch matrix. Every command execution is protected by a template wrapper (`safe_execute`) that catches standard exceptions and formats them into Redis-compliant `ERR <message>\r\n` responses, preventing runtime crashes from bringing down the server.

---

## 🗺️ Engineering Roadmap

Future milestones planned for subsequent versions:
- [ ] **I/O Multiplexing Event Loop**: Transition from sequential connection handling to non-blocking event loops using `select` / `epoll` / `kqueue` / Windows `IOCP`.
- [ ] **Extended Data Structures**: Native support for Lists (`LPUSH`, `RPOP`), Sets (`SADD`, `SMEMBERS`), and Hashes (`HSET`, `HGET`).
- [ ] **Key Expiration & TTL**: Active randomized sampling and lazy expiration using min-heaps for scheduled TTL sweeps.
- [ ] **Persistence Engines**: Append-Only File (AOF) background logging and RDB snapshotting.
- [ ] **Full RESP3 Protocol Support**: Full binary-safe protocol compliance enabling connections from official Redis SDKs (`redis-py`, `go-redis`, `Jedis`, `redis-cli`).

---

## 👨‍💻 Author

**Akash Pal**  
- **GitHub**: [@Akash4406pal](https://github.com/Akash4406pal)  
- **Repository**: [https://github.com/Akash4406pal/myredis](https://github.com/Akash4406pal/myredis)  

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).
