#include <stdio.h>
#include <sys/utsname.h>

int main() {
    struct utsname sysInfo;

    // Use uname to retrieve system information
    if (uname(&sysInfo) == -1) {
        perror("uname");
        return 1; // Exit with an error code
    }

    // Print system information and project group
    printf("Hello from the QEMU machine!\n");
    printf("System name: %s\n", sysInfo.sysname);
    printf("Node name: %s\n", sysInfo.nodename);
    printf("Machine: %s\n", sysInfo.machine);
    printf("Project Group Number: 6\n ");
    printf("Student Names: Poorvi Lakkadi | Prabath Reddy Sagili Venkata | Pranitha  Kakumanu | Sai Hruthik Karumanchi | Sai Sujith Reddy Ravula | Sri Harish Jayaram\n");

    return 0;
}

