/*
 * VEHICLE MANAGEMENT SYSTEM  (C, file handling)
 * ---------------------------------------------------------------
 * Algorithms used:
 *   1. Presorting          - merge sort first, then binary search / duplicate
 *                            detection / reports on the sorted data.
 *   2. Warshall            - transitive closure -> which locations are
 *                            reachable from which (road network).
 *   3. Floyd-Warshall      - all-pairs shortest distances + path rebuilding,
 *                            used to dispatch the nearest available vehicle.
 *
 * Files:
 *   vehicles.dat   (binary)  vehicle records
 *   locations.txt  (text)    number of locations + names
 *   roads.txt      (text)    directed roads: from to distance
 *
 * Compile:  gcc -Wall -o vehicle_mgmt vehicle_mgmt.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_VEH   200
#define MAX_LOC   20
#define INF       1000000
#define VEH_FILE  "vehicles.dat"
#define LOC_FILE  "locations.txt"
#define ROAD_FILE "roads.txt"

typedef struct {
    int  id;
    char plate[16];
    char model[32];
    int  type;      /* 0 Car, 1 Truck, 2 Bus, 3 Bike */
    int  mileage;   /* km driven */
    int  status;    /* 0 Available, 1 On trip, 2 In maintenance */
    int  loc;       /* current location index */
} Vehicle;

static const char *TYPE_NAME[]   = {"Car", "Truck", "Bus", "Bike"};
static const char *STATUS_NAME[] = {"Available", "On trip", "Maintenance"};

/* ---------------- global data ---------------- */
static Vehicle veh[MAX_VEH];
static int     nveh = 0;

static char locname[MAX_LOC][32];
static int  nloc = 0;
static int  adj[MAX_LOC][MAX_LOC];   /* direct road length, INF if none */

/* ---------------- input helpers ---------------- */
static void read_line(const char *prompt, char *buf, int size)
{
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, size, stdin)) { buf[0] = '\0'; return; }
    buf[strcspn(buf, "\r\n")] = '\0';
}

static int read_int(const char *prompt)
{
    char b[64];
    for (;;) {
        read_line(prompt, b, sizeof b);
        char *end;
        long v = strtol(b, &end, 10);
        if (end != b && *end == '\0') return (int)v;
        printf("  Please enter a whole number.\n");
    }
}

static int read_range(const char *prompt, int lo, int hi)
{
    for (;;) {
        int v = read_int(prompt);
        if (v >= lo && v <= hi) return v;
        printf("  Value must be between %d and %d.\n", lo, hi);
    }
}

static void upcase(char *s) { for (; *s; s++) *s = (char)toupper((unsigned char)*s); }

/* ---------------- file handling: vehicles ---------------- */
static void load_vehicles(void)
{
    FILE *fp = fopen(VEH_FILE, "rb");
    nveh = 0;
    if (!fp) return;
    while (nveh < MAX_VEH && fread(&veh[nveh], sizeof(Vehicle), 1, fp) == 1)
        nveh++;
    fclose(fp);
}

static void save_vehicles(void)
{
    FILE *fp = fopen(VEH_FILE, "wb");
    if (!fp) { printf("  ERROR: cannot write %s\n", VEH_FILE); return; }
    fwrite(veh, sizeof(Vehicle), (size_t)nveh, fp);
    fclose(fp);
}

/* ---------------- file handling: locations & roads ---------------- */
static void reset_adj(void)
{
    for (int i = 0; i < MAX_LOC; i++)
        for (int j = 0; j < MAX_LOC; j++)
            adj[i][j] = (i == j) ? 0 : INF;
}

static void save_locations(void)
{
    FILE *fp = fopen(LOC_FILE, "w");
    if (!fp) return;
    fprintf(fp, "%d\n", nloc);
    for (int i = 0; i < nloc; i++) fprintf(fp, "%s\n", locname[i]);
    fclose(fp);
}

static void save_roads(void)
{
    FILE *fp = fopen(ROAD_FILE, "w");
    if (!fp) return;
    for (int i = 0; i < nloc; i++)
        for (int j = 0; j < nloc; j++)
            if (i != j && adj[i][j] < INF)
                fprintf(fp, "%d %d %d\n", i, j, adj[i][j]);
    fclose(fp);
}

static void load_network(void)
{
    reset_adj();
    FILE *fp = fopen(LOC_FILE, "r");
    if (!fp) {
        /* first run: seed a small demo network */
        const char *names[] = {"Delhi", "Meerut", "Bijnor", "Haridwar",
                               "Dehradun", "Moradabad"};
        nloc = 6;
        for (int i = 0; i < nloc; i++) strcpy(locname[i], names[i]);
        int roads[][3] = {{0,1,70},{1,0,70},{1,2,90},{2,1,90},{2,3,60},{3,2,60},
                          {3,4,55},{4,3,55},{1,5,100},{5,1,100},{5,2,65},{2,5,65}};
        for (size_t k = 0; k < sizeof roads / sizeof roads[0]; k++)
            adj[roads[k][0]][roads[k][1]] = roads[k][2];
        save_locations();
        save_roads();
        return;
    }
    char line[64];
    if (!fgets(line, sizeof line, fp)) { fclose(fp); return; }
    nloc = atoi(line);
    if (nloc > MAX_LOC) nloc = MAX_LOC;
    for (int i = 0; i < nloc; i++) {
        if (!fgets(locname[i], sizeof locname[i], fp)) break;
        locname[i][strcspn(locname[i], "\r\n")] = '\0';
    }
    fclose(fp);

    fp = fopen(ROAD_FILE, "r");
    if (!fp) return;
    int u, v, d;
    while (fscanf(fp, "%d %d %d", &u, &v, &d) == 3)
        if (u >= 0 && u < nloc && v >= 0 && v < nloc) adj[u][v] = d;
    fclose(fp);
}

/* ---------------- presorting: merge sort with comparator ---------------- */
typedef int (*Cmp)(const Vehicle *, const Vehicle *);

static int cmp_id(const Vehicle *a, const Vehicle *b)      { return a->id - b->id; }
static int cmp_plate(const Vehicle *a, const Vehicle *b)   { return strcmp(a->plate, b->plate); }
static int cmp_mileage(const Vehicle *a, const Vehicle *b) { return b->mileage - a->mileage; } /* high first */

static void merge_sort(Vehicle *a, Vehicle *tmp, int lo, int hi, Cmp cmp)
{
    if (hi - lo < 1) return;
    int mid = (lo + hi) / 2;
    merge_sort(a, tmp, lo, mid, cmp);
    merge_sort(a, tmp, mid + 1, hi, cmp);
    int i = lo, j = mid + 1, k = lo;
    while (i <= mid && j <= hi)
        tmp[k++] = (cmp(&a[i], &a[j]) <= 0) ? a[i++] : a[j++];
    while (i <= mid) tmp[k++] = a[i++];
    while (j <= hi)  tmp[k++] = a[j++];
    for (k = lo; k <= hi; k++) a[k] = tmp[k];
}

static void sort_vehicles(Vehicle *a, int n, Cmp cmp)
{
    if (n < 2) return;
    Vehicle *tmp = malloc((size_t)n * sizeof(Vehicle));
    if (!tmp) return;
    merge_sort(a, tmp, 0, n - 1, cmp);
    free(tmp);
}

/* binary search on an array presorted by id */
static int binary_search_id(const Vehicle *a, int n, int id)
{
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (a[mid].id == id) return mid;
        if (a[mid].id < id) lo = mid + 1; else hi = mid - 1;
    }
    return -1;
}

/* ---------------- display helpers ---------------- */
static void print_header(void)
{
    printf("\n%-5s %-12s %-18s %-7s %-9s %-12s %-10s\n",
           "ID", "Plate", "Model", "Type", "Mileage", "Status", "Location");
    printf("---------------------------------------------------------------------------\n");
}

static void print_vehicle(const Vehicle *v)
{
    printf("%-5d %-12s %-18s %-7s %-9d %-12s %-10s\n",
           v->id, v->plate, v->model, TYPE_NAME[v->type], v->mileage,
           STATUS_NAME[v->status], locname[v->loc]);
}

static void list_locations(void)
{
    for (int i = 0; i < nloc; i++) printf("  %d. %s\n", i + 1, locname[i]);
}

static int pick_location(const char *prompt)
{
    list_locations();
    return read_range(prompt, 1, nloc) - 1;
}

static int find_index_by_id(int id)   /* uses presort + binary search */
{
    Vehicle copy[MAX_VEH];
    memcpy(copy, veh, (size_t)nveh * sizeof(Vehicle));
    sort_vehicles(copy, nveh, cmp_id);
    int pos = binary_search_id(copy, nveh, id);
    if (pos < 0) return -1;
    for (int i = 0; i < nveh; i++) if (veh[i].id == id) return i;
    return -1;
}

/* ---------------- vehicle operations ---------------- */
static int plate_exists(const char *plate)
{
    /* presort by plate, then a duplicate can only be adjacent */
    Vehicle copy[MAX_VEH + 1];
    memcpy(copy, veh, (size_t)nveh * sizeof(Vehicle));
    sort_vehicles(copy, nveh, cmp_plate);
    int lo = 0, hi = nveh - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        int c = strcmp(copy[mid].plate, plate);
        if (c == 0) return 1;
        if (c < 0) lo = mid + 1; else hi = mid - 1;
    }
    return 0;
}

static void add_vehicle(void)
{
    if (nveh >= MAX_VEH) { printf("  Fleet is full.\n"); return; }
    Vehicle v;
    memset(&v, 0, sizeof v);

    int maxid = 0;
    for (int i = 0; i < nveh; i++) if (veh[i].id > maxid) maxid = veh[i].id;
    v.id = maxid + 1;

    read_line("Plate number: ", v.plate, sizeof v.plate);
    upcase(v.plate);
    if (v.plate[0] == '\0') { printf("  Plate cannot be empty.\n"); return; }
    if (plate_exists(v.plate)) { printf("  A vehicle with plate %s already exists.\n", v.plate); return; }

    read_line("Model: ", v.model, sizeof v.model);
    v.type = read_range("Type (0 Car, 1 Truck, 2 Bus, 3 Bike): ", 0, 3);
    do { v.mileage = read_int("Mileage (km): "); } while (v.mileage < 0);
    v.status = 0;
    v.loc = pick_location("Current location number: ");

    veh[nveh++] = v;
    save_vehicles();
    printf("  Vehicle added with ID %d.\n", v.id);
}

static void list_vehicles(int mode)   /* 0 as stored, 1 by plate, 2 by mileage */
{
    if (nveh == 0) { printf("  No vehicles.\n"); return; }
    Vehicle copy[MAX_VEH];
    memcpy(copy, veh, (size_t)nveh * sizeof(Vehicle));
    if (mode == 1) sort_vehicles(copy, nveh, cmp_plate);
    if (mode == 2) sort_vehicles(copy, nveh, cmp_mileage);
    print_header();
    for (int i = 0; i < nveh; i++) print_vehicle(&copy[i]);
}

static void search_vehicle(void)
{
    int id = read_int("Vehicle ID to search: ");
    int idx = find_index_by_id(id);
    if (idx < 0) { printf("  Not found.\n"); return; }
    print_header();
    print_vehicle(&veh[idx]);
}

static void update_vehicle(void)
{
    int idx = find_index_by_id(read_int("Vehicle ID to update: "));
    if (idx < 0) { printf("  Not found.\n"); return; }
    printf("  1. Status   2. Location   3. Mileage\n");
    int c = read_range("Choice: ", 1, 3);
    if (c == 1) veh[idx].status = read_range("Status (0 Available, 1 On trip, 2 Maintenance): ", 0, 2);
    if (c == 2) veh[idx].loc = pick_location("New location number: ");
    if (c == 3) veh[idx].mileage = read_int("New mileage: ");
    save_vehicles();
    printf("  Updated.\n");
}

static void delete_vehicle(void)
{
    int idx = find_index_by_id(read_int("Vehicle ID to delete: "));
    if (idx < 0) { printf("  Not found.\n"); return; }
    for (int i = idx; i < nveh - 1; i++) veh[i] = veh[i + 1];
    nveh--;
    save_vehicles();
    printf("  Deleted.\n");
}

static void service_report(void)
{
    int limit = read_int("List vehicles with mileage at or above (km): ");
    Vehicle copy[MAX_VEH];
    memcpy(copy, veh, (size_t)nveh * sizeof(Vehicle));
    sort_vehicles(copy, nveh, cmp_mileage);        /* presort: highest first */
    int shown = 0;
    for (int i = 0; i < nveh && copy[i].mileage >= limit; i++) {  /* stop early */
        if (!shown) print_header();
        print_vehicle(&copy[i]);
        shown++;
    }
    if (!shown) printf("  No vehicle is due for service.\n");
}

/* ---------------- road network ---------------- */
static void add_location(void)
{
    if (nloc >= MAX_LOC) { printf("  Location limit reached.\n"); return; }
    read_line("Location name: ", locname[nloc], sizeof locname[nloc]);
    if (locname[nloc][0] == '\0') return;
    nloc++;
    save_locations();
    printf("  Added.\n");
}

static void add_road(void)
{
    if (nloc < 2) { printf("  Need at least two locations.\n"); return; }
    int u = pick_location("From location number: ");
    int v = pick_location("To location number: ");
    if (u == v) { printf("  Same location.\n"); return; }
    int d = 0;
    while (d <= 0) d = read_int("Distance (km): ");
    int two = read_range("Two-way road? (1 yes, 0 one-way): ", 0, 1);
    adj[u][v] = d;
    if (two) adj[v][u] = d;
    save_roads();
    printf("  Road saved.\n");
}

static void show_roads(void)
{
    int any = 0;
    for (int i = 0; i < nloc; i++)
        for (int j = 0; j < nloc; j++)
            if (i != j && adj[i][j] < INF) {
                printf("  %-10s -> %-10s %4d km\n", locname[i], locname[j], adj[i][j]);
                any = 1;
            }
    if (!any) printf("  No roads defined.\n");
}

/* ---------------- Warshall: reachability ---------------- */
static void warshall(int reach[MAX_LOC][MAX_LOC])
{
    for (int i = 0; i < nloc; i++)
        for (int j = 0; j < nloc; j++)
            reach[i][j] = (i == j) || (adj[i][j] < INF);
    for (int k = 0; k < nloc; k++)
        for (int i = 0; i < nloc; i++)
            if (reach[i][k])
                for (int j = 0; j < nloc; j++)
                    reach[i][j] = reach[i][j] || reach[k][j];
}

static void show_reachability(void)
{
    int reach[MAX_LOC][MAX_LOC];
    warshall(reach);
    printf("\nReachability matrix (1 = can travel, 0 = no route)\n\n%-11s", "");
    for (int j = 0; j < nloc; j++) printf("%-4.3s", locname[j]);
    printf("\n");
    for (int i = 0; i < nloc; i++) {
        printf("%-11.10s", locname[i]);
        for (int j = 0; j < nloc; j++) printf("%-4d", reach[i][j]);
        printf("\n");
    }
    int full = 1;
    for (int i = 0; i < nloc; i++)
        for (int j = 0; j < nloc; j++) if (!reach[i][j]) full = 0;
    printf("\n%s\n", full ? "Network is fully connected: every location can reach every other."
                          : "Some locations cannot be reached from others (see the 0 entries).");
}

/* ---------------- Floyd-Warshall: shortest paths ---------------- */
static void floyd(int dist[MAX_LOC][MAX_LOC], int next[MAX_LOC][MAX_LOC])
{
    for (int i = 0; i < nloc; i++)
        for (int j = 0; j < nloc; j++) {
            dist[i][j] = adj[i][j];
            next[i][j] = (adj[i][j] < INF) ? j : -1;
        }
    for (int k = 0; k < nloc; k++)
        for (int i = 0; i < nloc; i++)
            for (int j = 0; j < nloc; j++)
                if (dist[i][k] < INF && dist[k][j] < INF &&
                    dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                }
}

static void print_path(int next[MAX_LOC][MAX_LOC], int u, int v)
{
    printf("%s", locname[u]);
    while (u != v) {
        u = next[u][v];
        printf(" -> %s", locname[u]);
    }
}

static void show_distance_matrix(void)
{
    int dist[MAX_LOC][MAX_LOC], next[MAX_LOC][MAX_LOC];
    floyd(dist, next);
    printf("\nShortest distance matrix in km (- = unreachable)\n\n%-11s", "");
    for (int j = 0; j < nloc; j++) printf("%-6.5s", locname[j]);
    printf("\n");
    for (int i = 0; i < nloc; i++) {
        printf("%-11.10s", locname[i]);
        for (int j = 0; j < nloc; j++) {
            if (dist[i][j] >= INF) printf("%-6s", "-");
            else printf("%-6d", dist[i][j]);
        }
        printf("\n");
    }
}

static void shortest_route(void)
{
    int dist[MAX_LOC][MAX_LOC], next[MAX_LOC][MAX_LOC];
    floyd(dist, next);
    int u = pick_location("From location number: ");
    int v = pick_location("To location number: ");
    if (dist[u][v] >= INF) { printf("  No route from %s to %s.\n", locname[u], locname[v]); return; }
    printf("  Shortest distance: %d km\n  Route: ", dist[u][v]);
    print_path(next, u, v);
    printf("\n");
}

/* dispatch: nearest available vehicle using Floyd-Warshall distances */
static void dispatch_vehicle(void)
{
    int dist[MAX_LOC][MAX_LOC], next[MAX_LOC][MAX_LOC];
    floyd(dist, next);
    int dest = pick_location("Pickup / destination location number: ");
    int want = read_range("Vehicle type needed (0 Car, 1 Truck, 2 Bus, 3 Bike, 4 Any): ", 0, 4);

    int best = -1;
    for (int i = 0; i < nveh; i++) {
        if (veh[i].status != 0) continue;
        if (want != 4 && veh[i].type != want) continue;
        if (dist[veh[i].loc][dest] >= INF) continue;
        if (best < 0 || dist[veh[i].loc][dest] < dist[veh[best].loc][dest]) best = i;
    }
    if (best < 0) { printf("  No available vehicle can reach %s.\n", locname[dest]); return; }

    int d = dist[veh[best].loc][dest];
    printf("  Nearest vehicle: ID %d (%s, %s) at %s, %d km away.\n  Route: ",
           veh[best].id, veh[best].plate, veh[best].model, locname[veh[best].loc], d);
    print_path(next, veh[best].loc, dest);
    printf("\n");

    if (read_range("Assign this vehicle now? (1 yes, 0 no): ", 0, 1)) {
        veh[best].mileage += d;
        veh[best].loc = dest;
        veh[best].status = 1;
        save_vehicles();
        printf("  Vehicle %d assigned and moved to %s.\n", veh[best].id, locname[dest]);
    }
}

/* ---------------- menu ---------------- */
static void menu(void)
{
    printf("\n========== VEHICLE MANAGEMENT SYSTEM ==========\n"
           " 1. Add vehicle\n"
           " 2. List vehicles (stored order)\n"
           " 3. List vehicles sorted by plate\n"
           " 4. Search vehicle by ID (presort + binary search)\n"
           " 5. Update vehicle\n"
           " 6. Delete vehicle\n"
           " 7. Service-due report (presorted by mileage)\n"
           " 8. Add location\n"
           " 9. Add road\n"
           "10. Show roads\n"
           "11. Reachability between locations (Warshall)\n"
           "12. Shortest distance matrix (Floyd-Warshall)\n"
           "13. Shortest route between two locations\n"
           "14. Dispatch nearest available vehicle\n"
           " 0. Exit\n");
}

int main(void)
{
    load_vehicles();
    load_network();

    for (;;) {
        menu();
        int c = read_range("Choice: ", 0, 14);
        switch (c) {
            case 1:  add_vehicle(); break;
            case 2:  list_vehicles(0); break;
            case 3:  list_vehicles(1); break;
            case 4:  search_vehicle(); break;
            case 5:  update_vehicle(); break;
            case 6:  delete_vehicle(); break;
            case 7:  service_report(); break;
            case 8:  add_location(); break;
            case 9:  add_road(); break;
            case 10: show_roads(); break;
            case 11: show_reachability(); break;
            case 12: show_distance_matrix(); break;
            case 13: shortest_route(); break;
            case 14: dispatch_vehicle(); break;
            case 0:  save_vehicles(); save_locations(); save_roads();
                     printf("Data saved. Goodbye.\n"); return 0;
        }
    }
}
