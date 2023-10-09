//
// MultiTasking Programming Assignment
// Simulate a traffic light system for a two-way intersection using 6 LEDs connected to the BeagleBone Black
//      with a wait sensor (button) at each direction that interrupts the 2 minute wait time for the Red light
//      and triggers a transition to Green when held for 5 seconds
//
// SWE 660 - Fall 2023
// Project Group 6
// Tue/Thu Batch
// Group Members:
//	Poorvi Lakkadi (G01389351)
//	Prabath Reddy Sagili Venkata (G01393364)
//	Pranitha Kakumanu (G01379534)
//	Sai Hruthik Karumanchi (G01352466)
//	Sai Sujith Reddy Ravula (G01409395)
//	Sri Harish Jayaram (G01393332)
//
// Copyright 2023
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/utsname.h>

// GPIO paths for GPIO port access
#define RED1dir "/sys/class/gpio/gpio67/direction" //Red light connected to GPIO67
#define RED1val "/sys/class/gpio/gpio67/value"
#define YELLOW1dir "/sys/class/gpio/gpio68/direction" //Yellow light connected to GPIO68
#define YELLOW1val "/sys/class/gpio/gpio68/value"
#define GREEN1dir "/sys/class/gpio/gpio44/direction" //Green light connected to GPIO44
#define GREEN1val "/sys/class/gpio/gpio44/value"
#define RED2dir "/sys/class/gpio/gpio26/direction" //Red light connected to GPIO26
#define RED2val "/sys/class/gpio/gpio26/value"
#define YELLOW2dir "/sys/class/gpio/gpio46/direction" //Yellow light connected to GPIO46
#define YELLOW2val "/sys/class/gpio/gpio46/value"
#define GREEN2dir "/sys/class/gpio/gpio65/direction" //Green light connected to GPIO65
#define GREEN2val "/sys/class/gpio/gpio65/value"

// ON and OFF values for controlling LED light states
#define ON 1
#define OFF 0

// Time delays for delaying the next signal
#define FIVE_SEC_DELAY 5 //5 second delay, for the yellow lights
#define TWO_MIN_DELAY 120 //2 minutes delay, for holding the red/green light pattern

#define GPIO_PATH_LEN 40 // GPIO port access path length
#define ERROR_CODE (-1) // Set the default error code
#define NUM_OF_SIGNALS 3

// Function declarations
int16_t initialize_gpios();
void writeGPIO(int8_t* light, int16_t value);
void printSignalSetStatus(int8_t signalSetNum,int8_t light);
void setSignalLightColor(int8_t signalSet[NUM_OF_SIGNALS][GPIO_PATH_LEN], int8_t signalSetNum, int8_t light);
void simulateTwoWayIntersection(int8_t signalSet1[NUM_OF_SIGNALS][GPIO_PATH_LEN], int8_t signalSet2[NUM_OF_SIGNALS][GPIO_PATH_LEN]);

int main(void)
{
    struct utsname sysInfo;

    // Use uname to retrieve system information
    if (uname(&sysInfo) == -1) {
        perror("uname");
        return 1; // Exit with an error code
    }

    // Print system information and project group
    printf("\nSystem name: %s\n", sysInfo.sysname);
    printf("Node name: %s\n", sysInfo.nodename);
    printf("Machine: %s\n", sysInfo.machine);
    printf("Project Group Number: 6\n ");
    printf("Student Names: Poorvi Lakkadi | Prabath Reddy Sagili Venkata | Pranitha  Kakumanu | "
           "Sai Hruthik Karumanchi | Sai Sujith Reddy Ravula | Sri Harish Jayaram\n\n");

    // Initializing GPIO ports
    if (initialize_gpios() == ERROR_CODE){
        (void)printf("Error with GPIO initialization \n");
        return ERROR_CODE;
    }
    (void)printf("GPIO initialization successful!\n");  // Print statement confirming GPIO port access for debugging

    // Group each side's signals of Red, Yellow and Green into an array for better clarity and easy access
    int8_t signalSet1[][GPIO_PATH_LEN] = {RED1val, YELLOW1val, GREEN1val};
    int8_t signalSet2[][GPIO_PATH_LEN] = {RED2val, YELLOW2val, GREEN2val};

    while(1){   // Infinite loop for continuous running of the traffic light program
        simulateTwoWayIntersection(signalSet1, signalSet2);
    }
}

// Connecting to the GPIO ports through GPIO SysFS directory
int16_t initialize_gpios(){
    int16_t f=0;

    // Open the RED1 LED GPIO path in ReadWrite mode
    f=open(RED1dir, O_RDWR);
    if (f <= ERROR_CODE){     // If opening file fails, we get -1 as the return value, leading to us throwing an error upstream
        (void)perror("Error opening Red Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3); // Set the port as an output port, since it's for controlling an LED device
    (void)close((int16_t)f);

    // Following processes are for other LEDs similar to the above RED1 implementation
    f=open(YELLOW1dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Yellow Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    f=open(GREEN1dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Green Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    f=open(RED2dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Red Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    f=open(YELLOW2dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Yellow Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    f=open(GREEN2dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Green Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    return 0;
}

// For writing an output into the GPIO port
void writeGPIO(int8_t* light, int16_t value){
    int16_t f=0;
    f=open(light,O_WRONLY); // Open LED value path in Write Only mode
    value == ON ? (void)write(f,"1",1) : (void)write(f,"0",1);
    (void)close(f);
}

// Function to print the current status of a signal set, prints which color LED is ON and which are OFF
void printSignalSetStatus(int8_t signalSetNum,int8_t light) {
    (void)printf("SignalSideNumber %c : ",signalSetNum);
    switch (light) {        // Since there can only be 1 light ON at a time in a signal set, there are three cases
        case 'R':
            (void)printf("Red ON, ");
            (void)printf("Yellow OFF, ");
            (void)printf("Green OFF \n");
            break;
        case 'Y':
            (void)printf("Red OFF, ");
            (void)printf("Yellow ON, ");
            (void)printf("Green OFF \n");
            break;
        case 'G':
            (void)printf("Red OFF, ");
            (void)printf("Yellow OFF, ");
            (void)printf("Green ON \n");
            break;
        default:
            (void)perror("Invalid Color selected!\n");  // Throws an error if a color other than R,Y or G is passed
            break;
    }
}

// To set a signalSet's light color
// Pass the path of the SignalSet value, SignalSet Number and the first character of the light color that must be ON
void setSignalLightColor(int8_t signalSet[NUM_OF_SIGNALS][GPIO_PATH_LEN], int8_t signalSetNum, int8_t light) {
    int8_t invalidLightError = 0;
    switch (light) {        // Since there can only be 1 light ON at a time in a signal set, there are three cases
        case 'R':
            writeGPIO(signalSet[0],ON);
            writeGPIO(signalSet[1],OFF);
            writeGPIO(signalSet[2],OFF);
            break;
        case 'Y':
            writeGPIO(signalSet[0],OFF);
            writeGPIO(signalSet[1],ON);
            writeGPIO(signalSet[2],OFF);
            break;
        case 'G':
            writeGPIO(signalSet[0],OFF);
            writeGPIO(signalSet[1],OFF);
            writeGPIO(signalSet[2],ON);
            break;
        default:
            (void)perror("Invalid Color selected!\n");  // Throws an error if a color other than R,Y or G is passed
            invalidLightError = 1;
            break;
    }
    if(!invalidLightError) {
        printSignalSetStatus(signalSetNum, light);  // Prints the ON and OFF lights of this SignalSet, for debugging
    }
}

// Simulates a Two-way intersection having 3 traffic lights on each side
void simulateTwoWayIntersection(int8_t signalSet1[][GPIO_PATH_LEN], int8_t signalSet2[][GPIO_PATH_LEN]) {

    // Letting Side 1 go by setting Green1 light ON and the opposite side's Red2 light ON
    (void)printf("Triggering Side1 GO\n");
    setSignalLightColor(signalSet1, '1', 'G');
    setSignalLightColor(signalSet2, '2', 'R');
    (void)sleep(TWO_MIN_DELAY);
    (void)printf("--------------------\n"); // Separator for easy differentiation of each section during debugging

    // Preparing Side 1 to Stop, by turning Yellow1 ON and Green1 OFF
    (void)printf("Transitioning Side1 to Yellow\n");
    setSignalLightColor(signalSet1, '1', 'Y');
    (void)sleep(FIVE_SEC_DELAY);
    (void)printf("--------------------\n");

    // Letting Side 2 go by setting Green2 light ON and the opposite side's Red1 light ON
    (void)printf("Triggering Side2 GO\n");
    setSignalLightColor(signalSet1, '1', 'R');
    setSignalLightColor(signalSet2, '2', 'G');
    (void)sleep(TWO_MIN_DELAY);
    (void)printf("--------------------\n");

    // Preparing Side 2 to Stop, by turning Yellow2 ON and Green2 OFF
    (void)printf("Transitioning Side2 to Yellow\n");
    setSignalLightColor(signalSet2, '2', 'Y');
    (void)sleep(FIVE_SEC_DELAY);
    (void)printf("--------------------\n");
}