//
// Term Project
// Smart Home Security System
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
#define PIRdir "/sys/class/gpio/gpio27/direction"
#define PIRval "/sys/class/gpio/gpio27/value"
#define ALARMdir "/sys/class/gpio/gpio65/direction"
#define ALARMval "/sys/class/gpio/gpio65/value"

#define BUTTON1dir "/sys/class/gpio/gpio66/direction"
#define BUTTON1val "/sys/class/gpio/gpio66/value"
#define BUTTON2dir "/sys/class/gpio/gpio67/direction"
#define BUTTON2val "/sys/class/gpio/gpio67/value"
#define BUTTON3dir "/sys/class/gpio/gpio68/direction"
#define BUTTON3val "/sys/class/gpio/gpio68/value"
#define BUTTON4dir "/sys/class/gpio/gpio69/direction"
#define BUTTON4val "/sys/class/gpio/gpio69/value"
#define GREENLEDdir "/sys/class/gpio/gpio46/direction"
#define GREENLEDval "/sys/class/gpio/gpio46/value"
#define REDLEDdir "/sys/class/gpio/gpio47/direction"
#define REDLEDval "/sys/class/gpio/gpio47/value"

#define WATERSENSOR "/sys/bus/iio/devices/iio:device0/in_voltage2_raw"


// ON and OFF values for controlling LED light states
#define ON 1
#define OFF 0

// Time delays for delaying the next signal
#define ONE_SEC_DELAY 1 //1 second delay

#define GPIO_PATH_LEN 40 // GPIO port access path length
#define ERROR_CODE (-1) // Set the default error code

#define BUFFER_SIZE 64  // Buffer size to read data from file


// Function declarations
int16_t initialize_gpios();
static void readGPIO(int8_t* button, int8_t* value);
static void writeGPIO(int8_t* light, int16_t value);
static void readPIRSensor(int8_t* value);
static void pirSensor();
static void lockButton();
static void waterSensor();
static void triggerAlarm(int8_t alert[20]);

static void *startRoutine_pirSensor();
static void *startRoutine_lockButton();
static void *startRoutine_waterSensor();


// Global Variables
int16_t isLocked = OFF;   // Variable to keep track of the System's current security state


int main(void)
{
    struct utsname sysInfo;

    // Use uname to retrieve system information
    if (uname(&sysInfo) == ERROR_CODE) {
        (void)perror("uname");
        return ERROR_CODE; // Exit with an error code
    }

    // Print system information and project group information
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
    pthread_t t_waterSensor;

    // Create Threads for the three functionalities
    (void)pthread_create(&t_pirSensor, NULL, startRoutine_pirSensor, NULL);
    (void)pthread_create(&t_lockButton, NULL, startRoutine_lockButton, NULL);
    (void)pthread_create(&t_waterSensor, NULL, startRoutine_waterSensor, NULL);

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
    f=open(ALARMdir, O_RDWR);
    if (f <= ERROR_CODE){     // If opening file fails, we get -1 as the return value, leading to us throwing an error upstream
        (void)perror("Error opening Red Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3); // Set the port as an output port, since it's for controlling an LED device
    (void)close((int16_t)f);


    f=open(BUTTON1dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Wait Button 1 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"in",3);
    (void)close(f);

    f=open(BUTTON2dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Wait Button 2 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"in",3);
    (void)close(f);

    f=open(BUTTON3dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Wait Button 1 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"in",3);
    (void)close(f);

    f=open(BUTTON4dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Wait Button 2 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"in",3);
    (void)close(f);

    f=open(GREENLEDdir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Wait Button 1 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    f=open(REDLEDdir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Wait Button 1 Direction");
        return ERROR_CODE;
    }
    (void)write(f,"out",3);
    (void)close(f);

    return 0;
}

// For reading an input from the GPIO port
static void readGPIO(int8_t* button, int8_t* value) {
    int16_t f=0;
    f=open(button, O_RDONLY); // Open button value path in Read Only mode
    (void)read(f, value, BUFFER_SIZE);
    (void)close(f);
}

// For writing an output into the GPIO port
static void writeGPIO(int8_t* light, int16_t value) {
    int16_t f=0;
    f=open(light, O_WRONLY); // Open LED value path in Write Only mode
    value == ON ? (void)write(f,"1",1) : (void)write(f,"0",1);
    (void)close(f);
}

// For reading PIR sensor input from the GPIO port
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

// PIR sensor logic implementation
static void pirSensor() {
    int8_t value[10];
    while (1) {
        readPIRSensor(value);
        if(value[0] == '1') {
            if(isLocked == 1) {
                triggerAlarm("INTRUDER");
            }
        }
        sleep(ONE_SEC_DELAY); // delay for debounce
    }
}

// Lock Buttons Logic implementation
static void lockButton() {
    int patternUnlock[] = {1, 3, 4, 2}; // The keycode to unlock the doors, aka set the security state to unlocked mode
    int patternIndex = 0;
    int lastButton1State = 0;   // To store each button's previous state value
    int lastButton2State = 0;
    int lastButton3State = 0;
    int lastButton4State = 0;

    time_t start_time, end_time;

    char button1StateVal[BUFFER_SIZE];  // To store each button's state value
    char button2StateVal[BUFFER_SIZE];
    char button3StateVal[BUFFER_SIZE];
    char button4StateVal[BUFFER_SIZE];

    while (1) {
        readGPIO(BUTTON1val, button1StateVal);
        readGPIO(BUTTON2val, button2StateVal);
        readGPIO(BUTTON3val, button3StateVal);
        readGPIO(BUTTON4val, button4StateVal);

        int button1State = atoi(button1StateVal);
        int button2State = atoi(button2StateVal);
        int button3State = atoi(button3StateVal);
        int button4State = atoi(button4StateVal);

        if (button1State && !lastButton1State) {    // if button1 is pressed
            printf("Button 1 pressed\n");
            start_time = time(NULL);
            if (patternUnlock[patternIndex] == 1) { // if unlock pattern matches, move forward
                patternIndex++;
            } else {        // else reset the pattern to start from the beginning
                patternIndex = 0;
            }
        } else if (!button1State && lastButton1State) {
            end_time = time(NULL);
            if (difftime(end_time, start_time) >= 5) {  // if button1 is held pressed for 5 or more seconds, Lock the system
                writeGPIO(REDLEDval, ON);
                writeGPIO(GREENLEDval, OFF);
                printf("Locked\n");
                isLocked = ON;
            }
        } else if (button2State && !lastButton2State) { // Similar to button1 functionality, except for the locking part
            printf("Button 2 pressed\n");
            if (patternUnlock[patternIndex] == 2) {
                patternIndex++;
            } else {
                patternIndex = 0;
            }
        } else if (button3State && !lastButton3State) {
            printf("Button 3 pressed\n");
            if (patternUnlock[patternIndex] == 3) {
                patternIndex++;
            } else {
                patternIndex = 0;
            }
        } else if (button4State && !lastButton4State) {
            printf("Button 4 pressed\n");
            if (patternUnlock[patternIndex] == 4) {
                patternIndex++;
            } else {
                patternIndex = 0;
            }
        }

        if (patternIndex == 4) {    // if whole pattern is matched, unlock the system
            writeGPIO(GREENLEDval, ON);
            writeGPIO(REDLEDval, OFF);
            printf("Unlocked\n");
            isLocked = OFF;
            patternIndex = 0;
        }

        lastButton1State = button1State;
        lastButton2State = button2State;
        lastButton3State = button3State;
        lastButton4State = button4State;

        usleep(1000000); // delay for debounce
    }
}

// Water Sensor thread logic implementation
static void waterSensor() {
    int signal_fd;
    char buffer[4];
    int value = 0;
    // Configure the signal pin for analog reading
    signal_fd = open(WATERSENSOR, O_RDONLY);

    while (1) {
        // Read the analog value from the sensor
        lseek(signal_fd, 0, SEEK_SET);
        read(signal_fd, buffer, sizeof(buffer));
        value = atoi(buffer);

        if(value >= 200) {  // 200 value corresponds to ~1mm water depth in our testing
            triggerAlarm("WATER LEAKAGE");  // Water Leakage Detected
        }
        (void)sleep(ONE_SEC_DELAY); // delay for debounce
    }
}

// Alarm logic implementation
static void triggerAlarm(int8_t alert[20]) {
    printf("\n%s alert triggered\n", alert);    // Print the alarm type in console
    writeGPIO(ALARMval, ON);
    sleep(10);

    writeGPIO(ALARMval, OFF);
}

static void *startRoutine_pirSensor() {
    pirSensor();
}

static void *startRoutine_lockButton() {
    lockButton();
}

static void *startRoutine_waterSensor() {
    waterSensor();
}
