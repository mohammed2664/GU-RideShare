# GU Campus RideShare & Carpool Matcher (C11 Core Engine)
## The Complete Engineering Bible, From-Scratch Construction Manual & Comprehensive Viva Defense Guide
**Course:** Problem Solving and Programming Laboratory (CSA5003 PSPL)  
**Institution:** Discipline of Computer Science & Applications, Goa University  
**Degree:** Master of Computer Applications (MCA) — Semester I  
**Team 6:**
- **Mohammed Faaris Khan (Roll 2664):** Lead Architect, Search & Sort Algorithms
- **Ketan Naik (Roll 2623):** Dynamic Heap Memory (`malloc`/`realloc`/`free`), Flat-File Database I/O
- **Kevin Fernandes (Roll 2624):** Parameter Validation, $5 \times 5$ Matrices, Greedy Detour, Recursive Analytics
- **Kreisler Mozart (Roll 2625):** CLI Dispatcher, Terminal Menu, Python IPC Bridge, Leaflet GIS, Dynamic QR Pass

---

# COMPREHENSIVE TABLE OF CONTENTS
1. [Core Architectural Philosophy: Why C11 Machine Code + Python IPC?](#1-core-architectural-philosophy)
2. [Folder Structure & Strict Architectural Separation of Concerns](#2-folder-structure--file-responsibilities)
3. [Low-Level C11 Compilation, Memory Segments & OS Process Model](#3-low-level-c11-compilation--os-process-model)
4. [Minute C Syntax, Data Types, Memory Alignment & Struct Layout](#4-minute-c-syntax-data-types--memory-alignment)
5. [Complete PSPL Lab Syllabus Mapping (Lab 1 through Lab 8 & Lab #G)](#5-complete-pspl-lab-syllabus-mapping)
6. [Dynamic Heap Management & Contiguous Memory Defragmentation](#6-dynamic-heap-management--memory-defragmentation)
7. [Algorithmic Foundations, Big-O Formal Proofs & Recurrence Relations](#7-algorithmic-foundations--big-o-formal-proofs)
8. [Advanced Features Beyond the Lab Syllabus: Mathematical & Engineering Models](#8-advanced-features-beyond-the-syllabus)
9. [Flat-File Storage Engine, Scanset Parsing & ACID Transaction Analysis](#9-flat-file-storage-engine--acid-analysis)
10. [The Python IPC Process Bridge: Subprocess Pipes, Argument Vectors & Concurrency](#10-the-python-ipc-process-bridge)
11. [Web UI & GIS Frontend Layer: Leaflet Maps, State Machine & Dynamic QR](#11-web-ui--gis-frontend-layer)
12. [Step-by-Step Construction Guide: How to Build This From Scratch on a Blank PC](#12-step-by-step-construction-guide)
13. [File-by-File & Function-by-Function Exhaustive Codebase Anatomy](#13-file-by-file-exhaustive-codebase-anatomy)
14. [Individual Team Member Defense Blueprints (Roll 2664, 2623, 2624, 2625)](#14-individual-team-member-defense-blueprints)
15. [Blackboard Code Reference: Algorithms to Write from Memory](#15-blackboard-code-reference)
16. [Common Compiler Errors, Runtime Faults & GDB Debugging Playbook](#16-common-compiler-errors--debugging-playbook)

---

# 1. Core Architectural Philosophy

### 1.1 The Academic Context & The Engineering Problem
The Semester End Assessment (SEA) for **CSA5003: Problem Solving and Programming Laboratory** evaluates a student's mastery over foundational, low-level procedural computing:
- Static arrays, composite structures (`struct`), and multi-dimensional matrices.
- Explicit heap pointer management (`malloc()`, `realloc()`, `free()`).
- Algorithmic efficiency: $O(n)$ linear scan, $O(n^2)$ sorting, and $O(\log n)$ divide-and-conquer binary search.
- Recursive function execution and stack frame allocation.
- Direct filesystem persistence using POSIX/C standard streams (`FILE*`, `fopen()`, `fgets()`, `fprintf()`).

However, traditional academic C assignments run purely inside a primitive black terminal console (`printf` and `scanf`). While academically rigorous, a pure console CLI cannot visually demonstrate:
1. Spatial carpool routes across Goa's geographic terrain.
2. Real-time GPS movement and dynamic ETA updates.
3. Mobile QR boarding pass verification.
4. Modern multi-user session management and role-based access control.

### 1.2 The 3-Tier Decoupled Architectural Solution
Rather than compromising academic integrity by abandoning C for Python/Node.js, or building a fragile terminal UI, this project implements an **enterprise-grade 3-Tier Decoupled Architecture**:

```
+=============================================================================+
|             TIER 1: CLIENT PRESENTATION LAYER (Browser / Mobile)            |
|  • HTML5 Semantic DOM  • Tailwind CSS Glassmorphism  • Vanilla ES6 State    |
|  • Leaflet.js GIS Mapping (WGS-84)  • Dynamic SVG QR Transit Code Generator |
|  • Live GPS Telemetry Kinematic Stepper  • Activity Notification Drawer     |
+======================================+======================================+
                                       |
                     Standard HTTP/1.1 REST JSON Requests
                     (GET, POST, OPTIONS over port 8080)
                                       v
+=============================================================================+
|             TIER 2: PROCESS BRIDGE LAYER (Python: web/server.py)            |
|  • Native http.server (Zero external dependencies: No Flask, No FastAPI)   |
|  • Thread Concurrency Lock (threading.Lock) preventing race conditions      |
|  • Subprocess Execution via Argument Vector: subprocess.Popen(argv)         |
|  • Anonymous Pipe Stream Capture: stdout/stderr line-by-line reading        |
|  • Pipe-Delimited text to JSON Serialization                                |
+======================================+======================================+
                                       |
                   Anonymous OS Pipe IPC (CLI Flags & stdout)
                                       v
+=============================================================================+
|             TIER 3: CORE ALGORITHMIC ENGINE (Native C11 Machine Code)       |
|  • Compiled via GCC: gcc -Wall -Wextra -std=c11 -O2                         |
|  • Zero dynamic language runtime: Runs bare-metal on x86-64 CPU             |
|  • Dynamic Heap Memory Allocation: Initial pool + 2x geometric growth       |
|  • In-Memory Contiguous Array Compaction (Heap Defragmentation)             |
|  • 5x5 Symmetric Distance/Fare Matrices & Greedy 1-Stop Transfer Heuristic   |
|  • Selection Sort O(n^2) & Bubble Sort with Early Exit O(n)                 |
|  • Recursive Divide-and-Conquer Binary Search O(log n)                      |
|  • Recursive Fleet Aggregators O(n)                                         |
|  • Trust Scoring Engine & Kinematic Telemetry Interpolation                 |
+======================================+======================================+
                                       |
                     Direct POSIX/C File Stream I/O
                     (fopen, fgets, sscanf, fprintf, fclose)
                                       v
+=============================================================================+
|               PERSISTENT STORAGE LAYER (Flat-File Records in data/)         |
|  • data/rides.txt          • data/bookings.txt      • data/users.txt        |
|  • data/notifications.txt  • data/campus_summary.txt                        |
+=============================================================================+
```

### 1.3 Why Python Never Calculates Business Logic
An external examiner's first suspicion is often: *"Did you just write the carpool matcher in Python and use C as a dummy wrapper?"*
**The absolute proof of C supremacy:**
1. In `web/server.py`, there are **zero** calculations of distance, fares, seat availability, sorting, searching, or trust scores.
2. Every time a user clicks "Search", "Book", "Cancel", "Register", "Track", or "Sort", Python does exactly one thing:
   ```python
   cmd = ["build/rideshare.exe", "--search-route", origin, dest, keyword, date, min_seats]
   proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, ...)
   ```
3. Python merely captures the pipe-delimited raw stdout strings printed by C's `printf()` and wraps them in JSON brackets `{}`.
4. If you delete `web/server.py` and the entire `web/` folder, the project remains **100% operational** as an interactive terminal console application via `build/rideshare.exe` without missing a single feature.

---

# 2. Folder Structure & Strict Architectural Separation

The workspace is organized with zero duplicate files at the root:

```text
PSPL/
├── include/
│   └── rideshare.h          <-- Single Source of Truth: typedefs, macros, prototypes, matrix externs
├── src/
│   ├── main.c              <-- CLI dispatcher, argv parser, interactive console menu, 6/6 test suite
│   ├── storage.c           <-- Dynamic heap management (malloc/realloc/free), flat-file pipe-delimited I/O
│   └── algorithms.c        <-- Search, sort, 2D matrices, greedy routing, recursion, trust engine, telemetry
├── web/
│   ├── server.py           <-- Python HTTP bridge, session management, thread-safe subprocess spawner
│   ├── index.html          <-- Semantic HTML5 SPA, Leaflet container, live telemetry & pass modals
│   ├── app.js              <-- Frontend state, Leaflet GIS markers, authenticated API calls, SVG QR engine
│   └── styles.css          <-- Custom styles, map container rules, glassmorphism UI styling
├── data/
│   ├── rides.txt           <-- Pipe-delimited active carpool rides (auto-seeded)
│   ├── bookings.txt        <-- Pipe-delimited booking records & passenger manifests
│   ├── users.txt           <-- Verified @unigoa.ac.in user accounts, roles, hashed PINs, trust scores
│   ├── notifications.txt   <-- Campus activity log, security OTP alerts, driver status updates
│   └── campus_summary.txt  <-- Exported formal C audit report generated by C backend
├── build/
│   └── rideshare.exe       <-- Native 64-bit compiled binary generated by GCC
├── scripts/
│   ├── build.bat           <-- One-click clean compile, self-test verification, and localhost server launch
│   └── start-mobile.bat    <-- Binds server to 0.0.0.0:8080 for live mobile testing on college Wi-Fi
├── docs/
│   ├── Team 6_Campus RideShare & Carpool Matcher.pdf  <-- Official 8-page SEA submission report
│   ├── MASTER_STUDY_GUIDE.md                          <-- This exhaustive reference document
│   ├── report_template.html                           <-- High-precision HTML/CSS print template for report
│   ├── render_pdf.py                                  <-- Automated Playwright script that renders report PDF
│   └── images/                                        <-- High-resolution UI screenshots embedded in PDF
├── .gitignore              <-- Excludes build artifacts and temporary files
└── README.md               <-- Project overview and quickstart guide
```

---

# 3. Low-Level C11 Compilation, Memory Segments & OS Process Model

### 3.1 The 4 Stages of C Compilation
When you execute `scripts/build.bat`, GCC invokes four distinct internal tools to transform human-readable C text into binary machine code:

```
[rideshare.h, src/*.c]
         |
         v  Stage 1: Preprocessor (cpp)
    Expanded Source (.i)  <-- Macro expansion (#define), file inclusion (#include), conditional compilation (#ifndef)
         |
         v  Stage 2: Compiler Proper (cc1)
    Assembly Code (.s)    <-- Parses C syntax, generates abstract syntax tree (AST), optimizes, outputs x86-64 assembly
         |
         v  Stage 3: Assembler (as)
    Object Files (.o)     <-- Translates assembly mnemonics into relocatable machine instructions & symbol tables
         |
         v  Stage 4: Linker (ld)
    Executable Binary (.exe) <-- Resolves cross-file symbol references (extern), links C runtime (crt0.o, msvcrt.dll)
```

### 3.2 Detailed Breakdown of Compiler Flags
```bash
gcc -Wall -Wextra -std=c11 -O2 -Iinclude src/main.c src/storage.c src/algorithms.c -o build/rideshare.exe
```
- `-Wall`: Enables all primary compiler warning diagnostics (uninitialized variables, unused variables, format string mismatches).
- `-Wextra`: Enables secondary, rigorous compiler warnings (unused parameters, sign comparison warnings, empty bodies).
- `-std=c11`: Enforces strict adherence to the **ISO/IEC 9899:2011 (C11)** standard, forbidding compiler-specific non-standard extensions.
- `-O2`: Enables level 2 compiler optimizations:
  - **Loop Unrolling:** Duplicates small loop bodies to reduce branching overhead.
  - **Function Inlining:** Eliminates function call overhead for tiny static utility functions.
  - **Register Allocation:** Allocates frequently used local loop counters (`i`, `j`, `minIdx`) into CPU registers (`%rax`, `%rbx`, `%rcx`) rather than writing to RAM.
  - **Peephole Optimization:** Replaces redundant instruction sequences with single hardware instructions.
- `-Iinclude`: Informs the preprocessor that `#include "rideshare.h"` should search inside the `include/` directory.
- `-o build/rideshare.exe`: Specifies the destination path for the linked binary executable.

### 3.3 The 5 Memory Segments of the Running Process
When `build/rideshare.exe` is loaded into operating system memory by Windows (`ntdll!LdrInitializeThunk`), its virtual address space (VAS) is partitioned into 5 distinct segments:

```
+-------------------------------------------------------------+ High Memory (0x7FFFFFFFFFFF)
| Stack Segment (Grows Downward towards 0x0)                  |
| - Local function variables (int low, int high, int mid)     |
| - Return addresses of function calls                        |
| - Activation records of recursive binary search calls       |
+-------------------------------------------------------------+
|                               | (Free virtual address space)
|                               v
|                               ^
|                               |
+-------------------------------------------------------------+
| Heap Segment (Grows Upward towards High Memory)             |
| - Dynamically allocated memory via malloc() & realloc()     |
| - db->rides, db->bookings, db->users, db->notifications     |
| - Managed manually via free() to prevent leaks              |
+-------------------------------------------------------------+
| BSS Segment (Block Started by Symbol)                       |
| - Uninitialized global and static variables (zeroed by OS)  |
+-------------------------------------------------------------+
| Data Segment (Initialized Static/Global Data)               |
| - const int GOA_DISTANCE_MATRIX[5][5]                       |
| - const int GOA_FARE_MATRIX[5][5]                           |
| - const char *GOA_LOCATIONS[5]                              |
+-------------------------------------------------------------+
| Text Segment (Code Segment - Read Only)                     |
| - Compiled x86-64 machine instructions                      |
| - String literals ("data/rides.txt", "CONFIRMED", etc.)     |
+-------------------------------------------------------------+ Low Memory (0x000000000000)
```

---

# 4. Minute C Syntax, Data Types & Memory Alignment

### 4.1 Header Guards: Why `#ifndef RIDESHARE_H` is Mandatory
Every C header must have an include guard:
```c
#ifndef RIDESHARE_H
#define RIDESHARE_H
/* declarations */
#endif
```
**Why?** In C, if `storage.c` includes `rideshare.h` and `main.c` includes `rideshare.h`, and both are compiled together, without an include guard the compiler would see `typedef struct { ... } User;` defined twice, resulting in a fatal compilation error: `error: redefinition of 'struct User'`.
The guard ensures that after the preprocessor defines `RIDESHARE_H` during the first pass, all subsequent inclusions in the same translation unit evaluate to false and are skipped.

### 4.2 Struct Memory Alignment and Byte Padding
In C, the size of a structure is **not** necessarily equal to the sum of the sizes of its individual members!
Consider `struct User`:
```c
typedef struct {
    int  userId;                         /* 4 bytes */
    char fullName[MAX_NAME_LEN];        /* 64 bytes */
    char email[128];                     /* 128 bytes */
    char role[32];                       /* 32 bytes */
    char phone[32];                      /* 32 bytes */
    char passwordPin[64];                /* 64 bytes */
    int  isVerified;                     /* 4 bytes */
    char dlNumber[32];                   /* 32 bytes */
    int  trustScore;                     /* 4 bytes */
} User;
```
- Total raw bytes: $4 + 64 + 128 + 32 + 32 + 64 + 4 + 32 + 4 = 364\text{ bytes}$.
- **CPU Alignment Rule:** On modern 64-bit x86-64 architectures, the CPU reads data from memory in 4-byte or 8-byte aligned words. Accessing an unaligned 4-byte integer spanning two memory words requires two memory read cycles and bitwise shifting, degrading performance.
- Because 364 is a multiple of 4, no extra trailing padding is required, but if an odd-length `char flag` were placed after `userId`, the compiler would insert 3 bytes of invisible padding!

### 4.3 Pointers, Indirection & Arrow Operator Mechanics
In C, when a pointer to a struct is passed to a function:
```c
int ensure_ride_capacity(RideShareDB *db);
```
- `db` holds a 64-bit memory address (e.g., `0x000000A1F4`).
- `db->rideCount` is exact syntactic sugar for:
  $$\text{(*db).rideCount} \iff *((\text{int}*)(((\text{char}*)db) + \text{offsetof}(\text{RideShareDB, rideCount})))$$
- The arrow operator (`->`) first **dereferences** the pointer to find the struct in memory, then calculates the byte offset of the target field.

### 4.4 Pointer Arithmetic Deep-Dive
Consider:
```c
Ride *r = &db->rides[mid];
```
In C, array indexing `db->rides[mid]` is identical to `*(db->rides + mid)`.
When you add integer `mid` to the pointer `db->rides`:
$$\text{Target Address} = \text{Base Address} + (\text{mid} \times \text{sizeof}(\text{Ride}))$$
The compiler automatically multiplies `mid` by the size of the underlying data type (`sizeof(Ride)` $\approx 312\text{ bytes}$). You do **not** multiply by bytes manually.

### 4.5 The Null Terminator (`\0`) and String Safety
A string in C is merely a contiguous 1D array of `char` values terminated by the ASCII null byte `'\0'` (numeric value `0`).
- **The Buffer Overflow Hazard:** Using `strcpy(dest, src)` or `gets()` reads until it sees a null byte. If the input exceeds the destination buffer, it writes past the allocated array, corrupting the stack frame return address (the basis of classic stack-smashing exploits).
- **Why `strncpy` is Dangerous:** `strncpy(dest, src, n)` will **not** append a `'\0'` if `src` is equal to or longer than `n` bytes! This leaves a unterminated string that causes subsequent `printf("%s")` calls to read garbage bytes from adjacent memory until a random zero byte is hit.
- **Our C11 Solution:** In `algorithms.c`, we strictly use `snprintf`:
  ```c
  snprintf(outBooking->passengerName, sizeof(outBooking->passengerName), "%s", passengerName);
  ```
  `snprintf` guarantees that regardless of input length, at most `sizeof(buffer) - 1` characters are written, and the final byte is **always guaranteed to be `\0`**.

---

# 5. Complete PSPL Lab Syllabus Mapping

The following table proves complete coverage of the entire Goa University MCA CSA5003 syllabus:

| Lab Module | Syllabus Concept | Where Implemented | Code File & Line Range | Exact Architectural Implementation Details |
|---|---|---|---|---|
| **Lab 1 & 2** | Constants, Operators, Expressions & Formatted I/O | `include/rideshare.h`, `src/main.c` | `rideshare.h:21-34`, `main.c:20-65` | `#define` macro constants; 24-hr time arithmetic via integer division and modulo (`hours = depTime / 100`, `mins = depTime % 100`); tabular aligned console printing via `printf("%-8d \| %-20s \| %-8d\n", ...)`. |
| **Lab 3** | Decision Making, Branching (`if-else`, `switch`) | `src/algorithms.c`, `src/main.c` | `algorithms.c:60-105`, `main.c:540-620` | Parameter validation returning distinct error codes: `-1` (invalid origin), `-2` (invalid dest), `-3` (origin == dest), `-4` (invalid HHMM time $> 2359$ or mins $> 59$), `-5` (seats $< 1$ or $> 7$). 15-case interactive console menu (`switch(choice)`). |
| **Lab 4** | 1D Arrays & Linear Search | `src/algorithms.c` | `algorithms.c:14-20`, `algorithms.c:220-275` | 1D string array `GOA_LOCATIONS[5]`. Multi-criteria `linear_search_by_route()` filtering simultaneously across 5 parameters (origin, dest, substring keyword, date, min seats). Contiguous array left-shifting for in-memory deletion. |
| **Lab 5** | 2D Arrays & Selection Sort | `src/algorithms.c` | `algorithms.c:23-38`, `algorithms.c:278-298` | Symmetric static $5 \times 5$ matrices `GOA_DISTANCE_MATRIX[5][5]` and `GOA_FARE_MATRIX[5][5]`. Selection Sort (`selection_sort_by_time`): finds minimum departure time in unsorted subarray and swaps to front ($O(n^2)$). |
| **Lab 6** | Sorting Algorithms (Bubble Sort) | `src/algorithms.c` | `algorithms.c:300-335` | Bubble Sort (`bubble_sort_by_fare`): sorts rides by ascending fare per seat. Implements a boolean `swapped` early-exit flag to achieve optimal $O(n)$ best-case time complexity on pre-sorted data. |
| **Lab 7** | Recursion & String Manipulation | `src/algorithms.c` | `algorithms.c:338-362`, `algorithms.c:110-150` | Divide-and-conquer Recursive Binary Search (`binary_search_ride_recursive()`). Recursive aggregation accumulators (`recursive_sum_available_seats()`, `recursive_sum_fare_pool()`, `recursive_sum_booked_revenue()`). String sanitization (`sanitize_pipe_chars()`). |
| **Lab 8** | Structures & Dynamic Memory Allocation | `src/storage.c`, `include/rideshare.h` | `storage.c:25-140`, `rideshare.h:97-110` | Composite structs (`User`, `Ride`, `Booking`, `Notification`). Dynamic heap allocation: initial allocation via `malloc()`, geometric capacity doubling via `realloc()`, and complete heap cleanup via `free()`. |
| **Lab #G** | File Handling & Data Persistence | `src/storage.c` | `storage.c:145-340`, `storage.c:430-515` | Flat-file pipe-delimited persistence: `fopen()`, `fgets()`, scanset parsing via `sscanf()`, `fprintf()`, `fclose()`. Cross-platform path resolution (`resolve_read_path()`). C Audit Report generation (`campus_summary.txt`). |

---

# 6. Dynamic Heap Management & Memory Defragmentation

### 6.1 The Mechanics of `malloc()`, `realloc()` and `free()`
In C, the programmer is completely responsible for memory lifecycle management:
1. `malloc(size_t size)`: Requests a raw contiguous block of `size` bytes from the OS heap. Returns a `void*` pointer to the first byte. If the OS has no free virtual memory, it returns `NULL`. The memory contains uninitialized garbage bytes.
2. `realloc(void *ptr, size_t new_size)`: Resizes an existing heap allocation.
   - If contiguous space is available immediately following the current block, it expands the block in-place and returns the same pointer.
   - If not, it allocates a new block of `new_size` bytes elsewhere on the heap, copies the existing data into the new block, frees the old block automatically, and returns the new pointer.
   - If allocation fails, it returns `NULL`, but the original block at `ptr` remains untouched and valid!
3. `free(void *ptr)`: Returns the allocated heap block to the operating system's free list. It does not zero out the pointer variable, creating a potential "dangling pointer" unless explicitly set to `NULL`.

### 6.2 Geometric Capacity Doubling & Amortized $O(1)$ Analysis
Our database structure starts with an initial capacity of 4:
```c
int ensure_ride_capacity(RideShareDB *db) {
    if (db->rideCount >= db->rideCapacity) {
        int newCap = (db->rideCapacity == 0) ? INITIAL_CAPACITY : db->rideCapacity * 2;
        Ride *newPtr = (Ride *)realloc(db->rides, sizeof(Ride) * newCap);
        if (newPtr == NULL) return 0; /* Allocation failure guard */
        db->rides = newPtr;
        db->rideCapacity = newCap;
    }
    return 1;
}
```
**Why double by $2\times$ instead of incrementing by $+1$?**
- If you expand capacity by $+1$ on every insert, inserting $N$ items requires calling `realloc()` $N$ times and copying $1 + 2 + 3 + \dots + N = \frac{N(N-1)}{2} = O(N^2)$ elements!
- With **geometric doubling** ($4 \rightarrow 8 \rightarrow 16 \rightarrow 32 \dots$):
  - Copies occur only at powers of 2.
  - Total element copies for $N$ inserts:
    $$\sum_{k=0}^{\log_2 N} 2^k = 2^{\log_2 N + 1} - 1 = 2N - 1 = O(N)$$
  - **Amortized Time Per Insertion:** $\frac{O(N)}{N} = O(1)$ constant time!

### 6.3 Memory Leak Prevention Protocol
A memory leak occurs when heap memory is allocated via `malloc`/`realloc` but its pointer is lost without calling `free()`. Over time, the process consumes all available RAM and is terminated by the OS Out-Of-Memory (OOM) killer.
In `src/storage.c`, we implement strict teardown:
```c
void free_database(RideShareDB *db) {
    if (!db) return;
    if (db->rides)         { free(db->rides);         db->rides = NULL; }
    if (db->bookings)      { free(db->bookings);      db->bookings = NULL; }
    if (db->users)         { free(db->users);         db->users = NULL; }
    if (db->notifications) { free(db->notifications); db->notifications = NULL; }
    db->rideCount = db->rideCapacity = 0;
    db->bookingCount = db->bookingCapacity = 0;
    db->userCount = db->userCapacity = 0;
    db->notificationCount = db->notificationCapacity = 0;
}
```

### 6.4 Contiguous Heap Defragmentation (Purging Cancelled Bookings)
When a passenger cancels a booking, its status becomes `"CANCELLED"`. Over time, cancelled records pollute heap memory and file storage.
In `src/algorithms.c`, `purge_cancelled_bookings_transaction()` implements **in-place contiguous array compaction** using a two-pointer technique:

```c
int purge_cancelled_bookings_transaction(RideShareDB *db) {
    int writeIdx = 0;
    int purgedCount = 0;
    for (int readIdx = 0; readIdx < db->bookingCount; readIdx++) {
        if (strcmp(db->bookings[readIdx].bookingStatus, "CANCELLED") != 0) {
            if (writeIdx != readIdx) {
                db->bookings[writeIdx] = db->bookings[readIdx]; /* Struct copy */
            }
            writeIdx++;
        } else {
            purgedCount++;
        }
    }
    db->bookingCount = writeIdx; /* Truncate logical length */
    save_bookings_to_file(db, BOOKINGS_FILE);
    return purgedCount;
}
```
- **How it works:** `readIdx` scans every record. Active records are copied forward to `writeIdx`. Cancelled records are skipped.
- **Time Complexity:** $O(n)$ single-pass scan.
- **Space Complexity:** $O(1)$ auxiliary space (operates entirely in-place without allocating a temporary array).

---

# 7. Algorithmic Foundations, Big-O Formal Proofs & Recurrence Relations

### 7.1 Master Complexity Reference Table

| Algorithm | Function Name | Best Case Time | Average Case Time | Worst Case Time | Auxiliary Space Complexity | Stability |
|---|---|---|---|---|---|---|
| **Linear Search** | `linear_search_by_route` | $\Omega(1)$ | $\Theta(n)$ | $O(n)$ | $O(1)$ auxiliary | N/A |
| **Recursive Binary Search** | `binary_search_ride_recursive` | $\Omega(1)$ | $\Theta(\log n)$ | $O(\log n)$ | $O(\log n)$ call stack frames | N/A |
| **Selection Sort** | `selection_sort_by_time` | $\Omega(n^2)$ | $\Theta(n^2)$ | $O(n^2)$ | $O(1)$ in-place | Unstable |
| **Bubble Sort (Early Exit)** | `bubble_sort_by_fare` | $\Omega(n)$ | $\Theta(n^2)$ | $O(n^2)$ | $O(1)$ in-place | Stable |
| **Recursive Seat Accumulator**| `recursive_sum_available_seats`| $\Omega(n)$ | $\Theta(n)$ | $O(n)$ | $O(n)$ call stack frames | N/A |
| **Greedy 1-Stop Transfer** | `find_best_stopover_hub` | $\Omega(V)$ | $\Theta(V)$ | $O(V)$ where $V=5$ | $O(1)$ | N/A |
| **In-Memory Heap Compaction**| `purge_cancelled_bookings` | $\Omega(n)$ | $\Theta(n)$ | $O(n)$ | $O(1)$ in-place | Stable |

---

### 7.2 Selection Sort: Mathematical Derivation & Why It Is Always $\Theta(n^2)$
In `src/algorithms.c`:
```c
void selection_sort_by_time(Ride *rides, int count) {
    for (int i = 0; i < count - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < count; j++) {
            if (rides[j].departureTime < rides[minIdx].departureTime) {
                minIdx = j;
            }
        }
        if (minIdx != i) {
            Ride temp = rides[i];
            rides[i] = rides[minIdx];
            rides[minIdx] = temp;
        }
    }
}
```
**Mathematical Proof of Time Complexity:**
- Outer loop runs for $i = 0$ to $n - 2$ ($n - 1$ times).
- For a given $i$, the inner loop executes $n - 1 - i$ comparisons.
- Total comparisons $C(n)$:
  $$C(n) = \sum_{i=0}^{n-2} (n - 1 - i) = (n - 1) + (n - 2) + \dots + 2 + 1 = \frac{(n - 1)n}{2} = \frac{n^2 - n}{2}$$
- Therefore, $C(n) \in \Theta(n^2)$ across **all** inputs (best, average, and worst case), because the inner loop must always inspect every remaining element to guarantee that `minIdx` is indeed the absolute minimum.
- **Swaps:** At most $n - 1$ swaps across the entire sort. When writing to memory is significantly more expensive than reading (e.g. Flash/EEPROM), Selection Sort is superior to Bubble Sort!

---

### 7.3 Bubble Sort with Early Exit: Why Best Case is $\Omega(n)$
In `src/algorithms.c`:
```c
void bubble_sort_by_fare(Ride *rides, int count) {
    for (int i = 0; i < count - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < count - 1 - i; j++) {
            if (rides[j].farePerSeat > rides[j + 1].farePerSeat) {
                Ride temp = rides[j];
                rides[j] = rides[j + 1];
                rides[j + 1] = temp;
                swapped = 1;
            }
        }
        if (!swapped) break; /* Early exit: Array is fully sorted */
    }
}
```
**Why does the `swapped` flag matter?**
- In standard unoptimized Bubble Sort, the algorithm executes all $\frac{n(n-1)}{2}$ comparisons even if the input array is already sorted.
- With our `swapped` boolean flag:
  - If the array is already sorted, the first pass executes $n - 1$ comparisons.
  - Zero swaps occur (`swapped == 0`).
  - The `break` statement executes immediately.
  - **Best-case Time Complexity:** Exactly $n - 1$ operations $\implies \Omega(n)$ linear time!

---

### 7.4 Recursive Binary Search: Recurrence Relation & Master Theorem Proof
In `src/algorithms.c`:
```c
int binary_search_ride_recursive(const Ride *rides, int low, int high, int targetRideId) {
    if (low > high) {
        return -1; /* Base Case: Target does not exist */
    }
    int mid = low + (high - low) / 2; /* Guard against integer overflow */
    if (rides[mid].rideId == targetRideId) {
        return mid; /* Base Case: Found */
    } else if (rides[mid].rideId > targetRideId) {
        return binary_search_ride_recursive(rides, low, mid - 1, targetRideId);
    } else {
        return binary_search_ride_recursive(rides, mid + 1, high, targetRideId);
    }
}
```

**1. The Integer Overflow Trap:**
- Why write `low + (high - low) / 2` instead of `(low + high) / 2`?
- In C, signed 32-bit `int` has a maximum value of `INT_MAX` ($2,147,483,647$).
- If `low` and `high` are both large, `low + high` overflows into negative numbers (undefined behavior in C, wrapping to negative), resulting in a negative array index and an instant **Segmentation Fault**!
- Writing `low + (high - low) / 2` computes the exact same mathematical value while strictly preventing overflow because `high - low` is guaranteed to be non-negative and smaller than `high`.

**2. Master Theorem Proof:**
- At each step, the algorithm performs $O(1)$ comparisons and divides the search space in half.
- Recurrence relation:
  $$T(n) = T\left(\frac{n}{2}\right) + O(1)$$
- Applying the Master Theorem for divide-and-conquer recurrences:
  $$T(n) = a T\left(\frac{n}{b}\right) + f(n)$$
  Here $a = 1$, $b = 2$, and $f(n) = O(n^0) = O(1)$.
- Calculate the critical exponent:
  $$c = \log_b a = \log_2 1 = 0$$
- Since $f(n) = \Theta(n^c) = \Theta(n^0) = \Theta(1)$, Case 2 of the Master Theorem applies:
  $$T(n) = \Theta(n^c \log n) = \Theta(n^0 \log n) = \Theta(\log n)$$
- **Space Complexity:** Each recursive call pushes an activation record onto the OS stack. The maximum call depth is $\lceil \log_2 n \rceil + 1$, yielding $O(\log n)$ stack space.

---

### 7.5 Recursive Fleet Analytics: Call Stack Trace
In `src/algorithms.c`:
```c
int recursive_sum_available_seats(const Ride *rides, int count, int index) {
    if (index >= count) {
        return 0; /* Base Case */
    }
    return rides[index].availableSeats + recursive_sum_available_seats(rides, count, index + 1);
}
```
**Call Stack Unwinding Trace for $N=3$ rides (Seats: 3, 2, 4):**
```
Push Frame 1: index = 0 -> waits for Frame 2
  Push Frame 2: index = 1 -> waits for Frame 3
    Push Frame 3: index = 2 -> waits for Frame 4
      Push Frame 4: index = 3 -> Base Case hit (3 >= 3) -> Returns 0
    Frame 3 receives 0: returns rides[2].seats (4) + 0 = 4 (Pops Frame 3)
  Frame 2 receives 4: returns rides[1].seats (2) + 4 = 6 (Pops Frame 2)
Frame 1 receives 6: returns rides[0].seats (3) + 6 = 9 (Pops Frame 1)
Final Result: 9 seats
```

---

# 8. Advanced Features Beyond the Lab Syllabus

These next-level features elevate the project from a simple classroom assignment to an enterprise-grade carpool platform:

### 8.1 Live GPS Route Telemetry & Kinematic Interpolation
- **The Problem:** In real life, users want to see where their car is.
- **The Mathematical Solution in C (`algorithms.c:577-703`):**
  - Defines static WGS-84 coordinates for the 5 Goan hubs:
    - Goa University: `(15.4589, 73.8344)`
    - Panjim: `(15.4989, 73.8278)`
    - Margao: `(15.2736, 73.9582)`
    - Vasco: `(15.3982, 73.8113)`
    - Ponda: `(15.4026, 74.0152)`
  - Generates 5 realistic transit waypoints along each corridor.
  - Applies **Piece-wise Linear Interpolation (Lerp)** based on trip progress $p \in [0, 100]\%$:
    $$\text{currentLat} = \text{startLat} + \left(\frac{p - p_{\text{start}}}{p_{\text{end}} - p_{\text{start}}}\right) \times (\text{endLat} - \text{startLat})$$
  - Computes remaining distance $d_{\text{rem}} = d_{\text{total}} \times (1 - \frac{p}{100})$ and dynamic ETA assuming an average campus corridor speed $v = 35\text{ km/h}$:
    $$\text{etaMinutes} = \left(\frac{d_{\text{rem}}}{35.0}\right) \times 60$$
  - Transitions through a 5-step Finite State Machine:
    $1: \text{Scheduled} \rightarrow 2: \text{Driver Approaching} \rightarrow 3: \text{Boarding \& OTP} \rightarrow 4: \text{In Transit} \rightarrow 5: \text{Arrived}$.

### 8.2 Campus Trust Scoring Mathematical Engine
- **Formula (`algorithms.c:705-745`):**
  $$\text{Score} = \text{clamp}\Big(50 + (\text{isVerified} \times 30) + W_{\text{role}} + \min(15, 2 \times N_{\text{completed}}) - (10 \times N_{\text{cancelled}}), 10, 100\Big)$$
  - Base Score: $50$
  - University KYC Verification: $+30$ points
  - Role Weights ($W_{\text{role}}$): Faculty $= +10$, Staff $= +5$, Student $= +5$
  - Experience Bonus: $+2$ points per completed trip (capped at $+15$)
  - Cancellation Penalty: $-10$ points per cancellation
  - Clamping Function: $\text{clamp}(x, 10, 100) = \min(100, \max(10, x))$
- **Tier Classification:**
  - $\ge 90$: *Elite Campus Driver* (Gold Badge)
  - $75\text{--}89$: *Verified Driver* (Teal Badge)
  - $50\text{--}74$: *Standard Driver* (Gray Badge)
  - $< 50$: *Under Review* (Red Warning)

### 8.3 Greedy 1-Stop Transfer Heuristic (Graph Detour)
When no direct carpool ride exists between two points (e.g. Margao to Vasco), `find_best_stopover_hub()` evaluates all intermediate nodes $h \in \{0, 1, 2, 3, 4\} \setminus \{\text{origin}, \text{dest}\}$:
$$h^* = \arg\min_h \Big(\text{GOA\_DISTANCE\_MATRIX}[\text{origin}][h] + \text{GOA\_DISTANCE\_MATRIX}[h][\text{dest}]\Big)$$
It returns the hub that minimizes total detour kilometers (typically Panjim KTC Bus Stand), solving indirect connectivity in $O(V)$ time.

---

# 9. Flat-File Storage Engine, Scanset Parsing & ACID Analysis

### 9.1 The Pipe-Delimited Serialization Format
Data is stored in plain human-readable text files inside `data/`:
- **`data/rides.txt` (14 fields):**
  ```text
  101|Aarav Naik|GA-01-AB-1204|1|0|Panjim -> Miramar -> Goa Univ|0815|4|3|7|30|2026-11-23|1|95
  ```
- **`data/users.txt` (9 fields):**
  ```text
  1001|Mohammed Faaris Khan|2664@unigoa.ac.in|Student|9876543210|2664|1|GA-07-2024-DL|95
  ```
- **`data/bookings.txt` (6 fields):**
  ```text
  5001|101|Ketan Naik|1|30|CONFIRMED
  ```

### 9.2 Scanset Parsing with `sscanf()`
Reading structured text without crashing requires advanced `sscanf()` scanset format specifiers:
```c
sscanf(line, "%d|%63[^|]|%31[^|]|%d|%d|%127[^|]|%d|%d|%d|%d|%d|%15[^|]|%d|%d",
       &ride.rideId, ride.driverName, ride.vehicleNumber, &ride.originId, ...);
```
**Anatomy of `%63[^|]`:**
- `%`: Begins conversion specification.
- `63`: Maximum field width. Prevents buffer overflow by reading at most 63 characters into a 64-byte array!
- `[^|]`: Negated scanset. Matches any character **except** the pipe character `|`.
- When the pipe `|` is encountered, `sscanf` stops reading into the string, places a null terminator `'\0'`, matches the literal `|` in the format string, and proceeds to the next field.

### 9.3 ACID Transaction Analysis
Can a C flat-file system guarantee ACID properties?
1. **Atomicity:** Ensured via single-file rewriting. When booking a seat, C validates capacity, decrements `availableSeats` in memory, appends the booking struct, and writes both `rides.txt` and `bookings.txt` to completion before returning success.
2. **Consistency:** Invariants are strictly enforced by C validation functions (`origin != dest`, `availableSeats <= totalSeats`).
3. **Isolation:** The Python bridge enforces a global thread mutex (`threading.Lock()`). No two HTTP requests can invoke `rideshare.exe` concurrently, preventing dirty reads and write collisions.
4. **Durability:** File streams are closed with `fclose()`, which flushes OS buffers directly to physical persistent disk.

---

# 10. The Python IPC Process Bridge

### 10.1 Subprocess Argument Vectors & Pipe Redirection
In `web/server.py`, Python communicates with C via operating system pipes:
```python
C_LOCK = threading.Lock()

def run_c_backend(args):
    bin_path = get_binary_path() # build/rideshare.exe
    cmd = [bin_path] + [str(a) for a in args]
    with C_LOCK:
        proc = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            cwd=PROJECT_ROOT,
            universal_newlines=True
        )
        out, err = proc.communicate()
        lines = [l.strip() for l in out.splitlines() if l.strip()]
        return lines, proc.returncode
```
- `subprocess.PIPE`: Directs the child process's standard output file descriptor (FD 1) into an anonymous pipe buffer in memory.
- `proc.communicate()`: Reads all data until End-Of-File (EOF) and waits for the C process to terminate, preventing pipe deadlocks and zombie processes.
- `threading.Lock()`: Ensures that if two users click "Book Seat" at the exact same millisecond, the requests are queued sequentially.

### 10.2 JSON Serialization Protocol
C prints:
```text
RIDE|101|Aarav Naik|GA-01-AB-1204|1|Panjim|0|Goa University|Panjim -> Miramar -> Goa Univ|0815|4|3|7|30|2026-11-23|1|95
```
Python splits by `|` and constructs:
```json
{
  "rideId": 101,
  "driverName": "Aarav Naik",
  "vehicleNumber": "GA-01-AB-1204",
  "originName": "Panjim",
  "destinationName": "Goa University",
  "farePerSeat": 30,
  "isDriverVerified": 1,
  "driverTrustScore": 95
}
```

---

# 11. Web UI & GIS Frontend Layer

### 11.1 Leaflet.js GIS Mapping Integration
- **Coordinate Standard:** WGS-84 EPSG:4326.
- **Base Layer:** OpenStreetMap raster tiles rendered via CartoDB Positron / OSM.
- **Markers:** Custom CSS-styled pins placed at each Goan transit hub and 5 campus micro-landmarks on the Taleigão Plateau:
  1. Main University Administrative Gate
  2. Faculty of Science & Technology (CS/MCA Dept)
  3. University Central Library
  4. Silver Jubilee Hall
  5. University Sports Complex & Grounds
- **Dynamic Polyline:** When a ride is clicked, a polyline connects origin, waypoints, and destination with dynamic fit-bounds zooming.

### 11.2 Dynamic SVG Digital Boarding Pass & Boarding OTP
- **How It Works:** Upon seat confirmation, C assigns a unique booking ID and a 4-digit boarding OTP.
- **Frontend QR Engine:** `web/app.js` renders a scalable vector graphic (SVG) QR code containing:
  ```text
  GU-CARPOOL|BK:5001|RIDE:101|OTP:4821|PAX:Faaris Khan|FARE:30
  ```
- **Security Check:** Campus security guards or drivers scan the QR code to verify student credentials before boarding.

---

# 12. Step-by-Step Construction Guide: Build from Scratch on a Blank PC

If you sit down at an empty computer without AI assistance, follow this exact sequence:

### Step 1: Install Compiler & Tools
1. Download MinGW-w64 (GCC for Windows) or install via MSYS2 / `winget install GCC`.
2. Add `C:\mingw64\bin` to your system `PATH`.
3. Verify in command prompt: `gcc --version` (should show GCC 11 or higher).

### Step 2: Establish the Directory Tree
```bash
mkdir PSPL
cd PSPL
mkdir include src web data build scripts docs docs\images
```

### Step 3: Write `include/rideshare.h` First
- Define all constants (`LOCATION_COUNT`, buffer limits).
- Declare enum `GoaLocationId`.
- Declare structs `User`, `Ride`, `Booking`, `Notification`, `RideShareDB`.
- Declare extern matrices and all function prototypes.

### Step 4: Write `src/storage.c` (Heap & File I/O)
- Implement `init_database` with `malloc`.
- Implement `ensure_ride_capacity` with `realloc`.
- Implement `free_database` with `free`.
- Implement `load_rides_from_file` and `save_rides_to_file`.
- Write `seed_default_data_if_empty`.

### Step 5: Write `src/algorithms.c` (Core Logic)
- Define `GOA_DISTANCE_MATRIX` and `GOA_FARE_MATRIX`.
- Implement `validate_ride_parameters` and `validate_booking_parameters`.
- Implement `selection_sort_by_time` and `bubble_sort_by_fare`.
- Implement `binary_search_ride_recursive`.
- Implement `recursive_sum_available_seats`.
- Implement `create_booking_transaction` and `cancel_booking_transaction`.

### Step 6: Write `src/main.c` (CLI & Menu)
- Write `run_self_tests()` to verify algorithms immediately.
- Write `handle_cli_mode()` for `--list-rides`, `--book`, etc.
- Write `run_interactive_menu()` with a 15-case terminal menu.

### Step 7: Compile and Test Locally
```bash
gcc -Wall -Wextra -std=c11 -O2 -Iinclude src/main.c src/storage.c src/algorithms.c -o build/rideshare.exe
.\build\rideshare.exe --self-test
```

### Step 8: Add the Python Bridge & Web Interface
- Write `web/server.py` using standard `http.server`.
- Write `web/index.html` and `web/app.js`.
- Launch: `python web/server.py`.

---

# 13. File-by-File & Function-by-Function Exhaustive Anatomy

### 13.1 `include/rideshare.h`
- **Lines 20–27:** Application buffer limits (`MAX_NAME_LEN 64`, `MAX_ROUTE_LEN 128`, `INITIAL_CAPACITY 4`).
- **Lines 36–42:** `GoaLocationId` enum (0 to 4).
- **Lines 45–55:** `User` struct (identity, role, PIN, KYC flag, trust score).
- **Lines 58–73:** `Ride` struct (driver details, route, time, seats, matrices fare, verification).
- **Lines 76–83:** `Booking` struct (booking ID, ride foreign key, passenger, status).
- **Lines 86–94:** `Notification` struct (activity ledger, type, read status).
- **Lines 97–110:** `RideShareDB` struct (heap pointers and capacity counters).
- **Lines 113–115:** Matrix extern declarations.
- **Lines 118–220:** Prototypes for all 38 C engine functions.

### 13.2 `src/storage.c`
- **`init_database` (lines 25–48):** Allocates initial heap arrays via `malloc`.
- **`ensure_capacity` family (lines 50–115):** Geometric doubling ($2\times$) via `realloc`.
- **`free_database` (lines 118–138):** Safely frees all pointers and zeroes counts.
- **`load_rides_from_file` (lines 145–195):** Reads `rides.txt` line-by-line using `sscanf` with `%[^|]`.
- **`save_rides_to_file` (lines 198–225):** Writes active rides to `rides.txt` with pipe separators.
- **`export_campus_summary_report` (lines 430–515):** Generates `data/campus_summary.txt` using recursive aggregators.

### 13.3 `src/algorithms.c`
- **`GOA_DISTANCE_MATRIX` & `GOA_FARE_MATRIX` (lines 23–38):** Symmetric $5 \times 5$ lookup tables.
- **`validate_ride_parameters` (lines 60–85):** Enforces bounds, rejection codes `-1` to `-5`.
- **`find_best_stopover_hub` (lines 153–176):** Greedy 1-stop detour search.
- **`linear_search_by_route` (lines 220–275):** Multi-criteria filtering.
- **`selection_sort_by_time` (lines 278–298):** Selection Sort $O(n^2)$.
- **`bubble_sort_by_fare` (lines 300–335):** Bubble Sort with `swapped` flag $O(n)$.
- **`binary_search_ride_recursive` (lines 338–362):** Recursive binary search $O(\log n)$.
- **`create_booking_transaction` (lines 395–440):** Atomic seat reservation.
- **`cancel_booking_transaction` (lines 445–480):** Atomic rollback of booked seats.
- **`calculate_ride_telemetry` (lines 577–703):** Piece-wise linear GPS interpolation.
- **`calculate_driver_trust_score` (lines 705–745):** Trust scoring engine.
- **`purge_cancelled_bookings_transaction` (lines 785–815):** Contiguous in-memory array defragmentation.

### 13.4 `src/main.c`
- **`run_self_tests` (lines 20–110):** 6 automated unit tests validating all core modules.
- **`handle_cli_mode` (lines 115–535):** Parses `argv[1]` and routes commands to C functions.
- **`run_interactive_menu` (lines 540–810):** Terminal interactive console menu with `switch(choice)`.

---

# 14. Individual Team Member Defense Blueprints

Use this section to drill each team member on their exact syllabus responsibilities:

---

### Member 1: Mohammed Faaris Khan (Roll 2664) — Lead Architect, Search & Sort
- **Syllabus Responsibilities:** Lab 4 (Linear Search), Lab 5 (Selection Sort), Lab 6 (Bubble Sort), Lab 7 (Recursive Binary Search), Architecture.
- **Top 5 Defense Questions & Answers:**
  1. *Q: Why does Binary Search need the array to be sorted first?*  
     **A:** "Binary Search relies on monotonic ordering. At each midpoint comparison, if the target is smaller than `rides[mid]`, it can safely discard the entire right half because all elements there are guaranteed to be greater. If the array were unsorted, the target could be anywhere, invalidating the divide-and-conquer premise."
  2. *Q: How did you prevent integer overflow in binary search?*  
     **A:** "I used `mid = low + (high - low) / 2` instead of `(low + high) / 2`. If `low` and `high` are close to `INT_MAX`, adding them causes 32-bit signed integer overflow, resulting in a negative index and a segmentation fault."
  3. *Q: Why is Selection Sort unstable?*  
     **A:** "Because swapping the minimum element to the front can jump over an identical element earlier in the array, reversing their original relative order."
  4. *Q: When would Bubble Sort outperform QuickSort?*  
     **A:** "When the input array is already sorted or nearly sorted. Our Bubble Sort checks the `swapped` boolean flag and terminates in $O(n)$ time after just one pass, whereas standard QuickSort without randomization could degrade to $O(n^2)$."
  5. *Q: How does your multi-criteria linear search work?*  
     **A:** "`linear_search_by_route()` checks 5 conditions simultaneously: origin ID match, destination ID match, case-insensitive keyword search via `contains_ignore_case()`, date string match, and minimum seats threshold. It writes matched indices into a caller-allocated array."

---

### Member 2: Ketan Naik (Roll 2623) — Dynamic Heap Memory & Flat-File Storage
- **Syllabus Responsibilities:** Lab 8 (Dynamic Heap Allocation, Structs), Lab #G (Flat-File I/O, Persistence).
- **Top 5 Defense Questions & Answers:**
  1. *Q: Why did you use geometric doubling in `realloc` instead of incrementing by 1?*  
     **A:** "Incrementing by 1 on every insert requires $O(n^2)$ total memory copies for $n$ items. Geometric doubling ($2\times$) achieves amortized $O(1)$ constant time per insertion because copies only happen at powers of 2."
  2. *Q: What happens if `realloc()` fails? Does it free the existing memory?*  
     **A:** "No. If `realloc()` fails, it returns `NULL`, but the original memory block remains valid. That's why I assign the result to a temporary pointer `Ride *newPtr = realloc(...)` before updating `db->rides`. If I wrote `db->rides = realloc(...)`, a `NULL` return would overwrite the only pointer to the existing heap memory, causing an irreversible memory leak."
  3. *Q: Explain the scanset format `%63[^|]` in `sscanf`.*  
     **A:** "The `63` restricts `sscanf` to reading at most 63 characters into a 64-byte array, leaving the 64th byte for the `'\0'` terminator to prevent buffer overflow. The `[^|]` is a negated scanset that reads all characters until a pipe `|` delimiter is met."
  4. *Q: How do you prevent memory leaks when the program exits?*  
     **A:** "Our `free_database(RideShareDB *db)` function explicitly calls `free()` on `db->rides`, `db->bookings`, `db->users`, and `db->notifications`, and sets all pointers to `NULL` to avoid dangling references."
  5. *Q: Why use flat text files instead of SQLite or binary files?*  
     **A:** "Flat text files satisfy the academic syllabus requirement for direct stream I/O (`fopen`, `fgets`, `sscanf`, `fprintf`). Furthermore, pipe-delimited text files are human-auditable and platform-portable across operating systems without endianness incompatibilities."

---

### Member 3: Kevin Fernandes (Roll 2624) — Validation, Matrices & Recursive Analytics
- **Syllabus Responsibilities:** Lab 1 & 2 (Operators, Expressions), Lab 3 (Branching), Lab 5 (2D Arrays), Lab 7 (Recursion).
- **Top 5 Defense Questions & Answers:**
  1. *Q: What negative error codes does `validate_ride_parameters` return?*  
     **A:** "`-1` if origin is out of bounds ($< 0$ or $\ge 5$), `-2` if destination is out of bounds, `-3` if origin equals destination, `-4` if departure time is invalid ($> 2359$ or minutes $> 59$), and `-5` if seat count is outside 1 to 7."
  2. *Q: Why did you use a 2D matrix for distance and fares instead of an adjacency list?*  
     **A:** "The campus carpool network connects 5 fixed hubs. A $5 \times 5$ static 2D array provides $O(1)$ constant-time distance lookup via `GOA_DISTANCE_MATRIX[origin][dest]`. It consumes negligible memory ($5 \times 5 \times 4\text{ bytes} = 100\text{ bytes}$), provides excellent CPU cache locality, and requires zero pointer traversal overhead."
  3. *Q: How does your greedy 1-stop transfer heuristic work?*  
     **A:** "When there is no direct ride between origin and destination, `find_best_stopover_hub()` iterates through all intermediate hubs $h \in \{0, 1, 2, 3, 4\}$. It finds $h$ that minimizes $\text{Matrix}[\text{origin}][h] + \text{Matrix}[h][\text{dest}]$, returning the optimal transfer hub in $O(V)$ time."
  4. *Q: Explain the base case and recursive step in `recursive_sum_available_seats`.*  
     **A:** "The base case checks `if (index >= count) return 0;`. The recursive step returns `rides[index].availableSeats + recursive_sum_available_seats(rides, count, index + 1)`. As the stack unwinds, it sums the seats across all active vehicles."
  5. *Q: How is the driver trust score calculated?*  
     **A:** "It begins with a base score of 50. KYC verification adds $+30$. Roles add $+10$ (Faculty) or $+5$ (Student/Staff). Completed rides add $+2$ each (up to $+15$). Cancellations penalize $-10$ each. The final result is clamped between 10 and 100."

---

### Member 4: Kreisler Mozart (Roll 2625) — CLI Dispatcher, IPC Bridge & Web/GIS UI
- **Syllabus Responsibilities:** Lab 3 (Menu/Switch), Lab 8 (Pass Generation), Python IPC Bridge, Web UI, Leaflet GIS.
- **Top 5 Defense Questions & Answers:**
  1. *Q: How does the Python server communicate with the C executable?*  
     **A:** "Python executes `build/rideshare.exe` using `subprocess.Popen()` with command-line argument vectors (`argv`). It captures the output from C's standard output pipe (`stdout`), splits lines by pipe delimiters, and formats the data into JSON responses."
  2. *Q: Why is there a `threading.Lock()` in `server.py`?*  
     **A:** "Because HTTP requests in Python's `ThreadingHTTPServer` run on multiple threads. If two users book a seat concurrently, without the lock both processes might attempt to read and write `rides.txt` and `bookings.txt` simultaneously, causing a race condition or file corruption."
  3. *Q: How is the digital campus pass generated?*  
     **A:** "When a booking is confirmed, C generates a unique booking ID and a 4-digit boarding OTP. The web client constructs a standardized boarding string and renders an inline SVG QR code representing the pass."
  4. *Q: How does the interactive console menu work in `main.c`?*  
     **A:** "It displays a 15-option terminal menu using a `do-while` loop and a `switch(choice)` block. Each case prompts for input using `fgets`, sanitizes strings, calls the corresponding storage or algorithm function, and updates the database."
  5. *Q: What is the purpose of `scripts/start-mobile.bat`?*  
     **A:** "It binds the Python HTTP server to `0.0.0.0:8080` instead of `127.0.0.1`. This allows students on campus to connect to the laptop via local college Wi-Fi from their mobile smartphones to test real-world carpooling."

---

# 15. Blackboard Code Reference

Memorize these exact code snippets. You may be asked to write them on a blackboard during defense:

### 1. Recursive Binary Search (Lab 7)
```c
int binary_search_ride_recursive(const Ride *rides, int low, int high, int targetId) {
    if (low > high) {
        return -1; // Base case: Target not found
    }
    int mid = low + (high - low) / 2; // Prevents 32-bit integer overflow
    if (rides[mid].rideId == targetId) {
        return mid; // Target found
    } else if (rides[mid].rideId > targetId) {
        return binary_search_ride_recursive(rides, low, mid - 1, targetId); // Search left
    } else {
        return binary_search_ride_recursive(rides, mid + 1, high, targetId); // Search right
    }
}
```

### 2. Selection Sort by Departure Time (Lab 5)
```c
void selection_sort_by_time(Ride *rides, int count) {
    for (int i = 0; i < count - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < count; j++) {
            if (rides[j].departureTime < rides[minIdx].departureTime) {
                minIdx = j;
            }
        }
        if (minIdx != i) {
            Ride temp = rides[i];
            rides[i] = rides[minIdx];
            rides[minIdx] = temp;
        }
    }
}
```

### 3. Bubble Sort with Early Exit Flag (Lab 6)
```c
void bubble_sort_by_fare(Ride *rides, int count) {
    for (int i = 0; i < count - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < count - 1 - i; j++) {
            if (rides[j].farePerSeat > rides[j + 1].farePerSeat) {
                Ride temp = rides[j];
                rides[j] = rides[j + 1];
                rides[j + 1] = temp;
                swapped = 1;
            }
        }
        if (!swapped) break; // Exit early: Array is already sorted (O(n) best-case)
    }
}
```

### 4. Dynamic Array Geometric Doubling (Lab 8)
```c
int ensure_ride_capacity(RideShareDB *db) {
    if (db->rideCount >= db->rideCapacity) {
        int newCap = (db->rideCapacity == 0) ? 4 : db->rideCapacity * 2;
        Ride *newPtr = (Ride *)realloc(db->rides, sizeof(Ride) * newCap);
        if (newPtr == NULL) {
            return 0; // Allocation failure guard
        }
        db->rides = newPtr;
        db->rideCapacity = newCap;
    }
    return 1;
}
```

### 5. In-Memory Contiguous Array Defragmentation (Lab 4 & 8)
```c
int purge_cancelled_bookings_transaction(RideShareDB *db) {
    int writeIdx = 0;
    int purged = 0;
    for (int readIdx = 0; readIdx < db->bookingCount; readIdx++) {
        if (strcmp(db->bookings[readIdx].bookingStatus, "CANCELLED") != 0) {
            if (writeIdx != readIdx) {
                db->bookings[writeIdx] = db->bookings[readIdx];
            }
            writeIdx++;
        } else {
            purged++;
        }
    }
    db->bookingCount = writeIdx; // Truncate logical count
    save_bookings_to_file(db, BOOKINGS_FILE);
    return purged;
}
```

---

# 16. Common Compiler Errors, Runtime Faults & GDB Debugging Playbook

If an examiner asks how you debugged C issues, cite these exact real-world scenarios:

### 1. Segmentation Fault (`SIGSEGV` / Exception `0xC0000005`)
- **Root Cause:** Dereferencing a `NULL` pointer, accessing an array out of bounds (negative index or index $\ge$ capacity), or stack overflow caused by infinite recursion.
- **How to Debug:** Compile with debug symbols `-g` and run under GDB:
  ```bash
  gcc -g -std=c11 -Iinclude src/*.c -o build/debug.exe
  gdb build/debug.exe
  (gdb) run --self-test
  (gdb) backtrace # Shows exact file and line where crash occurred
  ```

### 2. Warning: `stringop-truncation` in `strncpy`
- **Root Cause:** Calling `strncpy(dest, src, sizeof(dest))` when `src` is equal to or longer than `dest` causes the null terminator to be omitted.
- **The Fix:** Replace `strncpy` with `snprintf(dest, sizeof(dest), "%s", src)`.

### 3. Linker Error: `undefined reference to 'GOA_DISTANCE_MATRIX'`
- **Root Cause:** The matrix was declared `extern` in `rideshare.h` but was not actually defined with memory allocated in any `.c` source file.
- **The Fix:** Ensure `const int GOA_DISTANCE_MATRIX[5][5] = { ... };` is defined in `src/algorithms.c`.

### 4. Floating Point Exception (`SIGFPE`)
- **Root Cause:** Division by zero in modulo or integer division (e.g. `val % count` when `count == 0`).
- **The Fix:** Always guard divisions with `if (count <= 0) return 0;`.

---
*Comprehensive Master Engineering Manual compiled for Team 6 (Mohammed Faaris Khan, Ketan Naik, Kevin Fernandes, Kreisler Mozart) for the CSA5003 PSPL Semester End Assessment on 23/11/2026 at Goa University.*
