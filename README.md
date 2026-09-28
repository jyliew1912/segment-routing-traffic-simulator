# 🌐 Discrete-Event Network Simulator with OSPF & Segment Routing (SR)

[![C++17](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg?logo=c%2B%2B)](https://en.cppreference.com/w/cpp/17)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](#)

A deterministic, high-performance discrete-event network simulator implemented in **C++17**. The project evaluates distributed **Open Shortest Path First (OSPF)** link-state discovery alongside **Segment Routing (SR)** traffic engineering, demonstrating how dynamic label-stack detours resolve link saturation and maximize flow throughput under constrained capacities.

## 👨‍💻 Engineering Ownership & Attribution

- **Algorithmic Engine & Optimization (My Core Contribution):**
  - **Dynamic Segment Routing (SR) Algorithm:** Designed and implemented path traversal and dynamic label-stack generation (`buildSegmentRoutingLabels()`, `traverseNetwork()`) to steer flows around congested bottlenecks.
  - **Distributed Link-State Discovery:** Implemented distributed neighbor state synchronization via `TRA_ctrl_packet` exchange and dynamic link capacity tracking.
  - **Admission Control Engine:** Authored residual capacity path evaluation logic (`canTransmitFlow()`) to dynamically balance network load under discrete-event constraints.

- **Harness & Simulation Architecture (Framework Scaffolding):**
  - The underlying event queue scheduler (`event`), generator factory patterns (`packet_generator`, `node_generator`), and physical link base models were adapted from an academic discrete-event network simulation harness.


## 💡 System Motivation & Problem Statement

### 1. The Bottleneck of Conventional OSPF
In modern data center topologies, switches typically determine forwarding paths using distributed shortest-path algorithms (e.g., Dijkstra-based OSPF). While effective for static workloads, shortest-path routing exhibits severe traffic engineering limitations:
- **Path Rigidity:** Every flow between a given source and destination is forced onto the identical shortest route.
- **Link Over-Subscription & Congestion:** If multiple concurrent flow requests converge on the same intermediate hops, those links quickly hit their maximum bandwidth capacity ($C$).
- **Low Throughput:** Subsequent flows contending for the saturated shortest path are immediately dropped, even if parallel redundant links in the network remain idle.

```text
Without Segment Routing (OSPF Only):
Flow 1: 1 -> 0 (Size: 3) ===> Accepted on Shortest Path (Link Capacity: 5)
Flow 2: 2 -> 0 (Size: 4) ===> DROPPED (Shortest Path saturated: 3 + 4 > 5)
Flow 3: 3 -> 0 (Size: 5) ===> DROPPED (Shortest Path saturated: 3 + 5 > 5)
Total Admitted Flow Rate = 3
```
(Reference: In a 15-node topology where all links have capacity 5, OSPF alone satisfies only 1 flow, resulting in an aggregate rate of 3).


## 🛠️ The Segment Routing (SR) Solution

Segment Routing eliminates the need for complex end-to-end reservation protocols (such as RSVP-TE) by encoding source-routed paths directly into packet headers as a stack of segment labels.

### How This Simulator Implements SR Traffic Engineering:

1. **Distributed Link-State Discovery:** Every switch periodically broadcasts its local Neighbor Information (NI)—including link capacity and allocated bandwidth—via `TRA_ctrl_packet` instances. Switches consume, register, and flood these packets to reconstruct the global topology graph autonomously.


2. **Shortest-Path Baseline Routing:** Each switch calculates all-pairs shortest paths to populate its primary forwarding table.


3. **Dynamic Label Stack Detouring:** When an incoming flow request (`TRA_data_packet`) exceeds the available residual capacity of the primary shortest path, the ingress switch explores alternative paths via intermediate segment nodes.


4. **Label Pushing & Popping:**
    * The ingress switch pushes intermediate waypoint IDs onto the packet's label stack (`TRA_data_header::push_label()`).
    * Packets are forwarded hop-by-hop toward the current top-of-stack label using standard shortest paths.
    * Upon arriving at an intermediate waypoint, the node pops its own label (`pop_label()`) and forwards the packet toward the subsequent segment or destination.





```text
With Segment Routing (OSPF + SR Enabled):
Flow 1: 1 -> 0 (Size: 3) ===> Routed along direct Shortest Path [1 -> 2 -> 10 -> 0]
Flow 2: 2 -> 0 (Size: 4) ===> Detoured via Segment Node 11     [2 -> 11 -> 12 -> 0]
Flow 3: 3 -> 0 (Size: 5) ===> Detoured via Segment Node 6      [3 -> 6 -> 5 -> 0]
Total Admitted Flow Rate = 12 (300% throughput increase)
```
(By detouring flows through underutilized segments, all 3 flows are admitted simultaneously without violating link capacity constraints.)


## 🏛️ Simulator Architecture & Design Patterns

The engine is structured around strict Object-Oriented Programming (OOP) paradigms and deterministic event scheduling:

```text
+-------------------------------------------------------------------+
|               Discrete-Event Priority Queue (event::)             |
|       Orders: send_event, recv_event, TRA_pkt_gen_event           |
+---------------------------------+---------------------------------+
                                  | Dispatches at timestamp t
                                  v
+-------------------------------------------------------------------+
|                           Node Layer                              |
|   +-----------------------------------------------------------+   |
|   |                  class TRA_switch : public node           |   |
|   |  - Flooding engine & link-state database (networkTopology)|   |
|   |  - Dijkstra / all-pairs shortest path calculation         |   |
|   |  - Online admission control (canTransmitFlow())           |   |
|   |  - Segment label generation & stack management            |   |
|   +-----------------------------------------------------------+   |
+---------------------------------+---------------------------------+
                                  | Forwards packets via
                                  v
+-------------------------------------------------------------------+
|                           Link Layer                              |
|       class simple_link : public link (Latency, Capacity, Occ)    |
+-------------------------------------------------------------------+
```

### Key Software Architecture Patterns:

* **Factory Method Pattern:** Base classes (`header`, `payload`, `packet`, `node`, `link`, `event`) implement internal generator registries, ensuring clean extension of new protocol headers or link types without modifying core simulation loops.
* **Event-Driven Execution:** All packet generation, wire transmission latency (`ONE_HOP_DELAY = 10`), and node receptions are scheduled through a global priority queue.
* **Tie-Breaking Determinism:** Event priority is resolved using a deterministic hash comparator (`mycomp`) to guarantee reproducibility across platforms.



## 📂 Repository Layout

```text
segment-routing-traffic-simulator/
├── .github/
│   └── workflows/
│       └── ci.yml             # Automated compilation & test pipeline
├── sample_inputs/
│   └── sample_15node.txt      # 15-node benchmark topology from specifications
├── src/
│   └── main.cpp               # Core simulator engine, network models, and main()
├── CMakeLists.txt             # Modern CMake configuration
├── Makefile                   # GNU Make build script
└── README.md                  # System documentation & technical specifications
```


## 🚀 Quickstart

### Prerequisites

* C++17 compatible compiler (`g++ >= 9.0` or `clang++ >= 10.0`)
* GNU Make or CMake ($\ge 3.14$)

### 1. Build the Binary

Using `make`:

```bash
make
```

Or using `g++` directly:

```bash
g++ -std=c++17 -O3 -Wall src/main.cpp -o network_sim
```

### 2. Execute with [Sample Topology](sample_inputs/sample_15node.txt)

**On Linux / macOS:**

```bash
./network_sim < sample_inputs/sample_15node.txt
```

**On Windows (PowerShell):**

```powershell
Get-Content sample_inputs/sample_15node.txt | .\network_sim.exe
```


## 📊 I/O Stream Specifications

### 1. Input Format (`stdin`)

Input consists of four structural sections:

```text
#Switches  #Links  #FlowPairs  #MaxLabels  BroadcastPeriod  SimulateTime
LinkID  Node1  Node2  Capacity
... [Repeated for #Links]
FlowID  Src  Dst  FlowSize  ArriveTime
... [Repeated for #FlowPairs]
```

#### Field Explanations:

| Parameter | Description |
| --- | --- |
| `#Switches` | Total number of switch nodes in the network (indexed $0$ to $N-1$).|
| `#Links` | Number of bidirectional physical links connecting the switches.|
| `#FlowPairs` | Number of online traffic flow requests to simulate.|
| `#MaxLabels` | Maximum number of segment labels allowed in a packet's header stack.|
| `BroadcastPeriod` | Interval ($t$) at which switches flood their Neighbor Information (`TRA_ctrl_packet`).|
| `SimulateTime` | Maximum duration of the discrete-event clock.|
| `LinkID Node1 Node2 Capacity` | Edge descriptor specifying endpoints and link bandwidth.|
| `FlowID Src Dst FlowSize ArriveTime` | Traffic demand specifying source, target destination, demand bandwidth, and arrival timestamp.|

#### Sample Input:

```text
15 28 3 2 50 500
0 0 5 5
1 0 10 5
...
0 1 0 3 100
1 2 0 4 200
2 3 0 5 300

```
(Defines a 15-node network with 28 links of capacity 5, requesting 3 sequential flows destined for node 0 at times 100, 200, and 300.)

---

### 2. Output Format (`stdout`)

The simulator generates two distinct types of output:

#### A. Event Execution Trace (Logged Automatically During Simulation)

As the discrete-event scheduler advances, state changes are printed in chronological order:

```text
time           0   senID           0   pktID           0   srcID           0   dstID  4294967295   preID           0   nexID           0   TRA_ctrl_packet from 0
time          10   recID           5   pktID           0   srcID           0   dstID  4294967295   preID           0   nexID           5   TRA_ctrl_packet from 0
...
time         100   recID           1   pktID         350   srcID           1   dstID           0   preID           1   nexID           2   TRA_data_packet label 0
```

* `time`: Current discrete simulator tick.
* `senID` / `recID`: Node transmitting or receiving the packet.
* `pktID`: Unique monotonic packet identifier.
* `srcID` / `dstID`: Originating node and final destination (`4294967295` indicates broadcast).
* `preID` / `nexID`: Immediate previous hop and scheduled next hop.
* `label`: Current top-of-stack segment routing waypoint guiding the packet.


#### B. Final Switch Routing Tables (Printed Upon Completion)

After event simulation completes, each switch outputs its computed next-hop forwarding table:

```text
0             <-- Switch 0
5 5           <-- (Destination 5 -> Forward to Next Hop 5)
10 10         <-- (Destination 10 -> Forward to Next Hop 10)
...
1             <-- Switch 1
0 2           <-- (Destination 0 -> Forward to Next Hop 2)
```

