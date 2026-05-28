# 🛩️ RTOS-Based 6-DOF Aircraft Flight Simulator

![C++](https://img.shields.io/badge/C++-17-blue.svg)
![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

## 🚀 Project Overview
This project is a high-performance, **multithreaded 6-Degrees-of-Freedom (6-DOF) aircraft flight simulator** built entirely from scratch in C++. 

Originally developed as a spin-stabilized sounding rocket simulation, the project has evolved to model complex aircraft aerodynamics and embedded flight software architectures. It demonstrates **Real-Time Operating System (RTOS)** concepts, custom physics engines, and advanced 3D mathematics without relying on external libraries like GLM or Eigen.

## 🧠 Core Engineering Features

### 1. Custom Mathematical Engine
- **`Vector3` & `Quaternion`:** Custom implementations for handling 3D transformations, preventing gimbal lock, and maintaining numerical stability during spatial rotations.

### 2. Multithreaded Execution Architecture
Simulating a real Flight Control Computer (FCC), the software decouples critical tasks into concurrent threads using `std::thread`, `std::mutex`, and `std::atomic`:
- 🔴 **Physics Thread (~1000Hz):** High-priority loop for rigorous Newtonian mechanics, lift/drag calculations, and Euler integration. Non-blocking.
- 🟡 **Control Thread (~100Hz):** Manages the **Flight State Machine** (`INIT_BOOT`, `IDLE`, `TAKEOFF`, `CRUISE`, `FAIL_SAFE`) and control surface actuation.
- 🟢 **Telemetry Thread (~10Hz):** Low-priority loop for safe data logging to `.csv` via Mutex-protected reads, ensuring I/O operations never delay the physics engine.

### 3. Aerodynamics & Physics
- Implementation of Lift ($L = \frac{1}{2} \rho v^2 S C_L$) and Aerodynamic Drag.
- Thrust vectoring and 6-DOF movement.
- Thread-safe data structures preventing race conditions during high-frequency integration.

## 📂 Repository Structure
* `src/` - Core source files (`main.cpp`, `Aircraft.cpp`, `Math/`).
* `include/` - Header files defining the multithreaded architecture.
* `data/` - Telemetry `.csv` outputs.
* `docs/` - System architecture documentation and LaTeX reports.
* `scripts/` - Python scripts (`plot.py`) for visualizing flight data.

## ⚡ How to Build and Run
1. **Compile the project (requires a compiler with C++11 or higher support):**
   ```bash
   g++ -std=c++17 src/*.cpp -o flight_sim -pthread
Execute the simulation:

```bash
./flight_sim
Visualize Telemetry:
Run the included Python script to plot Altitude, Velocity, and Acceleration graphs.

```bash
python scripts/plot.py
🎯 Purpose
This project was developed to explore aerospace dynamics and embedded software engineering patterns, specifically targeting the architectural demands of defense and aerospace systems (e.g., TÜBİTAK).