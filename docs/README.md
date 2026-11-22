# GU RideShare — Technical Documentation

GU RideShare is a comprehensive campus carpooling platform engineered for the Goa University community (Taleigão Plateau). It bridges a high-performance C11 algorithmic core with a Python micro-server and a responsive web dashboard.

---

## Architecture

```text
[ Web Browser / Mobile Phone ]
               |
               | HTTP (JSON REST API)
               v
      [ web/server.py ]
    Python Micro-Bridge (Auth, Sessions, Metadata)
               |
               | CLI Argument Vector & Pipe IPC
               v
     [ build/rideshare.exe ]
     Pure C11 Backend Engine
    ├── src/main.c        (CLI controller, self-tests, 15-option interactive menu)
    ├── src/storage.c     (Dynamic heap memory & flat-file I/O)
    └── src/algorithms.c  (Validation, 2D matrix, sorting, search & recursion)
               |
               | Flat-file persistence
               v
   [ data/rides.txt ] • [ data/bookings.txt ] • [ data/users.txt ]
```

---

## Directory Organization

```text
PSPL/
├── src/                  C11 source code
│   ├── main.c           CLI dispatcher, menu, and automated test suite
│   ├── storage.c        Heap memory management & flat-file serialization
│   └── algorithms.c     Matrix routing, sorting, search & transaction logic
├── include/              C header files
│   └── rideshare.h      Structures, matrix declarations, function prototypes
├── web/                  Frontend & Application Server
│   ├── server.py        HTTP server, authentication & IPC bridge
│   ├── index.html       Responsive dashboard markup
│   ├── styles.css       Custom map and layout styles
│   └── app.js           Client application logic & Leaflet map integration
├── data/                 Persistent records (auto-seeded on launch)
│   ├── rides.txt        Pipe-delimited active carpool rides
│   ├── bookings.txt     Pipe-delimited seat reservations
│   ├── users.txt        Verified campus user accounts (@unigoa.ac.in)
│   └── *.json           Local metadata and sessions (ignored by git)
├── scripts/              Convenience scripts
│   ├── build.bat        Compiles C core, runs self-tests, and starts server
│   └── start-mobile.bat Binds server to 0.0.0.0 for LAN/phone testing
├── docs/                 Project documentation
│   └── README.md        This documentation guide
├── build/                Compiled native binaries
│   └── rideshare.exe    C backend executable
├── campus_summary.txt    Exported C11 audit report file
└── .gitignore            Version control exclusion rules
```

---

## Computer Science & Programming Concepts Demonstrated

1. **Structured Data Modeling & Multi-User Identity (`include/rideshare.h`)**
   - Fixed-size character buffers with bounds checking and null termination.
   - Enums for location hubs (`GoaLocationId`: Goa University, Panjim, Margao, Vasco, Ponda).
   - Composite structs for `Ride`, `Booking`, `User` (`userId`, `fullName`, `email`, `role`, `phone`, `passwordPin`), and dynamic database context `RideShareDB`.

2. **Dynamic Memory Allocation & Heap Management (`src/storage.c`)**
   - Amortized $O(1)$ array expansion using `realloc` doubling (`ensure_ride_capacity`, `ensure_booking_capacity`, `ensure_user_capacity`).
   - Safe cleanup with explicit heap deallocation (`free_database`).

3. **Pipe-Delimited Flat-File Serialization (`src/storage.c`)**
   - Resilient file parser (`sscanf`) with line-level input sanitization.
   - Transparent path resolution (`resolve_read_path`, `resolve_write_path`) supporting both `data/` and root execution contexts.
   - Idempotent bootstrap seeding (`seed_default_data_if_empty`) loading 12 realistic Goa University carpool routes across all 5 hubs and 4 team user accounts.

4. **Algorithms & Data Structures (`src/algorithms.c`)**
   - **2D Distance & Auto-Fare Matrices**: Static $5 \times 5$ lookup tables for Goa transit nodes (`Panjim`, `Margao`, `Vasco`, `Ponda`, `Goa University`).
   - **Greedy Intermediate Hub Route Recommendation**: `find_best_stopover_hub()` evaluates all intermediate nodes to suggest the shortest single-stop connection when direct rides are unavailable.
   - **Sorting Algorithms**:
     - *Selection Sort* (`selection_sort_by_time`): Deterministic earliest-departure sorting (Lab 5).
     - *Bubble Sort* with early termination flag (`bubble_sort_by_fare`): Lowest-cost ordering (Lab 6).
     - *Primary-Key Sort* (`sort_rides_by_id`): Guarantees precondition for binary search.
   - **Searching Algorithms**:
     - *Recursive Binary Search* (`binary_search_ride_recursive`): $O(\log n)$ lookup by Ride ID (Lab 4 & Lab 7).
     - *Multi-Field Linear Search* (`linear_search_by_route`): Filter by Origin, Destination, Keyword, Travel Date (`YYYY-MM-DD` or `ALL`), and Minimum Available Seats (`minSeats`).
   - **Atomic In-Place Transaction Logic**:
     - *Edit Ride Transaction* (`edit_ride_transaction`): Uses Binary Search to locate the record, performs validation, and updates departure time and seat capacity in place while preserving passenger bookings.
     - *Booking Reservation & Rollback* (`create_booking_transaction`, `cancel_booking_transaction`).
   - **Campus Domain Validation**:
     - `validate_campus_email()` validates that email ends with `@unigoa.ac.in` using `strstr()`.
   - **Recursion (Lab 7)**:
     - `recursive_sum_available_seats()`
     - `recursive_sum_fare_pool()`
     - `recursive_sum_booked_revenue()`

5. **Formal C Audit Report Export (`src/storage.c`)**
   - `export_campus_summary_report()` generates `campus_summary.txt` with recursive fleet KPIs, per-hub traffic matrices, active carpool roster, registered user directory, and bookings audit ledger.

6. **Python Web Bridge & Frontend (`web/server.py`, `web/app.js`)**
   - Subprocess invocation of `rideshare.exe` using `threading.Lock()`.
   - Leaflet.js interactive transit map and campus landmark micro-markers.
   - Digital Campus Transit Pass modal with dynamic QR Code generator and 4-digit Boarding OTP.
   - UPI QR Code settlement with deep-link payment intents.

---

## Running the Project

### Prerequisites
- GCC / MinGW C compiler (supporting C11 standard)
- Python 3.8+

### Step-by-Step Execution
1. Open PowerShell or Command Prompt in the project root directory:
   ```cmd
   scripts\build.bat
   ```
2. The script will:
   - Compile C11 files with `gcc -Wall -Wextra -std=c11 -O2` into `rideshare.exe`.
   - Run the automated unit test suite (5/5 tests passing).
   - Launch the HTTP dashboard server at `http://127.0.0.1:8080`.
3. Open your browser at `http://127.0.0.1:8080`.

### Standalone Interactive Console Mode
The C core can also be run independently in terminal mode:
```cmd
.\rideshare.exe
```
This provides an interactive 15-option terminal menu covering all Lab 3–8 and Lab #G features.
