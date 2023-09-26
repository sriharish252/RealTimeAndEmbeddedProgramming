//
// Simulate a traffic light system for a two-way intersection using 6 LEDs connected to the BeagleBone Black
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
#include <stdlib.h>

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

#define ON 1
#define OFF 0

#define FIVE_SEC_DELAY 5 //5 second delay, for the yellow lights
#define TWO_MIN_DELAY 120 //2 minutes delay

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

void triggerYellowsON() {
    writeGPIO(YELLOW1val,ON);
    writeGPIO(YELLOW2val,ON);
}

void triggerYellowsOFF() {
    writeGPIO(YELLOW1val,OFF);
    writeGPIO(YELLOW2val,OFF);
}

void triggerYellows() {
    triggerYellowsON();
    printf("Yellows ON\n");
    sleep(FIVE_SEC_DELAY);
    triggerYellowsOFF();
    printf("Yellows OFF\n");
}

void letSide1Go() {
    writeGPIO(RED2val,ON);
    printf("Red2 ON\n");
    writeGPIO(GREEN1val,ON);
    printf("Green1 ON\n");
}

void letSide1Wait() {
    writeGPIO(RED2val,OFF);
    printf("Red2 OFF\n");
    writeGPIO(GREEN1val,OFF);
    printf("Green1 OFF\n");
}

void letSide2Go() {
    writeGPIO(RED1val,ON);
    printf("Red1 ON\n");
    writeGPIO(GREEN2val,ON);
    printf("Green2 ON\n");
}

void letSide2Wait() {
    writeGPIO(RED1val,OFF);
    printf("Red1 OFF\n");
    writeGPIO(GREEN2val,OFF);
    printf("Green2 OFF\n");
}

int main (void)
{
    if (initialize_gpios() == -1){
        printf("Error with GPIO initialization \n");
        return -1;
    }

    printf("GPIO initialization successful!\n");

    while(1){
        // Setting Side1 as GO
        letSide1Go();
        sleep(TWO_MIN_DELAY);
        letSide1Wait();

        triggerYellows();

        // Setting Side2 as GO
        letSide2Go();
        sleep(TWO_MIN_DELAY);
        letSide2Wait();

        triggerYellows();
    }

    return 0;
}