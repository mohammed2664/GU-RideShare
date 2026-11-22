/*
 * main.c — GU RideShare CLI Controller & Interactive Console
 *
 * Provides two operational modes:
 *  1. Pipe-delimited CLI interface consumed by the Python HTTP server bridge
 *  2. Interactive console menu for standalone terminal evaluation
 *
 * Includes automated unit self-test suite (Lab 3–8), user registration,
 * ride editing, multi-field linear filtering, and audit report generation.
 */
#include "rideshare.h"

/* ── Human-readable console tables ─────────────────────────────── */

static void print_rides_table_human(const Ride *rides, int count) {
    printf("\n================================================================================================================================\n");
    printf("%-6s | %-18s | %-13s | %-32s | %-6s | %-10s | %-5s | %-4s | %-6s\n",
           "RideID", "Driver Name", "Vehicle No", "Route Description", "Time", "Date", "Seats", "Dist", "Fare");
    printf("--------------------------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-6d | %-18s | %-13s | %-32.32s | %02d:%02d  | %-10s | %d/%-3d | %2dkm | Rs.%-4d\n",
               rides[i].rideId,
               rides[i].driverName,
               rides[i].vehicleNumber,
               rides[i].routeDescription,
               rides[i].departureTime / 100,
               rides[i].departureTime % 100,
               rides[i].rideDate[0] ? rides[i].rideDate : "2026-11-23",
               rides[i].availableSeats,
               rides[i].totalSeats,
               rides[i].distanceKm,
               rides[i].farePerSeat);
    }
    printf("================================================================================================================================\n");
}

static void print_bookings_table_human(const Booking *bookings, int count) {
    printf("\n================================================================================\n");
    printf("%-9s | %-6s | %-24s | %-5s | %-9s | %-10s\n",
           "BookingID", "RideID", "Passenger Name", "Seats", "TotalFare", "Status");
    printf("--------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-9d | %-6d | %-24s | %-5d | Rs.%-6d | %-10s\n",
               bookings[i].bookingId,
               bookings[i].rideId,
               bookings[i].passengerName,
               bookings[i].seatsBooked,
               bookings[i].totalFare,
               bookings[i].bookingStatus);
    }
    printf("================================================================================\n");
}

static void print_users_table_human(const User *users, int count) {
    printf("\n================================================================================\n");
    printf("%-8s | %-22s | %-26s | %-10s | %-12s\n",
           "UserID", "Full Name", "University Email", "Role", "Phone");
    printf("--------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-8d | %-22s | %-26s | %-10s | %-12s\n",
               users[i].userId,
               users[i].fullName,
               users[i].email,
               users[i].role,
               users[i].phone);
    }
    printf("================================================================================\n");
}

static void print_matrix_human(void) {
    printf("\n=== 2D GOA CAMPUS DISTANCE (KM) & AUTO-FARE (INR) MATRIX ===\n");
    printf("%-16s", "Origin \\ Dest");
    for (int j = 0; j < LOCATION_COUNT; j++) {
        printf(" | %-15s", GOA_LOCATIONS[j]);
    }
    printf("\n----------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < LOCATION_COUNT; i++) {
        printf("%-16s", GOA_LOCATIONS[i]);
        for (int j = 0; j < LOCATION_COUNT; j++) {
            printf(" | %2dkm (Rs.%3d)  ", GOA_DISTANCE_MATRIX[i][j], GOA_FARE_MATRIX[i][j]);
        }
        printf("\n");
    }
    printf("==========================================================================================================\n");
}

/* ── Pipe-delimited output for the Python HTTP bridge ──────────── */

static void print_ride_pipe_row(const Ride *r) {
    printf("RIDE|%d|%s|%s|%d|%s|%d|%s|%s|%04d|%d|%d|%d|%d|%s|%d|%d\n",
           r->rideId,
           r->driverName,
           r->vehicleNumber,
           r->originId,
           get_location_name(r->originId),
           r->destinationId,
           get_location_name(r->destinationId),
           r->routeDescription,
           r->departureTime,
           r->totalSeats,
           r->availableSeats,
           r->distanceKm,
           r->farePerSeat,
           r->rideDate[0] ? r->rideDate : "2026-11-23",
           r->isDriverVerified,
           r->driverTrustScore);
}

static void print_booking_pipe_row(const Booking *b) {
    printf("BOOKING|%d|%d|%s|%d|%d|%s\n",
           b->bookingId,
           b->rideId,
           b->passengerName,
           b->seatsBooked,
           b->totalFare,
           b->bookingStatus);
}

static void print_user_pipe_row(const User *u) {
    printf("USER|%d|%s|%s|%s|%s|%d|%s|%d\n",
           u->userId,
           u->fullName,
           u->email,
           u->role,
           u->phone,
           u->isVerified,
           u->dlNumber[0] ? u->dlNumber : "N/A",
           u->trustScore);
}

/* ── Automated self-test suite (Lab 3–8) ───────────────────────── */

static int run_self_tests(const RideShareDB *db) {
    int passed = 0;
    int total = 6;

    printf("=== CSA5003 PSPL AUTOMATED C BACKEND SELF-TEST ===\n");
    if (validate_ride_parameters(1, 1, 830, 3) == -3) {
        printf("[PASS] 1/6 Lab 3 Branching: Rejects origin == destination\n");
        passed++;
    }
    if (validate_ride_parameters(1, 0, 2599, 3) == -4) {
        printf("[PASS] 2/6 Lab 3 Branching: Rejects invalid HHMM time (2599)\n");
        passed++;
    }
    if (validate_ride_parameters(1, 0, 830, 9) == -5) {
        printf("[PASS] 3/6 Lab 3 Branching: Rejects out-of-range seat count (9)\n");
        passed++;
    }
    if (calculate_matrix_fare(2, 0) == 145 && get_matrix_distance(2, 0) == 36) {
        printf("[PASS] 4/6 Lab 5 2D Matrix: Margao -> Goa Univ = 36 km, Rs.145\n");
        passed++;
    }
    if (db->rideCount > 0 && recursive_sum_available_seats(db->rides, db->rideCount, 0) >= 0) {
        printf("[PASS] 5/6 Lab 7 Recursion & Lab 8 Heap: %d rides loaded cleanly\n", db->rideCount);
        passed++;
    }
    int tsVerified = calculate_driver_trust_score(1, "Faculty", 3, 0);
    int tsUnverified = calculate_driver_trust_score(0, "Student", 0, 0);
    if (tsVerified >= 90 && tsUnverified <= 60) {
        printf("[PASS] 6/6 Lab 1-3 & Lab #G: Trust Score Engine (Verified=%d%%, Unverified=%d%%)\n", tsVerified, tsUnverified);
        passed++;
    }

    printf("Result: %d/%d tests passed.\n", passed, total);
    return passed == total ? 0 : 1;
}

/* ── CLI command dispatcher ────────────────────────────────────── */

static int handle_cli_mode(RideShareDB *db, int argc, char *argv[]) {
    const char *cmd = argv[1];

    if (strcmp(cmd, "--self-test") == 0) {
        return run_self_tests(db);
    }

    if (strcmp(cmd, "--list-rides") == 0) {
        if (argc >= 3 && strcmp(argv[2], "time") == 0) {
            selection_sort_by_time(db->rides, db->rideCount);
        } else if (argc >= 3 && strcmp(argv[2], "fare") == 0) {
            bubble_sort_by_fare(db->rides, db->rideCount);
        } else if (argc >= 3 && strcmp(argv[2], "id") == 0) {
            sort_rides_by_id(db->rides, db->rideCount);
        }
        for (int i = 0; i < db->rideCount; i++) {
            print_ride_pipe_row(&db->rides[i]);
        }
        return 0;
    }

    if (strcmp(cmd, "--list-bookings") == 0) {
        for (int i = 0; i < db->bookingCount; i++) {
            print_booking_pipe_row(&db->bookings[i]);
        }
        return 0;
    }

    if (strcmp(cmd, "--list-users") == 0) {
        for (int i = 0; i < db->userCount; i++) {
            print_user_pipe_row(&db->users[i]);
        }
        return 0;
    }

    if (strcmp(cmd, "--hubs") == 0) {
        for (int i = 0; i < LOCATION_COUNT; i++) {
            printf("HUB|%d|%s\n", i, GOA_LOCATIONS[i]);
        }
        return 0;
    }

    if (strcmp(cmd, "--search-route") == 0 && argc >= 5) {
        int originId = atoi(argv[2]);
        int destId = atoi(argv[3]);
        const char *keyword = argv[4];
        const char *date = (argc >= 6) ? argv[5] : "ALL";
        int minSeats = (argc >= 7) ? atoi(argv[6]) : 0;

        int *matchedIndices = (int *)malloc(sizeof(int) * (db->rideCount > 0 ? db->rideCount : 1));
        if (matchedIndices == NULL) {
            printf("ERROR|Memory allocation failed\n");
            return 1;
        }

        int matches = linear_search_by_route(db->rides, db->rideCount, originId, destId, keyword, date, minSeats, matchedIndices);
        for (int i = 0; i < matches; i++) {
            print_ride_pipe_row(&db->rides[matchedIndices[i]]);
        }
        if (matches == 0 && originId >= 0 && destId >= 0 && originId != destId) {
            int totalDist = 0;
            int hub = find_best_stopover_hub(originId, destId, &totalDist);
            if (hub >= 0) {
                printf("SUGGEST|%s -> %s -> %s|%d\n",
                       get_location_name(originId),
                       get_location_name(hub),
                       get_location_name(destId),
                       totalDist);
            }
        }
        free(matchedIndices);
        return 0;
    }

    if (strcmp(cmd, "--find-id") == 0 && argc >= 3) {
        int targetId = atoi(argv[2]);
        sort_rides_by_id(db->rides, db->rideCount);
        int idx = binary_search_ride_recursive(db->rides, 0, db->rideCount - 1, targetId);
        if (idx >= 0) {
            print_ride_pipe_row(&db->rides[idx]);
        } else {
            printf("NOT_FOUND|Ride ID %d not found via Recursive Binary Search\n", targetId);
        }
        return 0;
    }

    if (strcmp(cmd, "--track-ride") == 0 && argc >= 3) {
        int targetId = atoi(argv[2]);
        int progress = (argc >= 4) ? atoi(argv[3]) : 35;
        sort_rides_by_id(db->rides, db->rideCount);
        int idx = binary_search_ride_recursive(db->rides, 0, db->rideCount - 1, targetId);
        if (idx < 0) {
            printf("NOT_FOUND|Ride ID %d not found for tracking\n", targetId);
            return 1;
        }

        RideTelemetry telem;
        RouteWaypoint waypoints[10];
        int wpCount = 0;
        calculate_ride_telemetry(&db->rides[idx], progress, &telem, waypoints, &wpCount);

        printf("TELEMETRY|%d|%.5f|%.5f|%s|%s|%d|%d|%d|%d|%s\n",
               telem.rideId, telem.currentLat, telem.currentLng,
               telem.currentCheckpoint, telem.nextCheckpoint,
               telem.distanceRemainingKm, telem.etaMinutes,
               telem.progressPercent, telem.statusStep, telem.statusText);

        for (int i = 0; i < wpCount; i++) {
            printf("WAYPOINT|%d|%.5f|%.5f|%s|%d\n",
                   i, waypoints[i].lat, waypoints[i].lng, waypoints[i].checkpointName, waypoints[i].percent);
        }
        return 0;
    }

    if (strcmp(cmd, "--add-ride") == 0 && argc >= 8) {
        const char *driver = argv[2];
        const char *vehicle = argv[3];
        int originId = atoi(argv[4]);
        int destId = atoi(argv[5]);
        int depTime = atoi(argv[6]);
        int seats = atoi(argv[7]);
        const char *viaStop = (argc >= 9) ? argv[8] : "Direct";
        const char *dateStr = (argc >= 10) ? argv[9] : "2026-11-23";

        int status = validate_ride_parameters(originId, destId, depTime, seats);
        if (status < 0) {
            switch (status) {
                case -1: printf("ERROR|Invalid origin ID (must be 0-4).\n"); break;
                case -2: printf("ERROR|Invalid destination ID (must be 0-4).\n"); break;
                case -3: printf("ERROR|Origin and destination cannot be the same.\n"); break;
                case -4: printf("ERROR|Invalid departure time (use 24h HHMM, e.g. 0830 or 1730).\n"); break;
                case -5: printf("ERROR|Seats must be between 1 and 7.\n"); break;
                default: printf("ERROR|Invalid ride parameters.\n"); break;
            }
            return 1;
        }

        int maxId = 100;
        for (int i = 0; i < db->rideCount; i++) {
            if (db->rides[i].rideId > maxId) {
                maxId = db->rides[i].rideId;
            }
        }

        Ride newRide;
        newRide.rideId = maxId + 1;
        strncpy(newRide.driverName, driver, MAX_NAME_LEN - 1);
        newRide.driverName[MAX_NAME_LEN - 1] = '\0';
        sanitize_pipe_chars(newRide.driverName);
        strncpy(newRide.vehicleNumber, vehicle, MAX_VEHICLE_LEN - 1);
        newRide.vehicleNumber[MAX_VEHICLE_LEN - 1] = '\0';
        sanitize_pipe_chars(newRide.vehicleNumber);
        newRide.originId = originId;
        newRide.destinationId = destId;
        newRide.departureTime = depTime;
        newRide.totalSeats = seats;
        newRide.availableSeats = seats;
        newRide.distanceKm = get_matrix_distance(originId, destId);
        newRide.farePerSeat = calculate_matrix_fare(originId, destId);
        strncpy(newRide.rideDate, dateStr, sizeof(newRide.rideDate) - 1);
        newRide.rideDate[sizeof(newRide.rideDate) - 1] = '\0';
        sanitize_pipe_chars(newRide.rideDate);
        build_route_description(newRide.routeDescription, originId, destId, viaStop);

        newRide.isDriverVerified = 0;
        newRide.driverTrustScore = 55;
        for (int i = 0; i < db->userCount; i++) {
            if (strcasecmp(db->users[i].fullName, newRide.driverName) == 0) {
                newRide.isDriverVerified = db->users[i].isVerified;
                newRide.driverTrustScore = db->users[i].trustScore;
                break;
            }
        }

        if (!add_ride_to_memory(db, newRide) || !save_rides_to_file(db, RIDES_FILE)) {
            printf("ERROR|Failed to save new ride\n");
            return 1;
        }
        printf("OK|Ride %d added successfully\n", newRide.rideId);
        print_ride_pipe_row(&newRide);
        return 0;
    }

    if (strcmp(cmd, "--edit-ride") == 0 && argc >= 5) {
        int rideId = atoi(argv[2]);
        int newDepTime = atoi(argv[3]);
        int newSeats = atoi(argv[4]);

        Ride updated;
        int res = edit_ride_transaction(db, rideId, newDepTime, newSeats, &updated);
        if (res < 0) {
            switch (res) {
                case -1: printf("ERROR|Ride ID %d not found.\n", rideId); break;
                case -2: printf("ERROR|Invalid departure time (use 24h HHMM, e.g. 0830 or 1730).\n"); break;
                case -3: printf("ERROR|Seats must be between 1 and 7.\n"); break;
                case -4: printf("ERROR|Cannot reduce seats below already booked reservations.\n"); break;
                default: printf("ERROR|Edit ride failed.\n"); break;
            }
            return 1;
        }
        save_rides_to_file(db, RIDES_FILE);
        printf("OK|Ride %d updated successfully\n", updated.rideId);
        print_ride_pipe_row(&updated);
        return 0;
    }

    if (strcmp(cmd, "--delete-ride") == 0 && argc >= 3) {
        int rideId = atoi(argv[2]);
        if (delete_ride_transaction(db, rideId) < 0) {
            printf("ERROR|Ride %d not found\n", rideId);
            return 1;
        }
        save_rides_to_file(db, RIDES_FILE);
        printf("OK|Ride %d deleted from rides.txt\n", rideId);
        return 0;
    }

    if (strcmp(cmd, "--book") == 0 && argc >= 5) {
        int rideId = atoi(argv[2]);
        const char *passenger = argv[3];
        int seats = atoi(argv[4]);

        Booking created;
        int res = create_booking_transaction(db, rideId, passenger, seats, &created);
        if (res < 0) {
            switch (res) {
                case -1: printf("ERROR|Ride ID %d not found.\n", rideId); break;
                case -2: printf("ERROR|Must request at least 1 seat.\n"); break;
                case -3: printf("ERROR|This ride is fully booked (0 seats available).\n"); break;
                case -4: printf("ERROR|Not enough seats available for your request.\n"); break;
                case -5: printf("ERROR|Internal error: could not save booking to memory.\n"); break;
                default: printf("ERROR|Booking failed.\n"); break;
            }
            return 1;
        }
        save_rides_to_file(db, RIDES_FILE);
        save_bookings_to_file(db, BOOKINGS_FILE);
        printf("OK|Booking %d confirmed for Ride %d (Total: Rs.%d)\n", created.bookingId, created.rideId, created.totalFare);
        print_booking_pipe_row(&created);
        return 0;
    }

    if (strcmp(cmd, "--cancel-booking") == 0 && argc >= 3) {
        int bookingId = atoi(argv[2]);
        Booking cancelled;
        int res = cancel_booking_transaction(db, bookingId, &cancelled);
        if (res < 0) {
            switch (res) {
                case -1: printf("ERROR|Booking ID %d not found.\n", bookingId); break;
                case -2: printf("ERROR|Booking %d is already cancelled.\n", bookingId); break;
                default: printf("ERROR|Cancellation failed.\n"); break;
            }
            return 1;
        }
        save_rides_to_file(db, RIDES_FILE);
        save_bookings_to_file(db, BOOKINGS_FILE);
        printf("OK|Booking %d cancelled and %d seat(s) restored to Ride %d\n",
               cancelled.bookingId, cancelled.seatsBooked, cancelled.rideId);
        print_booking_pipe_row(&cancelled);
        return 0;
    }

    if (strcmp(cmd, "--register") == 0 && argc >= 7) {
        const char *name = argv[2];
        const char *email = argv[3];
        const char *role = argv[4];
        const char *phone = argv[5];
        const char *pin = argv[6];

        User created;
        int res = register_user_transaction(db, name, email, role, phone, pin, &created);
        if (res < 0) {
            switch (res) {
                case -1: printf("ERROR|Invalid university email domain. Must be @unigoa.ac.in.\n"); break;
                case -2: printf("ERROR|An account with email %s is already registered.\n", email); break;
                case -3: printf("ERROR|Internal error: failed to allocate memory for user.\n"); break;
                default: printf("ERROR|User registration failed.\n"); break;
            }
            return 1;
        }
        save_users_to_file(db, USERS_FILE);
        printf("OK|User %d registered successfully\n", created.userId);
        print_user_pipe_row(&created);
        return 0;
    }

    if (strcmp(cmd, "--login") == 0 && argc >= 4) {
        const char *email = argv[2];
        const char *pin = argv[3];

        User user;
        if (authenticate_user(db, email, pin, &user)) {
            printf("OK|Login successful|%d|%s|%s|%s|%s\n",
                   user.userId, user.fullName, user.email, user.role, user.phone);
            return 0;
        } else {
            printf("ERROR|Invalid credentials. Verify your @unigoa.ac.in email and PIN.\n");
            return 1;
        }
    }

    if (strcmp(cmd, "--reset-pin") == 0 && argc >= 5) {
        const char *email = argv[2];
        const char *phone = argv[3];
        const char *newPin = argv[4];

        int res = reset_user_password_transaction(db, email, phone, newPin);
        if (res == 1) {
            printf("OK|Security PIN reset successfully for %s\n", email);
            return 0;
        } else {
            switch (res) {
                case -1: printf("ERROR|Invalid email format. Must be @unigoa.ac.in.\n"); break;
                case -2: printf("ERROR|User account not found with email %s.\n", email); break;
                case -3: printf("ERROR|Phone number verification failed. Does not match registered mobile.\n"); break;
                case -4: printf("ERROR|New PIN must be at least 4 digits/characters.\n"); break;
                default: printf("ERROR|Password reset failed.\n"); break;
            }
            return 1;
        }
    }

    if (strcmp(cmd, "--export-report") == 0) {
        const char *filename = (argc >= 3) ? argv[2] : SUMMARY_REPORT_FILE;
        if (export_campus_summary_report(db, filename)) {
            printf("OK|Report generated successfully at %s\n", filename);
            return 0;
        } else {
            printf("ERROR|Could not generate report file %s\n", filename);
            return 1;
        }
    }

    if (strcmp(cmd, "--stats") == 0) {
        int totalAvailSeats = recursive_sum_available_seats(db->rides, db->rideCount, 0);
        int openFarePool = recursive_sum_fare_pool(db->rides, db->rideCount, 0);
        int bookedRevenue = recursive_sum_booked_revenue(db->bookings, db->bookingCount, 0);
        printf("STATS|TotalRides=%d|TotalBookings=%d|AvailableSeats=%d|OpenFarePool=%d|BookedRevenue=%d|TotalUsers=%d\n",
               db->rideCount, db->bookingCount, totalAvailSeats, openFarePool, bookedRevenue, db->userCount);
        return 0;
    }

    if (strcmp(cmd, "--matrix") == 0) {
        for (int i = 0; i < LOCATION_COUNT; i++) {
            for (int j = 0; j < LOCATION_COUNT; j++) {
                printf("MATRIX|%d|%s|%d|%s|%d|%d\n",
                       i, GOA_LOCATIONS[i], j, GOA_LOCATIONS[j],
                       GOA_DISTANCE_MATRIX[i][j], GOA_FARE_MATRIX[i][j]);
            }
        }
        return 0;
    }

    if (strcmp(cmd, "--list-notifications") == 0) {
        const char *email = (argc >= 3) ? argv[2] : NULL;
        Notification list[30];
        int count = 0, unread = 0;
        get_user_notifications(db, email, list, 30, &count, &unread);
        printf("NOTIF_SUMMARY|Total=%d|Unread=%d\n", count, unread);
        for (int i = 0; i < count; i++) {
            printf("NOTIF|%d|%s|%s|%s|%s|%s|%d\n",
                   list[i].notificationId, list[i].recipientEmail,
                   list[i].title, list[i].message,
                   list[i].type, list[i].timestamp, list[i].isRead);
        }
        return 0;
    }

    if (strcmp(cmd, "--read-notifications") == 0) {
        const char *email = (argc >= 3) ? argv[2] : NULL;
        int updated = mark_notifications_read(db, email);
        printf("OK|Marked %d notifications as read\n", updated);
        return 0;
    }

    if (strcmp(cmd, "--push-notification") == 0 && argc >= 6) {
        int notifId = log_notification(db, argv[2], argv[3], argv[4], argv[5]);
        printf("OK|Notification %d created\n", notifId);
        return 0;
    }

    if (strcmp(cmd, "--verify-driver") == 0 && argc >= 3) {
        const char *email = argv[2];
        const char *dl = (argc >= 4) ? argv[3] : "GA-07-2024-GU01";
        if (verify_driver_transaction(db, email, dl, 1)) {
            printf("OK|Driver %s verified successfully\n", email);
            return 0;
        } else {
            printf("ERROR|User with email %s not found\n", email);
            return 1;
        }
    }

    if (strcmp(cmd, "--unverify-driver") == 0 && argc >= 3) {
        const char *email = argv[2];
        if (verify_driver_transaction(db, email, NULL, 0)) {
            printf("OK|Driver %s unverified\n", email);
            return 0;
        } else {
            printf("ERROR|User with email %s not found\n", email);
            return 1;
        }
    }

    if (strcmp(cmd, "--trust-score") == 0 && argc >= 3) {
        const char *email = argv[2];
        for (int i = 0; i < db->userCount; i++) {
            if (strcasecmp(db->users[i].email, email) == 0) {
                int score = db->users[i].trustScore;
                printf("TRUST_SCORE|email=%s|name=%s|role=%s|isVerified=%d|dl=%s|score=%d|badge=%s\n",
                       db->users[i].email, db->users[i].fullName, db->users[i].role,
                       db->users[i].isVerified, db->users[i].dlNumber, score,
                       get_trust_badge_label(score));
                return 0;
            }
        }
        printf("ERROR|User with email %s not found\n", email);
        return 1;
    }

    if (strcmp(cmd, "--admin-purge-cancelled") == 0) {
        int purged = purge_cancelled_bookings_transaction(db);
        printf("OK|Purged %d cancelled booking(s) and reclaimed contiguous heap memory\n", purged);
        return 0;
    }

    if (strcmp(cmd, "--admin-heap-stats") == 0) {
        int totalBytes = 0, rBytes = 0, bBytes = 0, uBytes = 0, nBytes = 0;
        get_heap_memory_metrics(db, &totalBytes, &rBytes, &bBytes, &uBytes, &nBytes);
        printf("HEAP_STATS|TotalAllocatedBytes=%d|RidesBytes=%d|BookingsBytes=%d|UsersBytes=%d|NotifsBytes=%d|RideCount=%d|RideCap=%d|BookCount=%d|BookCap=%d|UserCount=%d|UserCap=%d|NotifCount=%d|NotifCap=%d\n",
               totalBytes, rBytes, bBytes, uBytes, nBytes,
               db->rideCount, db->rideCapacity,
               db->bookingCount, db->bookingCapacity,
               db->userCount, db->userCapacity,
               db->notificationCount, db->notificationCapacity);
        return 0;
    }

    printf("ERROR|Unknown CLI command: %s\n", cmd);
    return 1;
}

/* ── Interactive console menu ──────────────────────────────────── */

static void run_interactive_menu(RideShareDB *db) {
    int choice = -1;
    while (choice != 0) {
        printf("\n==================================================================\n");
        printf("   GOA UNIVERSITY CAMPUS RIDESHARE & CARPOOL MATCHER (C11)       \n");
        printf("   Problem Solving and Programming Laboratory (CSA5003 PSPL)     \n");
        printf("==================================================================\n");
        printf(" 1. View All Active Rides\n");
        printf(" 2. Sort Rides by Departure Time (Selection Sort - Lab 5)\n");
        printf(" 3. Sort Rides by Lowest Fare (Bubble Sort - Lab 6)\n");
        printf(" 4. Search Rides (Route, Keyword, Date, Min Seats - Lab 4/7)\n");
        printf(" 5. Find Ride by Ride ID (Recursive Binary Search - Lab 4/7)\n");
        printf(" 6. Offer a New Campus Ride (2D Matrix & Validation - Lab 3/5)\n");
        printf(" 7. Edit Existing Ride (Departure Time / Seats - Lab 4/8)\n");
        printf(" 8. Book Seats on a Ride (Dynamic Structs & File I/O - Lab 8/#G)\n");
        printf(" 9. Cancel a Booking & Restore Seats (Rollback - Lab 8/#G)\n");
        printf("10. Delete a Ride by ID (Array Shift CRUD - Lab 4/#G)\n");
        printf("11. View All Bookings & Recursive Summary (Recursion - Lab 7)\n");
        printf("12. View 5x5 Goa Distance & Fare Matrix (2D Array - Lab 5)\n");
        printf("13. Register Campus User (@unigoa.ac.in - Lab 8/#G)\n");
        printf("14. View Registered Campus Users\n");
        printf("15. Export Formal C Audit Report (campus_summary.txt)\n");
        printf(" 0. Save & Exit\n");
        printf("------------------------------------------------------------------\n");
        printf("Enter choice (0-15): ");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            choice = -1;
            continue;
        }
        while (getchar() != '\n');

        switch (choice) {
            case 1:
                print_rides_table_human(db->rides, db->rideCount);
                break;
            case 2:
                selection_sort_by_time(db->rides, db->rideCount);
                print_rides_table_human(db->rides, db->rideCount);
                break;
            case 3:
                bubble_sort_by_fare(db->rides, db->rideCount);
                print_rides_table_human(db->rides, db->rideCount);
                break;
            case 4: {
                int orig, dest, minSeats;
                char kw[64], date[32];
                printf("Locations: 0=Goa Univ, 1=Panjim, 2=Margao, 3=Vasco, 4=Ponda (-1 for Any)\n");
                printf("Origin ID: "); scanf("%d", &orig);
                printf("Destination ID: "); scanf("%d", &dest);
                while (getchar() != '\n');
                printf("Keyword (or ALL): ");
                fgets(kw, sizeof(kw), stdin);
                kw[strcspn(kw, "\r\n")] = '\0';
                printf("Date (YYYY-MM-DD or ALL): ");
                fgets(date, sizeof(date), stdin);
                date[strcspn(date, "\r\n")] = '\0';
                printf("Minimum Available Seats (0 for Any): ");
                scanf("%d", &minSeats);
                while (getchar() != '\n');

                int *matched = (int *)malloc(sizeof(int) * (db->rideCount > 0 ? db->rideCount : 1));
                int count = linear_search_by_route(db->rides, db->rideCount, orig, dest, kw, date, minSeats, matched);
                Ride *filtered = (Ride *)malloc(sizeof(Ride) * (count > 0 ? count : 1));
                for (int i = 0; i < count; i++) filtered[i] = db->rides[matched[i]];
                print_rides_table_human(filtered, count);
                if (count == 0 && orig >= 0 && dest >= 0 && orig != dest) {
                    int totalDist = 0;
                    int hub = find_best_stopover_hub(orig, dest, &totalDist);
                    if (hub >= 0) {
                        printf("\n[SUGGESTION] No direct ride found. Best 1-Stop Hub: %s -> %s -> %s (%d km)\n",
                               get_location_name(orig), get_location_name(hub), get_location_name(dest), totalDist);
                    }
                }
                free(filtered);
                free(matched);
                break;
            }
            case 5: {
                int targetId;
                printf("Enter Ride ID to find: "); scanf("%d", &targetId);
                while (getchar() != '\n');
                sort_rides_by_id(db->rides, db->rideCount);
                int idx = binary_search_ride_recursive(db->rides, 0, db->rideCount - 1, targetId);
                if (idx >= 0) print_rides_table_human(&db->rides[idx], 1);
                else printf("Ride ID %d not found via Recursive Binary Search.\n", targetId);
                break;
            }
            case 6: {
                char driver[MAX_NAME_LEN], vehicle[MAX_VEHICLE_LEN], via[MAX_NAME_LEN], date[16];
                int orig, dest, depTime, seats;
                printf("Driver Name: "); fgets(driver, sizeof(driver), stdin); driver[strcspn(driver, "\r\n")] = '\0';
                printf("Vehicle No: "); fgets(vehicle, sizeof(vehicle), stdin); vehicle[strcspn(vehicle, "\r\n")] = '\0';
                printf("Origin ID (0-4): "); scanf("%d", &orig);
                printf("Destination ID (0-4): "); scanf("%d", &dest);
                printf("Departure Time (HHMM): "); scanf("%d", &depTime);
                printf("Total Seats (1-7): "); scanf("%d", &seats);
                while (getchar() != '\n');
                printf("Via Landmark (or Direct): "); fgets(via, sizeof(via), stdin); via[strcspn(via, "\r\n")] = '\0';
                printf("Date (YYYY-MM-DD): "); fgets(date, sizeof(date), stdin); date[strcspn(date, "\r\n")] = '\0';
                if (date[0] == '\0') strcpy(date, "2026-11-23");

                int v = validate_ride_parameters(orig, dest, depTime, seats);
                if (v < 0) {
                    printf("Ride was not added: verify origin!=dest, HHMM time (0000-2359), and seats (1-7).\n");
                } else {
                    int maxId = 100;
                    for (int i = 0; i < db->rideCount; i++) if (db->rides[i].rideId > maxId) maxId = db->rides[i].rideId;
                    Ride r;
                    r.rideId = maxId + 1;
                    strncpy(r.driverName, driver, MAX_NAME_LEN - 1); r.driverName[MAX_NAME_LEN - 1] = '\0';
                    sanitize_pipe_chars(r.driverName);
                    strncpy(r.vehicleNumber, vehicle, MAX_VEHICLE_LEN - 1); r.vehicleNumber[MAX_VEHICLE_LEN - 1] = '\0';
                    sanitize_pipe_chars(r.vehicleNumber);
                    r.originId = orig;
                    r.destinationId = dest;
                    r.departureTime = depTime;
                    r.totalSeats = seats;
                    r.availableSeats = seats;
                    r.distanceKm = get_matrix_distance(orig, dest);
                    r.farePerSeat = calculate_matrix_fare(orig, dest);
                    strncpy(r.rideDate, date, sizeof(r.rideDate) - 1); r.rideDate[sizeof(r.rideDate) - 1] = '\0';
                    build_route_description(r.routeDescription, orig, dest, via);
                    add_ride_to_memory(db, r);
                    save_rides_to_file(db, RIDES_FILE);
                    printf("Ride #%d published successfully!\n", r.rideId);
                }
                break;
            }
            case 7: {
                int rideId, newTime, newSeats;
                printf("Enter Ride ID to edit: "); scanf("%d", &rideId);
                printf("Enter new Departure Time (HHMM): "); scanf("%d", &newTime);
                printf("Enter new Total Seats (1-7): "); scanf("%d", &newSeats);
                while (getchar() != '\n');
                Ride updated;
                int res = edit_ride_transaction(db, rideId, newTime, newSeats, &updated);
                if (res < 0) {
                    printf("Ride edit failed (Code %d). Check ride ID, valid time, and seat limits.\n", res);
                } else {
                    save_rides_to_file(db, RIDES_FILE);
                    printf("Ride #%d updated successfully! New Time: %02d:%02d, Available Seats: %d/%d\n",
                           updated.rideId, updated.departureTime / 100, updated.departureTime % 100,
                           updated.availableSeats, updated.totalSeats);
                }
                break;
            }
            case 8: {
                int rideId, seats;
                char passenger[MAX_NAME_LEN];
                printf("Enter Ride ID to book: "); scanf("%d", &rideId);
                while (getchar() != '\n');
                printf("Passenger Full Name: "); fgets(passenger, sizeof(passenger), stdin); passenger[strcspn(passenger, "\r\n")] = '\0';
                printf("Seats to reserve: "); scanf("%d", &seats);
                while (getchar() != '\n');
                Booking b;
                int res = create_booking_transaction(db, rideId, passenger, seats, &b);
                if (res < 0) {
                    printf("Booking failed: check ride ID and available seat count.\n");
                } else {
                    save_rides_to_file(db, RIDES_FILE);
                    save_bookings_to_file(db, BOOKINGS_FILE);
                    printf("Booking #%d confirmed! Ride #%d, Total Fare: Rs.%d\n", b.bookingId, b.rideId, b.totalFare);
                }
                break;
            }
            case 9: {
                int bookingId;
                printf("Enter Booking ID to cancel: "); scanf("%d", &bookingId);
                while (getchar() != '\n');
                Booking b;
                int res = cancel_booking_transaction(db, bookingId, &b);
                if (res < 0) {
                    printf("Cancellation failed: verify booking ID and active status.\n");
                } else {
                    save_rides_to_file(db, RIDES_FILE);
                    save_bookings_to_file(db, BOOKINGS_FILE);
                    printf("Booking #%d cancelled. %d seat(s) restored to Ride #%d.\n", b.bookingId, b.seatsBooked, b.rideId);
                }
                break;
            }
            case 10: {
                int rideId;
                printf("Enter Ride ID to delete: "); scanf("%d", &rideId);
                while (getchar() != '\n');
                if (delete_ride_transaction(db, rideId) < 0) {
                    printf("Ride #%d not found.\n", rideId);
                } else {
                    save_rides_to_file(db, RIDES_FILE);
                    printf("Ride #%d removed from active fleet.\n", rideId);
                }
                break;
            }
            case 11:
                print_bookings_table_human(db->bookings, db->bookingCount);
                printf("\nRecursive Fleet KPI Summary (Lab 7):\n");
                printf("  Available Fleet Seats : %d\n", recursive_sum_available_seats(db->rides, db->rideCount, 0));
                printf("  Open Fleet Fare Pool  : Rs.%d\n", recursive_sum_fare_pool(db->rides, db->rideCount, 0));
                printf("  Booked Seat Revenue   : Rs.%d\n", recursive_sum_booked_revenue(db->bookings, db->bookingCount, 0));
                break;
            case 12:
                print_matrix_human();
                break;
            case 13: {
                char name[MAX_NAME_LEN], email[128], role[32], phone[32], pin[64];
                printf("Full Name: "); fgets(name, sizeof(name), stdin); name[strcspn(name, "\r\n")] = '\0';
                printf("University Email (@unigoa.ac.in): "); fgets(email, sizeof(email), stdin); email[strcspn(email, "\r\n")] = '\0';
                printf("Role (Student / Faculty / Staff): "); fgets(role, sizeof(role), stdin); role[strcspn(role, "\r\n")] = '\0';
                printf("Phone Number: "); fgets(phone, sizeof(phone), stdin); phone[strcspn(phone, "\r\n")] = '\0';
                printf("PIN / Password: "); fgets(pin, sizeof(pin), stdin); pin[strcspn(pin, "\r\n")] = '\0';

                User u;
                int res = register_user_transaction(db, name, email, role, phone, pin, &u);
                if (res < 0) {
                    if (res == -1) printf("Registration failed: email must be a valid @unigoa.ac.in address.\n");
                    else if (res == -2) printf("Registration failed: an account with that email already exists.\n");
                    else printf("Registration failed: memory error.\n");
                } else {
                    save_users_to_file(db, USERS_FILE);
                    printf("User registered successfully! Assigned UserID: #%d\n", u.userId);
                }
                break;
            }
            case 14:
                print_users_table_human(db->users, db->userCount);
                break;
            case 15:
                if (export_campus_summary_report(db, SUMMARY_REPORT_FILE)) {
                    printf("Audit report successfully written to %s!\n", SUMMARY_REPORT_FILE);
                } else {
                    printf("Failed to generate report file.\n");
                }
                break;
            case 0:
                save_rides_to_file(db, RIDES_FILE);
                save_bookings_to_file(db, BOOKINGS_FILE);
                save_users_to_file(db, USERS_FILE);
                printf("Saved all databases. Goodbye!\n");
                break;
            default:
                printf("Invalid selection. Choose an option from 0 to 15.\n");
        }
    }
}

/* ── Entry point ───────────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    RideShareDB db;
    if (!init_database(&db)) {
        fprintf(stderr, "Fatal error: memory allocation failed.\n");
        return 1;
    }

    load_rides_from_file(&db, RIDES_FILE);
    load_bookings_from_file(&db, BOOKINGS_FILE);
    load_users_from_file(&db, USERS_FILE);
    load_notifications_from_file(&db, NOTIFICATIONS_FILE);
    seed_default_data_if_empty(&db, RIDES_FILE, BOOKINGS_FILE, USERS_FILE, NOTIFICATIONS_FILE);

    int exitCode = 0;
    if (argc > 1) {
        exitCode = handle_cli_mode(&db, argc, argv);
    } else {
        run_interactive_menu(&db);
    }

    free_database(&db);
    return exitCode;
}
