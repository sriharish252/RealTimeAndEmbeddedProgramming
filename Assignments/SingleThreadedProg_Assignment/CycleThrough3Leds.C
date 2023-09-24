//
// Cycle through a set of 3 LEDs connected to the BeagleBone Black
//
// SWE 660 - Fall 2023
// R. Pettit
// Copyright 2023
//


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define REDdir "/sys/class/gpio/gpio67/direction" //Red light connected to GPIO67
#define REDval "/sys/class/gpio/gpio67/value" 
#define YELLOWdir "/sys/class/gpio/gpio68/direction" //Yellow light connected to GPIO68
#define YELLOWval "/sys/class/gpio/gpio68/value" 
#define GREENdir "/sys/class/gpio/gpio44/direction" //Green light connected to GPIO44
#define GREENval "/sys/class/gpio/gpio44/value" 
#define BUTdir "/sys/class/gpio/gpio65/direction" //Button connected to GPIO65
#define BUTval "/sys/class/gpio/gpio65/value" 

#define ON 1
#define OFF 0


#define DELAY 500000 //Half second delay in microseconds

int initialize_gpios(){
     int f=0;
     
     f=open(REDdir, O_RDWR);
     if (f < 0){
        perror("Error opening Red Direction");
        return -1;
     }
     
     write(f,"out",3);
     close(f);
     
     f=open(YELLOWdir, O_RDWR);
     if (f < 0){
        perror("Error opening Yellow Direction");
        return -1;
     }
     
     write(f,"out",3);
     close(f);
     
     f=open(GREENdir, O_RDWR);
     if (f < 0){
        perror("Error opening Green Direction");
        return -1;
     }
     
     write(f,"out",3);
     close(f);
     
     f=open(BUTdir, O_RDWR);
     if (f < 0){
        perror("Error opening Button Direction");
        return -1;
     }
     
     write(f,"in",2);
     close(f);
     
     return 0;
}


int writeGPIO(char* light, int value){

     int f=0;
     
     f=open(light,O_WRONLY);
     
     value == ON ? write(f,"1",1) : write(f,"0",1);

    close(f);
     
}

int pressed(){

    char strVal[2]; //Will need to convert string to integer values when reading from GPIO
    int numVal=0;
    
    FILE *fp=fopen(BUTval,"r");
    fscanf(fp, "%s", strVal);
    numVal=atoi(strVal);
    
    fclose(fp);
   
    return numVal;
   
}
    
    
int main (void)
{
    if (initialize_gpios() == -1){
       printf("Error with GPIO initialization \n");
       return -1;
    }
 
    while(!pressed()){
     //Wait for button press to start
    }
    
    usleep(DELAY); //Make sure the next part isn't triggered while we're still holding the button
    
    while(!pressed()){
       writeGPIO(REDval,ON);
       usleep(DELAY);
       writeGPIO(REDval,OFF);
       writeGPIO(YELLOWval,ON);
       usleep(DELAY);
       writeGPIO(YELLOWval,OFF);
       writeGPIO(GREENval,ON);
       usleep(DELAY);
       writeGPIO(GREENval,OFF);
   }
   
   return 0; 
}