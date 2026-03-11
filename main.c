/*
 * Project Title: Bus Ticket Reservation + Cancellation System
 * Student Name : [Your Name]
 * Register No  : [Your Reg No]
 *
 * Concepts Used: structs, file I/O, functions, switch/case, input validation
 * Compatible   : GCC (local) + Emscripten WebAssembly (browser)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   CONSTANTS
   ============================================================ */
#define MAX_SEATS     40
#define MAX_ROUTES     3
#define MAX_BOOKINGS 200
#define FILE_NAME    "bookings.txt"
#define BUF          128   /* General input buffer size */

/* ============================================================
   STRUCTURE DEFINITIONS
   ============================================================ */

/* Represents a bus route */
struct Route {
    int  routeID;                  /* Unique route identifier (1-3)  */
    char source[30];               /* Origin city                    */
    char destination[30];          /* Destination city               */
    float fare;                    /* Ticket fare in INR             */
    int  totalSeats;               /* Total seats on the bus         */
    int  bookedSeats[MAX_SEATS];   /* 0 = free, 1 = booked           */
};

/* Represents a single booking/ticket */
struct Booking {
    int  ticketID;          /* Auto-generated unique ticket ID        */
    int  routeID;           /* Route booked on                        */
    int  seatNumber;        /* Seat number chosen (1-40)              */
    char passengerName[50]; /* Full name of passenger                 */
    float farePaid;         /* Amount paid for this ticket            */
    int  isCancelled;       /* 0 = active, 1 = cancelled              */
};

/* ============================================================
   GLOBAL DATA
   ============================================================ */
struct Route   routes[MAX_ROUTES];
struct Booking bookings[MAX_BOOKINGS];
int totalBookings = 0;
int nextTicketID  = 1001;

/* ============================================================
   HELPER: readLine
   Reads one full line from stdin into buf (max len).
   Trims the trailing newline.
   Works correctly on both terminal and Emscripten browser console.
   ============================================================ */
void readLine(char *buf, int len) {
    if (fgets(buf, len, stdin) != NULL) {
        buf[strcspn(buf, "\n")] = '\0';  /* strip newline */
    } else {
        buf[0] = '\0';
    }
}

/* ============================================================
   HELPER: readInt
   Reads a line and parses one integer from it.
   Returns 1 on success, 0 on failure.
   ============================================================ */
int readInt(int *out) {
    char buf[BUF];
    readLine(buf, BUF);
    return sscanf(buf, "%d", out) == 1;
}

/* ============================================================
   FUNCTION PROTOTYPES
   ============================================================ */
void initRoutes(void);
void loadBookingsFromFile(void);
void saveBookingsToFile(void);
void displayRoutes(void);
void displayAvailableSeats(int routeID);
int  bookTicket(void);
int  cancelTicket(void);
void searchTicket(void);
void dailyReport(void);
int  isValidName(const char *name);

/* ============================================================
   MAIN
   ============================================================ */
int main(void) {
    int choice;

    initRoutes();
    loadBookingsFromFile();

    printf("\n========================================\n");
    printf("   BUS TICKET RESERVATION SYSTEM\n");
    printf("========================================\n");

    do {
        printf("\n--- MAIN MENU ---\n");
        printf("1. Display All Routes\n");
        printf("2. Display Available Seats for a Route\n");
        printf("3. Book Ticket\n");
        printf("4. Cancel Ticket\n");
        printf("5. Search Ticket by Ticket ID\n");
        printf("6. Daily Collection Report\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        fflush(stdout);

        if (!readInt(&choice)) {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        switch (choice) {
            case 1: displayRoutes();   break;
            case 2: {
                int r;
                printf("Enter Route ID (1-%d): ", MAX_ROUTES);
                fflush(stdout);
                if (!readInt(&r) || r < 1 || r > MAX_ROUTES) {
                    printf("Invalid route ID.\n");
                } else {
                    displayAvailableSeats(r);
                }
                break;
            }
            case 3: bookTicket();      break;
            case 4: cancelTicket();    break;
            case 5: searchTicket();    break;
            case 6: dailyReport();     break;
            case 0: printf("Goodbye!\n"); break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 0);

    return 0;
}

/* ============================================================
   initRoutes: Hard-code 3 bus routes
   ============================================================ */
void initRoutes(void) {
    routes[0].routeID = 1;
    strcpy(routes[0].source,      "Chennai");
    strcpy(routes[0].destination, "Bangalore");
    routes[0].fare       = 350.0f;
    routes[0].totalSeats = MAX_SEATS;
    memset(routes[0].bookedSeats, 0, sizeof(routes[0].bookedSeats));

    routes[1].routeID = 2;
    strcpy(routes[1].source,      "Bangalore");
    strcpy(routes[1].destination, "Hyderabad");
    routes[1].fare       = 450.0f;
    routes[1].totalSeats = MAX_SEATS;
    memset(routes[1].bookedSeats, 0, sizeof(routes[1].bookedSeats));

    routes[2].routeID = 3;
    strcpy(routes[2].source,      "Chennai");
    strcpy(routes[2].destination, "Coimbatore");
    routes[2].fare       = 280.0f;
    routes[2].totalSeats = MAX_SEATS;
    memset(routes[2].bookedSeats, 0, sizeof(routes[2].bookedSeats));
}

/* ============================================================
   loadBookingsFromFile
   ============================================================ */
void loadBookingsFromFile(void) {
    FILE *fp = fopen(FILE_NAME, "r");
    if (fp == NULL) return;  /* First run - no file yet */

    totalBookings = 0;
    while (totalBookings < MAX_BOOKINGS &&
           fscanf(fp, "%d,%d,%d,%49[^,],%f,%d\n",
                  &bookings[totalBookings].ticketID,
                  &bookings[totalBookings].routeID,
                  &bookings[totalBookings].seatNumber,
                   bookings[totalBookings].passengerName,
                  &bookings[totalBookings].farePaid,
                  &bookings[totalBookings].isCancelled) == 6) {

        if (!bookings[totalBookings].isCancelled) {
            int r = bookings[totalBookings].routeID - 1;
            int s = bookings[totalBookings].seatNumber - 1;
            if (r >= 0 && r < MAX_ROUTES && s >= 0 && s < MAX_SEATS)
                routes[r].bookedSeats[s] = 1;
        }
        if (bookings[totalBookings].ticketID >= nextTicketID)
            nextTicketID = bookings[totalBookings].ticketID + 1;

        totalBookings++;
    }
    fclose(fp);
    printf("[Info] Loaded %d booking record(s).\n", totalBookings);
}

/* ============================================================
   saveBookingsToFile
   ============================================================ */
void saveBookingsToFile(void) {
    FILE *fp = fopen(FILE_NAME, "w");
    if (fp == NULL) {
        printf("[Error] Could not open file for saving.\n");
        return;
    }
    for (int i = 0; i < totalBookings; i++) {
        fprintf(fp, "%d,%d,%d,%s,%.2f,%d\n",
                bookings[i].ticketID,
                bookings[i].routeID,
                bookings[i].seatNumber,
                bookings[i].passengerName,
                bookings[i].farePaid,
                bookings[i].isCancelled);
    }
    fclose(fp);
}

/* ============================================================
   displayRoutes
   ============================================================ */
void displayRoutes(void) {
    printf("\n%-6s %-15s %-15s %8s %10s\n",
           "ID", "Source", "Destination", "Fare(Rs)", "Available");
    printf("------------------------------------------------------\n");
    for (int i = 0; i < MAX_ROUTES; i++) {
        int avail = 0;
        for (int s = 0; s < routes[i].totalSeats; s++)
            if (!routes[i].bookedSeats[s]) avail++;
        printf("%-6d %-15s %-15s %8.2f %10d\n",
               routes[i].routeID,
               routes[i].source,
               routes[i].destination,
               routes[i].fare,
               avail);
    }
}

/* ============================================================
   displayAvailableSeats
   ============================================================ */
void displayAvailableSeats(int routeID) {
    int idx = routeID - 1;
    printf("\nSeat map for Route %d (%s -> %s):\n",
           routeID, routes[idx].source, routes[idx].destination);
    printf("[ O = Available   X = Booked ]\n\n");
    for (int s = 0; s < routes[idx].totalSeats; s++) {
        printf("Seat %2d:[%c]  ", s + 1,
               routes[idx].bookedSeats[s] ? 'X' : 'O');
        if ((s + 1) % 4 == 0) printf("\n");
    }
    printf("\n");
}

/* ============================================================
   bookTicket
   ============================================================ */
int bookTicket(void) {
    int  routeID, seatNo;
    char name[50];
    char buf[BUF];

    displayRoutes();
    printf("Enter Route ID to book (1-%d): ", MAX_ROUTES);
    fflush(stdout);
    if (!readInt(&routeID) || routeID < 1 || routeID > MAX_ROUTES) {
        printf("[Error] Invalid route ID.\n");
        return -1;
    }

    displayAvailableSeats(routeID);
    printf("Enter Seat Number (1-%d): ", MAX_SEATS);
    fflush(stdout);
    if (!readInt(&seatNo) || seatNo < 1 || seatNo > MAX_SEATS) {
        printf("[Error] Seat number out of range.\n");
        return -1;
    }

    if (routes[routeID - 1].bookedSeats[seatNo - 1]) {
        printf("[Error] Seat %d is already booked. Choose another.\n", seatNo);
        return -1;
    }

    printf("Enter Passenger Name: ");
    fflush(stdout);
    readLine(buf, BUF);
    strncpy(name, buf, 49);
    name[49] = '\0';

    if (!isValidName(name)) {
        printf("[Error] Name cannot be empty or contain digits.\n");
        return -1;
    }

    if (totalBookings >= MAX_BOOKINGS) {
        printf("[Error] Booking limit reached.\n");
        return -1;
    }

    struct Booking b;
    b.ticketID    = nextTicketID++;
    b.routeID     = routeID;
    b.seatNumber  = seatNo;
    b.farePaid    = routes[routeID - 1].fare;
    b.isCancelled = 0;
    strncpy(b.passengerName, name, 49);
    b.passengerName[49] = '\0';

    bookings[totalBookings++] = b;
    routes[routeID - 1].bookedSeats[seatNo - 1] = 1;
    saveBookingsToFile();

    printf("\n=== BOOKING CONFIRMED ===\n");
    printf("Ticket ID    : %d\n", b.ticketID);
    printf("Passenger    : %s\n", b.passengerName);
    printf("Route        : %s -> %s\n",
           routes[routeID-1].source, routes[routeID-1].destination);
    printf("Seat         : %d\n", b.seatNumber);
    printf("Fare Paid    : Rs %.2f\n", b.farePaid);
    printf("=========================\n");

    return b.ticketID;
}

/* ============================================================
   cancelTicket
   ============================================================ */
int cancelTicket(void) {
    int ticketID;
    printf("Enter Ticket ID to cancel: ");
    fflush(stdout);
    if (!readInt(&ticketID)) {
        printf("[Error] Invalid Ticket ID.\n");
        return -1;
    }

    for (int i = 0; i < totalBookings; i++) {
        if (bookings[i].ticketID == ticketID) {
            if (bookings[i].isCancelled) {
                printf("[Info] Ticket %d is already cancelled.\n", ticketID);
                return -1;
            }
            bookings[i].isCancelled = 1;
            routes[bookings[i].routeID - 1].bookedSeats[bookings[i].seatNumber - 1] = 0;
            saveBookingsToFile();
            printf("[Success] Ticket %d cancelled. Seat %d on Route %d is now free.\n",
                   ticketID, bookings[i].seatNumber, bookings[i].routeID);
            return 0;
        }
    }
    printf("[Error] Ticket ID %d not found.\n", ticketID);
    return -1;
}

/* ============================================================
   searchTicket
   ============================================================ */
void searchTicket(void) {
    int ticketID;
    printf("Enter Ticket ID to search: ");
    fflush(stdout);
    if (!readInt(&ticketID)) {
        printf("[Error] Invalid input.\n");
        return;
    }

    for (int i = 0; i < totalBookings; i++) {
        if (bookings[i].ticketID == ticketID) {
            int r = bookings[i].routeID - 1;
            printf("\n--- TICKET DETAILS ---\n");
            printf("Ticket ID    : %d\n", bookings[i].ticketID);
            printf("Passenger    : %s\n", bookings[i].passengerName);
            printf("Route        : %s -> %s\n",
                   routes[r].source, routes[r].destination);
            printf("Seat Number  : %d\n", bookings[i].seatNumber);
            printf("Fare Paid    : Rs %.2f\n", bookings[i].farePaid);
            printf("Status       : %s\n",
                   bookings[i].isCancelled ? "CANCELLED" : "ACTIVE");
            printf("----------------------\n");
            return;
        }
    }
    printf("[Error] Ticket ID %d not found.\n", ticketID);
}

/* ============================================================
   dailyReport
   ============================================================ */
void dailyReport(void) {
    printf("\n========== DAILY COLLECTION REPORT ==========\n");
    printf("%-6s %-20s %10s %12s\n",
           "Route", "Path", "Tickets", "Revenue(Rs)");
    printf("---------------------------------------------\n");

    int   grandTickets = 0;
    float grandRevenue = 0.0f;

    for (int r = 0; r < MAX_ROUTES; r++) {
        int   tickets = 0;
        float revenue = 0.0f;
        for (int i = 0; i < totalBookings; i++) {
            if (bookings[i].routeID == r + 1 && !bookings[i].isCancelled) {
                tickets++;
                revenue += bookings[i].farePaid;
            }
        }
        char path[64];
        snprintf(path, sizeof(path), "%s -> %s",
                 routes[r].source, routes[r].destination);
        printf("%-6d %-20s %10d %12.2f\n",
               routes[r].routeID, path, tickets, revenue);
        grandTickets += tickets;
        grandRevenue += revenue;
    }
    printf("---------------------------------------------\n");
    printf("%-27s %10d %12.2f\n", "TOTAL", grandTickets, grandRevenue);
    printf("=============================================\n");
}

/* ============================================================
   isValidName: non-empty, no digits
   ============================================================ */
int isValidName(const char *name) {
    if (name == NULL || name[0] == '\0') return 0;
    for (int i = 0; name[i] != '\0'; i++)
        if (name[i] >= '0' && name[i] <= '9') return 0;
    return 1;
}
