#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE 64
#define GPIO_BUTTON1 66
#define GPIO_BUTTON2 67
#define GPIO_BUTTON3 68
#define GPIO_BUTTON4 69
#define GPIO_LED_GREEN 46
#define GPIO_LED_RED 47

void writeGPIO(char filename[], char value[]) {
    int fd = open(filename, O_WRONLY);
    write(fd, value, strlen(value));
    close(fd);
}

int readGPIO(char filename[]) {
    char buffer[BUFFER_SIZE];
    int fd = open(filename, O_RDONLY);
    read(fd, buffer, BUFFER_SIZE);
    close(fd);
    return atoi(buffer);
}

int main() {
    char button1StatePath[] = "/sys/class/gpio/gpio66/value";
    char button2StatePath[] = "/sys/class/gpio/gpio67/value";
    char button3StatePath[] = "/sys/class/gpio/gpio68/value";
    char button4StatePath[] = "/sys/class/gpio/gpio69/value";
    char ledGreenStatePath[] = "/sys/class/gpio/gpio46/value";
    char ledRedStatePath[] = "/sys/class/gpio/gpio47/value";

    int patternUnlock[] = {1, 3, 4, 2};
    int patternIndex = 0;

    int lastButton1State = 0;
    int lastButton2State = 0;
    int lastButton3State = 0;
    int lastButton4State = 0;

    time_t start_time, end_time;

    while (1) {
        int button1State = readGPIO(button1StatePath);
        int button2State = readGPIO(button2StatePath);
        int button3State = readGPIO(button3StatePath);
        int button4State = readGPIO(button4StatePath);

        if (button1State && !lastButton1State) {
            printf("Button 1 pressed\n");
            start_time = time(NULL);
            if (patternUnlock[patternIndex] == 1) {
                patternIndex++;
            } else {
                patternIndex = 0;
            }
        } else if (!button1State && lastButton1State) {
            end_time = time(NULL);
            if (difftime(end_time, start_time) >= 5) {
                writeGPIO(ledRedStatePath, "1");
                writeGPIO(ledGreenStatePath, "0");
                printf("Locked\n");
            }
        } else if (button2State && !lastButton2State) {
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

        if (patternIndex == 4) {
            writeGPIO(ledGreenStatePath, "1");
            writeGPIO(ledRedStatePath, "0");
            printf("Unlocked\n");
            patternIndex = 0;
        }

        lastButton1State = button1State;
        lastButton2State = button2State;
        lastButton3State = button3State;
        lastButton4State = button4State;

        usleep(1000000); // delay for debounce
    }
    return 0;
}
