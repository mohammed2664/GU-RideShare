/*
 * algorithms.c — GU RideShare Algorithms Module
 *
 * Implements input validation with distinct error codes (Lab 3), 2D distance
 * and fare matrix lookups (Lab 5), multi-field linear search with date and seat
 * filters (Lab 4 & 7), sorting algorithms (Selection Sort - Lab 5, Bubble Sort - Lab 6,
 * ID-sort), recursive binary search (Lab 4 & 7), greedy 1-stop hub calculation,
 * recursive aggregation (Lab 7), user campus authentication, and transaction-safe
 * CRUD operations for rides and bookings (Lab 8).
 */
#include "rideshare.h"

/* ── 5×5 Goa campus transit hub names ──────────────────────────── */
const char *GOA_LOCATIONS[LOCATION_COUNT] = {
    "Goa University",
    "Panjim",
    "Margao",
    "Vasco",
    "Ponda"
};

/* Symmetric distance matrix between the five hubs in kilometres (Lab 5). */
const int GOA_DISTANCE_MATRIX[LOCATION_COUNT][LOCATION_COUNT] = {
    { 0,  7, 36, 28, 34},
    { 7,  0, 33, 29, 29},
    {36, 33,  0, 27, 23},
    {28, 29, 27,  0, 38},
    {34, 29, 23, 38,  0}
};

/* Pre-computed auto-rickshaw fare estimate between each hub pair in INR (Lab 5). */
const int GOA_FARE_MATRIX[LOCATION_COUNT][LOCATION_COUNT] = {
    {  0,  30, 145, 115, 135},
    { 30,   0, 130, 115, 115},
    {145, 130,   0, 110,  95},
    {115, 115, 110,   0, 150},
    {135, 115,  95, 150,   0}
};

/* Map a location enum to its display name (Lab 3). */
const char *get_location_name(int locationId) {
    switch (locationId) {
        case LOC_GOA_UNIVERSITY:
            return GOA_LOCATIONS[0];
        case LOC_PANJIM:
            return GOA_LOCATIONS[1];
        case LOC_MARGAO:
            return GOA_LOCATIONS[2];
        case LOC_VASCO:
            return GOA_LOCATIONS[3];
        case LOC_PONDA:
            return GOA_LOCATIONS[4];
        default:
            return "Unknown Location";
    }
}

/* Replace pipe, CR, and LF characters to prevent record injection (Lab 7). */
void sanitize_pipe_chars(char *str) {
    if (str == NULL) {
        return;
    }
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == '|' || str[i] == '\r' || str[i] == '\n') {
            str[i] = '/';
        }
    }
}

/*
 * Validate new-ride parameters (Lab 3).
 *   -1  invalid origin ID       -2  invalid destination ID
 *   -3  origin == destination   -4  bad HHMM time
 *   -5  seats out of range (1–7)
 */
int validate_ride_parameters(int originId, int destinationId, int departureTime, int seats) {
    if (originId < 0 || originId >= LOCATION_COUNT) {
        return -1;
    } else if (destinationId < 0 || destinationId >= LOCATION_COUNT) {
        return -2;
    } else if (originId == destinationId) {
        return -3;
    }

    int hours = departureTime / 100;
    int minutes = departureTime % 100;
    if (departureTime < 0 || hours > 23 || minutes > 59) {
        return -4;
    }

    if (seats <= 0 || seats > MAX_SEATS_LIMIT) {
        return -5;
    }

    return 1;
}

/*
 * Validate a booking request against available seats (Lab 3).
 *   -1  ride not found (NULL)    -2  requested seats <= 0
 *   -3  ride fully booked        -4  not enough seats
 */
int validate_booking_parameters(const Ride *ride, int requestedSeats) {
    if (ride == NULL) {
        return -1;
    } else if (requestedSeats <= 0) {
        return -2;
    } else if (ride->availableSeats == 0) {
        return -3;
    } else if (requestedSeats > ride->availableSeats) {
        return -4;
    }
    return 1;
}

/* Validate Goa University institutional email domain using strstr (Lab 7). */
int validate_campus_email(const char *email) {
    if (email == NULL || strlen(email) < 14) {
        return 0;
    }
    const char *domain = strstr(email, "@unigoa.ac.in");
    if (domain == NULL) {
        return 0;
    }
    /* Must terminate with @unigoa.ac.in */
    if (strcmp(domain, "@unigoa.ac.in") != 0) {
        return 0;
    }
    /* Must have a username before the @ */
    if (domain == email) {
        return 0;
    }
    return 1;
}

/* Look up distance between two hubs from 2D matrix (Lab 5). */
int get_matrix_distance(int originId, int destinationId) {
    if (originId < 0 || originId >= LOCATION_COUNT || destinationId < 0 || destinationId >= LOCATION_COUNT) {
        return 0;
    }
    return GOA_DISTANCE_MATRIX[originId][destinationId];
}

/* Look up auto-fare between two hubs from 2D matrix (Lab 5). */
int calculate_matrix_fare(int originId, int destinationId) {
    if (originId < 0 || originId >= LOCATION_COUNT || destinationId < 0 || destinationId >= LOCATION_COUNT) {
        return 0;
    }
    return GOA_FARE_MATRIX[originId][destinationId];
}

/* Greedy 1-stop hub heuristic: suggests shortest connection when direct rides are unavailable. */
int find_best_stopover_hub(int originId, int destinationId, int *bestTotalDistance) {
    if (originId < 0 || originId >= LOCATION_COUNT || destinationId < 0 || destinationId >= LOCATION_COUNT || originId == destinationId) {
        return -1;
    }

    int bestHub = -1;
    int minDist = 999999;

    for (int hub = 0; hub < LOCATION_COUNT; hub++) {
        if (hub == originId || hub == destinationId) {
            continue;
        }
        int totalDist = GOA_DISTANCE_MATRIX[originId][hub] + GOA_DISTANCE_MATRIX[hub][destinationId];
        if (totalDist < minDist) {
            minDist = totalDist;
            bestHub = hub;
        }
    }

    if (bestTotalDistance != NULL) {
        *bestTotalDistance = minDist;
    }
    return bestHub;
}

/* Construct formatted "Origin -> Via -> Destination" route string (Lab 7). */
void build_route_description(char *destBuffer, int originId, int destinationId, const char *viaStop) {
    char originName[32];
    char destinationName[32];
    char safeVia[64];

    strcpy(originName, get_location_name(originId));
    strcpy(destinationName, get_location_name(destinationId));

    if (viaStop != NULL && strlen(viaStop) > 0 && strcmp(viaStop, "Direct") != 0) {
        strncpy(safeVia, viaStop, sizeof(safeVia) - 1);
        safeVia[sizeof(safeVia) - 1] = '\0';
        sanitize_pipe_chars(safeVia);
        snprintf(destBuffer, MAX_ROUTE_LEN, "%.32s -> %.32s -> %.32s", originName, safeVia, destinationName);
    } else {
        snprintf(destBuffer, MAX_ROUTE_LEN, "%.32s -> %.32s", originName, destinationName);
    }
}

/* Case-insensitive substring search (Lab 7). */
int contains_ignore_case(const char *haystack, const char *needle) {
    char hLower[MAX_ROUTE_LEN];
    char nLower[MAX_ROUTE_LEN];
    size_t i;

    for (i = 0; i < MAX_ROUTE_LEN - 1 && haystack[i] != '\0'; i++) {
        hLower[i] = (char)tolower((unsigned char)haystack[i]);
    }
    hLower[i] = '\0';

    for (i = 0; i < MAX_ROUTE_LEN - 1 && needle[i] != '\0'; i++) {
        nLower[i] = (char)tolower((unsigned char)needle[i]);
    }
    nLower[i] = '\0';

    return strstr(hLower, nLower) != NULL;
}

/*
 * Linear Search (Lab 4 & Lab 7): Filter rides by optional origin/dest,
 * keyword (route, driver, vehicle), travel date, and minimum open seats.
 */
int linear_search_by_route(const Ride *rides, int count, int originId, int destinationId,
                            const char *keyword, const char *date, int minSeats, int *matchedIndices) {
    int matchCount = 0;

    for (int i = 0; i < count; i++) {
        int originMatch = (originId < 0) || (rides[i].originId == originId);
        int destMatch = (destinationId < 0) || (rides[i].destinationId == destinationId);
        int keywordMatch = 1;

        if (keyword != NULL && strlen(keyword) > 0 && strcmp(keyword, "ALL") != 0) {
            if (!contains_ignore_case(rides[i].routeDescription, keyword) &&
                !contains_ignore_case(rides[i].driverName, keyword) &&
                !contains_ignore_case(rides[i].vehicleNumber, keyword)) {
                keywordMatch = 0;
            }
        }

        int dateMatch = 1;
        if (date != NULL && strlen(date) > 0 && strcmp(date, "ALL") != 0) {
            if (rides[i].rideDate[0] != '\0' && strstr(rides[i].rideDate, date) == NULL) {
                dateMatch = 0;
            }
        }

        int seatMatch = 1;
        if (minSeats > 0) {
            if (rides[i].availableSeats < minSeats) {
                seatMatch = 0;
            }
        }

        if (originMatch && destMatch && keywordMatch && dateMatch && seatMatch) {
            matchedIndices[matchCount] = i;
            matchCount++;
        }
    }

    return matchCount;
}

/* Selection sort rides by ID in ascending order (guarantees binary search precondition). */
void sort_rides_by_id(Ride *rides, int count) {
    for (int i = 0; i < count - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < count; j++) {
            if (rides[j].rideId < rides[minIdx].rideId) {
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

/* Recursive Binary Search: O(log n) lookup by Ride ID (Lab 4 & Lab 7). */
int binary_search_ride_recursive(const Ride *rides, int low, int high, int targetRideId) {
    if (low > high) {
        return -1;
    }
    int mid = low + (high - low) / 2;
    if (rides[mid].rideId == targetRideId) {
        return mid;
    } else if (rides[mid].rideId < targetRideId) {
        return binary_search_ride_recursive(rides, mid + 1, high, targetRideId);
    } else {
        return binary_search_ride_recursive(rides, low, mid - 1, targetRideId);
    }
}

/* Selection Sort by departure time (earliest first), breaking ties by ID (Lab 5). */
void selection_sort_by_time(Ride *rides, int count) {
    for (int i = 0; i < count - 1; i++) {
        int earliestIdx = i;
        for (int j = i + 1; j < count; j++) {
            if (rides[j].departureTime < rides[earliestIdx].departureTime ||
               (rides[j].departureTime == rides[earliestIdx].departureTime &&
                rides[j].rideId < rides[earliestIdx].rideId)) {
                earliestIdx = j;
            }
        }
        if (earliestIdx != i) {
            Ride temp = rides[i];
            rides[i] = rides[earliestIdx];
            rides[earliestIdx] = temp;
        }
    }
}

/* Bubble Sort by lowest fare with early-exit flag (Lab 6). */
void bubble_sort_by_fare(Ride *rides, int count) {
    for (int i = 0; i < count - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < count - 1 - i; j++) {
            int shouldSwap = 0;
            if (rides[j].farePerSeat > rides[j + 1].farePerSeat) {
                shouldSwap = 1;
            } else if (rides[j].farePerSeat == rides[j + 1].farePerSeat &&
                       rides[j].departureTime > rides[j + 1].departureTime) {
                shouldSwap = 1;
            }

            if (shouldSwap) {
                Ride temp = rides[j];
                rides[j] = rides[j + 1];
                rides[j + 1] = temp;
                swapped = 1;
            }
        }
        if (!swapped) {
            break;
        }
    }
}

/* Recursively total available seats across all rides (Lab 7). */
int recursive_sum_available_seats(const Ride *rides, int count, int index) {
    if (index >= count) {
        return 0;
    }
    return rides[index].availableSeats + recursive_sum_available_seats(rides, count, index + 1);
}

/* Recursively total the open fleet fare pool (Lab 7). */
int recursive_sum_fare_pool(const Ride *rides, int count, int index) {
    if (index >= count) {
        return 0;
    }
    int currentRideValue = rides[index].availableSeats * rides[index].farePerSeat;
    return currentRideValue + recursive_sum_fare_pool(rides, count, index + 1);
}

/* Recursively total confirmed carpool booking revenue (Lab 7). */
int recursive_sum_booked_revenue(const Booking *bookings, int count, int index) {
    if (index >= count) {
        return 0;
    }
    int activeFare = (strcmp(bookings[index].bookingStatus, "CONFIRMED") == 0) ? bookings[index].totalFare : 0;
    return activeFare + recursive_sum_booked_revenue(bookings, count, index + 1);
}

/* Create a booking transaction atomically updating seat availability (Lab 8). */
int create_booking_transaction(RideShareDB *db, int rideId, const char *passengerName, int seatsRequested, Booking *outBooking) {
    sort_rides_by_id(db->rides, db->rideCount);
    int rideIdx = binary_search_ride_recursive(db->rides, 0, db->rideCount - 1, rideId);
    if (rideIdx < 0) {
        return -1;
    }

    Ride *targetRide = &db->rides[rideIdx];
    int validationStatus = validate_booking_parameters(targetRide, seatsRequested);
    if (validationStatus < 0) {
        return validationStatus;
    }

    targetRide->availableSeats -= seatsRequested;

    Booking newBooking;
    int maxBookingId = 5000;
    for (int i = 0; i < db->bookingCount; i++) {
        if (db->bookings[i].bookingId > maxBookingId) {
            maxBookingId = db->bookings[i].bookingId;
        }
    }

    newBooking.bookingId = maxBookingId + 1;
    newBooking.rideId = rideId;
    strncpy(newBooking.passengerName, passengerName, MAX_NAME_LEN - 1);
    newBooking.passengerName[MAX_NAME_LEN - 1] = '\0';
    sanitize_pipe_chars(newBooking.passengerName);
    newBooking.seatsBooked = seatsRequested;
    newBooking.totalFare = seatsRequested * targetRide->farePerSeat;
    strcpy(newBooking.bookingStatus, "CONFIRMED");

    if (!add_booking_to_memory(db, newBooking)) {
        return -5;
    }

    if (outBooking != NULL) {
        *outBooking = newBooking;
    }
    return 1;
}

/* Cancel a booking and restore the seats to the associated ride (Lab 8). */
int cancel_booking_transaction(RideShareDB *db, int bookingId, Booking *outBooking) {
    int bIdx = -1;
    for (int i = 0; i < db->bookingCount; i++) {
        if (db->bookings[i].bookingId == bookingId) {
            bIdx = i;
            break;
        }
    }
    if (bIdx < 0) {
        return -1;
    }

    Booking *targetBooking = &db->bookings[bIdx];
    if (strcmp(targetBooking->bookingStatus, "CANCELLED") == 0) {
        return -2;
    }

    sort_rides_by_id(db->rides, db->rideCount);
    int rideIdx = binary_search_ride_recursive(db->rides, 0, db->rideCount - 1, targetBooking->rideId);
    if (rideIdx >= 0) {
        db->rides[rideIdx].availableSeats += targetBooking->seatsBooked;
        if (db->rides[rideIdx].availableSeats > db->rides[rideIdx].totalSeats) {
            db->rides[rideIdx].availableSeats = db->rides[rideIdx].totalSeats;
        }
    }

    strcpy(targetBooking->bookingStatus, "CANCELLED");
    if (outBooking != NULL) {
        *outBooking = *targetBooking;
    }
    return 1;
}

/*
 * Edit Ride Transaction: Updates departure time and seat capacity in place
 * using Binary Search + struct pointer modification (Lab 4 & 8).
 *   -1  ride not found
 *   -2  invalid departure time
 *   -3  invalid seat count (1-7)
 *   -4  cannot reduce total seats below already booked seats
 */
int edit_ride_transaction(RideShareDB *db, int rideId, int newDepTime, int newSeats, Ride *outRide) {
    sort_rides_by_id(db->rides, db->rideCount);
    int idx = binary_search_ride_recursive(db->rides, 0, db->rideCount - 1, rideId);
    if (idx < 0) {
        return -1;
    }

    int hours = newDepTime / 100;
    int mins = newDepTime % 100;
    if (newDepTime < 0 || hours > 23 || mins > 59) {
        return -2;
    }

    if (newSeats <= 0 || newSeats > MAX_SEATS_LIMIT) {
        return -3;
    }

    Ride *r = &db->rides[idx];
    int bookedSeats = r->totalSeats - r->availableSeats;
    if (newSeats < bookedSeats) {
        return -4;
    }

    r->departureTime = newDepTime;
    r->totalSeats = newSeats;
    r->availableSeats = newSeats - bookedSeats;

    if (outRide != NULL) {
        *outRide = *r;
    }
    return 1;
}

/* Delete a ride record by shifting elements left in the array (Lab 4 CRUD). */
int delete_ride_transaction(RideShareDB *db, int rideId) {
    sort_rides_by_id(db->rides, db->rideCount);
    int idx = binary_search_ride_recursive(db->rides, 0, db->rideCount - 1, rideId);
    if (idx < 0) {
        return -1;
    }

    for (int i = idx; i < db->rideCount - 1; i++) {
        db->rides[i] = db->rides[i + 1];
    }
    db->rideCount--;
    return 1;
}

/* Register a new campus user with @unigoa.ac.in validation (Lab 8 & Lab #G). */
int register_user_transaction(RideShareDB *db, const char *fullName, const char *email,
                               const char *role, const char *phone, const char *passwordPin, User *outUser) {
    if (!validate_campus_email(email)) {
        return -1; /* invalid university email */
    }

    /* Check for duplicate registered email */
    for (int i = 0; i < db->userCount; i++) {
        if (strcasecmp(db->users[i].email, email) == 0) {
            return -2; /* email already registered */
        }
    }

    int maxId = 1000;
    for (int i = 0; i < db->userCount; i++) {
        if (db->users[i].userId > maxId) {
            maxId = db->users[i].userId;
        }
    }

    User newUser;
    newUser.userId = maxId + 1;
    strncpy(newUser.fullName, fullName, MAX_NAME_LEN - 1);
    newUser.fullName[MAX_NAME_LEN - 1] = '\0';
    sanitize_pipe_chars(newUser.fullName);

    strncpy(newUser.email, email, sizeof(newUser.email) - 1);
    newUser.email[sizeof(newUser.email) - 1] = '\0';
    sanitize_pipe_chars(newUser.email);

    strncpy(newUser.role, role ? role : "Student", sizeof(newUser.role) - 1);
    newUser.role[sizeof(newUser.role) - 1] = '\0';
    sanitize_pipe_chars(newUser.role);

    strncpy(newUser.phone, phone ? phone : "", sizeof(newUser.phone) - 1);
    newUser.phone[sizeof(newUser.phone) - 1] = '\0';
    sanitize_pipe_chars(newUser.phone);

    strncpy(newUser.passwordPin, passwordPin, sizeof(newUser.passwordPin) - 1);
    newUser.passwordPin[sizeof(newUser.passwordPin) - 1] = '\0';
    sanitize_pipe_chars(newUser.passwordPin);

    newUser.isVerified = 0;
    newUser.dlNumber[0] = '\0';
    newUser.trustScore = calculate_driver_trust_score(0, newUser.role, 0, 0);

    if (!add_user_to_memory(db, newUser)) {
        return -3; /* memory allocation failed */
    }

    if (outUser != NULL) {
        *outUser = newUser;
    }
    return 1;
}

/* Authenticate campus user credentials against in-memory records (Lab 8). */
int authenticate_user(const RideShareDB *db, const char *email, const char *passwordPin, User *outUser) {
    if (email == NULL || passwordPin == NULL) {
        return 0;
    }

    for (int i = 0; i < db->userCount; i++) {
        if (strcasecmp(db->users[i].email, email) == 0 &&
            strcmp(db->users[i].passwordPin, passwordPin) == 0) {
            if (outUser != NULL) {
                *outUser = db->users[i];
            }
            return 1;
        }
    }
    return 0;
}

/* Reset campus user security PIN with phone verification (Lab 2, 3, 8). */
int reset_user_password_transaction(RideShareDB *db, const char *email, const char *phone, const char *newPin) {
    if (db == NULL || email == NULL || phone == NULL || newPin == NULL) {
        return -1;
    }
    if (strstr(email, "@unigoa.ac.in") == NULL) {
        return -1;
    }
    if (strlen(newPin) < 4) {
        return -4;
    }

    int userIdx = -1;
    for (int i = 0; i < db->userCount; i++) {
        if (strcasecmp(db->users[i].email, email) == 0) {
            userIdx = i;
            break;
        }
    }
    if (userIdx < 0) {
        return -2;
    }

    /* Verify phone (either exact match or last 4 digits) */
    const char *dbPhone = db->users[userIdx].phone;
    if (strlen(dbPhone) >= 4 && strlen(phone) >= 4) {
        const char *dbLast4 = dbPhone + (strlen(dbPhone) - 4);
        const char *inLast4 = phone + (strlen(phone) - 4);
        if (strcmp(dbLast4, inLast4) != 0 && strcmp(dbPhone, phone) != 0) {
            return -3;
        }
    }

    snprintf(db->users[userIdx].passwordPin, sizeof(db->users[userIdx].passwordPin), "%s", newPin);
    sanitize_pipe_chars(db->users[userIdx].passwordPin);

    save_users_to_file(db, USERS_FILE);

    /* Security notification */
    log_notification(db, email, "Security PIN Reset",
                     "Your Goa University account PIN has been reset successfully. If this wasn't you, report to CS IT Admin.",
                     "SECURITY_OTP");

    return 1;
}

/* Hub baseline coordinates (Taleigão Plateau & Goan municipalities) */
static const double HUB_LATLNG[5][2] = {
    {15.4593, 73.8340}, /* 0: Goa University */
    {15.4909, 73.8278}, /* 1: Panjim */
    {15.2736, 73.9580}, /* 2: Margao */
    {15.3982, 73.8113}, /* 3: Vasco */
    {15.4026, 74.0086}  /* 4: Ponda */
};

/* Calculate piece-wise linear interpolated telemetry along Goan carpool corridors (Lab 1, 4, 8) */
int calculate_ride_telemetry(const Ride *ride, int progressPercent,
                             RideTelemetry *outTelem, RouteWaypoint *outWaypoints, int *outWaypointCount) {
    if (ride == NULL || outTelem == NULL) {
        return 0;
    }

    /* Clamp progress to 0 - 100 */
    if (progressPercent < 0) progressPercent = 0;
    if (progressPercent > 100) progressPercent = 100;

    int orig = ride->originId;
    int dest = ride->destinationId;
    if (orig < 0 || orig >= LOCATION_COUNT) orig = 1;
    if (dest < 0 || dest >= LOCATION_COUNT) dest = 0;

    RouteWaypoint wp[6];
    int count = 0;

    /* Build realistic corridor waypoints based on origin/destination */
    if (orig == LOC_PANJIM && dest == LOC_GOA_UNIVERSITY) {
        wp[0] = (RouteWaypoint){15.4909, 73.8278, "Panjim KTC Terminal", 0};
        wp[1] = (RouteWaypoint){15.4820, 73.8120, "Miramar Circle", 25};
        wp[2] = (RouteWaypoint){15.4690, 73.8150, "Caranzalem Beach Road", 50};
        wp[3] = (RouteWaypoint){15.4635, 73.8260, "Taleigao Plateau Incline", 75};
        wp[4] = (RouteWaypoint){15.4620, 73.8345, "GU Main Administrative Gate", 100};
        count = 5;
    } else if (orig == LOC_MARGAO && dest == LOC_GOA_UNIVERSITY) {
        wp[0] = (RouteWaypoint){15.2736, 73.9580, "Margao KTC Station", 0};
        wp[1] = (RouteWaypoint){15.3520, 73.9310, "Verna Industrial Bypass", 25};
        wp[2] = (RouteWaypoint){15.4080, 73.8680, "Zuari Bridge / Agassaim", 55};
        wp[3] = (RouteWaypoint){15.4530, 73.8520, "Bambolim GMC Junction", 80};
        wp[4] = (RouteWaypoint){15.4593, 73.8340, "Goa University Plateau Hub", 100};
        count = 5;
    } else if (orig == LOC_VASCO && dest == LOC_GOA_UNIVERSITY) {
        wp[0] = (RouteWaypoint){15.3982, 73.8113, "Vasco City Hub", 0};
        wp[1] = (RouteWaypoint){15.4050, 73.8370, "Chicalim Junction", 30};
        wp[2] = (RouteWaypoint){15.4180, 73.8650, "Zuari Overpass", 60};
        wp[3] = (RouteWaypoint){15.4530, 73.8520, "Bambolim GMC Cross", 85};
        wp[4] = (RouteWaypoint){15.4570, 73.8328, "GU Science Faculty Gate", 100};
        count = 5;
    } else if (orig == LOC_PONDA && dest == LOC_GOA_UNIVERSITY) {
        wp[0] = (RouteWaypoint){15.4026, 74.0086, "Ponda Tisk Bus Stand", 0};
        wp[1] = (RouteWaypoint){15.4510, 73.9450, "Banastarim Bridge", 30};
        wp[2] = (RouteWaypoint){15.5030, 73.9120, "Old Goa Bypass", 60};
        wp[3] = (RouteWaypoint){15.4720, 73.8550, "Ribandar Causeway", 85};
        wp[4] = (RouteWaypoint){15.4582, 73.8336, "GU Central Library Gate", 100};
        count = 5;
    } else {
        /* General corridor interpolation between any two hubs */
        wp[0] = (RouteWaypoint){HUB_LATLNG[orig][0], HUB_LATLNG[orig][1], "Origin Departure Hub", 0};
        wp[1] = (RouteWaypoint){(HUB_LATLNG[orig][0] * 2 + HUB_LATLNG[dest][0]) / 3.0 + 0.008,
                                (HUB_LATLNG[orig][1] * 2 + HUB_LATLNG[dest][1]) / 3.0, "Corridor Waypoint A", 33};
        wp[2] = (RouteWaypoint){(HUB_LATLNG[orig][0] + HUB_LATLNG[dest][0] * 2) / 3.0 + 0.005,
                                (HUB_LATLNG[orig][1] + HUB_LATLNG[dest][1] * 2) / 3.0, "Corridor Waypoint B", 66};
        wp[3] = (RouteWaypoint){HUB_LATLNG[dest][0], HUB_LATLNG[dest][1], "Destination Hub", 100};
        count = 4;
    }

    if (outWaypoints != NULL && outWaypointCount != NULL) {
        for (int i = 0; i < count; i++) {
            outWaypoints[i] = wp[i];
        }
        *outWaypointCount = count;
    }

    /* Locate bounding waypoints for piece-wise interpolation */
    int segIdx = 0;
    for (int i = 0; i < count - 1; i++) {
        if (progressPercent >= wp[i].percent && progressPercent <= wp[i + 1].percent) {
            segIdx = i;
            break;
        }
    }

    int span = wp[segIdx + 1].percent - wp[segIdx].percent;
    double t = 0.0;
    if (span > 0) {
        t = (double)(progressPercent - wp[segIdx].percent) / (double)span;
    }

    double curLat = wp[segIdx].lat + t * (wp[segIdx + 1].lat - wp[segIdx].lat);
    double curLng = wp[segIdx].lng + t * (wp[segIdx + 1].lng - wp[segIdx].lng);

    /* Telemetry metrics */
    double remainingRatio = (100.0 - progressPercent) / 100.0;
    int distRem = (int)(ride->distanceKm * remainingRatio + 0.5);
    int etaMin = (int)((distRem / 35.0) * 60.0 + 0.5);
    if (distRem > 0 && etaMin < 2) etaMin = 2; /* minimum ETA */
    if (progressPercent >= 100) { distRem = 0; etaMin = 0; }

    outTelem->rideId = ride->rideId;
    outTelem->currentLat = curLat;
    outTelem->currentLng = curLng;
    strncpy(outTelem->currentCheckpoint, wp[segIdx].checkpointName, sizeof(outTelem->currentCheckpoint) - 1);
    outTelem->currentCheckpoint[sizeof(outTelem->currentCheckpoint) - 1] = '\0';
    strncpy(outTelem->nextCheckpoint, wp[segIdx + 1].checkpointName, sizeof(outTelem->nextCheckpoint) - 1);
    outTelem->nextCheckpoint[sizeof(outTelem->nextCheckpoint) - 1] = '\0';

    outTelem->distanceRemainingKm = distRem;
    outTelem->etaMinutes = etaMin;
    outTelem->progressPercent = progressPercent;

    /* 5-step trip progress status */
    if (progressPercent <= 15) {
        outTelem->statusStep = 1;
        strncpy(outTelem->statusText, "Confirmed & Scheduled", sizeof(outTelem->statusText) - 1);
    } else if (progressPercent <= 35) {
        outTelem->statusStep = 2;
        strncpy(outTelem->statusText, "Driver Approaching Pickup", sizeof(outTelem->statusText) - 1);
    } else if (progressPercent <= 50) {
        outTelem->statusStep = 3;
        strncpy(outTelem->statusText, "Boarding & OTP Verification", sizeof(outTelem->statusText) - 1);
    } else if (progressPercent < 95) {
        outTelem->statusStep = 4;
        strncpy(outTelem->statusText, "In Transit to Campus", sizeof(outTelem->statusText) - 1);
    } else {
        outTelem->statusStep = 5;
        strncpy(outTelem->statusText, "Arrived at Destination", sizeof(outTelem->statusText) - 1);
    }
    outTelem->statusText[sizeof(outTelem->statusText) - 1] = '\0';

    return 1;
}

/* Calculate algorithmic Driver Campus Trust Score (Lab 1, 2, 3 & Lab #G). */
int calculate_driver_trust_score(int isVerified, const char *role, int completedRides, int cancelledRides) {
    int score = 50;
    if (isVerified) {
        score += 30;
    }
    if (role != NULL) {
        if (strcmp(role, "Faculty") == 0) {
            score += 10;
        } else if (strcmp(role, "Staff") == 0) {
            score += 5;
        } else if (strcmp(role, "Student") == 0) {
            score += 5;
        }
    }
    int expBonus = completedRides * 2;
    if (expBonus > 15) expBonus = 15;
    score += expBonus;

    score -= (cancelledRides * 10);

    if (score < 10) score = 10;
    if (score > 100) score = 100;

    return score;
}

const char *get_trust_badge_label(int trustScore) {
    if (trustScore >= 90) return "Elite Campus Driver";
    if (trustScore >= 75) return "Verified Driver";
    if (trustScore >= 50) return "Standard Driver";
    return "Under Review";
}

/* Verify driver credentials and recalculate trust scores across rides (Lab 3 & 8). */
int verify_driver_transaction(RideShareDB *db, const char *email, const char *dlNumber, int verifyStatus) {
    if (db == NULL || email == NULL) return 0;

    int userFound = 0;
    char driverName[MAX_NAME_LEN] = "";
    char role[32] = "Student";

    for (int i = 0; i < db->userCount; i++) {
        if (strcasecmp(db->users[i].email, email) == 0) {
            db->users[i].isVerified = verifyStatus;
            if (dlNumber != NULL && dlNumber[0] != '\0') {
                snprintf(db->users[i].dlNumber, sizeof(db->users[i].dlNumber), "%s", dlNumber);
            }
            db->users[i].trustScore = calculate_driver_trust_score(verifyStatus, db->users[i].role, 3, 0);
            snprintf(driverName, sizeof(driverName), "%s", db->users[i].fullName);
            snprintf(role, sizeof(role), "%s", db->users[i].role);
            userFound = 1;
            break;
        }
    }

    if (!userFound) return 0;

    /* Update trust score on all rides posted by this driver */
    for (int i = 0; i < db->rideCount; i++) {
        if (strcasecmp(db->rides[i].driverName, driverName) == 0) {
            db->rides[i].isDriverVerified = verifyStatus;
            db->rides[i].driverTrustScore = calculate_driver_trust_score(verifyStatus, role, 3, 0);
        }
    }

    save_users_to_file(db, USERS_FILE);
    save_rides_to_file(db, RIDES_FILE);

    /* Also log a campus notification */
    char notifMsg[160];
    snprintf(notifMsg, sizeof(notifMsg), "Your driver profile has been %s by Campus Security. Trust Score: %d%%.",
             verifyStatus ? "VERIFIED" : "UNVERIFIED",
             calculate_driver_trust_score(verifyStatus, role, 3, 0));
    log_notification(db, email, verifyStatus ? "Driver Profile Verified" : "Verification Status Updated", notifMsg, "SYSTEM");

    return 1;
}

/* Purge cancelled bookings from in-memory heap array via left-shifting (Lab 4 contiguous array deletion & Lab 8 dynamic memory). */
int purge_cancelled_bookings_transaction(RideShareDB *db) {
    if (db == NULL || db->bookingCount == 0) return 0;

    int purgedCount = 0;
    int writeIdx = 0;

    for (int readIdx = 0; readIdx < db->bookingCount; readIdx++) {
        if (strcmp(db->bookings[readIdx].bookingStatus, "CANCELLED") == 0) {
            purgedCount++;
        } else {
            if (writeIdx != readIdx) {
                db->bookings[writeIdx] = db->bookings[readIdx];
            }
            writeIdx++;
        }
    }

    db->bookingCount = writeIdx;
    if (purgedCount > 0) {
        save_bookings_to_file(db, BOOKINGS_FILE);
    }
    return purgedCount;
}

/* Calculate exact dynamic heap memory metrics in bytes (Lab 8 Dynamic Heap Allocation). */
void get_heap_memory_metrics(const RideShareDB *db, int *totalBytes, int *rideBytes, int *bookBytes, int *userBytes, int *notifBytes) {
    int rB = (db != NULL) ? (db->rideCapacity * (int)sizeof(Ride)) : 0;
    int bB = (db != NULL) ? (db->bookingCapacity * (int)sizeof(Booking)) : 0;
    int uB = (db != NULL) ? (db->userCapacity * (int)sizeof(User)) : 0;
    int nB = (db != NULL) ? (db->notificationCapacity * (int)sizeof(Notification)) : 0;
    int tot = rB + bB + uB + nB;

    if (rideBytes != NULL) *rideBytes = rB;
    if (bookBytes != NULL) *bookBytes = bB;
    if (userBytes != NULL) *userBytes = uB;
    if (notifBytes != NULL) *notifBytes = nB;
    if (totalBytes != NULL) *totalBytes = tot;
}
