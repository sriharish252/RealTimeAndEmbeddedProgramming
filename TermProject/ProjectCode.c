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

// GPIO paths for GPIO port access
#define PIRdir "/sys/class/gpio/gpio66/direction"
#define PIRval "/sys/class/gpio/gpio66/value"

// ON and OFF values for controlling LED light states
#define ON 1
#define OFF 0

// Time delays for delaying the next signal
#define FIVE_SEC_DELAY 5 //5 second delay

#define GPIO_PATH_LEN 40 // GPIO port access path length
#define ERROR_CODE (-1) // Set the default error code

// Function declarations
int16_t initialize_gpios();
void readPIRSensor(int8_t* value);


int main(void)
{
    // Initializing GPIO ports
    if (initialize_gpios() == ERROR_CODE){
        (void)printf("Error with GPIO initialization \n");
        return ERROR_CODE;
    }
    (void)printf("GPIO initialization successful!\n");  // Print statement confirming GPIO port access for debugging

    int8_t value[10];
    while (1) {
        readPIRSensor(&value);
        printf("PIR Sensor Value: %s\n", value); // Assuming the value is a character
        usleep(500000); // Sleep for .5 second
    }

}

// Connecting to the GPIO ports through GPIO SysFS directory
int16_t initialize_gpios(){
    int16_t f=0;

    // Open the RED1 LED GPIO path in ReadWrite mode
    f=open(PIRdir, O_RDWR);
    if (f <= ERROR_CODE){     // If opening file fails, we get -1 as the return value, leading to us throwing an error upstream
        (void)perror("Error opening Red Direction");
        return ERROR_CODE;
    }
    (void)write(f,"in",3); // Set the port as an output port, since it's for controlling an LED device
    (void)close((int16_t)f);

    return 0;
}

void readPIRSensor(int8_t* value) {
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