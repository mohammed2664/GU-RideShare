/*
 * rideshare.h — GU RideShare Core Definitions
 *
 * Shared header for the Goa University Campus Carpool Matcher.
 * Declares all data structures, constants, matrix externs, and
 * function prototypes used by main.c, storage.c, and algorithms.c.
 *
 * Course: CSA5003 PSPL — Problem Solving and Programming Laboratory
 * Goa University, Master of Computer Applications (MCA)
 */

#ifndef RIDESHARE_H
#define RIDESHARE_H

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

/* ── Application-wide limits ─────────────────────────────────── */
#define LOCATION_COUNT 5       /* Number of Goa transit hubs     */
#define MAX_NAME_LEN 64        /* Max driver / passenger name    */
#define MAX_ROUTE_LEN 128      /* Max route description string   */
#define MAX_VEHICLE_LEN 32     /* Max vehicle registration text  */
#define MAX_SEATS_LIMIT 7      /* Upper bound on seats per ride  */
#define INITIAL_CAPACITY 4     /* Starting array size for dynamic heap */

/* ── Persistent file names ───────────────────────────────────── */
#define RIDES_FILE "data/rides.txt"
#define BOOKINGS_FILE "data/bookings.txt"
#define USERS_FILE "data/users.txt"
#define SUMMARY_REPORT_FILE "data/campus_summary.txt"
#define NOTIFICATIONS_FILE "data/notifications.txt"

/* Five campus transit nodes mapped to integer IDs (Lab 3 & 5). */
typedef enum {
    LOC_GOA_UNIVERSITY = 0,
    LOC_PANJIM         = 1,
    LOC_MARGAO         = 2,
    LOC_VASCO          = 3,
    LOC_PONDA          = 4
} GoaLocationId;

/* User account model for campus authentication (Lab 8 & Lab #G). */
typedef struct {
    int  userId;
    char fullName[MAX_NAME_LEN];
    char email[128];                    /* Validated @unigoa.ac.in   */
    char role[32];                      /* Student, Faculty, Staff  */
    char phone[32];
    char passwordPin[64];               /* PIN or password hash      */
    int  isVerified;                    /* 1 = Verified by GU Dean/Security, 0 = Pending */
    char dlNumber[32];                  /* Driving License or Faculty ID */
    int  trustScore;                    /* 10 - 100 calculated trust score */
} User;

/* A single offered ride with route, timing, date, and seat capacity. */
typedef struct {
    int  rideId;
    char driverName[MAX_NAME_LEN];
    char vehicleNumber[MAX_VEHICLE_LEN];
    int  originId;                       /* GoaLocationId enum       */
    int  destinationId;                  /* GoaLocationId enum       */
    char routeDescription[MAX_ROUTE_LEN];
    int  departureTime;                  /* HHMM 24-hour format      */
    int  totalSeats;
    int  availableSeats;
    int  distanceKm;                     /* From 2D distance matrix  */
    int  farePerSeat;                    /* From 2D fare matrix      */
    char rideDate[16];                   /* YYYY-MM-DD format        */
    int  isDriverVerified;               /* 1 = verified driver, 0 = pending */
    int  driverTrustScore;               /* 10 - 100 calculated score */
} Ride;

/* A passenger's reservation on a specific ride. */
typedef struct {
    int  bookingId;
    int  rideId;
    char passengerName[MAX_NAME_LEN];
    int  seatsBooked;
    int  totalFare;
    char bookingStatus[MAX_VEHICLE_LEN]; /* "CONFIRMED" or "CANCELLED" */
} Booking;

/* Campus activity notification model (Lab 4, 8 & Lab #G). */
typedef struct {
    int  notificationId;
    char recipientEmail[128];
    char title[64];
    char message[160];
    char type[32];      /* "BOOKING", "CANCEL", "RIDE_EDIT", "SECURITY_OTP", "SYSTEM" */
    char timestamp[32];
    int  isRead;        /* 0 = unread, 1 = read */
} Notification;

/* In-memory database backed by heap dynamic arrays (malloc/realloc/free). */
typedef struct {
    Ride         *rides;
    int           rideCount;
    int           rideCapacity;
    Booking      *bookings;
    int           bookingCount;
    int           bookingCapacity;
    User         *users;
    int           userCount;
    int           userCapacity;
    Notification *notifications;
    int           notificationCount;
    int           notificationCapacity;
} RideShareDB;

/* ── Shared data: 5×5 distance and fare lookup tables ─────────── */
extern const char *GOA_LOCATIONS[LOCATION_COUNT];
extern const int GOA_DISTANCE_MATRIX[LOCATION_COUNT][LOCATION_COUNT];
extern const int GOA_FARE_MATRIX[LOCATION_COUNT][LOCATION_COUNT];

/* ── storage.c — Heap dynamic array management and flat-file I/O ─ */
int  init_database(RideShareDB *db);
void free_database(RideShareDB *db);

int  ensure_ride_capacity(RideShareDB *db);
int  ensure_booking_capacity(RideShareDB *db);
int  ensure_user_capacity(RideShareDB *db);
int  ensure_notification_capacity(RideShareDB *db);

int  add_ride_to_memory(RideShareDB *db, Ride newRide);
int  add_booking_to_memory(RideShareDB *db, Booking newBooking);
int  add_user_to_memory(RideShareDB *db, User newUser);
int  add_notification_to_memory(RideShareDB *db, Notification newNotif);

int  load_rides_from_file(RideShareDB *db, const char *filename);
int  save_rides_to_file(const RideShareDB *db, const char *filename);

int  load_bookings_from_file(RideShareDB *db, const char *filename);
int  save_bookings_to_file(const RideShareDB *db, const char *filename);

int  load_users_from_file(RideShareDB *db, const char *filename);
int  save_users_to_file(const RideShareDB *db, const char *filename);

int  load_notifications_from_file(RideShareDB *db, const char *filename);
int  save_notifications_to_file(const RideShareDB *db, const char *filename);

int  log_notification(RideShareDB *db, const char *email, const char *title, const char *msg, const char *type);
int  get_user_notifications(const RideShareDB *db, const char *email, Notification *outArr, int maxCount, int *outCount, int *outUnreadCount);
int  mark_notifications_read(RideShareDB *db, const char *email);

void seed_default_data_if_empty(RideShareDB *db, const char *ridesFile, const char *bookingsFile, const char *usersFile, const char *notificationsFile);
int  export_campus_summary_report(const RideShareDB *db, const char *filename);

/* ── algorithms.c — Validation, search, sort, and transactions ── */
const char *get_location_name(int locationId);
void sanitize_pipe_chars(char *str);

int  validate_ride_parameters(int originId, int destinationId, int departureTime, int seats);
int  validate_booking_parameters(const Ride *ride, int requestedSeats);
int  validate_campus_email(const char *email);

int  get_matrix_distance(int originId, int destinationId);
int  calculate_matrix_fare(int originId, int destinationId);
int  find_best_stopover_hub(int originId, int destinationId, int *bestTotalDistance);
void build_route_description(char *destBuffer, int originId, int destinationId, const char *viaStop);

/* Search and sort algorithms */
int  contains_ignore_case(const char *haystack, const char *needle);
int  linear_search_by_route(const Ride *rides, int count, int originId, int destinationId,
                            const char *keyword, const char *date, int minSeats, int *matchedIndices);
void sort_rides_by_id(Ride *rides, int count);
int  binary_search_ride_recursive(const Ride *rides, int low, int high, int targetRideId);
void selection_sort_by_time(Ride *rides, int count);
void bubble_sort_by_fare(Ride *rides, int count);

/* Recursive aggregation analytics (Lab 7) */
int  recursive_sum_available_seats(const Ride *rides, int count, int index);
int  recursive_sum_fare_pool(const Ride *rides, int count, int index);
int  recursive_sum_booked_revenue(const Booking *bookings, int count, int index);

/* Transaction-safe booking, edit, cancellation, deletion, and auth */
int  create_booking_transaction(RideShareDB *db, int rideId, const char *passengerName, int seatsRequested, Booking *outBooking);
int  cancel_booking_transaction(RideShareDB *db, int bookingId, Booking *outBooking);
int  edit_ride_transaction(RideShareDB *db, int rideId, int newDepTime, int newSeats, Ride *outRide);
int  delete_ride_transaction(RideShareDB *db, int rideId);

int  register_user_transaction(RideShareDB *db, const char *fullName, const char *email,
                               const char *role, const char *phone, const char *passwordPin, User *outUser);
int  authenticate_user(const RideShareDB *db, const char *email, const char *passwordPin, User *outUser);
int  reset_user_password_transaction(RideShareDB *db, const char *email, const char *phone, const char *newPin);

/* ── Live Route Telemetry & Waypoint Tracking (Lab 1, 4, 8) ──── */
typedef struct {
    double lat;
    double lng;
    char   checkpointName[48];
    int    percent;
} RouteWaypoint;

typedef struct {
    int    rideId;
    double currentLat;
    double currentLng;
    char   currentCheckpoint[48];
    char   nextCheckpoint[48];
    int    distanceRemainingKm;
    int    etaMinutes;
    int    progressPercent;
    int    statusStep;  /* 1: Scheduled, 2: Driver Approaching, 3: Boarding & OTP, 4: In Transit, 5: Arrived */
    char   statusText[48];
} RideTelemetry;

int calculate_ride_telemetry(const Ride *ride, int progressPercent,
                             RideTelemetry *outTelem, RouteWaypoint *outWaypoints, int *outWaypointCount);

/* ── Driver Verification & Trust Score Engine (Lab 1, 2, 3 & Lab #G) ── */
int  calculate_driver_trust_score(int isVerified, const char *role, int completedRides, int cancelledRides);
const char *get_trust_badge_label(int trustScore);
int  verify_driver_transaction(RideShareDB *db, const char *email, const char *dlNumber, int verifyStatus);

/* ── Admin Oversight & Dynamic Heap Diagnostics (Lab 4, 8 & Lab #G) ── */
int  purge_cancelled_bookings_transaction(RideShareDB *db);
void get_heap_memory_metrics(const RideShareDB *db, int *totalBytes, int *rideBytes, int *bookBytes, int *userBytes, int *notifBytes);

#endif
