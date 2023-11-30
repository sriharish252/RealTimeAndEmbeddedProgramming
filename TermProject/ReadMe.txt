To Run the program, set up your BeagleBone Black with appropriate wiring to a PIR sensor, 4 buttons, 1 Red and 1 Green LED for system lock status, 1 Active Buzzer and 1 Red LED for the Alarm. 
There are no dependent files required apart from a c compiler, preferably gcc.
Compile the ProjectCode.c file, using: 
  gcc ProjectCode.c -lpthread -o projectCode
Run the out file using:
  ./projectCode.out
