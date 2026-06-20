#include "../include/Aircraft.h"
#include "../include/Physics.h"
#include <iostream>

using namespace chrono;

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

    logFile.open("data/flight_data.csv");
    logFile << "time,altitude,velocity,acceleration\n";

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

    logFile.close();
}

Vector3 Aircraft::GetPosition()
{
    lock_guard<mutex> lock(stateMutex);
    return position;
}

float Aircraft::GetMass() 
{ 
    lock_guard<mutex> lock(stateMutex);
    return mass; 
}

//normally wing area, lift coef and drag coef could be constant but i think about their possibility of change
float Aircraft::GetWingArea() 
{ 
    lock_guard<mutex> lock(stateMutex);
    return wingArea; 
}

float Aircraft::GetLiftCoef() 
{ 
    lock_guard<mutex> lock(stateMutex);
    return liftCoefficient; 
}

float Aircraft::GetDragCoef() 
{ 
    lock_guard<mutex> lock(stateMutex);
    return dragCoefficient; 
}

float Aircraft::GetCurrentThrust() 
{ 
    lock_guard<mutex> lock(stateMutex);
    return currentThrust; 
}

void Aircraft::PhysicsLoop()
{
    auto period = milliseconds(1); //to achieve 1000 hz
    auto target_time = steady_clock::now();

    int i = 0;
    float dt = 0.001f;

    Vector3 acc; //acceleration
    
    Vector3 gravityVector(0, -9.81f, 0);
    Vector3 gravityForce;
    
    float liftForceMag;
    Vector3 liftForce;
    Vector3 posNormalized;
    Vector3 liftDir; //for now assume that aircraft does not lean to right and left

    float dragForceMag;
    Vector3 dragForce; 
    Vector3 dragDir;

    float thrustMag;
    Vector3 thrustForce;
    Vector3 thrustDir;

    Vector3 totalForce;

    while(isRunning)
    {
        target_time += period; //will wake up after 1 milliseconds

        this_thread::sleep_for(microseconds(500)); //lets say physics calculations took half of a milliseconds

        //force calculations
        gravityForce = gravityVector * mass; //calculate the gravitational force

        liftForceMag = 0.5f * RHO * velocity.LengthSquared() * wingArea * liftCoefficient; //v^2'den emin olamadım vektörün karesini mi almam lazımdı böyle?
        liftDir = orientation.RotateVector(Vector3(0,1,0));
        liftForce = liftDir * liftForceMag;

        dragForceMag = 0.5f * RHO * velocity.LengthSquared() * wingArea * dragCoefficient;
        
        dragDir = (velocity * -1.0f).Normalized();
        dragForce = dragDir * dragForceMag;

        thrustMag = GetCurrentThrust();
        thrustDir = orientation.RotateVector(Vector3(0,0,1)); //lets assume that aircraft looks at (0,0,1) locally. now we find the thrust direction in real world by multiplying it with its orientation 
        thrustForce = thrustDir * thrustMag;

        totalForce = gravityForce + liftForce + dragForce + thrustForce;

        acc = totalForce / mass; //find the acceleration

        {//to be able to release the lock immediately
            //velocity and position are shared variables so we need to protect them
            lock_guard<mutex> lock(stateMutex);

            velocity = velocity + (acc * dt); //update the velocity
            position = position + (velocity * dt); //update the position
        }

        this_thread::sleep_until(target_time); //job is done sleep until the target time
        
        i++;
    }
}

void Aircraft::ControlLoop()
{
    auto period = milliseconds(10); //to achieve 100 hz
    auto target_time = steady_clock::now();

    int i = 0;
    while(isRunning)
    {
        target_time += period;

        {
            lock_guard<mutex> lock(stateMutex);

            if((isnan(velocity.x) || isnan(velocity.y) || isnan(velocity.z))
                ||
                velocity.Length() > 5000.0f)
            {
                currentState = FlightState::FAIL_SAFE;
            }

            switch(currentState)
            {
                case FlightState::INIT_BOOT:
                    break;
                
                case FlightState::IDLE:
                    
                    currentThrust = 0;
                    //for now as we dont have an external command from pilot
                    //we change the state here to takeofff automatically, for test purposes
                    currentState = FlightState::TAKEOFF;
                
                    break;

                case FlightState::TAKEOFF:

                    if (currentThrust < 50000.0f) 
                    {
                        currentThrust += 250.0f; 
                    }

                    if (currentThrust < 100000.0f) 
                    {
                        currentThrust += 500.0f; 
                    }
                    
                    if(velocity.Length() > 100.0f)
                    {
                        currentState = FlightState::CRUISE;
                    }

                    break;

                case FlightState::CRUISE:

                    currentThrust = 30000.0f;

                    break;
                
                case FlightState::FAIL_SAFE:
                    currentThrust = 0; //bu kısım daha akıllı olmalı. hata tipine bağlı olarak başta başladığı yere dönme olabilir
                        //bunun için uçağın başlangıç konumunu tutmalıyız ve hareket ettikçe güncellemeliyiz konumunu, fail safe olunca da geri dönebilir.
                    break;
            }
        }
        
        this_thread::sleep_for(milliseconds(5)); //for now assume control job takes 5 milliseconds

        this_thread::sleep_until(target_time);

        i++;
    }
}

void Aircraft::TelemetryLoop()
{
    auto period = milliseconds(100);
    auto target_time = steady_clock::now();

    int i = 0;

    while(isRunning)
    {
        target_time += period;

        this_thread::sleep_for(milliseconds(50)); //for now assume telemetry job takes 50 milliseconds
        
        {
            lock_guard<mutex> lock(stateMutex);

            logFile << (i * 0.1f) << "," 
                    << position.y << "," 
                    << velocity.y << "," 
                    << currentThrust << "\n";
        }

        this_thread::sleep_until(target_time);

        i++;
    }

    cout << "telemetry job is done" << endl;
}