#include "../include/Aircraft.h"

Aircraft::Aircraft()
{
    currentState = FlightState::INIT_BOOT;
}

Aircraft::~Aircraft()
{
    Shutdown();
}

void Aircraft::BootSystem()
{
    isRunning = true;
    currentState = FlightState::IDLE;

    //start the 3 threads
    physicsThread = thread(&Aircraft::PhysicsLoop, this);
    controlThread = thread(&Aircraft::ControlLoop, this);
    telemetryThread = thread(&Aircraft::TelemetryLoop, this);
}

void Aircraft::Shutdown()
{
    isRunning = false;

    if(physicsThread.joinable()) physicsThread.join();
    if(controlThread.joinable()) controlThread.join();
    if(telemetryThread.joinable()) telemetryThread.join();
}

Vector3 Aircraft::GetPosition()
{
    lock_guard<mutex> lock(stateMutex);
    return position;
}

void Aircraft::PhysicsLoop()
{
    while(isRunning)
    {

    }
}

void Aircraft::ControlLoop()
{
    //...
}

void Aircraft::TelemetryLoop()
{
    //...
}