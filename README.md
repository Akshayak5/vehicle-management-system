# Vehicle Management System

A fleet management system that keeps vehicle records, models a road network between locations, and dispatches the nearest available vehicle to a pickup point. It is built around three classic algorithms: **presorting**, **Warshall's reachability** and **Floyd-Warshall shortest paths**.

The project comes in two forms that share the same logic:

| Version | File | Description |
|---|---|---|
| Command line | `vehicle_mgmt.c` | Menu-driven C program with persistent file storage |
| Web app | `index.html` | Single-page browser app, no installation or server required |

## Table of contents

- [Features](#features)
- [Algorithms](#algorithms)
- [Getting started](#getting-started)
- [Usage](#usage)
- [Data files](#data-files)
- [Project structure](#project-structure)
- [Configuration](#configuration)
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
- Find the nearest available vehicle for a pickup location, optionally filtered by vehicle type
- Assign the vehicle, which updates its location, mileage and status

## Algorithms

| Algorithm | Purpose in this project | Time complexity |
|---|---|---|
| **Presorting** (merge sort) | Sorted vehicle lists, the service-due report, ranking dispatch candidates by distance | O(n log n) |
| **Binary search** | Find a vehicle by ID and detect duplicate plates, on presorted data | O(log n) |
| **Warshall's algorithm** | Transitive closure of the road graph: can a vehicle travel from A to B at all? | O(V³) |
| **Floyd-Warshall algorithm** | All-pairs shortest distances, with path reconstruction | O(V³) |

Here `n` is the number of vehicles and `V` is the number of locations.

### How presorting is used

Sorting the data once makes later operations cheaper. After a merge sort by ID, a vehicle can be found with binary search instead of scanning every record. After a sort by plate, duplicates can be found with binary search too. After a sort by mileage, the service-due report stops reading as soon as it reaches a vehicle below the limit.

### How the graph algorithms are used

Locations are nodes and roads are weighted directed edges (distance in km). A two-way road is stored as two directed edges, so one-way roads are supported.

- **Warshall** answers reachability. If a location has no route to the rest of the network, the program reports it, and the dispatcher skips vehicles that cannot reach the pickup point.
- **Floyd-Warshall** computes the shortest distance between every pair of locations and keeps a `next` matrix so the actual route can be rebuilt. The dispatcher uses these distances to pick the nearest available vehicle.

## Getting started

### Prerequisites

- **C program:** a C compiler such as GCC or clang
- **Web app:** any modern browser

### Clone the repository

```bash
git clone https://github.com/<your-username>/vehicle-management-system.git
cd vehicle-management-system
```

### Run the C program

```bash
gcc -Wall -o vehicle_mgmt vehicle_mgmt.c
./vehicle_mgmt
```

On Windows, run `vehicle_mgmt.exe` instead of `./vehicle_mgmt`.

### Run the web app

Open `index.html` in a browser:

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

The app has four tabs:

| Tab | What it does |
|---|---|
| **Fleet** | Vehicle table with sorting, find by ID, inline status and location changes, and an add form |
| **Road network** | Locations, roads, the reachability matrix and the shortest distance matrix |
| **Shortest route** | Pick two locations to see the total distance and the path |
| **Dispatch** | Pick a pickup location and vehicle type, see candidates ranked by distance, and assign one |

The app starts with a sample fleet of six vehicles. Changes are saved in the browser's local storage, so they stay on the same browser and device. **Reset to sample data** at the bottom of the page restores the starting data.

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
├── vehicle_mgmt.c    C program
├── index.html        Web app
├── README.md         Project documentation
├── LICENSE           MIT license
└── .gitignore        Ignores the compiled binary and generated data files
```

## Configuration

The C program holds up to 200 vehicles and 20 locations. To change this, edit the constants at the top of `vehicle_mgmt.c` and recompile:

```c
#define MAX_VEH   200
#define MAX_LOC   20
```

## Contributing

Contributions are welcome. To suggest a change, fork the repository, create a branch, and open a pull request describing what you changed and why.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Author

AKSHAYA K
