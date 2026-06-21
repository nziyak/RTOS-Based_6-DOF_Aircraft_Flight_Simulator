#include <fstream>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "../include/Vector3.h"
#include "../include/Quaternion.h"
#include "../include/Aircraft.h"

#define TOTAL_FLIGHT_TIME 10

using namespace std;
using namespace std::chrono;

// Klavyeye basılıp basılmadığını anlık (non-blocking) kontrol eder
int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}


int main()
{
    Aircraft aircraft;
    aircraft.BootSystem();
    
    float dt = 0.01f;
    float flight_time = 0.0f; //to measure the total flight time
    
    PilotCommand cmd = {0.0f, 0.0f, 0.0f, 0.0f};

    bool hasTakenOff = false;

    char c;
    int iskbhit = 0;

    while(true)
    {
        iskbhit = kbhit();

        if(iskbhit == 1)
        {
            c = getchar();

            switch(c)
            {
                case 'w': //lift up
                    cmd.elevator = 1.0f;
                    break;
                
                case 's': //lift down
                    cmd.elevator = -1.0f;
                    break;
                
                case 'a': //roll left
                    cmd.aileron = -2.0f;
                    break;
                
                case 'd': //roll right
                    cmd.aileron = 2.0f;
                    break;

                case 'x': //reset the control surfaces
                    cmd.aileron = 0.0f;
                    break;

                case 't': //increase thrust
                    cmd.thrustTarget += 5000.0f;
                    break;

                case '-': //decrease thrust
                    cmd.thrustTarget -= 5000.0f;
                    break;
            }
        }

        else
        {
            cmd.aileron *= 0.8f;
            cmd.elevator *= 0.8f;
        }

        aircraft.SetPilotCommand(cmd); //send the updated cmd package to aircraft

        if(aircraft.GetPosition().y > 10) hasTakenOff = true;

        if(aircraft.GetPosition().y < 0.1f && hasTakenOff)
        {
            cout << "aircraft is landed" << endl;
            break;
        }

        this_thread::sleep_for(milliseconds(50));
    }

    aircraft.Shutdown();
    
    return 0;
}