#pragma once

#include "Vector3.h"
#include "Quaternion.h"
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include <chrono>
#include <cmath>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

using namespace std;

//state machine of the aircraft
enum class FlightState 
{
    INIT_BOOT,
    IDLE,
    TAKEOFF,
    CRUISE, //FLIGHT
    MANUAL_FLIGHT,
    FAIL_SAFE
};

struct PilotCommand
{
    float thrustTarget;
    float aileron;
    float elevator;
    float rudder;
};

class Aircraft 
{
    private:
        
        int udpSocket;
        struct sockaddr_in serverAddr;

        PilotCommand currentPilotCommand;
        ofstream logFile; //to write the telemetry

        //physical state(shared data)
        //these data will be written by physics thread and read by telemetry
        //so we protect them with mutex
        mutex stateMutex;

        Vector3 position;
        Vector3 velocity;
        
        Vector3 angularVelocity;
        Vector3 angularAcceleration;
        Vector3 inertia; //vector because rotating the aircraft around all axis is not same. roll is easier than pitch
        
        float aileronDeflection; //roll control
        float elevatorDeflection; //pitch control
        float rudderDeflection; // yaw control;

        Quaternion orientation;
        
        float currentThrust;

        //aircraft properties
        float mass = 1200.0f; // kg
        float wingArea = 16.0f; // m^2
        float liftCoefficient = 0.3f; 
        float dragCoefficient = 0.05f;

        //rtos control variables
        atomic<FlightState> currentState;
        atomic<bool> isRunning; //to shutdown all system

        //threads
        thread physicsThread;
        thread controlThread;
        thread telemetryThread;

        //thread functions
        void PhysicsLoop();    // 1000Hz loop
        void ControlLoop();    // 100Hz loop
        void TelemetryLoop();  // 10Hz loop

    public:
        Aircraft();
        ~Aircraft();

        //Boot Sequencer Function
        void BootSystem();
        
        //safe shutdown
        void Shutdown();

        //thread-safe getter to read data
        Vector3 GetPosition();
        float GetMass();
        float GetWingArea();
        float GetLiftCoef();
        float GetDragCoef();
        float GetCurrentThrust();
        void SetPilotCommand(PilotCommand& cmd);
};