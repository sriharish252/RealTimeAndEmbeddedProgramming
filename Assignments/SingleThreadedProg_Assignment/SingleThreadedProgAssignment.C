//
// Simulate a traffic light system for a two-way intersection using 6 LEDs connected to the BeagleBone Black
//
// SWE 660 - Fall 2023
// Project Group 6
// Tue/Thu Batch
//
// Copyright 2023
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define RED1dir "/sys/class/gpio/gpio67/direction" //Red light connected to GPIO67
#define RED1val "/sys/class/gpio/gpio67/value"
#define YELLOW1dir "/sys/class/gpio/gpio68/direction" //Yellow light connected to GPIO68
#define YELLOW1val "/sys/class/gpio/gpio68/value"
#define GREEN1dir "/sys/class/gpio/gpio44/direction" //Green light connected to GPIO44
#define GREEN1val "/sys/class/gpio/gpio44/value"
#define RED2dir "/sys/class/gpio/gpio26/direction" //Red light connected to GPIO67
#define RED2val "/sys/class/gpio/gpio26/value"
#define YELLOW2dir "/sys/class/gpio/gpio46/direction" //Yellow light connected to GPIO68
#define YELLOW2val "/sys/class/gpio/gpio46/value"
#define GREEN2dir "/sys/class/gpio/gpio65/direction" //Green light connected to GPIO44
#define GREEN2val "/sys/class/gpio/gpio65/value"

#define ON 1
#define OFF 0

#define FIVE_SEC_DELAY 500000 //Half second delay in microseconds set now for testing, need to change it to 2 minutes
#define TWO_MIN_DELAY 2000000 //2 second delay in microseconds set now for testing, need to change it to 2 minutes

int initialize_gpios(){
    int f=0;

    f=open(RED1dir, O_RDWR);
    if (f < 0){
        perror("Error opening Red Direction");
        return -1;
    }
    write(f,"out",3);
    close(f);

    f=open(YELLOW1dir, O_RDWR);
    if (f < 0){
        perror("Error opening Yellow Direction");
        return -1;
    }
    write(f,"out",3);
    close(f);

    f=open(GREEN1dir, O_RDWR);
    if (f < 0){
        perror("Error opening Green Direction");
        return -1;
    }
    write(f,"out",3);
    close(f);

    f=open(RED2dir, O_RDWR);
    if (f < 0){
        perror("Error opening Red Direction");
        return -1;
    }
    write(f,"out",3);
    close(f);

    f=open(YELLOW2dir, O_RDWR);
    if (f < 0){
        perror("Error opening Yellow Direction");
        return -1;
    }
    write(f,"out",3);
    close(f);

    f=open(GREEN2dir, O_RDWR);
    if (f < 0){
        perror("Error opening Green Direction");
        return -1;
    }
    write(f,"out",3);
    close(f);

    return 0;
}

int writeGPIO(char* light, int value){

    int f=0;
    f=open(light,O_WRONLY);

    value == ON ? write(f,"1",1) : write(f,"0",1);

    close(f);
    return 1;
}

int main (void)
{
    if (initialize_gpios() == -1){
        printf("Error with GPIO initialization \n");
        return -1;
    }

    printf("GPIO initialization successful!\n");

    while(true){
        writeGPIO(RED1val,ON);
        writeGPIO(GREEN2val,ON);
        printf("Green2 ON\n");
        usleep(TWO_MIN_DELAY);
        writeGPIO(RED1val,OFF);
        writeGPIO(GREEN2val,OFF);
        writeGPIO(YELLOW1val,ON);
        writeGPIO(YELLOW2val,ON);
        printf("Yellows ON\n");
        usleep(FIVE_SEC_DELAY);
        writeGPIO(YELLOW1val,OFF);
        writeGPIO(YELLOW2val,OFF);
        writeGPIO(GREEN1val,ON);
        writeGPIO(RED2val,ON);
        printf("Green1 ON\n");
        usleep(TWO_MIN_DELAY);
        writeGPIO(GREEN1val,OFF);
        writeGPIO(RED2val,OFF);
        writeGPIO(YELLOW1val,ON);
        writeGPIO(YELLOW2val,ON);
        printf("Yellows ON\n");
        usleep(FIVE_SEC_DELAY);
        writeGPIO(YELLOW1val,OFF);
        writeGPIO(YELLOW2val,OFF);
    }

    return 0;
}
