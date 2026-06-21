#include "../include/Aircraft.h"
#include "../include/Physics.h"
#include <iostream>

using namespace chrono;

Aircraft::Aircraft()
{
    currentState = FlightState::INIT_BOOT;
    inertia = Vector3(10000.0f, 20000.0f, 30000.0f);
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
    
    //open the udp socket
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5005); //port that python bridge listens to
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // Localhost

    logFile << "time,altitude,velocity,acceleration,qx,qy,qz,qw\n";

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
    close(udpSocket);
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

void Aircraft::SetPilotCommand(PilotCommand& cmd)
{
    lock_guard<mutex> lock(stateMutex);
    currentPilotCommand = cmd;
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

        //if the aircraft is fast enough bend the velocity vector to the aircraft look direction
        if(velocity.Length() > 10.0f) 
        {
            Vector3 forwardDir = orientation.RotateVector(Vector3(0,0,1));
            velocity = (velocity * 0.98f) + (forwardDir * velocity.Length() * 0.02f);
        }

        {//to be able to release the lock immediately
            //velocity and position are shared variables so we need to protect them
            lock_guard<mutex> lock(stateMutex);

            float q = 0.5f * RHO * velocity.LengthSquared(); //air flow
            //then calculate the torque in the 3 axis
            Vector3 torque;
            torque.x = elevatorDeflection * q * 1.0f;
            torque.y = rudderDeflection * q * 0.5f;
            torque.z = aileronDeflection * q * 2.0f;

            //then calculate the angular acceleration from torque and inertia
            angularAcceleration = torque / inertia;

            //again with euler integration find the velocity from acceleration
            angularVelocity = angularVelocity + (angularAcceleration * dt); 

            //damping factor decreases as velocity increases and rotating is suppressed
            float dampingFactor = 1.0f - (0.01f + (velocity.Length() * 0.0001f));
            if (dampingFactor < 0.5f) dampingFactor = 0.5f; //bound it

            angularVelocity = angularVelocity * dampingFactor; //aerodynamic damping

            float angleRad = angularVelocity.Length() * dt; //how many radians we turned in dt time?

            if(angularVelocity.LengthSquared() > 0.000001f) //to avoid divide by zero
            {
                //update the orientation with that angularVelocity
                //find the amount of rotation
                Quaternion deltaRot = Quaternion::AngleAxis(angularVelocity.Normalized(), angleRad * (180.0f / PI));
            
                //multiply the old orientation with the new rotation amount to find new orientation
                orientation = deltaRot * orientation;
                orientation.Normalize(); //to avoid mathematical drifts 
            }

            velocity = velocity + (acc * dt); //update the velocity
            position = position + (velocity * dt); //update the position
        
            if (position.y < 0.0f) 
            {
                position.y = 0.0f;
                if (velocity.y < 0.0f) velocity.y = 0.0f;
            }
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
                    //currentState = FlightState::TAKEOFF;

                    currentState = FlightState::MANUAL_FLIGHT;
                
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
                        //currentState = FlightState::CRUISE;
                        currentState = FlightState::MANUAL_FLIGHT;
                    }

                    break;
                
                case FlightState::MANUAL_FLIGHT:
                    
                    currentThrust = currentPilotCommand.thrustTarget;
                    aileronDeflection = currentPilotCommand.aileron;
                    elevatorDeflection = currentPilotCommand.elevator;
                    rudderDeflection = currentPilotCommand.rudder;

                    break;

                case FlightState::CRUISE:

                    currentThrust = 30000.0f;
                    
                    if(i < 300) //first 3 seconds roll right
                    {
                        aileronDeflection = 1.0f;
                    }
                    else if(i < 500) //between 3-5 seconds fly straight
                    {
                        aileronDeflection = 0.0f; //we assume in 3 seconds aircrraft became straight by completing rolling
                    }
                    else
                    {
                        aileronDeflection = -1.0f;
                    }

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
                    << currentThrust << "," 
                    << orientation.x << "," 
                    << orientation.y << "," 
                    << orientation.z << "," 
                    << orientation.w << "\n";
            
            char buffer[256];
            //we write the time altitude and 4 orientation data into a c-like string that is gonna be sent to python script
            sprintf(buffer, "%f,%f,%f,%f,%f,%f,%f", (i * 0.1f), position.y, velocity.Length(), orientation.x, orientation.y, orientation.z, orientation.w);
            //send the data through the port we opened
            sendto(udpSocket, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        }

        this_thread::sleep_until(target_time);

        i++;
    }

    cout << "telemetry job is done" << endl;
}