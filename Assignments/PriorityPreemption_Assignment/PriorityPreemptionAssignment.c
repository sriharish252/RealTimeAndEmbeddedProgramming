//
// RTOS and Priority Preemption Programming Assignment
// Stopwatch Application
//      With two buttons- start/stop and reset, and two lights- red and green 
//      to indicate the stopwatch timer status.
//      Display time in seconds. Running time resolution - 100ms, Stopped time resolution - 10ms
//      Rate Monotonic Scheduling (RMS) to assign priorities to threads.
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
// #include <sched.h>

// GPIO paths for GPIO port access
#define REDdir "/sys/class/gpio/gpio67/direction" //Red light connected to GPIO67
#define REDval "/sys/class/gpio/gpio67/value"
#define GREENdir "/sys/class/gpio/gpio68/direction" //Yellow light connected to GPIO68
#define GREENval "/sys/class/gpio/gpio68/value"
#define START_BUTTONdir "/sys/class/gpio/gpio66/direction"
#define START_BUTTONval "/sys/class/gpio/gpio66/value"
#define RESET_BUTTONdir "/sys/class/gpio/gpio27/direction"
#define RESET_BUTTONval "/sys/class/gpio/gpio27/value"

// ON and OFF values for controlling LED light states
#define ON 1
#define OFF 0

// Time delays for delaying the next signal
#define ONE_SEC_DELAY 1 //1 second delay
#define TEN_SEC_DELAY 10 //10 seconds delay
#define HUNDRED_SEC_DELAY 100 //100 second delay

#define GPIO_PATH_LEN 40 // GPIO port access path length
#define ERROR_CODE (-1) // Set the default error code

// Function declarations
static int16_t initialize_gpios();
static void readGPIO(int8_t* button, int8_t* value);
static void writeGPIO(int8_t* light, int16_t value);
static void printSignalSetStatus(int8_t light, int8_t status);
static void setSignalLightColor(int8_t light[], int8_t lightColor, int8_t status);
static void runStopwatchTimer();
static void stopwatch();
static void startButton();
static void resetButton();
static void *startRoutine_runStopwatchTimer();
static void *startRoutine_stopwatch();
static void *startRoutine_startButton();
static void *startRoutine_resetButton();

// Global Variables
static pthread_mutex_t lock_red, lock_green;   // Mutexes for the red and green lights
static pthread_mutex_t lock_stopwatchTimer;   // Mutexes for the stopwatch timer
static pthread_mutex_t lock_isStopwatchOn;   // Mutexes for the stopwatch timer
static int16_t stopwatchTimer = 0;  // The number of seconds of the stopwatch timer
static int8_t isStopwatchOn = OFF;



int main(void)
{
    struct utsname sysInfo;
    pthread_t t_RunStopwatchTimer;
    pthread_t t_Stopwatch;
    pthread_t t_StartButton;
    pthread_t t_ResetButton;

    pthread_attr_t attr_RunStopwatchTimer;
    pthread_attr_t attr_Stopwatch;
    pthread_attr_t attr_StartButton;
    pthread_attr_t attr_ResetButton;;
    pthread_attr_init(&attr_RunStopwatchTimer);
    pthread_attr_init(&attr_Stopwatch);
    pthread_attr_init(&attr_StartButton);
    pthread_attr_init(&attr_ResetButton);

    // Set the thread to inherit scheduling attributes from the parent thread (optional).
    pthread_attr_setinheritsched(&attr_RunStopwatchTimer, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setinheritsched(&attr_Stopwatch, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setinheritsched(&attr_StartButton, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setinheritsched(&attr_ResetButton, PTHREAD_EXPLICIT_SCHED);

    // Set the scheduling policy (e.g., SCHED_FIFO for real-time scheduling).
    pthread_attr_setschedpolicy(&attr_RunStopwatchTimer, SCHED_OTHER);
    pthread_attr_setschedpolicy(&attr_Stopwatch, SCHED_OTHER);
    pthread_attr_setschedpolicy(&attr_StartButton, SCHED_OTHER);
    pthread_attr_setschedpolicy(&attr_ResetButton, SCHED_OTHER);

    // Create a struct for the priority and set the desired priority.
    struct sched_param param_RunStopwatchTimer;
    param_RunStopwatchTimer.sched_priority = 50; // Set the priority value (adjust as needed).
    struct sched_param param_Stopwatch;
    param_Stopwatch.sched_priority = 30;
    struct sched_param param_StartButton;
    param_StartButton.sched_priority = 20;
    struct sched_param param_ResetButton;
    param_ResetButton.sched_priority = 10;

    // Set the scheduling parameters (priority) for the thread.
    pthread_attr_setschedparam(&attr_RunStopwatchTimer, &param_RunStopwatchTimer);
    pthread_attr_setschedparam(&attr_Stopwatch, &param_Stopwatch);
    pthread_attr_setschedparam(&attr_StartButton, &param_StartButton);
    pthread_attr_setschedparam(&attr_ResetButton, &param_ResetButton);
    

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
    // if (initialize_gpios() == ERROR_CODE){
    //     (void)printf("Error with GPIO initialization \n");
    //     return ERROR_CODE;
    // }
    (void)printf("GPIO initialization successful!\n");  // Print statement confirming GPIO port access for debugging

    // Create Threads for Stopwatch and it's Start/Stop and Reset Buttons
    (void)pthread_create(&t_RunStopwatchTimer, &attr_RunStopwatchTimer, startRoutine_runStopwatchTimer, NULL);
    (void)pthread_create(&t_Stopwatch, &attr_Stopwatch, startRoutine_stopwatch, NULL);
    (void)pthread_create(&t_StartButton, &attr_StartButton, startRoutine_startButton, NULL);
    (void)pthread_create(&t_ResetButton, &attr_ResetButton, startRoutine_resetButton, NULL);

    pthread_exit(NULL); // Waits for the child threads to exit
}


// Connecting to the GPIO ports through GPIO SysFS directory
static int16_t initialize_gpios(){
    int16_t f=0;

    // Open the RED LED GPIO path in ReadWrite mode
    f=open(REDdir, O_RDWR);
    if (f <= ERROR_CODE){     // If opening file fails, we get -1 as the return value, leading to us throwing an error upstream
        (void)perror("Error opening Red Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3); // Set the port as an output port, since it's for controlling an LED device
    (void)close((int16_t)f);

    // Following processes are for other LEDs and WaitButtons similar to the above RED implementation
    f=open(GREENdir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Green Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    f=open(START_BUTTONdir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Start/Stop Button Direction");
        return ERROR_CODE;
    }
    (void)write(f,"in",3);
    (void)close(f);

    f=open(RESET_BUTTONdir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Reset Button Direction");
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

// Function to print the current status of LED light indicator
static void printSignalSetStatus(int8_t light, int8_t status) {
    switch (light) {
        case 'R':
            switch (status) {
                case ON:
                    (void)printf("Red ON \n");
                    break;
                case OFF:
                    (void)printf("Red OFF \n");
                    break;
                default:
                    break;
            }
            break;
        case 'G':
            switch (status) {
                case ON:
                    (void)printf("Green ON \n");
                    break;
                case OFF:
                    (void)printf("Green OFF \n");
                    break;
                default:
                    break;
            }
            break;
        default:
            (void)perror("Invalid Light Color selected!\n");  // Throws an error if a color other than R or G is passed
            break;
    }
}

// To set a signalSet's light color
// Pass the path of the SignalSet value, SignalSet Number and the first character of the light color that must be ON
static void setSignalLightColor(int8_t light[], int8_t lightColor, int8_t status) {

    writeGPIO(light, status);
    printSignalSetStatus(lightColor, status);  // Prints the ON and OFF lights of this SignalSet, for debugging
}

static void runStopwatchTimer() {
    while(1) {   // Infinite loop for continuous running
        if(isStopwatchOn == ON) {
            (void)pthread_mutex_lock(&lock_stopwatchTimer);
            stopwatchTimer+=1;
            (void)pthread_mutex_unlock(&lock_stopwatchTimer);
        }
        (void)sleep(ONE_SEC_DELAY);
    }
}

// Start the Stopwatch
static void stopwatch() {

    while(1) {   // Infinite loop for continuous running        
        (void)pthread_mutex_lock(&lock_red);
        (void)pthread_mutex_lock(&lock_green);
        if(isStopwatchOn == OFF) {
            setSignalLightColor(REDval, 'R', ON);
            setSignalLightColor(GREENval, 'G', OFF);
        } else if (isStopwatchOn == ON) {
            setSignalLightColor(GREENval, 'G', ON);
            setSignalLightColor(REDval, 'R', OFF);
        }
        (void)pthread_mutex_unlock(&lock_red);
        (void)pthread_mutex_unlock(&lock_green);
        (void)printf("StopwatchTimer value: %d \n", stopwatchTimer);
        (void)sleep(TEN_SEC_DELAY);
    }
}

static void startButton() {
    int8_t startButtonValue[10];
    while(1) {   // Infinite loop for continuous running
        readGPIO(START_BUTTONval, startButtonValue);
        if(startButtonValue[0] == '1') {
            (void)pthread_mutex_lock(&lock_isStopwatchOn);
            switch (isStopwatchOn) {
                case OFF:
                    isStopwatchOn = ON;
                    break;
                case ON:
                    isStopwatchOn = OFF;
                    break;
                default:
                    break;
            }
            (void)pthread_mutex_unlock(&lock_isStopwatchOn);
        }
        (void)sleep(ONE_SEC_DELAY);
    }
}

static void resetButton() {
    int8_t resetButtonValue[10];
    while(1) {   // Infinite loop for continuous running
        readGPIO(START_BUTTONval, resetButtonValue);
        if(resetButtonValue[0] == '1') {
            (void)pthread_mutex_lock(&lock_stopwatchTimer);
            stopwatchTimer = 0;
            (void)pthread_mutex_unlock(&lock_stopwatchTimer);
        }
        (void)sleep(ONE_SEC_DELAY);
    }
}


static void *startRoutine_runStopwatchTimer() {
    runStopwatchTimer();
}

static void *startRoutine_stopwatch() {
    stopwatch();
}

static void *startRoutine_startButton() {
    startButton();
}

static void *startRoutine_resetButton() {
    resetButton();
}

