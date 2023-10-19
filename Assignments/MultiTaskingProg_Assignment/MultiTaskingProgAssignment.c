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
#include <pthread.h>
#include <string.h>

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
#define WAIT_BUTTON1dir "/sys/class/gpio/gpio66/direction"
#define WAIT_BUTTON1val "/sys/class/gpio/gpio66/value"
#define WAIT_BUTTON2dir "/sys/class/gpio/gpio27/direction"
#define WAIT_BUTTON2val "/sys/class/gpio/gpio27/value"

// ON and OFF values for controlling LED light states
#define ON 1
#define OFF 0

// Time delays for delaying the next signal
#define FIVE_SEC_DELAY 5 //5 second delay, for the yellow lights
#define TWO_MIN_DELAY 120 //2 minutes delay, for holding the red/green light pattern
#define ONE_SEC_DELAY 1 //1 second delay, for using the sleep function

#define GPIO_PATH_LEN 40 // GPIO port access path length
#define ERROR_CODE (-1) // Set the default error code
#define NUM_OF_SIGNALS 3

// Function declarations
static int16_t initialize_gpios();
static void readGPIO(int8_t* button, int8_t* value);
static void writeGPIO(int8_t* light, int16_t value);
static void printSignalSetStatus(int8_t signalSetNum,int8_t light);
static void setSignalLightColor(int8_t signalSet[NUM_OF_SIGNALS][GPIO_PATH_LEN], int8_t signalSetNum, int8_t light);
static void simulateTwoWaySignalSet1_StartGreen();
static void simulateTwoWaySignalSet2_StartRed();
static void enableWaitButton1();
static void enableWaitButton2();

static void* startRoutine_TwoWay_SignalSet1_Green();
static void* startRoutine_TwoWay_SignalSet2_Red();
static void* startRoutine_enableWaitButton1();
static void* startRoutine_enableWaitButton2();


// Global Variables
// Group each side's signals of Red, Yellow and Green into an array for better clarity and easy access
static int8_t signalSet1[][GPIO_PATH_LEN] = {RED1val, YELLOW1val, GREEN1val};
static int8_t signalSet2[][GPIO_PATH_LEN] = {RED2val, YELLOW2val, GREEN2val};
static int8_t waitButton1[] = WAIT_BUTTON1val;
static int8_t waitButton2[] = WAIT_BUTTON2val;
static pthread_mutex_t lock_signalSet1, lock_signalSet2;   // Mutexes for signals and wait buttons
static pthread_mutex_t lock_waitButton1_timer, lock_waitButton2_timer;
static pthread_cond_t cond_Signal1_Red = PTHREAD_COND_INITIALIZER; // Condition variables used to signal that the current side is set to Red
static pthread_cond_t cond_Signal2_Red = PTHREAD_COND_INITIALIZER;
static int16_t isSignal1_Red = OFF;    // To validate if the other side is OFF(0) or ON(1)
static int16_t isSignal2_Red = OFF;
static int16_t waitButton1_timer = 0;  // To indicate the number of seconds the waitButton is pushed and held on
static int16_t waitButton2_timer = 0;

int main(void)
{
    struct utsname sysInfo;
    pthread_t t_SignalSide1;
    pthread_t t_SignalSide2;
    pthread_t t_WaitButtonSide1;
    pthread_t t_WaitButtonSide2;

    // Use uname to retrieve system information
    if (uname(&sysInfo) == ERROR_CODE) {
        (void)perror("uname");
        return ERROR_CODE; // Exit with an error code
    }

    // Print system information and project group
    (void)printf("\nSystem name: %s\n", sysInfo.sysname);
    (void)printf("Node name: %s\n", sysInfo.nodename);
    (void)printf("Machine: %s\n", sysInfo.machine);
    (void)printf("Project Group Number: 6\n");
    (void)printf("Student Names: Poorvi Lakkadi | Prabath Reddy Sagili Venkata | Pranitha  Kakumanu | "
           "Sai Hruthik Karumanchi | Sai Sujith Reddy Ravula | Sri Harish Jayaram\n\n");

    // Initializing GPIO ports
    if (initialize_gpios() == ERROR_CODE){
        (void)printf("Error with GPIO initialization \n");
        return ERROR_CODE;
    }
    (void)printf("GPIO initialization successful!\n");  // Print statement confirming GPIO port access for debugging

    // Create Threads for Traffic Signals and their corresponding WaitButtons
    (void)pthread_create(&t_SignalSide1, NULL, startRoutine_TwoWay_SignalSet1_Green, NULL);
    (void)pthread_create(&t_SignalSide2, NULL, startRoutine_TwoWay_SignalSet2_Red, NULL);
    (void)pthread_create(&t_SignalSide1, NULL, startRoutine_enableWaitButton1, NULL);
    (void)pthread_create(&t_SignalSide2, NULL, startRoutine_enableWaitButton2, NULL);

    pthread_exit(NULL); // Waits for the child threads to exit
}

// Connecting to the GPIO ports through GPIO SysFS directory
static int16_t initialize_gpios(){
    int16_t f=0;

    // Open the RED1 LED GPIO path in ReadWrite mode
    f=open(RED1dir, O_RDWR);
    if (f <= ERROR_CODE){     // If opening file fails, we get -1 as the return value, leading to us throwing an error upstream
        (void)perror("Error opening Red 1 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3); // Set the port as an output port, since it's for controlling an LED device
    (void)close((int16_t)f);

    // Following processes are for other LEDs and WaitButtons similar to the above RED1 implementation
    f=open(YELLOW1dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Yellow 1 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    f=open(GREEN1dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Green 1 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    f=open(RED2dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Red 2 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    f=open(YELLOW2dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Yellow 2 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    f=open(GREEN2dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Green 2 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    f=open(WAIT_BUTTON1dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Wait Button 1 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"in",3);
    (void)close(f);

    f=open(WAIT_BUTTON2dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Wait Button 2 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"in",3);
    (void)close(f);

    return 0;
}

// For reading an input from the GPIO port
static void readGPIO(int8_t* button, int8_t* value) {
    int16_t f=0;
    f=open(button, O_RDONLY); // Open button value path in Read Only mode
    (void)read(f, value, 6);
    (void)close(f);
}

// For writing an output into the GPIO port
static void writeGPIO(int8_t* light, int16_t value) {
    int16_t f=0;
    f=open(light, O_WRONLY); // Open LED value path in Write Only mode
    value == ON ? (void)write(f,"1",1) : (void)write(f,"0",1);
    (void)close(f);
}

// Function to print the current status of a signal set, prints which color LED is ON and which are OFF
static void printSignalSetStatus(int8_t signalSetNum,int8_t light) {
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
static void setSignalLightColor(int8_t signalSet[NUM_OF_SIGNALS][GPIO_PATH_LEN], int8_t signalSetNum, int8_t light) {
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

// Simulates a Two-way intersection with SignalSet1 starting on Green
static void simulateTwoWaySignalSet1_StartGreen() {
    int8_t sideNumber = '1';

    while(1){   // Infinite loop for continuous running of the traffic light program
        
        (void)pthread_mutex_lock(&lock_signalSet2);   // Locks the mutex
        while(isSignal2_Red == OFF) {   // Checks if the other side's signal is not set to Red
            (void)pthread_cond_wait(&cond_Signal2_Red,&lock_signalSet2);  //
        }
        (void)pthread_mutex_unlock(&lock_signalSet2);   // Unlocks the mutex

        isSignal1_Red = OFF;
        (void)pthread_mutex_lock(&lock_signalSet1);
        setSignalLightColor(signalSet1, sideNumber, 'G');
        (void)pthread_mutex_unlock(&lock_signalSet1);
        
        // Two Min Delay unless interrupted by wait button
        for(int16_t i=0; i<=TWO_MIN_DELAY; i++) {
            if(waitButton2_timer >= 5) {
                break;
            }
            (void)sleep(ONE_SEC_DELAY);
        }

        // Preparing Side 1 to Stop, by turning Yellow1 ON and Green1 OFF
        (void)pthread_mutex_lock(&lock_signalSet1);
        setSignalLightColor(signalSet1, sideNumber, 'Y');
        (void)pthread_mutex_unlock(&lock_signalSet1);
        (void)sleep(FIVE_SEC_DELAY);

        // Letting Side 2 go by setting Green2 light ON and the opposite side's Red1 light ON
        (void)pthread_mutex_lock(&lock_signalSet1);
        setSignalLightColor(signalSet1, sideNumber, 'R');
        isSignal1_Red = ON;
        (void)pthread_cond_signal(&cond_Signal1_Red);
        (void)pthread_mutex_unlock(&lock_signalSet1);
        
        // Two Min Delay unless interrupted by wait button
        for(int16_t i=0; i<=TWO_MIN_DELAY; i++) {
            if(waitButton1_timer >= 5) {
                break;
            }
            (void)sleep(ONE_SEC_DELAY);
        }
    }
}

// Simulates a Two-way intersection with SignalSet2 starting on Red
static void simulateTwoWaySignalSet2_StartRed() {
    int8_t sideNumber = '2';
    
    while(1){   // Infinite loop for continuous running of the traffic light program
        
        (void)pthread_mutex_lock(&lock_signalSet2);   // Locks the mutex
        isSignal2_Red = ON;
        setSignalLightColor(signalSet2, sideNumber, 'R');
        (void)pthread_cond_signal(&cond_Signal2_Red);
        (void)pthread_mutex_unlock(&lock_signalSet2);   // Unlocks the mutex

        // Two Min Delay unless interrupted by wait button
        for(int16_t i=0; i<=TWO_MIN_DELAY; i++) {
            if(waitButton2_timer >= 5) {
                break;
            }
            (void)sleep(ONE_SEC_DELAY);
        }

        // Letting Side 2 go by setting Green2 light ON and the opposite side's Red1 light ON
        
        (void)pthread_mutex_lock(&lock_signalSet1);
        while(isSignal1_Red == OFF) {
            (void)pthread_cond_wait(&cond_Signal1_Red,&lock_signalSet1);
        }
        (void)pthread_mutex_unlock(&lock_signalSet1);
        isSignal2_Red = OFF;

        (void)pthread_mutex_lock(&lock_signalSet2);
        setSignalLightColor(signalSet2, sideNumber, 'G');
        (void)pthread_mutex_unlock(&lock_signalSet2);
        
        // Two Min Delay unless interrupted by wait button
        for(int16_t i=0; i<=TWO_MIN_DELAY; i++) {
            if(waitButton1_timer >= 5) {
                break;
            }
            (void)sleep(ONE_SEC_DELAY);
        }

        (void)pthread_mutex_lock(&lock_signalSet2);
        setSignalLightColor(signalSet2, sideNumber, 'Y');
        (void)pthread_mutex_unlock(&lock_signalSet2);
        (void)sleep(FIVE_SEC_DELAY);    // Preparing Side 2 to Stop
    }
}

static void enableWaitButton1() {
    int8_t value[10];
    while(1){   // Infinite loop for continuous running of the wait button program
        readGPIO(waitButton1, value);
        (void)pthread_mutex_lock(&lock_waitButton1_timer);
        if(value[0] == '1') {
            waitButton1_timer++;
        } else {
            waitButton1_timer = 0;
        }
        
        // Reset the WaitButton after 8 seconds to avoid false triggers
        if(waitButton1_timer >= 8) {
            waitButton1_timer = 0;
        }
        (void)pthread_mutex_unlock(&lock_waitButton1_timer);
        (void)sleep(ONE_SEC_DELAY); //Set to 1 second to update the button every second
    }
}

static void enableWaitButton2() {
    int8_t value[10];
    while(1){   // Infinite loop for continuous running of the wait button program
        readGPIO(waitButton2, value);
        (void)pthread_mutex_lock(&lock_waitButton2_timer);
        if(value[0] == '1') {
            waitButton2_timer++;
        } else {
            waitButton2_timer = 0;
        }

        // Reset the WaitButton after 8 seconds to avoid false triggers
        if(waitButton2_timer >= 8) {
            waitButton2_timer = 0;
        }
        (void)pthread_mutex_unlock(&lock_waitButton2_timer);
        (void)sleep(ONE_SEC_DELAY); //Set to 1 second to update the button every second
    }
}

static void *startRoutine_TwoWay_SignalSet1_Green() {
    simulateTwoWaySignalSet1_StartGreen();
}

static void *startRoutine_TwoWay_SignalSet2_Red() {
    simulateTwoWaySignalSet2_StartRed();
}

static void *startRoutine_enableWaitButton1() {
    enableWaitButton1();
}

static void *startRoutine_enableWaitButton2() {
    enableWaitButton2();
}
