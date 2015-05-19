/**	TeensyApp
 *	Author : Felix Hellman
 *	Version: 1.0
 *	Purpose: Control linux client with DIY USB-HID device using Teensy Devkit
 */



#define mute "amixer -D pulse sset Master toggle & exit"
#define screenoff "xset dpms force off & exit"
#define screenon "xset dpms force on & exit"



#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include "hid.h"
#include <sys/types.h>
#include <signal.h>
#include <time.h>



/**
 *  Returns a pointer to a properly formated string for the volume command 
 */
char *volumeString(int volume)
{
    int i;
    char string[4];
    static char cmd[] = "amixer -D pulse sset Master      & exit\0";
    string[0] =   (char)(volume/100)+48;
    string[1] =   (char)(volume%100/10)+48;
    string[2] = (char)(volume%10)+48;
    string[3] = '%';
    for(i = 29; i < 33 ;i++) 
    {
        cmd[i] = string[i - 29];
    }   
    return cmd;
}
/**
 * Takes data from bytestream and converts into an int
 */
int getData(char* Value)
{
    unsigned int x = 0;
    x = (Value[0] & 255) << 8;
    x = x + (Value[1] & 255);
    x = x >> 4; 
    return x;
}
/**
 * Tries to connect to the HID-Device
 */
int connectDevice(int d)
{
    int r;
    r = rawhid_open(1, 0x16C0, 0x0480, 0xFFAB, 0x0200);

    if (r <= 0) 
    {
        r = rawhid_open(1, 0x16C0, 0x0486, 0xFFAB, 0x0200);

        if (r <= 0) 
        {
            return -1;
        }
        else
        {
	        printf("Device connected\n");
	        return 0;
        }
    }

    else
    {
      return -1;
    }
}
/**
 *  Creates a child process to run certain separate linux-bash commands.
 *  Child process is terminated on command completion
 */
void run_command(int n)
{
    char *command[4]; 
    int select = 0;

    if(n >= 110)select = n/10;
    else select  = 0;

    switch(select)
    {
        case 0:
        command[2] = volumeString(n);
        break;

        case 20:
        command[2] = mute;
        break;

        case 21:
        command[2] = screenoff;
        break;

        case 22:
        command[2] = screenon;
        break;
    }
    
    command[0] = "sh";
    command[1] = "-c";
    command[3] = NULL;
    pid_t childPID;
    childPID = fork();
    if(childPID > 0)
    {
        wait(NULL);
    }
    else if(childPID == 0)
    {
         execv("/bin/sh",command);
        _exit(EXIT_SUCCESS);
    }

    kill(childPID,SIGTERM);
}
/**
 *  Waits for n amount of milliseconds
 */

void delay(clock_t milli)
{
    milli = milli*1000 + clock();
    while(clock() < milli);
}

int main()
{
    char buf[64];
    int  analogValue, toggleStatus, buttonStatus,prevButtonStatus, data,r,num, volume,prevVol;
  
    buttonStatus = 0;
    r = connectDevice(0);
    prevVol = 0;
    volume = 0;	
    prevButtonStatus = 0;
    toggleStatus = 0;





    if (r == -1)
    {
        printf("%s","No device found\n");
        r = 1;
        while(r != 0){r = connectDevice(1000);}
    }
    while (1) 
    {
        num = rawhid_recv(0, buf, 64, 220);
        if (num < 0) 
        {
             printf("%s","Device was disconnected\n");
             r = 1;
        while(r != 0){r = connectDevice(5000);}
        }

        if (num > 0) 
        {
            data = getData(buf);
            analogValue = data & 1023;
            toggleStatus = (data & 1024)>>10;
            buttonStatus = (data & 2048)>>11;	    
            volume = analogValue / 10;
        }
        if((prevVol - volume) > 1 || (volume - prevVol) > 1)
        {
	        run_command(volume);
	        prevVol = volume;
        }
    
        if(buttonStatus == 1 && prevButtonStatus != 1) run_command(200);
    
        prevButtonStatus = buttonStatus;
        if(toggleStatus == 1) run_command(210);
        else run_command(220);
    }
}





