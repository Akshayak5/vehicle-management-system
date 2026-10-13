# Vehicle Management System

A fleet management system that keeps vehicle records, models a road network between locations, and dispatches the nearest available vehicle to a pickup point. It is built around three classic algorithms: **presorting**, **Warshall's reachability** and **Floyd-Warshall shortest paths**.

**Live demo:** https://akshayak5.github.io/vehicle-management-system/

The project comes in two forms that share the same logic:

| Version | File | Description |
|---|---|---|
| Command line | `vehicle_mgmt.c` | Menu-driven C program with persistent file storage |
| Web app | `index.html` | Single-page browser app with login, an admin side and a customer side. No installation or server required. [Try it online](https://akshayak5.github.io/vehicle-management-system/) |

## Table of contents

- [Features](#features)
- [Algorithms](#algorithms)
- [Getting started](#getting-started)
- [Usage](#usage)
- [Accounts and roles](#accounts-and-roles)
- [Data files](#data-files)
- [Project structure](#project-structure)
- [Configuration](#configuration)
- [Limitations](#limitations)
- [Contributing](#contributing)
- [License](#license)

## Features

**Fleet management**
- Add, list, update and delete vehicles
- Sort the fleet by plate number, mileage or ID
- Reject duplicate plate numbers
- Service-due report for vehicles above a mileage limit

**Road network**
- Add locations and roads, one-way or two-way
- Reachability matrix showing which locations can reach which
- Shortest distance matrix for every pair of locations
- Shortest route between any two locations, with the path

**Dispatch**
- Find the nearest available vehicle for a pickup location, with an optional drop-off destination
- See the full route and total trip distance for each candidate
- Assign the vehicle, which updates its location, mileage and status

**Accounts and bookings (web app)**
- Register and log in, with separate admin and customer sides
- Customers book rides with a route, distance, fare and travel time estimate
- Admins approve or reject requests, and the nearest suitable vehicle is assigned automatically
- Admins manage users and promote customers to admins

## Algorithms

| Algorithm | Purpose in this project | Time complexity |
|---|---|---|
| **Presorting** (merge sort) | Sorted vehicle lists, the service-due report, ranking dispatch candidates by distance, ordering bookings | O(n log n) |
| **Binary search** | Find a vehicle by ID and detect duplicate plates, on presorted data | O(log n) |
| **Warshall's algorithm** | Transitive closure of the road graph: can a vehicle travel from A to B at all? | O(V³) |
| **Floyd-Warshall algorithm** | All-pairs shortest distances, with path reconstruction | O(V³) |

Here `n` is the number of vehicles and `V` is the number of locations.

### How presorting is used

Sorting the data once makes later operations cheaper. After a merge sort by ID, a vehicle can be found with binary search instead of scanning every record. After a sort by plate, duplicates can be found with binary search too. After a sort by mileage, the service-due report stops reading as soon as it reaches a vehicle below the limit.

### How the graph algorithms are used

Locations are nodes and roads are weighted directed edges (distance in km). A two-way road is stored as two directed edges, so one-way roads are supported.

- **Warshall** answers reachability. If a location has no route to the rest of the network, the program reports it, and the dispatcher skips vehicles that cannot reach the pickup point.
- **Floyd-Warshall** computes the shortest distance between every pair of locations and keeps a `next` matrix so the actual route can be rebuilt. The dispatcher and the booking approval use these distances to pick the nearest available vehicle, and customers see the same distances in their trip estimates.

## Getting started

### Prerequisites

- **C program:** a C compiler such as GCC or clang
- **Web app:** any modern browser

### Clone the repository

```bash
git clone https://github.com/Akshayak5/vehicle-management-system.git
cd vehicle-management-system
```

### Run the C program

```bash
gcc -Wall -o vehicle_mgmt vehicle_mgmt.c
./vehicle_mgmt
```

On Windows, run `vehicle_mgmt.exe` instead of `./vehicle_mgmt`.

### Run the web app

**Online:** open the live demo at https://akshayak5.github.io/vehicle-management-system/. Nothing to install.

**Locally:** open `index.html` in a browser:

```bash
open index.html        # macOS
xdg-open index.html    # Linux
start index.html       # Windows
```

You can also use the **Live Server** extension in VS Code: right-click `index.html` and choose **Open with Live Server**.

## Usage

### C program

The program shows a numbered menu. Type a number and press Enter.

```
========== VEHICLE MANAGEMENT SYSTEM ==========
 1. Add vehicle
 2. List vehicles (stored order)
 3. List vehicles sorted by plate
 4. Search vehicle by ID (presort + binary search)
 5. Update vehicle
 6. Delete vehicle
 7. Service-due report (presorted by mileage)
 8. Add location
 9. Add road
10. Show roads
11. Reachability between locations (Warshall)
12. Shortest distance matrix (Floyd-Warshall)
13. Shortest route between two locations
14. Dispatch nearest available vehicle
 0. Exit
```

A sample network of six cities is created on the first run. Option 12 prints:

```
Shortest distance matrix in km (- = unreachable)

           Delhi Meeru Bijno Harid Dehra Morad
Delhi      0     70    160   220   275   170
Meerut     70    0     90    150   205   100
Bijnor     160   90    0     60    115   65
Haridwar   220   150   60    0     55    125
Dehradun   275   205   115   55    0     180
Moradabad  170   100   65    125   180   0
```

Option 13 from Delhi to Dehradun prints:

```
Shortest distance: 275 km
Route: Delhi -> Meerut -> Bijnor -> Haridwar -> Dehradun
```

### Web app

Log in or register first. The tabs you see depend on your role.

**Admin tabs**

| Tab | What it does |
|---|---|
| **Fleet** | Vehicle table with sorting, find by ID, inline status and location changes, and an add form |
| **Road network** | Locations, roads, the reachability matrix and the shortest distance matrix |
| **Shortest route** | Pick two locations to see the total distance and the path |
| **Dispatch** | Pick a pickup, an optional drop-off and a vehicle type, see candidates ranked by distance with the full route, and assign one |
| **Bookings** | Customer requests. Approve (assigns the nearest available vehicle of the requested type), reject, or mark completed |
| **Users** | List of accounts. Promote a customer to admin, or remove an account |

**Customer tabs**

| Tab | What it does |
|---|---|
| **Book a ride** | Pick a pickup, a drop-off and a vehicle type to see the route, distance, estimated fare and travel time, then request the booking |
| **My bookings** | Status of each request (pending, approved, completed, rejected or cancelled) and the assigned vehicle. Pending requests can be cancelled |
| **Route planner** | Shortest route between any two locations |

A booking goes through these states: a customer requests it (**Pending**), an admin approves it and a vehicle is assigned (**Approved**), and the admin marks it **Completed** when the trip ends, which makes the vehicle available again.

Fares are an illustrative estimate: a base fare plus a rate per km that depends on the vehicle type (for example ₹60 + ₹14/km for a car). Travel time assumes an average speed of 45 km/h. Both can be changed in the `RATES` and `AVG_SPEED` constants at the top of the script in `index.html`.

The app starts with a sample fleet of six vehicles. Admins can restore the sample data with **Reset fleet and bookings to sample data** at the bottom of the page.

## Accounts and roles

| Role | Can do |
|---|---|
| **Customer** | Book rides, view and cancel their own pending bookings, use the route planner |
| **Admin** | Everything customers can see on the admin tabs: manage the fleet and road network, dispatch vehicles, handle bookings, manage users |

New sign-ups are always customers. An admin can promote a customer to admin from the **Users** tab. Admins cannot change their own role or delete their own account.

### Demo accounts

| Role | Username | Password |
|---|---|---|
| Admin | `admin` | `admin123` |
| Customer | `demo` | `demo123` |

The login screen has buttons that fill these in. Change or delete them if you deploy the app for real use.

## Data files

The C program creates these files in the folder it runs from.

| File | Format | Contents |
|---|---|---|
| `vehicles.dat` | Binary | Vehicle records |
| `locations.txt` | Text | Number of locations, then one name per line |
| `roads.txt` | Text | One directed road per line: `from to distance` |

Delete the three files to start fresh. They are listed in `.gitignore`, so they are not committed.

### Vehicle record

| Field | Description |
|---|---|
| `id` | Unique number, assigned automatically |
| `plate` | Plate number, stored in uppercase |
| `model` | Vehicle model |
| `type` | 0 car, 1 truck, 2 bus, 3 bike |
| `mileage` | Distance driven in km |
| `status` | 0 available, 1 on trip, 2 in maintenance |
| `loc` | Index of the current location |

## Project structure

```
vehicle-management-system/
├── vehicle_mgmt.c       C program
├── index.html           Web app (HTML, CSS and JavaScript in one file)
├── README.md            Project documentation
├── LICENSE              MIT license
├── .gitignore           Ignores the compiled binary and generated data files
└── .github/workflows/   Build check that compiles the C program on every push
```

## Configuration

The C program holds up to 200 vehicles and 20 locations. To change this, edit the constants at the top of `vehicle_mgmt.c` and recompile:

```c
#define MAX_VEH   200
#define MAX_LOC   20
```

The web app holds up to 20 locations (`MAX_LOC` in `index.html`).

## Limitations

- **The web app has no server.** Accounts, vehicles and bookings are saved in the browser's local storage, so they stay on one browser and one device. A customer who registers on one device does not exist on another, and an admin will not see bookings made on someone else's device.
- **Login is not real security.** Passwords are hashed with a salt (PBKDF2 where the browser supports it), but all checks run in the browser, so a determined user can read or change the stored data. Use a backend with proper authentication for anything beyond a demo or a course project.
- The C program and the web app keep separate data. Changes in one do not appear in the other.

## Contributing

Contributions are welcome. To suggest a change, fork the repository, create a branch, and open a pull request describing what you changed and why.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Author

AKSHAYA K