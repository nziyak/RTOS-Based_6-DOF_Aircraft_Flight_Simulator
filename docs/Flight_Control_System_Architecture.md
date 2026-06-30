# Flight Control System Architecture
## Multithreaded 6-DOF Aircraft Simulation

**Nuri Ziya Kırtepe**
Gebze Technical University

### Abstract
This document outlines the software architecture of a 6-Degrees-of-Freedom (6-DOF) aircraft flight simulator. Built from scratch in C++, the project features a custom physics engine and implements Real-Time Operating System (RTOS) concepts, specifically multithreading and resource synchronization, to emulate the behavior of a modern Flight Control Computer (FCC). Furthermore, it integrates a real-time UDP telemetry stream, a Python-based WebSocket server, and a 3D web visualizer to monitor flight dynamics interactively.

---

### 1. Introduction
Transitioning from a 1-DOF sounding rocket model, this project introduces a fully multithreaded 6-DOF flight simulator. The architecture is designed to handle complex aerodynamic forces, full rotational dynamics using quaternions, and concurrent task scheduling critical for embedded systems in aerospace engineering. The system also supports real-time manual pilot control and 3D visualization.

### 2. Object-Oriented System Design
The core physics calculations are handled using custom, dependency-free mathematical structures:
*   **Vector3:** Manages 3D spatial data (position, velocity, angular velocity, etc.) and provides vector arithmetic.
*   **Quaternion:** Handles rotational dynamics to avoid gimbal lock, heavily relying on `Vector3` for axis-angle and rotational operations (a **Dependency** relationship).
*   **Aircraft:** The central entity that aggregates physical properties. It possesses a **Composition** relationship with `Vector3` and `Quaternion`, holding them as internal states (position, orientation, inertia, etc.).
*   *Future Expansion:* The architecture is designed such that `Aircraft` can be abstracted into a broader `RigidBody` or `FlyingObject` base class, allowing seamless integration of spacecraft or drones via inheritance.

### 3. Flight State Machine
The aircraft’s operational modes are managed by a robust State Machine. This prevents invalid system operations, such as attempting a takeoff sequence before initialization is complete.

*   **INIT_BOOT:** System startup, memory allocation, and initial sensor/variable calibration.
*   **IDLE / STANDBY:** Systems are online, telemetries are active, but the engine throttle is disabled.
*   **TAKEOFF:** Maximum thrust applied. The system monitors velocity to execute pitch-up commands at the defined rotation speed (VR).
*   **CRUISE:** Airborne state where Newton’s laws of motion are actively balancing thrust against drag, and lift against weight.
*   **MANUAL_FLIGHT:** Direct user-driven thrust and pitch maneuvers via control surfaces (aileron, elevator, rudder).
*   **FAIL_SAFE:** An emergency mode triggered by thread failures or anomalous data, ensuring the system defaults to a stable, predictable state.

### 4. RTOS and Task Scheduling Architecture
To prevent blocking operations (such as File I/O and networking) from disrupting the physics calculations, the main execution is decoupled into three distinct threads using `std::thread`. This mimics Rate Monotonic Scheduling (RMS) principles.

*   **Physics & Dynamics Thread (High Priority, ~1000Hz):** Strictly responsible for numerical integration (Euler method) and aerodynamic calculations. It calculates lift, drag, thrust, and updates the aircraft’s position and orientation. It contains no blocking delays or I/O operations.
*   **Flight Control & State Thread (Medium Priority, ~100Hz):** Responsible for reading control inputs, updating the Flight State, and calculating the desired control surface deflections and engine thrust.
*   **Telemetry & Logging Thread (Low Priority, ~10Hz):** Handles File I/O and Network operations. It safely extracts the current state of the aircraft, transmits it via UDP, and logs it to a `.csv` file for post-flight analysis.

### 5. Data Synchronization
Given the concurrent nature of the architecture, shared resources (e.g., Position, Velocity, Orientation) are subject to race conditions.

*   **Mutexes (`std::mutex`):** Applied during the read/write operations of the physical state. When the Physics Thread updates the position, it locks the mutex. The Telemetry Thread must acquire this lock before reading, ensuring data integrity.
*   **Atomics (`std::atomic`):** Used for single-value state flags (like `FlightState` or system `isRunning` booleans) to allow lock-free, thread-safe state checking across all loops.

### 6. Real-Time Telemetry and Visualization
To verify flight performance and control responsiveness, the simulation features a full-stack telemetry pipeline:
1.  **C++ UDP Client:** The Telemetry Thread broadcasts the aircraft's 6-DOF state (position, velocity, orientation) over a local UDP socket.
2.  **Python WebSocket Bridge (`telemetry_server.py`):** Receives the UDP packets and relays them to web clients via WebSockets, ensuring low-latency data transmission.
3.  **3D Web Visualizer (`visualizer.html`):** An HTML/Three.js dashboard that renders the aircraft model in 3D, providing real-time visual feedback on flight dynamics, orientation, and pilot inputs.

### 7. Conclusion
By utilizing custom mathematics without relying on external libraries, implementing a threaded execution model, and bridging C++ backend physics with a 3D frontend visualizer, this simulation serves as a highly accurate proxy for embedded avionics software development.