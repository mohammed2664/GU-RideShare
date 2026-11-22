# GU RideShare — Goa University Campus Carpool Platform

GU RideShare is a campus carpooling system engineered for students, faculty, and staff of Goa University (Taleigão Plateau). It connects an algorithmic C11 backend engine with a responsive browser dashboard via a lightweight Python server.

## Highlights

- **Pure C11 Engine**: Dynamic heap arrays, flat-file persistence, 2D distance/fare matrices, sorting, recursive search, and transactional booking operations.
- **Python HTTP Bridge**: Thread-safe subprocess orchestration, PBKDF2 authentication, local sessions, and metadata management.
- **Modern Responsive Web UI**: Clean interface built with Tailwind CSS, interactive route mapping with Leaflet.js, boarding OTP validation, and UPI QR settlements.

## Directory Structure

```text
PSPL/
├── src/          Pure C11 implementation files (main.c, storage.c, algorithms.c)
├── include/      C header definitions (rideshare.h)
├── web/          Server & frontend (server.py, index.html, styles.css, app.js)
├── data/         Persistent flat-files (rides.txt, bookings.txt, users.txt)
├── scripts/      Build and execution scripts (build.bat, start-mobile.bat)
├── docs/         Detailed technical architecture and documentation
└── build/        Compiled native binaries (rideshare.exe)
```

## Quick Start

### 1. Compile & Launch
Run the automated build script:
```cmd
scripts\build.bat
```
This compiles the C11 core, executes the 5 automated verification tests, and starts the server at `http://127.0.0.1:8080`.

### 2. Mobile Testing (Local Network)
To test across devices on the same Wi-Fi network:
```cmd
scripts\start-mobile.bat
```
Access the dashboard on your phone via `http://<laptop-ip>:8080`.

### 3. Standalone C Terminal Mode
To interact directly with the C engine without the web interface:
```cmd
build\rideshare.exe
```

For full architectural details and algorithm explanations, see [docs/README.md](docs/README.md).
