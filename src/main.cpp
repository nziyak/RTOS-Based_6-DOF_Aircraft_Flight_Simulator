#include <fstream>
#include <iostream>

#include "../include/Vector3.h"
#include "../include/Quaternion.h"
#include "../include/Rocket.h"

using namespace std;

int main()
{
    Rocket rocket;
    rocket.position = Vector3(0,0,0);
    

    float dt = 0.01f;
    float flight_time = 0.0f; //to measure the total flight time
    bool running = true;

    ofstream log("data/flight_data.csv");
    log << "time,altitude,velocity,acceleration\n";

    while(running)
    {
        flight_time += dt;

        //update
        rocket.Update(dt);

        log << flight_time << ","
            << rocket.position.y << ","
            << rocket.velocity.y << ","
            << rocket.lastAcceleration.y << "\n";

        if(rocket.position.y < 0.0f)
        {
            running = false;
            break;
        }
    }
    
    return 0;
}