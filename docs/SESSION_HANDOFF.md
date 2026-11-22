# GU RideShare — Project Session Handoff & Synchronization Document

**Course:** CSA5003 Problem Solving and Programming Laboratory (PSPL)  
**Institution:** Discipline of Computer Science & Applications, Goa University  
**Degree:** Master of Computer Applications (MCA) — Semester I  
**Assessment:** Semester End Assessment (SEA) Practical & Viva Defense  
**Team 6 Members:**
- **Mohammed Faaris Khan (Roll 2664):** Lead Architect, Search & Sort Algorithms
- **Ketan Naik (Roll 2623):** Dynamic Heap Memory (`malloc`/`realloc`/`free`), Flat-File Storage Engine
- **Kevin Fernandes (Roll 2624):** Parameter Validation, $5 \times 5$ Matrices, Greedy Detour, Recursive Analytics
- **Kreisler Mozart (Roll 2625):** CLI Dispatcher, Terminal Menu, Python IPC Bridge, Leaflet GIS, Dynamic QR Pass

**GitHub Repository:** [https://github.com/mohammed2664/GU-RideShare](https://github.com/mohammed2664/GU-RideShare)  
**Active Branch:** `main`  
**Last Synchronized:** 22nd November 2026

---

## 1. Executive Summary & Project Status
The **GU Campus RideShare & Carpool Matcher** is fully built, compiled, verified, and pushed to GitHub. It strictly adheres to pure C11 machine code standards for all algorithmic computation, wrapped seamlessly with an anonymous pipe IPC bridge in Python and a responsive Tailwind + Leaflet GIS client presentation layer.

### System Verification Checklist:
- [x] **C11 Core Engine:** Clean compilation with `gcc -Wall -Wextra -std=c11 -O2` into `build/rideshare.exe` with **0 warnings**.
- [x] **Automated Self-Tests:** **6/6 tests passing** (Lab 3 branching error codes, Lab 5 $5 \times 5$ matrix distance/fare calculations, Lab 7 recursion + Lab 8 heap loading, and trust scoring engine).
- [x] **Standalone Terminal Mode:** Fully interactive 15-option terminal CLI menu running directly in console.
- [x] **Web Dashboard & GIS:** Python `http.server` IPC bridge running on port 8080 with zero third-party framework dependencies.
- [x] **Submission Report:** Formal 8-page SEA submission report rendered to `docs/Team 6_Campus RideShare & Carpool Matcher.pdf`.
- [x] **Master Study Guide:** Complete 963-line engineering manual and defense guide at `docs/MASTER_STUDY_GUIDE.md`.
- [x] **Vehicle Recognition Guide:** Description for visual recognition (make, model, color, GU parking tags) integrated across Ride Cards, Offer/Edit modals, Digital Pass QR, and Live GPS tracker.
- [x] **Business Logic & Booking Guards:** Driver self-booking prevention, duplicate active reservation rejection, and past-date input guards.
- [x] **Version Control:** Repository published to GitHub under `mohammed2664/GU-RideShare` on branch `main`.

---

## 2. The 3-Tier Decoupled Architecture

```
+=============================================================================+
|             TIER 1: CLIENT PRESENTATION LAYER (Browser / Mobile)            |
|  • HTML5 Semantic DOM  • Tailwind CSS Glassmorphism  • Vanilla ES6 State    |
|  • Leaflet.js GIS Mapping (WGS-84)  • Dynamic SVG QR Transit Code Generator |
|  • Live GPS Telemetry Kinematic Stepper  • Activity Notification Drawer     |
|  • Vehicle Recognition Chips & Security Pass Verification                   |
+======================================+======================================+
                                       | HTTP/1.1 REST (Port 8080)
                                       v
+=============================================================================+
|             TIER 2: PROCESS BRIDGE LAYER (Python: web/server.py)            |
|  • Native http.server (Zero external dependencies: No Flask, No FastAPI)   |
|  • Thread Concurrency Lock (threading.Lock) preventing race conditions      |
|  • Subprocess Execution via Argument Vector: subprocess.Popen(argv)         |
|  • Anonymous Pipe Stream Capture: stdout/stderr line-by-line reading        |
|  • Pipe-Delimited text to JSON Serialization                                |
|  • Business Guards: Driver self-booking & duplicate booking prevention      |
+======================================+======================================+
                                       | OS Pipe IPC (CLI Flags & stdout)
                                       v
+=============================================================================+
|             TIER 3: CORE ALGORITHMIC ENGINE (Native C11 Machine Code)       |
|  • Compiled via GCC: gcc -Wall -Wextra -std=c11 -O2 (0 warnings)            |
|  • Zero dynamic runtime: Runs bare-metal on x86-64 CPU                      |
|  • Dynamic Heap Memory Allocation: Initial pool + 2x geometric growth       |
|  • In-Memory Contiguous Array Compaction (Heap Defragmentation)             |
|  • 5x5 Symmetric Distance/Fare Matrices & Greedy 1-Stop Transfer Heuristic   |
|  • Selection Sort O(n^2) & Bubble Sort with Early Exit O(n)                 |
|  • Recursive Divide-and-Conquer Binary Search O(log n)                      |
|  • Recursive Fleet Aggregators O(n)                                         |
|  • Trust Scoring Engine & Kinematic Telemetry Interpolation                 |
+======================================+======================================+
                                       | C File Stream I/O (fopen, fgets, etc.)
                                       v
+=============================================================================+
|               PERSISTENT STORAGE LAYER (Flat-File Records in data/)         |
|  • data/rides.txt          • data/bookings.txt      • data/users.txt        |
|  • data/notifications.txt  • data/campus_summary.txt                        |
+=============================================================================+
```

---

## 3. Goa University PSPL Lab Syllabus Mapping

| Lab Module | Syllabus Concept | Implementation File | Key Details |
|---|---|---|---|
| **Lab 1 & 2** | Constants, Operators, Expressions & Formatted I/O | `include/rideshare.h`, `src/main.c` | `#define` buffer caps, 24-hr time arithmetic (`hours = dep / 100`, `mins = dep % 100`), tabular formatted console output. |
| **Lab 3** | Decision Making, Branching (`if-else`, `switch`) | `src/algorithms.c`, `src/main.c` | Parameter validation returning distinct error codes (`-1` to `-5`), 15-case terminal menu via `switch(choice)`. |
| **Lab 4** | 1D Arrays & Linear Search | `src/algorithms.c` | Hub name array `GOA_LOCATIONS[5]`, multi-criteria `linear_search_by_route()` checking 5 parameters simultaneously. |
| **Lab 5** | 2D Arrays & Selection Sort | `src/algorithms.c` | Static $5 \times 5$ lookup matrices `GOA_DISTANCE_MATRIX` & `GOA_FARE_MATRIX`, Selection Sort (`selection_sort_by_time`) $O(n^2)$. |
| **Lab 6** | Sorting Algorithms (Bubble Sort) | `src/algorithms.c` | Bubble Sort by fare (`bubble_sort_by_fare`) with boolean `swapped` early-exit optimization yielding $\Omega(n)$ best case. |
| **Lab 7** | Recursion & String Manipulation | `src/algorithms.c` | Divide-and-conquer Recursive Binary Search (`binary_search_ride_recursive`), recursive fleet sum accumulators (`recursive_sum_available_seats`, `recursive_sum_fare_pool`, `recursive_sum_booked_revenue`). |
| **Lab 8** | Structures & Dynamic Memory Allocation | `src/storage.c`, `include/rideshare.h` | Composite structs (`User`, `Ride`, `Booking`), initial allocation via `malloc()`, geometric capacity doubling ($2\times$) via `realloc()`, and cleanup via `free()`. |
| **Lab #G** | File Handling & Data Persistence | `src/storage.c` | Flat-file pipe-delimited storage, scanset parsing with buffer guard (`%63[^|]`), cross-platform path resolution, formal audit export (`data/campus_summary.txt`). |

---

## 4. Individual Team Defense Roles & Viva Blueprints

1. **Mohammed Faaris Khan (Roll 2664) — Lead Architect, Search & Sort**
   - *Key Algorithms:* Selection Sort ($O(n^2)$), Bubble Sort with early exit ($\Omega(n)$ best case), Recursive Binary Search ($O(\log n)$), Multi-criteria Linear Search ($O(n)$).
   - *Blackboard Defense:* Writing recursive binary search with integer overflow protection (`mid = low + (high - low) / 2`).

2. **Ketan Naik (Roll 2623) — Dynamic Heap Memory & Flat-File Storage**
   - *Key Concepts:* Heap pointer lifecycle (`malloc`/`realloc`/`free`), amortized $O(1)$ geometric array doubling, buffer overflow prevention with scanset `%63[^|]`, safe `realloc` error handling with temporary pointers.
   - *Blackboard Defense:* Dynamic array geometric doubling function and memory cleanup protocol.

3. **Kevin Fernandes (Roll 2624) — Parameter Validation, Matrices & Recursive Analytics**
   - *Key Concepts:* Return code specification for validation (`-1` through `-5`), symmetric $5 \times 5$ distance/fare matrix lookup ($O(1)$), Greedy 1-stop transfer hub detour algorithm ($O(V)$), recursive seat/fare summations with call stack analysis.
   - *Blackboard Defense:* Call stack unwinding trace and parameter validation branching logic.

4. **Kreisler Mozart (Roll 2625) — CLI Dispatcher, IPC Bridge & Web/GIS UI**
   - *Key Concepts:* 15-option terminal menu with `switch(choice)`, argument vector CLI routing (`argv`), Python `subprocess.Popen` anonymous pipe IPC, `threading.Lock()` concurrency isolation, Leaflet.js GIS, SVG dynamic QR boarding pass.
   - *Blackboard Defense:* Subprocess pipe mechanics and thread-safety mutex analysis.

---

## 5. Detailed Session Change Ledger (22nd November 2026)

### A. Feature Addition: Vehicle Recognition Description for Campus Gates
- **Objective:** Enable students/passengers waiting at campus gates (Main Gate, Science/MCA Gate, Library Gate, etc.) to visually identify their vehicle in seconds without memorizing license plates.
- **Data Layer:** `data/rides_meta.json` seeded with realistic campus vehicles (e.g. *Silver Hyundai i20 (White GU Pass #12)*, *White Maruti Swift (Red Roof & CS Dept Tag)*, *Dark Grey Activa 6G (Neon Helmet)*).
- **Backend Bridge:** Added `DEFAULT_VEHICLE_DESCRIPTIONS` dictionary in `web/server.py`; updated `parse_rides()` to attach `vehicleDesc`; updated `/api/add-ride` and `/api/edit-ride` to accept and persist descriptions; updated `/api/search` to match keywords against vehicle descriptions.
- **Frontend Presentation:** 
  - Active ride cards display a dedicated vehicle recognition chip beneath plate numbers.
  - "Offer a Ride" form and "Edit Ride" modal contain input fields for vehicle recognition description.
  - Digital Boarding Pass modal and Live GPS Tracker modal display the visual vehicle guide under the registration number.

### B. C11 Engine Zero-Warning Optimization
- **Problem:** Compiling with `gcc -Wall -Wextra -std=c11 -O2` produced 3 warnings for implicit declaration of `strcasecmp` because standard ISO C11 requires `<strings.h>` for POSIX case-insensitive string comparison.
- **Solution:** Added `#include <strings.h>` to `include/rideshare.h`. Compilation now completes with **absolute zero warnings**.

### C. Booking Logic & Security Guards
1. **Driver Self-Booking Prevention:** Added backend check in `web/server.py` (`/api/book`) rejecting reservations where the passenger is the driver; updated `populateBookingSelect()` in `web/app.js` to filter out driver's own rides from the dropdown.
2. **Duplicate Reservation Prevention:** Added backend validation rejecting duplicate active `CONFIRMED` reservations by the same passenger on the same ride.
3. **Unauthenticated UX Trigger:** When an unauthenticated visitor clicks "Reserve Seat" on a ride card, `applyCurrentUser()` automatically opens the Auth modal with 1-click team demo logins.
4. **Date Validation:** `min` attribute on `#offer-date` and `#filter-date` dynamically locked to today's date to prevent past-date bookings.

### D. GitHub Repository Setup
- Local repository initialized, initial commit `a4dd797` created.
- Remote linked to `https://github.com/mohammed2664/GU-RideShare.git`.
- Collaborator permissions verified and branch `main` pushed to remote.

---

## 6. Execution Commands Quick Reference

### Rebuild and Run Automated Unit Tests:
```powershell
gcc -Wall -Wextra -std=c11 -O2 -Iinclude src/main.c src/storage.c src/algorithms.c -o build/rideshare.exe
.\build\rideshare.exe --self-test
```

### Run Standalone Interactive Terminal Console Mode:
```powershell
.\build\rideshare.exe
```

### Run Web Dashboard (Localhost):
```powershell
scripts\build.bat
# or
python web/server.py
```

### Run on Local Campus Wi-Fi for Mobile Phones:
```powershell
scripts\start-mobile.bat
```

### Synchronize with GitHub:
```powershell
git add .
git commit -m "Update session context"
git push origin main
```
