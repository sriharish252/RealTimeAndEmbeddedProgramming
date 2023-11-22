//
// Simulate a PIR Sensor
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

// GPIO paths for GPIO port access
#define PIRdir "/sys/class/gpio/gpio66/direction"
#define PIRval "/sys/class/gpio/gpio66/value"
#define PIRLEDdir "/sys/class/gpio/gpio67/direction"
#define PIRLEDval "/sys/class/gpio/gpio67/value"
#define LOCKBUTTONdir "/sys/class/gpio/gpio68/direction"
#define LOCKBUTTONval "/sys/class/gpio/gpio68/value"
#define UNLOCKBUTTONdir "/sys/class/gpio/gpio44/direction"
#define UNLOCKBUTTONval "/sys/class/gpio/gpio44/value"

// ON and OFF values for controlling LED light states
#define ON 1
#define OFF 0

// Time delays for delaying the next signal
#define FIVE_SEC_DELAY 5 //5 second delay
#define ONE_SEC_DELAY 1 //5 second delay

#define GPIO_PATH_LEN 40 // GPIO port access path length
#define ERROR_CODE (-1) // Set the default error code

// Function declarations
int16_t initialize_gpios();
static void readGPIO(int8_t* button, int8_t* value);
static void writeGPIO(int8_t* light, int16_t value);
static void readPIRSensor(int8_t* value);
static void pirSensor();
static void lockButton();
static void unlockButton();
static void triggerAlarm(int8_t alert[20]);

static void *startRoutine_pirSensor();
static void *startRoutine_lockButton();
static void *startRoutine_unlockButton();


// Global Variables
int16_t lockButtonTimer = 0;
int16_t unlockButtonTimer = 0;
int16_t checkRFIDUser = 0;
int16_t isLocked = 0;

static pthread_mutex_t lock_lockButtonTimer, lock_unlockButtonTimer;


int main(void)
{
    struct utsname sysInfo;

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


    pthread_t t_pirSensor;
    pthread_t t_lockButton;
    pthread_t t_unlockButton;

    // Create Threads for Traffic Signals and their corresponding WaitButtons
    (void)pthread_create(&t_pirSensor, NULL, startRoutine_pirSensor, NULL);
    (void)pthread_create(&t_lockButton, NULL, startRoutine_lockButton, NULL);
    (void)pthread_create(&t_unlockButton, NULL, startRoutine_unlockButton, NULL);

    pthread_exit(NULL); // Waits for the child threads to exit
    

}

// Connecting to the GPIO ports through GPIO SysFS directory
int16_t initialize_gpios(){
    int16_t f=0;

    // Open the PIR Sensor GPIO path in ReadWrite mode
    f=open(PIRdir, O_RDWR);
    if (f <= ERROR_CODE){     // If opening file fails, we get -1 as the return value, leading to us throwing an error upstream
        (void)perror("Error opening Red Direction");
        return ERROR_CODE;
    }
    (void)write(f,"in",3); // Set the port as an output port, since it's for controlling an LED device
    (void)close((int16_t)f);

    // Open the PIR alert LED GPIO path in ReadWrite mode
    f=open(PIRLEDdir, O_RDWR);
    if (f <= ERROR_CODE){     // If opening file fails, we get -1 as the return value, leading to us throwing an error upstream
        (void)perror("Error opening Red Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3); // Set the port as an output port, since it's for controlling an LED device
    (void)close((int16_t)f);


    f=open(LOCKBUTTONdir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Wait Button 1 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"in",3);
    (void)close(f);

    f=open(UNLOCKBUTTONdir, O_RDWR);
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

static void readPIRSensor(int8_t* value) {
    int16_t f = 0;
    f = open(PIRval, O_RDONLY); // Open PIR sensor value path in Read Only mode
    if (f == -1) {
        perror("Failed to open PIR sensor file");
        return;
    }

    if (read(f, value, 6) == -1) {
        perror("Failed to read PIR sensor value");
    }

    (void)close(f);
}

static void pirSensor() {
    int8_t value[10];
    while (1) {
        readPIRSensor(value);
        
        printf("PIR Sensor Value: %s\n", value); // Assuming the value is a character

        if(value[0] == '1') {
            printf("Motion Detected!\n");
            if(isLocked == 1) {
                triggerAlarm("INTRUDER");
            }

        }

        sleep(ONE_SEC_DELAY);
    }
}

static void lockButton() {
    int8_t value[10];
    while(1){   // Infinite loop for continuous running of the wait button program
        readGPIO(LOCKBUTTONval, value);
        (void)pthread_mutex_lock(&lock_lockButtonTimer);
        if(value[0] == '1') {
            lockButtonTimer++;
        } else {
            lockButtonTimer = 0;
        }
        
        // Reset the WaitButton after 8 seconds to avoid false triggers
        if(lockButtonTimer >= 8) {
            isLocked = 1;
            printf("LOCKED!\n");
            lockButtonTimer = 0;
        }
        (void)pthread_mutex_unlock(&lock_lockButtonTimer);
        (void)sleep(ONE_SEC_DELAY); //Set to 1 second to update the button every second
    }
}

static void unlockButton() {
    int8_t value[10];
    while(1){   // Infinite loop for continuous running of the wait button program
        readGPIO(UNLOCKBUTTONval, value);
        (void)pthread_mutex_lock(&lock_unlockButtonTimer);
        if(value[0] == '1') {
            unlockButtonTimer++;
        } else {
            unlockButtonTimer = 0;
        }
        
        // Reset the WaitButton after 8 seconds to avoid false triggers
        if(unlockButtonTimer >= 8) {
            isLocked = 0;
            printf("UNLOCKED!\n");
            unlockButtonTimer = 0;
        }
        (void)pthread_mutex_unlock(&lock_unlockButtonTimer);
        (void)sleep(ONE_SEC_DELAY); //Set to 1 second to update the button every second
    }
}

static void triggerAlarm(int8_t alert[20]) {
    printf("\n%s alert triggered\n", alert);
    writeGPIO(PIRLEDval, 1);
    sleep(10);

    writeGPIO(PIRLEDval, 0);
}


static void *startRoutine_pirSensor() {
    pirSensor();
}

static void *startRoutine_lockButton() {
    lockButton();
}

static void *startRoutine_unlockButton() {
    unlockButton();
}