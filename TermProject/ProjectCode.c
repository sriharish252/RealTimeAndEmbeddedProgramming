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

// ON and OFF values for controlling LED light states
#define ON 1
#define OFF 0

// Time delays for delaying the next signal
#define FIVE_SEC_DELAY 5 //5 second delay

#define GPIO_PATH_LEN 40 // GPIO port access path length
#define ERROR_CODE (-1) // Set the default error code

// Function declarations
int16_t initialize_gpios();
static void writeGPIO(int8_t* light, int16_t value);
static void readPIRSensor(int8_t* value);
static void pirSensor();
static void checkRFIDAuthorized();
static void triggerAlarm(int8_t alert[20]);

static void *startRoutine_pirSensor();
static void *startRoutine_checkRFIDAuthorized();


// Global Variables
int16_t pirAlertTimer = 0;
int16_t checkRFIDUser = 0;


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
    pthread_t t_checkRFID;

    // Create Threads for Traffic Signals and their corresponding WaitButtons
    (void)pthread_create(&t_pirSensor, NULL, startRoutine_pirSensor, NULL);
    (void)pthread_create(&t_checkRFID, NULL, startRoutine_checkRFIDAuthorized, NULL);

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

    return 0;
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
            printf("If you are an authorized user present RFID tag now!\n"); 
            printf("Else an Intruder alert will be triggered!\n\n");
            checkRFIDUser = 1;
        }

        sleep(1); // Sleep for 1 second
    }
}

static void checkRFIDAuthorized() {
    while(1) {
        if(checkRFIDUser == 1) {
            //code to check RFID
            // if valid
            checkRFIDUser = 0;
            // else
            triggerAlarm("INTRUDER");
        }
        sleep(1);
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

static void *startRoutine_checkRFIDAuthorized() {
    checkRFIDAuthorized();
}