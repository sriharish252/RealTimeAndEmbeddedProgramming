# Real-Time and Embedded Programming

Coursework from SWE 660 (Real-Time and Embedded Systems) at George Mason University, Fall 2023, built on a BeagleBone Black in C with POSIX threads.

## Term project: smart home security system

A multithreaded controller that runs three concurrent tasks on the board:

- **Motion detection:** a PIR sensor triggers the alarm (buzzer and red LED) while the system is armed.
- **Keypad unlock:** four push buttons; the correct sequence toggles between armed and disarmed, shown on red and green status LEDs.
- **Water leak detection:** a water sensor read through the on-board ADC raises the alarm when water is detected.

GPIO is driven through the Linux sysfs interface, and the ADC is read through IIO.

**Hardware:** BeagleBone Black, PIR sensor, 4 push buttons, red and green LEDs (lock status), active buzzer and red LED (alarm), water level sensor.

**Build and run** (on the board):

```bash
cd TermProject
gcc ProjectCode.c -lpthread -o projectCode
./projectCode
```

`buttons_pattern.c` and `waterSensor.c` are the standalone component tests used during development.

## Assignments

| Folder | Topic |
|---|---|
| `SingleThreadedProg_Assignment` | Two-way traffic light with 6 LEDs, single-threaded GPIO control |
| `MultiTaskingProg_Assignment` | Traffic light with wait-sensor buttons using pthreads and mutexes; includes a GPIO-free build for emulation |
| `PriorityPreemption_Assignment` | Stopwatch with start/stop and reset buttons; thread priorities assigned by rate-monotonic scheduling |
| `Qemu_Assignment` | Cross-compiling for ARM and running under QEMU |

## Team

Group project (Group 6) with Poorvi Lakkadi, Prabath Reddy Sagili Venkata, Pranitha Kakumanu, Sai Hruthik Karumanchi, Sai Sujith Reddy Ravula and Sri Harish Jayaram.
