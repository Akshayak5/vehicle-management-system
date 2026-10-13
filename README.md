# Vehicle Management System

A fleet management system that keeps vehicle records, models a road network between locations, and dispatches the nearest available vehicle to a pickup point. It is built around three classic algorithms: **presorting**, **Warshall's reachability** and **Floyd-Warshall shortest paths**.

**Live demo:** https://akshayak5.github.io/vehicle-management-system/

The project comes in two forms that share the same logic:

| Version | File | Description |
|---|---|---|
| Command line | `vehicle_mgmt.c` | Menu-driven C program with persistent file storage |
| Web app | `index.html` | Single-page browser app with login, an admin side and a customer side, including vehicle model selection, passenger capacity and billing. No installation or server required. [Try it online](https://akshayak5.github.io/vehicle-management-system/) |

## Table of contents

- [Features](#features)
- [Algorithms](#algorithms)
- [Getting started](#getting-started)
- [Usage](#usage)
- [Accounts and roles](#accounts-and-roles)
- [Billing](#billing)
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
- Passenger capacity (seats) for each vehicle, editable in the web app's Fleet tab

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
- Customers choose a vehicle model and enter the number of passengers
- A table of every model shows its passenger capacity, rate per km and how many are available now, with models too small for the group greyed out
- Customers see the route, distance, fare breakdown and travel time estimate before they book
- Admins approve or reject requests, and the nearest available vehicle of the requested model that seats the passengers is assigned automatically
- Admins manage users and promote customers to admins

**Billing (web app)**
- An invoice is created automatically when an admin marks a trip completed
- Each invoice shows the base fare, distance charge, subtotal, GST and total
- Customers view their bills and pay from their own Billing tab
- Admins see all invoices with billed, collected and outstanding totals, and can mark an invoice paid

## Algorithms

| Algorithm | Purpose in this project | Time complexity |
|---|---|---|
| **Presorting** (merge sort) | Sorted vehicle lists, the service-due report, ranking dispatch candidates by distance, ordering bookings and invoices | O(n log n) |
| **Binary search** | Find a vehicle by ID and detect duplicate plates, on presorted data | O(log n) |
| **Warshall's algorithm** | Transitive closure of the road graph: can a vehicle travel from A to B at all? | O(V³) |
| **Floyd-Warshall algorithm** | All-pairs shortest distances, with path reconstruction | O(V³) |

Here `n` is the number of vehicles and `V` is the number of locations.

### How presorting is used

Sorting the data once makes later operations cheaper. After a merge sort by ID, a vehicle can be found with binary search instead of scanning every record. After a sort by plate, duplicates can be found with binary search too. After a sort by mileage, the service-due report stops reading as soon as it reaches a vehicle below the limit.

### How the graph algorithms are used

Locations are nodes and roads are weighted directed edges (distance in km). A two-way road is stored as two directed edges, so one-way roads are supported.

- **Warshall** answers reachability. If a location has no route to the rest of the network, the program reports it, and the dispatcher skips vehicles that cannot reach the pickup point.
- **Floyd-Warshall** computes the shortest distance between every pair of locations and keeps a `next` matrix so the actual route can be rebuilt. The dispatcher and the booking approval use these distances to pick the nearest available vehicle, and customers see the same distances in their trip estimates and bills.

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

The C program does not track passenger capacity, bookings or billing. Those are web app features.

### Web app

Log in or register first. The tabs you see depend on your role.

**Admin tabs**

| Tab | What it does |
|---|---|
| **Fleet** | Vehicle table with sorting, find by ID, inline status, location and passenger capacity changes, and an add form |
| **Road network** | Locations, roads, the reachability matrix and the shortest distance matrix |
| **Shortest route** | Pick two locations to see the total distance and the path |
| **Dispatch** | Pick a pickup, an optional drop-off and a vehicle type, see candidates ranked by distance with the full route and seat count, and assign one |
| **Bookings** | Customer requests with the model and passenger count asked for. Approve (assigns the nearest available vehicle of the requested model that seats the passengers), reject, or mark completed, which creates the invoice |
| **Billing** | All invoices with billed, collected and outstanding totals, a full bill view, and a Mark paid button for cash payments |
| **Users** | List of accounts. Promote a customer to admin, or remove an account |

**Customer tabs**

| Tab | What it does |
|---|---|
| **Book a ride** | Pick a pickup and a drop-off, enter the number of passengers and choose a vehicle model. See the route, distance, fare breakdown and travel time, then request the booking |
| **My bookings** | Status of each request (pending, approved, completed, rejected or cancelled), the model and passenger count asked for, and the assigned vehicle. Pending requests can be cancelled, and completed trips link to their bill |
| **Billing** | Amount to pay, amount paid, and every invoice with a full bill view and a Pay button |
| **Route planner** | Shortest route between any two locations |

**Choosing a model and passengers**

- **Passenger count:** enter 1 to 60 passengers. The model list updates as you type.
- **Passenger capacity:** each model shows how many passengers it seats, not counting the driver. If vehicles of one model have different seat counts, the range is shown, such as 4 to 7.
- **Too-small models:** models that cannot seat the group stay visible but are greyed out and cannot be selected.
- **Capacity table:** the Vehicles and passenger capacity card lists every model with its type, capacity, rate per km and how many are available now. Choose a row to select that model.

A booking goes through these states: a customer requests it (**Pending**), an admin approves it and a vehicle is assigned (**Approved**), and the admin marks it **Completed** when the trip ends, which makes the vehicle available again and creates the invoice.

Fares are an illustrative estimate: a base fare plus a rate per km that depends on the vehicle type (for example ₹60 + ₹14/km for a car), plus 5% GST. Travel time assumes an average speed of 45 km/h. These can be changed in the `RATES`, `TAX` and `AVG_SPEED` constants at the top of the script in `index.html`.

The app starts with a sample fleet of six vehicles. Admins can restore the sample data with **Reset fleet, bookings and invoices to sample data** at the bottom of the page.

## Accounts and roles

| Role | Can do |
|---|---|
| **Customer** | Book rides by model and passenger count, view and cancel their own pending bookings, view and pay their own invoices, use the route planner |
| **Admin** | Everything customers can see on the admin tabs: manage the fleet and road network, dispatch vehicles, handle bookings, view all invoices and mark them paid, manage users |

New sign-ups are always customers. An admin can promote a customer to admin from the **Users** tab. Admins cannot change their own role or delete their own account.

### Demo accounts

| Role | Username | Password |
|---|---|---|
| Admin | `admin` | `admin123` |
| Customer | `demo` | `demo123` |

The login screen has buttons that fill these in. Change or delete them if you deploy the app for real use.

## Billing

- **Invoice creation:** an invoice is created when an admin marks an approved booking completed. Its number looks like `INV-0001`.
- **Bill lines:** base fare, distance charge (km times the rate per km), subtotal, GST at 5%, and the total.
- **Bill details:** each bill also shows the customer, date, trip, vehicle, passenger count and distance.
- **Invoice status:** an invoice is **Unpaid** until it is paid, then **Paid** with the payment method and date.
- **Customer payment:** customers pay from their Billing tab using UPI, Card or Cash. This is a demo and no real payment is taken.
- **Admin payment:** admins can mark an unpaid invoice as paid by cash when money is received.
- **Totals:** customers see how much they owe and have paid. Admins see billed, collected and outstanding amounts across all customers.
- **Ordering:** unpaid invoices are listed first, then the newest.

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

The web app's vehicle record has one more field, `seats`, the passenger capacity from 1 to 60 not counting the driver. New vehicles start with a default for their type (car 4, truck 2, bus 30, bike 1), which can be changed.

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

The web app holds up to 20 locations (`MAX_LOC` in `index.html`). Other constants at the top of its script:

- **`RATES`:** base fare and rate per km for each vehicle type
- **`TAX`:** the GST rate added to invoices
- **`AVG_SPEED`:** the speed used for travel time estimates
- **`DEFAULT_SEATS`:** the starting passenger capacity for each vehicle type
- **`PAY_METHODS`:** the payment methods customers can choose

## Limitations

- **The web app has no server.** Accounts, vehicles, bookings and invoices are saved in the browser's local storage, so they stay on one browser and one device. A customer who registers on one device does not exist on another, and an admin will not see bookings made on someone else's device.
- **Login is not real security.** Passwords are hashed with a salt (PBKDF2 where the browser supports it), but all checks run in the browser, so a determined user can read or change the stored data. Use a backend with proper authentication for anything beyond a demo or a course project.
- **Payments are a demo.** Marking an invoice paid only changes its status. No card, UPI or cash transaction takes place, and the fares and GST are illustrative.
- The C program and the web app keep separate data. Changes in one do not appear in the other, and the C program has no passenger capacity, bookings or billing.

## Contributing

Contributions are welcome. To suggest a change, fork the repository, create a branch, and open a pull request describing what you changed and why.

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Author

AKSHAYA K
