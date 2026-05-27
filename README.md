<h1 align="center">Active Reconnaissance Scanner</h1>

<p align="center">
  <strong>A high-performance, multi-threaded C++ network scanner for service fingerprinting and banner grabbing.</strong>
</p>

---

## 🚀 Overview

This project is a custom-built network reconnaissance engine designed to map out active services across target IPs. Built in C++ for raw socket control, it utilizes a sophisticated, two-layer threading architecture to manage hundreds of concurrent connections without resource exhaustion. 

It includes a Python stub server (`FakeServerAllBanners.py`) that deploys randomized service banners across local ports, providing a safe, controlled environment to test active probing logic.

## 🛠️ Usage

The scanner is executed via the command line and accepts single IPs, comma-separated lists, or hyphenated IP ranges.

**Syntax:**
```bash
./scanner <IP address or range>

# Scan a single IP
./scanner 127.0.0.1

# Scan a specific IP range
./scanner 127.0.0.1-35

# Scan a range across different subnets
./scanner 192.168.0.10-192.168.0.12

# Scan a comma-separated list
./scanner "127.0.0.1, 192.168.0.1"
```


## 🧠 Architecture & Workflow

The execution follows a clean pipeline from user input to final HTML report generation.

### 1. Target Acquisition (`main.cpp`)
The program begins by parsing the command-line arguments. It utilizes utility functions to expand IP ranges (e.g., `127.0.0.1-35`) and comma-separated inputs into a flattened, validated `std::vector` of target IP strings.

### 2. Environment Setup (`Scanner.cpp`)
Before network operations begin, `Scanner::initFile` initializes `PortScanResults.html`. It writes the HTML headers and a timestamp, setting up the dashboard that will asynchronously receive data from the worker threads.

### 3. Target Dispatch (`ThreadManager.cpp`)
The `ThreadManager` takes ownership of the IP list. It spins up a pool of up to 50 concurrent threads, iterating through every combination of the provided IPs and the target port range (5000 to 6000).

### 4. Active Probing (`Scanner.cpp` & `Socket.cpp`)
For each IP/Port pair, a worker thread initiates a TCP connection.
* If the connection is accepted, the scanner first attempts to grab an immediate banner passively.
* If the target remains silent, the worker cross-references the `portProbes` map (loaded from `PortProbe.txt`) and sends a service-specific payload to coax a response.

### 5. Thread-Safe Reporting (`Scanner.cpp`)
Upon capturing a banner, the worker thread triggers `Scanner::writeToFile`. It locks a static mutex (`fileMutex`) to prevent race conditions, writes the `[IP Port Banner]` entry directly into the HTML file, and safely terminates.

---

## ⚙️ The Core Engine: Layered Threading Mechanism

Handling thousands of potential network connections requires a highly structured concurrency model. This engine is split into two distinct layers to ensure stability and speed.

### Layer 1: The Dispatcher (`ThreadManager::watchThreads`)
This acts as the orchestrator. It runs a continuous state machine loop tracking `currentIP` and `currentPort`.
* **The Availability Array:** The manager maintains a boolean array, `threadFree[50]`, tracking the status of 50 thread slots.
* **Task Assignment:** It loops through the slots. Upon finding a `true` (free) slot, it allocates a new `Scanner` object for the current IP/Port, increments the port state, marks the slot as `false` (busy), and dispatches the task via `pthread_create`.
* **Resource Reaping:** At the bottom of the loop, `pthread_join` is called non-blockingly to clean up finished threads and recycle their slots for the next batch of ports.

### Layer 2: The Worker (`Scanner::processWorker`)
This is the payload execution layer. Each dispatched thread operates in total isolation.
* **Socket RAII (`Socket.cpp`):** The thread creates a `Socket` object configured with a strict 3-second receive/send timeout (`SO_RCVTIMEO`). This defensive programming ensures dead IPs or tarpits do not hang the worker indefinitely.
* **Execution:** It runs the banner grab logic (passive listening followed by active probing if necessary).
* **Cleanup:** Once the task is complete—whether it resulted in a successful read, a timeout, or a connection refusal—the worker cleans up its heap-allocated objects and exits cleanly, allowing the Dispatcher to reclaim the slot.
Cleanup: Once the task is complete—whether it resulted in a successful read, a timeout, or a connection refusal—the worker cleans up its heap-allocated objects and exits cleanly, allowing the Dispatcher to reclaim the slot.
