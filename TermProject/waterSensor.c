#include <stdio.h>
#include <stdlib.h>  // Included the stdlib.h header for atoi
#include <unistd.h>
#include <fcntl.h>

#define SIGNAL_PIN "/sys/bus/iio/devices/iio:device0/in_voltage2_raw"

int main() {
    int signal_fd;
    char buffer[4];
    int value = 0;

    // Configure the signal pin for analog reading
    signal_fd = open(SIGNAL_PIN, O_RDONLY);

    while (1) {
        // Read the analog value from the sensor
        lseek(signal_fd, 0, SEEK_SET);
        read(signal_fd, buffer, sizeof(buffer));
        value = atoi(buffer);

        printf("Sensor value: %d\n", value);

        sleep(1);  // Delay for 1 second
    }

    // No need to unexport a power control GPIO in this case.

    return 0;
}