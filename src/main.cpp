#include <fstream>
#include <iostream>

#include "../include/Vector3.h"
#include "../include/Quaternion.h"
#include "../include/Aircraft.h"

#define TOTAL_FLIGHT_TIME 10

using namespace std;
using namespace std::chrono;

int main()
{
    Aircraft aircraft;
    aircraft.BootSystem();
    
    float dt = 0.01f;
    float flight_time = 0.0f; //to measure the total flight time

    ofstream log("data/flight_data.csv");
    log << "time,altitude,velocity,acceleration\n";

    this_thread::sleep_for(seconds(10));

    aircraft.Shutdown();
    
    return 0;
}