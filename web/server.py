"""server.py — High-Performance Python Bridge to C11 Campus RideShare Engine.

Connects the responsive web frontend to the compiled C11 rideshare executable.
Implements multi-user campus authentication (@unigoa.ac.in), ride CRUD,
multi-criteria search, recursive seat/revenue analytics, boarding OTP issuance,
UPI QR payment settlement, and formal C audit report generation.
"""

import base64
import datetime
import hashlib
import json
import os
import secrets
import subprocess
import sys
import threading
from http.server import BaseHTTPRequestHandler, HTTPServer
from urllib.parse import parse_qs, urlparse

# Determine project and web directory paths reliably
CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
if os.path.basename(CURRENT_DIR).lower() == "web":
    WEB_DIR = CURRENT_DIR
    PROJECT_ROOT = os.path.dirname(CURRENT_DIR)
else:
    PROJECT_ROOT = CURRENT_DIR
    WEB_DIR = os.path.join(PROJECT_ROOT, "web") if os.path.isdir(os.path.join(PROJECT_ROOT, "web")) else PROJECT_ROOT

BUILD_DIR = os.path.join(PROJECT_ROOT, "build")
DATA_DIR = os.path.join(PROJECT_ROOT, "data")

USERS_PATH = os.path.join(DATA_DIR, "users.txt")
SESSIONS_PATH = os.path.join(DATA_DIR, "sessions.json")
RIDES_META_PATH = os.path.join(DATA_DIR, "rides_meta.json")
BOOKINGS_META_PATH = os.path.join(DATA_DIR, "bookings_meta.json")

BACKEND_LOCK = threading.Lock()


def _ensure_directories():
    """Ensure required subdirectories exist."""
    os.makedirs(BUILD_DIR, exist_ok=True)
    os.makedirs(DATA_DIR, exist_ok=True)


def _load_json_file(path, default_factory):
    """Load optional application metadata without crashing on missing file."""
    if not os.path.exists(path):
        return default_factory()
    try:
        with open(path, "r", encoding="utf-8") as f:
            return json.load(f)
    except (OSError, json.JSONDecodeError) as error:
        print(f"[WARN] Could not read {os.path.basename(path)}: {error}", file=sys.stderr, flush=True)
        return default_factory()


def _save_json_file(path, data):
    """Atomically save metadata and log explicit warning on failure."""
    try:
        _ensure_directories()
        tmp_path = path + ".tmp"
        with open(tmp_path, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2)
        os.replace(tmp_path, path)
        return True
    except (OSError, TypeError) as error:
        print(f"[ERROR] Failed to save {os.path.basename(path)}: {error}", file=sys.stderr, flush=True)
        return False


def _clean_text(value, maximum):
    return " ".join(str(value or "").strip().split())[:maximum]


SESSIONS = _load_json_file(SESSIONS_PATH, dict)


def _issue_session(user):
    token = secrets.token_urlsafe(32)
    SESSIONS[token] = {
        "id": user.get("id", 1),
        "name": user["name"],
        "email": user["email"],
        "role": user.get("role", "Student"),
        "phone": user.get("phone", ""),
        "issuedAt": datetime.datetime.now().isoformat(),
    }
    _save_json_file(SESSIONS_PATH, SESSIONS)
    return token


def _revoke_session(token):
    if token in SESSIONS:
        del SESSIONS[token]
        _save_json_file(SESSIONS_PATH, SESSIONS)


def get_binary_path():
    """Find the compiled C backend executable across project directories."""
    binary_name = "rideshare.exe" if os.name == "nt" else "rideshare"
    candidates = [
        os.path.join(BUILD_DIR, binary_name),
        os.path.join(PROJECT_ROOT, binary_name),
        os.path.join(CURRENT_DIR, binary_name),
    ]
    for c in candidates:
        if os.path.isfile(c):
            return c
    return candidates[0]


def ensure_backend_compiled():
    """Ensure the C backend is compiled, compiling if missing."""
    bin_path = get_binary_path()
    if os.path.exists(bin_path):
        return bin_path

    _ensure_directories()
    out_path = os.path.join(BUILD_DIR, "rideshare.exe" if os.name == "nt" else "rideshare")

    main_c = os.path.join(PROJECT_ROOT, "src", "main.c")
    if not os.path.exists(main_c):
        main_c = os.path.join(PROJECT_ROOT, "main.c")

    storage_c = os.path.join(PROJECT_ROOT, "src", "storage.c")
    if not os.path.exists(storage_c):
        storage_c = os.path.join(PROJECT_ROOT, "storage.c")

    algorithms_c = os.path.join(PROJECT_ROOT, "src", "algorithms.c")
    if not os.path.exists(algorithms_c):
        algorithms_c = os.path.join(PROJECT_ROOT, "algorithms.c")

    cmd = [
        "gcc", "-Wall", "-Wextra", "-std=c11", "-O2",
        "-Iinclude", "-I.",
        main_c,
        storage_c,
        algorithms_c,
        "-o", out_path
    ]
    print(f"[INFO] Compiling C backend: {' '.join(cmd)}")
    subprocess.run(cmd, cwd=PROJECT_ROOT, check=True)
    return out_path


def run_c_backend(args):
    """Run one serialized C backend command from PROJECT_ROOT."""
    bin_path = ensure_backend_compiled()
    with BACKEND_LOCK:
        result = subprocess.run([bin_path] + [str(a) for a in args], cwd=PROJECT_ROOT, capture_output=True, text=True)
    return result.stdout.strip().splitlines(), result.returncode


DEFAULT_VEHICLE_DESCRIPTIONS = {
    "101": "Silver Hyundai i20 (White GU Parking Pass #12)",
    "102": "White Maruti Swift (Red Roof & CS Dept Tag)",
    "103": "Grey Honda City (Yellow Fog Lights)",
    "104": "Blue Tata Nexon (MCA Sports Council Badge)",
    "105": "Red Hyundai Creta (Faculty Parking Pass)",
    "106": "Dark Grey Honda Activa 6G (Neon Yellow Helmet)",
    "107": "Brown Kia Seltos (Library Gate Pass)",
    "108": "Silver Maruti Baleno (CS Dept Sticker)",
}


def parse_rides(lines):
    rides_meta = _load_json_file(RIDES_META_PATH, dict)
    today_str = datetime.date.today().isoformat()
    rides = []
    for line in lines:
        parts = line.split("|")
        if len(parts) >= 14 and parts[0] == "RIDE":
            raw_time = parts[9].zfill(4)
            ride_id = int(parts[1])
            meta = rides_meta.get(str(ride_id), {})
            c_date = parts[14].strip() if len(parts) >= 15 else ""
            ride_date = meta.get("rideDate") or (c_date if c_date else today_str)
            status = meta.get("status", "ACTIVE")
            vehicle_desc = meta.get("vehicleDesc") or DEFAULT_VEHICLE_DESCRIPTIONS.get(str(ride_id), "Verified Campus Vehicle")

            is_expired = False
            if ride_date < today_str and status == "ACTIVE":
                is_expired = True

            is_driver_verified = int(parts[15]) if len(parts) >= 16 and parts[15].strip().isdigit() else 1
            driver_trust_score = int(parts[16]) if len(parts) >= 17 and parts[16].strip().isdigit() else 95

            rides.append({
                "rideId": ride_id,
                "driverName": parts[2],
                "vehicleNumber": parts[3],
                "vehicleDesc": vehicle_desc,
                "originId": int(parts[4]),
                "originName": parts[5],
                "destinationId": int(parts[6]),
                "destinationName": parts[7],
                "routeDescription": parts[8],
                "departureTime": raw_time,
                "formattedTime": f"{raw_time[:2]}:{raw_time[2:]}",
                "totalSeats": int(parts[10]),
                "availableSeats": int(parts[11]),
                "distanceKm": int(parts[12]),
                "farePerSeat": int(parts[13]),
                "driverEmail": meta.get("driverEmail", ""),
                "driverPhone": meta.get("driverPhone", ""),
                "rideDate": ride_date,
                "campusPoint": meta.get("campusPoint", "Main Admin Gate"),
                "status": "EXPIRED" if is_expired else status,
                "isDriverVerified": is_driver_verified,
                "driverTrustScore": driver_trust_score,
            })
    return rides


def parse_suggestion(lines):
    for line in lines:
        parts = line.split("|")
        if len(parts) >= 3 and parts[0] == "SUGGEST":
            return {"route": parts[1], "distanceKm": int(parts[2])}
    return None


def parse_bookings(lines):
    bookings_meta = _load_json_file(BOOKINGS_META_PATH, dict)
    bookings = []
    for line in lines:
        parts = line.split("|")
        if len(parts) >= 7 and parts[0] == "BOOKING":
            booking_id = int(parts[1])
            meta = bookings_meta.get(str(booking_id), {})
            bookings.append({
                "bookingId": booking_id,
                "rideId": int(parts[2]),
                "passengerName": parts[3],
                "seatsBooked": int(parts[4]),
                "totalFare": int(parts[5]),
                "bookingStatus": parts[6],
                "passengerEmail": meta.get("passengerEmail", ""),
                "otp": meta.get("otp", ""),
                "paymentStatus": meta.get("paymentStatus", "UNPAID"),
                "bookedAt": meta.get("bookedAt", ""),
            })
    return bookings


class RideShareHandler(BaseHTTPRequestHandler):
    def _send_json(self, payload, status=200):
        body = json.dumps(payload).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _read_json_body(self):
        length = int(self.headers.get("Content-Length", 0))
        if length == 0:
            return {}
        return json.loads(self.rfile.read(length).decode("utf-8"))

    def _current_user(self):
        auth_header = self.headers.get("Authorization", "")
        token = auth_header[7:] if auth_header.startswith("Bearer ") else ""
        return SESSIONS.get(token)

    def _require_user(self):
        user = self._current_user()
        if user is None:
            self._send_json({"error": "Please sign in with your @unigoa.ac.in account to continue."}, status=401)
        return user

    def do_GET(self):
        parsed = urlparse(self.path)
        path = parsed.path
        qs = parse_qs(parsed.query)

        if path == "/api/auth/me":
            user = self._current_user()
            if user is None:
                self._send_json({"error": "Session expired."}, status=401)
            else:
                self._send_json({"user": user})
            return

        # Serve Web UI files
        if path in ("/", "/index.html"):
            index_path = os.path.join(WEB_DIR, "index.html")
            if not os.path.exists(index_path):
                index_path = os.path.join(PROJECT_ROOT, "index.html")
            with open(index_path, "rb") as f:
                data = f.read()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(data)))
            self.end_headers()
            self.wfile.write(data)
            return

        if path in ("/styles.css", "/app.js"):
            file_name = path.lstrip("/")
            file_path = os.path.join(WEB_DIR, file_name)
            if not os.path.exists(file_path):
                file_path = os.path.join(PROJECT_ROOT, file_name)
            if os.path.exists(file_path):
                mime = "text/css" if file_name.endswith(".css") else "application/javascript"
                with open(file_path, "rb") as f:
                    data = f.read()
                self.send_response(200)
                self.send_header("Content-Type", f"{mime}; charset=utf-8")
                self.send_header("Content-Length", str(len(data)))
                self.end_headers()
                self.wfile.write(data)
                return

        # Direct view/download of campus audit summary report
        if path == "/campus_summary.txt":
            rep_path = os.path.join(PROJECT_ROOT, "campus_summary.txt")
            if not os.path.exists(rep_path):
                rep_path = os.path.join(DATA_DIR, "campus_summary.txt")
            if os.path.exists(rep_path):
                with open(rep_path, "rb") as f:
                    data = f.read()
                self.send_response(200)
                self.send_header("Content-Type", "text/plain; charset=utf-8")
                self.send_header("Content-Length", str(len(data)))
                self.end_headers()
                self.wfile.write(data)
                return

        if path == "/api/rides":
            sort_mode = qs.get("sort", ["id"])[0]
            lines, _ = run_c_backend(["--list-rides", sort_mode])
            rides = parse_rides(lines)
            if qs.get("mine", ["0"])[0] == "1":
                user = self._current_user()
                if user is None:
                    self._send_json({"error": "Please sign in to view your rides."}, status=401)
                    return
                user_email = user.get("email", "").casefold()
                user_name = user.get("name", "").casefold()
                rides = [r for r in rides if (r.get("driverEmail", "").casefold() == user_email) or (not r.get("driverEmail") and r["driverName"].casefold() == user_name)]
            self._send_json({"rides": rides})
            return

        if path == "/api/search":
            origin = qs.get("origin", ["-1"])[0]
            dest = qs.get("dest", ["-1"])[0]
            keyword = qs.get("keyword", ["ALL"])[0].strip() or "ALL"
            date = qs.get("date", ["ALL"])[0].strip() or "ALL"
            min_seats = qs.get("minSeats", ["0"])[0].strip() or "0"
            lines, _ = run_c_backend(["--search-route", origin, dest, keyword, date, min_seats])
            matched_rides = parse_rides(lines)
            if not matched_rides and keyword != "ALL":
                lines_all, _ = run_c_backend(["--search-route", origin, dest, "ALL", date, min_seats])
                all_found = parse_rides(lines_all)
                kw_lower = keyword.casefold()
                matched_rides = [r for r in all_found if kw_lower in r.get("vehicleDesc", "").casefold()]
            self._send_json({
                "rides": matched_rides,
                "suggestion": parse_suggestion(lines),
            })
            return

        if path == "/api/find":
            ride_id = qs.get("id", [""])[0].strip()
            if not ride_id.isdigit():
                self._send_json({"error": "Enter a numeric Ride ID (e.g., 101)."}, status=400)
                return
            lines, _ = run_c_backend(["--find-id", ride_id])
            if lines and lines[0].startswith("NOT_FOUND|"):
                self._send_json({"rides": [], "message": lines[0].split("|", 1)[1]})
                return
            self._send_json({"rides": parse_rides(lines)})
            return

        if path == "/api/track-ride":
            ride_id = qs.get("id", [""])[0].strip()
            progress = qs.get("progress", ["35"])[0].strip()
            if not ride_id.isdigit():
                self._send_json({"error": "Valid numeric Ride ID required."}, status=400)
                return
            if not progress.isdigit():
                progress = "35"

            lines, code = run_c_backend(["--track-ride", ride_id, progress])
            if code != 0 or not lines or lines[0].startswith("NOT_FOUND|"):
                self._send_json({"error": lines[0].split("|", 1)[1] if lines else "Ride not found"}, status=404)
                return

            telem = {}
            waypoints = []
            for line in lines:
                parts = line.split("|")
                if parts[0] == "TELEMETRY" and len(parts) >= 11:
                    telem = {
                        "rideId": int(parts[1]),
                        "lat": float(parts[2]),
                        "lng": float(parts[3]),
                        "currentCheckpoint": parts[4],
                        "nextCheckpoint": parts[5],
                        "distanceRemainingKm": int(parts[6]),
                        "etaMinutes": int(parts[7]),
                        "progressPercent": int(parts[8]),
                        "statusStep": int(parts[9]),
                        "statusText": parts[10],
                    }
                elif parts[0] == "WAYPOINT" and len(parts) >= 6:
                    waypoints.append({
                        "index": int(parts[1]),
                        "lat": float(parts[2]),
                        "lng": float(parts[3]),
                        "checkpointName": parts[4],
                        "percent": int(parts[5]),
                    })

            lines_find, _ = run_c_backend(["--find-id", ride_id])
            matched_rides = parse_rides(lines_find)
            ride_obj = matched_rides[0] if matched_rides else {}

            self._send_json({
                "telemetry": telem,
                "waypoints": waypoints,
                "ride": ride_obj,
            })
            return

        if path == "/api/bookings":
            lines, _ = run_c_backend(["--list-bookings"])
            bookings = parse_bookings(lines)
            if qs.get("mine", ["0"])[0] == "1":
                user = self._current_user()
                if user is None:
                    self._send_json({"error": "Please sign in to view your bookings."}, status=401)
                    return
                user_email = user.get("email", "").casefold()
                user_name = user.get("name", "").casefold()
                bookings = [b for b in bookings if (b.get("passengerEmail", "").casefold() == user_email) or (not b.get("passengerEmail") and b["passengerName"].casefold() == user_name)]
            self._send_json({"bookings": bookings})
            return

        if path == "/api/stats":
            lines, _ = run_c_backend(["--stats"])
            stats = {}
            for line in lines:
                if line.startswith("STATS|"):
                    for item in line.split("|")[1:]:
                        if "=" in item:
                            k, v = item.split("=", 1)
                            stats[k] = int(v) if v.isdigit() else v
            self._send_json({"stats": stats})
            return

        if path == "/api/matrix":
            lines, _ = run_c_backend(["--matrix"])
            cells = []
            for line in lines:
                parts = line.split("|")
                if len(parts) >= 7 and parts[0] == "MATRIX":
                    cells.append({
                        "originId": int(parts[1]),
                        "originName": parts[2],
                        "destId": int(parts[3]),
                        "destName": parts[4],
                        "distanceKm": int(parts[5]),
                        "fare": int(parts[6]),
                    })
            self._send_json({"matrix": cells})
            return

        if path == "/api/users":
            lines, _ = run_c_backend(["--list-users"])
            users = []
            for line in lines:
                if line.startswith("USER|"):
                    parts = line.split("|")
                    if len(parts) >= 6:
                        users.append({
                            "userId": int(parts[1]),
                            "fullName": parts[2],
                            "email": parts[3],
                            "role": parts[4],
                            "phone": parts[5],
                            "isVerified": int(parts[6]) if len(parts) >= 7 and parts[6].strip().isdigit() else 1,
                            "dlNumber": parts[7] if len(parts) >= 8 else "GA-07-2024-GU01",
                            "trustScore": int(parts[8]) if len(parts) >= 9 and parts[8].strip().isdigit() else 95,
                        })
            self._send_json({"users": users})
            return

        if path == "/api/driver/trust-score":
            email = qs.get("email", [""])[0].strip().lower()
            if not email:
                user = self._current_user()
                email = user["email"] if user else ""
            if not email:
                self._send_json({"error": "Email is required."}, status=400)
                return
            lines, code = run_c_backend(["--trust-score", email])
            if code != 0 or not lines or lines[0].startswith("ERROR|"):
                self._send_json({"error": lines[0].split("|", 1)[1] if lines else "User not found."}, status=404)
                return
            parts = lines[0].split("|")
            info = {}
            for p in parts[1:]:
                if "=" in p:
                    k, v = p.split("=", 1)
                    info[k] = int(v) if v.isdigit() else v
            self._send_json({"trustScoreInfo": info})
            return

        if path == "/api/notifications":
            user = self._current_user()
            email = user["email"] if user else "all@unigoa.ac.in"
            lines, _ = run_c_backend(["--list-notifications", email])
            notifs = []
            unread_count = 0
            for line in lines:
                if line.startswith("NOTIF_SUMMARY|"):
                    # NOTIF_SUMMARY|Total=4|Unread=3
                    parts = line.split("|")
                    for p in parts[1:]:
                        if p.startswith("Unread="):
                            unread_count = int(p.split("=")[1])
                elif line.startswith("NOTIF|"):
                    parts = line.split("|")
                    if len(parts) >= 8:
                        notifs.append({
                            "id": int(parts[1]),
                            "recipientEmail": parts[2],
                            "title": parts[3],
                            "message": parts[4],
                            "type": parts[5],
                            "timestamp": parts[6],
                            "isRead": int(parts[7]) == 1,
                        })
            self._send_json({"notifications": notifs, "unreadCount": unread_count})
            return

        if path == "/api/admin/heap-stats":
            lines, code = run_c_backend(["--admin-heap-stats"])
            if code != 0 or not lines or not lines[0].startswith("HEAP_STATS|"):
                self._send_json({"error": "Failed to read C heap memory statistics."}, status=500)
                return
            stats = {}
            for item in lines[0].split("|")[1:]:
                if "=" in item:
                    k, v = item.split("=", 1)
                    stats[k] = int(v) if v.isdigit() else v
            self._send_json({"heapStats": stats})
            return

        if path == "/api/export-report":
            report_filename = os.path.join("data", "campus_summary.txt")
            lines, code = run_c_backend(["--export-report", report_filename])
            rep_path = os.path.join(DATA_DIR, "campus_summary.txt")
            if not os.path.exists(rep_path):
                rep_path = os.path.join(PROJECT_ROOT, report_filename)
            report_text = ""
            if os.path.exists(rep_path):
                with open(rep_path, "r", encoding="utf-8") as rf:
                    report_text = rf.read()
            if code != 0 or not report_text:
                self._send_json({"error": "Failed to generate C campus summary report."}, status=500)
                return
            self._send_json({
                "message": "Campus summary audit report exported successfully.",
                "filename": "campus_summary.txt",
                "report": report_text,
            })
            return

        self._send_json({"error": "Not found"}, status=404)

    def do_POST(self):
        parsed = urlparse(self.path)
        try:
            data = self._read_json_body()
        except (UnicodeDecodeError, json.JSONDecodeError):
            self._send_json({"error": "Invalid JSON request payload."}, status=400)
            return

        # Campus Registration via C11 Backend (--register)
        if parsed.path in ("/api/register", "/api/auth/register"):
            name = _clean_text(data.get("name"), 63)
            email = _clean_text(data.get("email"), 120).lower()
            role = _clean_text(data.get("role", "Student"), 20)
            phone = _clean_text(data.get("phone", "9876543210"), 20)
            pin = str(data.get("pin") or data.get("password") or "1234").strip()

            if len(name) < 2 or "@unigoa.ac.in" not in email or len(pin) < 4:
                self._send_json({
                    "error": "Registration requires your full name, verified @unigoa.ac.in email, and a 4+ digit PIN."
                }, status=400)
                return

            lines, code = run_c_backend(["--register", name, email, role, phone, pin])
            if code != 0 or not lines or lines[0].startswith("ERROR|"):
                msg = lines[0].split("|", 1)[1] if lines and "|" in lines[0] else "Registration failed."
                self._send_json({"error": msg}, status=400)
                return

            # Extract created user from C pipe output
            user_info = None
            for line in lines:
                if line.startswith("USER|"):
                    p = line.split("|")
                    if len(p) >= 6:
                        user_info = {
                            "id": int(p[1]),
                            "name": p[2],
                            "email": p[3],
                            "role": p[4],
                            "phone": p[5],
                        }
                        break
            if not user_info:
                user_info = {"id": 1, "name": name, "email": email, "role": role, "phone": phone}

            token = _issue_session(user_info)
            self._send_json({
                "message": f"Welcome, {user_info['name']}! Registered with Goa University domain.",
                "token": token,
                "user": user_info,
            }, status=201)
            return

        # Campus Login via C11 Backend (--login)
        if parsed.path in ("/api/login", "/api/auth/login"):
            email = _clean_text(data.get("email"), 120).lower()
            pin = str(data.get("pin") or data.get("password") or "").strip()

            if not email or not pin:
                self._send_json({"error": "Please provide your university email and PIN."}, status=400)
                return

            lines, code = run_c_backend(["--login", email, pin])
            if code != 0 or not lines or lines[0].startswith("ERROR|"):
                msg = lines[0].split("|", 1)[1] if lines and "|" in lines[0] else "Invalid credentials. Verify your @unigoa.ac.in email and PIN."
                self._send_json({"error": msg}, status=401)
                return

            # Output format: OK|Login successful|<userId>|<fullName>|<email>|<role>|<phone>
            parts = lines[0].split("|")
            if len(parts) >= 7:
                user_info = {
                    "id": int(parts[2]),
                    "name": parts[3],
                    "email": parts[4],
                    "role": parts[5],
                    "phone": parts[6],
                }
            else:
                user_info = {"id": 1, "name": email.split("@")[0].title(), "email": email, "role": "Member", "phone": ""}

            token = _issue_session(user_info)
            self._send_json({
                "message": f"Welcome back, {user_info['name']}!",
                "token": token,
                "user": user_info,
            })
            return

        if parsed.path in ("/api/auth/reset-pin", "/api/reset-pin"):
            email = _clean_text(data.get("email"), 120).lower()
            phone = _clean_text(data.get("phone"), 32)
            new_pin = str(data.get("newPin") or data.get("pin") or "").strip()

            if not email or not phone or not new_pin:
                self._send_json({"error": "Email, mobile number, and new PIN are required."}, status=400)
                return

            lines, code = run_c_backend(["--reset-pin", email, phone, new_pin])
            if code != 0 or not lines or lines[0].startswith("ERROR|"):
                msg = lines[0].split("|", 1)[1] if lines and "|" in lines[0] else "Password reset failed."
                self._send_json({"error": msg}, status=400)
                return

            msg = lines[0].split("|", 1)[1] if lines and "|" in lines[0] else "PIN reset successfully."
            self._send_json({"message": msg, "email": email})
            return

        if parsed.path == "/api/auth/logout":
            auth_header = self.headers.get("Authorization", "")
            token = auth_header[7:] if auth_header.startswith("Bearer ") else ""
            _revoke_session(token)
            self._send_json({"message": "Logged out successfully."})
            return

        if parsed.path == "/api/notifications/read":
            user = self._require_user()
            if user is None:
                return
            run_c_backend(["--read-notifications", user["email"]])
            self._send_json({"message": "All notifications marked as read."})
            return

        if parsed.path == "/api/driver/verify":
            user = self._require_user()
            if user is None:
                return
            target_email = _clean_text(data.get("email"), 120).lower()
            dl_num = _clean_text(data.get("dlNumber"), 32)
            verify_status = "1" if data.get("verified", True) else "0"

            if not target_email:
                self._send_json({"error": "Target user email is required."}, status=400)
                return

            cmd = ["--verify-driver", target_email, dl_num or "GA-07-2024-GU01"] if verify_status == "1" else ["--unverify-driver", target_email]
            lines, code = run_c_backend(cmd)
            if code != 0 or not lines or lines[0].startswith("ERROR|"):
                self._send_json({"error": lines[0].split("|", 1)[1] if lines else "Verification update failed."}, status=400)
                return

            self._send_json({
                "message": lines[0].split("|", 1)[1] if lines else "Driver verification status updated.",
                "verified": verify_status == "1",
            })
            return

        if parsed.path == "/api/admin/purge-cancelled":
            user = self._require_user()
            if user is None:
                return
            lines, code = run_c_backend(["--admin-purge-cancelled"])
            if code != 0 or not lines:
                self._send_json({"error": "Failed to purge cancelled bookings."}, status=500)
                return
            msg = lines[0].split("|", 1)[1] if "|" in lines[0] else lines[0]
            self._send_json({"message": msg})
            return

        # Edit Ride via C11 Backend (--edit-ride)
        if parsed.path == "/api/edit-ride":
            user = self._require_user()
            if user is None:
                return
            ride_id = str(data.get("rideId", "")).strip()
            dep_time = str(data.get("departureTime", "")).replace(":", "").strip()
            seats = str(data.get("totalSeats", "")).strip()

            if not ride_id.isdigit() or not dep_time.isdigit() or not seats.isdigit():
                self._send_json({"error": "Valid numeric Ride ID, Departure Time (HHMM), and Seats required."}, status=400)
                return

            rides_meta = _load_json_file(RIDES_META_PATH, dict)
            meta = rides_meta.get(ride_id, {})
            lines_find, _ = run_c_backend(["--find-id", ride_id])
            existing_rides = parse_rides(lines_find)
            if not existing_rides:
                self._send_json({"error": "Ride not found."}, status=404)
                return
            ride = existing_rides[0]

            user_email = user.get("email", "").casefold()
            user_name = user.get("name", "").casefold()
            is_driver = (meta.get("driverEmail", "").casefold() == user_email) or (ride["driverName"].casefold() == user_name)
            if not is_driver:
                self._send_json({"error": "Only the driver who posted this ride can edit it."}, status=403)
                return

            lines, code = run_c_backend(["--edit-ride", ride_id, dep_time, seats])
            if code != 0 or not lines or lines[0].startswith("ERROR|"):
                msg = lines[0].split("|", 1)[1] if lines and "|" in lines[0] else "Failed to update ride."
                self._send_json({"error": msg}, status=400)
                return

            if "vehicleDesc" in data:
                meta["vehicleDesc"] = _clean_text(data.get("vehicleDesc", ""), 120)
                rides_meta[ride_id] = meta
                _save_json_file(RIDES_META_PATH, rides_meta)

            msg = lines[0].split("|", 1)[1] if lines and "|" in lines[0] else "Ride updated successfully."
            updated_rides = parse_rides(lines[1:])
            self._send_json({"message": msg, "rides": updated_rides})
            return

        # Export C Audit Report via POST
        if parsed.path == "/api/export-report":
            report_filename = os.path.join("data", "campus_summary.txt")
            lines, code = run_c_backend(["--export-report", report_filename])
            rep_path = os.path.join(DATA_DIR, "campus_summary.txt")
            if not os.path.exists(rep_path):
                rep_path = os.path.join(PROJECT_ROOT, report_filename)
            report_text = ""
            if os.path.exists(rep_path):
                with open(rep_path, "r", encoding="utf-8") as rf:
                    report_text = rf.read()
            if code != 0 or not report_text:
                self._send_json({"error": "Failed to generate C campus summary report."}, status=500)
                return
            self._send_json({
                "message": "Campus summary audit report exported successfully.",
                "filename": "campus_summary.txt",
                "report": report_text,
            })
            return

        if parsed.path == "/api/add-ride":
            user = self._require_user()
            if user is None:
                return
            driver = user["name"]
            vehicle = str(data.get("vehicleNumber", "")).strip()
            origin_id = str(data.get("originId", "1"))
            dest_id = str(data.get("destinationId", "0"))
            dep_time = str(data.get("departureTime", "0830")).replace(":", "").strip()
            seats = str(data.get("totalSeats", "4"))
            via = str(data.get("viaStop", "Direct")).strip() or "Direct"
            driver_phone = _clean_text(data.get("driverPhone", user.get("phone", "")), 20)
            ride_date = str(data.get("rideDate", datetime.date.today().isoformat())).strip()
            campus_point = _clean_text(data.get("campusPoint", "Main Admin Gate"), 64)
            vehicle_desc = _clean_text(data.get("vehicleDesc", ""), 120) or "Verified Campus Vehicle"

            lines, code = run_c_backend(["--add-ride", driver, vehicle, origin_id, dest_id, dep_time, seats, via, ride_date])
            if code != 0 or (lines and lines[0].startswith("ERROR|")):
                msg = lines[0].split("|", 1)[1] if lines and "|" in lines[0] else "Failed to publish ride."
                self._send_json({"error": msg}, status=400)
                return

            msg = lines[0].split("|", 1)[1] if lines and "|" in lines[0] else "Ride added"
            created_rides = parse_rides(lines[1:])
            if created_rides:
                new_ride_id = str(created_rides[0]["rideId"])
                rides_meta = _load_json_file(RIDES_META_PATH, dict)
                rides_meta[new_ride_id] = {
                    "driverEmail": user["email"],
                    "driverPhone": driver_phone,
                    "rideDate": ride_date,
                    "campusPoint": campus_point,
                    "status": "ACTIVE",
                    "vehicleDesc": vehicle_desc,
                }
                _save_json_file(RIDES_META_PATH, rides_meta)
                created_rides = parse_rides(lines[1:])

            self._send_json({"message": msg, "rides": created_rides})
            return

        if parsed.path == "/api/cancel-ride":
            user = self._require_user()
            if user is None:
                return
            ride_id = str(data.get("rideId", "")).strip()
            if not ride_id.isdigit():
                self._send_json({"error": "Invalid Ride ID."}, status=400)
                return

            rides_meta = _load_json_file(RIDES_META_PATH, dict)
            meta = rides_meta.get(ride_id, {})
            lines, _ = run_c_backend(["--find-id", ride_id])
            existing_rides = parse_rides(lines)
            if not existing_rides:
                self._send_json({"error": "Ride not found."}, status=404)
                return
            ride = existing_rides[0]

            is_driver = (meta.get("driverEmail", "").casefold() == user["email"].casefold()) or (ride["driverName"].casefold() == user["name"].casefold())
            if not is_driver:
                self._send_json({"error": "Only the driver can cancel this ride."}, status=403)
                return

            # Cancel all active bookings on this ride
            b_lines, _ = run_c_backend(["--list-bookings"])
            for b in parse_bookings(b_lines):
                if str(b["rideId"]) == ride_id and b["bookingStatus"] == "CONFIRMED":
                    run_c_backend(["--cancel-booking", str(b["bookingId"])])

            # Delete the ride from C database
            run_c_backend(["--delete-ride", ride_id])
            meta["status"] = "CANCELLED"
            rides_meta[ride_id] = meta
            _save_json_file(RIDES_META_PATH, rides_meta)

            self._send_json({"message": f"Ride #{ride_id} cancelled and all passenger seats restored."})
            return

        if parsed.path == "/api/complete-ride":
            user = self._require_user()
            if user is None:
                return
            ride_id = str(data.get("rideId", "")).strip()
            rides_meta = _load_json_file(RIDES_META_PATH, dict)
            meta = rides_meta.get(ride_id, {})
            lines, _ = run_c_backend(["--find-id", ride_id])
            existing_rides = parse_rides(lines)
            if not existing_rides:
                self._send_json({"error": "Ride not found."}, status=404)
                return
            ride = existing_rides[0]
            is_driver = (meta.get("driverEmail", "").casefold() == user["email"].casefold()) or (ride["driverName"].casefold() == user["name"].casefold())
            if not is_driver:
                self._send_json({"error": "Only the driver can mark this ride completed."}, status=403)
                return

            meta["status"] = "COMPLETED"
            rides_meta[ride_id] = meta
            _save_json_file(RIDES_META_PATH, rides_meta)
            self._send_json({"message": f"Ride #{ride_id} marked as completed! Thank you for carpooling."})
            return

        if parsed.path == "/api/book":
            user = self._require_user()
            if user is None:
                return
            ride_id = str(data.get("rideId", "")).strip()
            passenger = user["name"]
            seats = str(data.get("seatsRequested", "1")).strip()

            # Guard 1: Prevent driver from booking their own ride
            rides_meta = _load_json_file(RIDES_META_PATH, dict)
            ride_meta = rides_meta.get(ride_id, {})
            lines_find, _ = run_c_backend(["--find-id", ride_id])
            existing_rides = parse_rides(lines_find)
            if existing_rides:
                r = existing_rides[0]
                if (r["driverName"].casefold() == user["name"].casefold()) or (ride_meta.get("driverEmail", "").casefold() == user["email"].casefold()):
                    self._send_json({"error": "Drivers cannot reserve seats on their own ride."}, status=400)
                    return

            # Guard 2: Prevent duplicate confirmed bookings by the same user on the same ride
            lines_b, _ = run_c_backend(["--list-bookings"])
            all_b = parse_bookings(lines_b)
            user_email = user.get("email", "").casefold()
            user_name = user.get("name", "").casefold()
            has_existing = any(
                str(b["rideId"]) == ride_id and b["bookingStatus"] == "CONFIRMED" and
                ((b.get("passengerEmail", "").casefold() == user_email) or (b["passengerName"].casefold() == user_name))
                for b in all_b
            )
            if has_existing:
                self._send_json({"error": f"You already have an active reservation on Ride #{ride_id}."}, status=400)
                return

            lines, code = run_c_backend(["--book", ride_id, passenger, seats])
            if code != 0 or (lines and lines[0].startswith("ERROR|")):
                msg = lines[0].split("|", 1)[1] if lines and "|" in lines[0] else "Booking failed."
                self._send_json({"error": msg}, status=400)
                return

            msg = lines[0].split("|", 1)[1] if lines and "|" in lines[0] else "Booking confirmed"
            created_bookings = parse_bookings(lines[1:])
            if created_bookings:
                new_booking = created_bookings[0]
                b_id_str = str(new_booking["bookingId"])
                otp = f"{secrets.randbelow(9000) + 1000}"
                bookings_meta = _load_json_file(BOOKINGS_META_PATH, dict)
                bookings_meta[b_id_str] = {
                    "passengerEmail": user["email"],
                    "otp": otp,
                    "paymentStatus": "UNPAID",
                    "bookedAt": datetime.datetime.now().isoformat(),
                }
                _save_json_file(BOOKINGS_META_PATH, bookings_meta)
                created_bookings = parse_bookings(lines[1:])

            self._send_json({"message": msg, "bookings": created_bookings})
            return

        if parsed.path == "/api/cancel-booking":
            user = self._require_user()
            if user is None:
                return
            booking_id = str(data.get("bookingId", "")).strip()
            lines, _ = run_c_backend(["--list-bookings"])
            bookings = parse_bookings(lines)
            booking = next((entry for entry in bookings if str(entry["bookingId"]) == booking_id), None)
            if booking is None:
                self._send_json({"error": "Booking not found."}, status=404)
                return

            user_email = user.get("email", "").casefold()
            user_name = user.get("name", "").casefold()
            is_owner = (booking.get("passengerEmail", "").casefold() == user_email) or (booking["passengerName"].casefold() == user_name)
            if not is_owner:
                self._send_json({"error": "You can only cancel your own booking."}, status=403)
                return

            lines, code = run_c_backend(["--cancel-booking", booking_id])
            if code != 0 or (lines and lines[0].startswith("ERROR|")):
                msg = lines[0].split("|", 1)[1] if lines and "|" in lines[0] else "Cancellation failed."
                self._send_json({"error": msg}, status=400)
                return

            msg = lines[0].split("|", 1)[1] if lines and "|" in lines[0] else "Booking cancelled"
            self._send_json({"message": msg, "bookings": parse_bookings(lines[1:])})
            return

        if parsed.path == "/api/payment-status":
            user = self._require_user()
            if user is None:
                return
            booking_id = str(data.get("bookingId", "")).strip()
            new_status = "PAID" if str(data.get("paymentStatus", "")).upper() == "PAID" else "UNPAID"

            bookings_meta = _load_json_file(BOOKINGS_META_PATH, dict)
            meta = bookings_meta.get(booking_id, {})
            meta["paymentStatus"] = new_status
            bookings_meta[booking_id] = meta
            _save_json_file(BOOKINGS_META_PATH, bookings_meta)

            self._send_json({"message": f"Payment status set to {new_status}.", "paymentStatus": new_status})
            return

        self._send_json({"error": "Not found"}, status=404)

    def log_message(self, format, *args):
        return


if __name__ == "__main__":
    _ensure_directories()
    ensure_backend_compiled()
    port = int(os.environ.get("PORT", "8080"))
    host = os.environ.get("HOST", "127.0.0.1")
    server = HTTPServer((host, port), RideShareHandler)
    print(f"GU RideShare Dashboard running at http://{host}:{port}")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n[INFO] Server shutting down.")
        server.server_close()
