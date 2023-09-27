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
#define TWO_MIN_DELAY 120 //2 minutes delay, for holding the red/green light pattern

#define GPIO_PATH_LEN 40

int initialize_gpios();
int writeGPIO(char* light, int value);
void printSignalSetStatus(char signalSetNum,char light);
void setSignalLightColor(char signalSet[][GPIO_PATH_LEN], char signalSetNum, char light);
void simulateTwoWayIntersection(char signalSet1[][GPIO_PATH_LEN], char signalSet2[][GPIO_PATH_LEN]);

int main()
{
    if (initialize_gpios() == -1){
        printf("Error with GPIO initialization \n");
        return -1;
    }
    char signalSet1[][GPIO_PATH_LEN] = {RED1val, YELLOW1val, GREEN1val};
    char signalSet2[][GPIO_PATH_LEN] = {RED2val, YELLOW2val, GREEN2val};

    printf("GPIO initialization successful!\n");

    while(1){
        simulateTwoWayIntersection(signalSet1, signalSet2);
    }
}

int initialize_gpios(){
    int f=0;
    
    f=open(RED1dir, O_RDWR);
    if (f < 0){
        (void)perror("Error opening Red Direction");
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

void printSignalSetStatus(char signalSetNum,char light) {
    printf("SignalSideNumber %c : ",signalSetNum);
    switch (light) {
        case 'R':
            printf("Red ON, ");
            printf("Yellow OFF, ");
            printf("Green OFF \n");
            break;
        case 'Y':
            printf("Red OFF, ");
            printf("Yellow ON, ");
            printf("Green OFF \n");
            break;
        case 'G':
            printf("Red OFF, ");
            printf("Yellow OFF, ");
            printf("Green ON \n");
            break;
        default:
            perror("Invalid Color selected!\n");
    }
}

void setSignalLightColor(char signalSet[][GPIO_PATH_LEN], char signalSetNum, char light) {
    switch (light) {
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
            perror("Invalid Color selected!\n");
            return;
    }
    printSignalSetStatus(signalSetNum, light);
}


void simulateTwoWayIntersection(char signalSet1[][GPIO_PATH_LEN], char signalSet2[][GPIO_PATH_LEN]) {
    printf("Triggering Side1 GO\n");
    setSignalLightColor(signalSet1, '1', 'G');
    setSignalLightColor(signalSet2, '2', 'R');
    sleep(TWO_MIN_DELAY);
    printf("--------------------\n");

    printf("Transitioning Side1 to Yellow\n");
    setSignalLightColor(signalSet1, '1', 'Y');
    sleep(FIVE_SEC_DELAY);
    printf("--------------------\n");

    printf("Triggering Side2 GO\n");
    setSignalLightColor(signalSet1, '1', 'R');
    setSignalLightColor(signalSet2, '2', 'G');
    sleep(TWO_MIN_DELAY);
    printf("--------------------\n");

    printf("Transitioning Side2 to Yellow\n");
    setSignalLightColor(signalSet2, '2', 'Y');
    sleep(FIVE_SEC_DELAY);
    printf("--------------------\n");
}
