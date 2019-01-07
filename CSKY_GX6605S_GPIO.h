// File:     CSKY_GX6605S_GPIO.h
// Author:   Karbon
// Github:   https://github.com/CSY-tvgo/Flappy8659-Run-On-C-SKY-gx6605s-dev-board
// Date:     2018.12.28
// Brief:    conveniently use GPIO

/**************************************
 *                                   *
 *             WARNING!              *
 *       请勿将此文件直接用于作业      *
 *                                   *
**************************************/

#ifndef CSKY_GX6605_GPIO_H
#define CSKY_GX6605_GPIO_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#define HIGH 1
#define LOW 0
#define GPIO_OUT 1
#define GPIO_IN 0

int GPIO_SETMODE(int ID, int direction);
int GPIO_WRITE(int ID, int value);
int GPIO_READ(int ID);

// Return value : 0 Execute success
//               -1 ID is wrong
//               -2 direction is wrong
int GPIO_SETMODE(int ID, int direction)
{
    int fd;
    char loc[100];
    char directionstr[5];
    char tempcommand[100];
    if (ID < 5 || ID > 13)
    {
        printf("[GPIO_SETMODE ERROR]"
               "ID is wrong! It should be 5 ~ 13, but your input is %d.",
               ID);
        return -1; // ID is wrong
    }
    if (direction != 0 && direction != 1)
    {
        printf("[GPIO_SETMODE ERROR]"
               "direction is wrong! It should be 0 or 1, but your input is %d.",
               direction);
        return -2; // direction is wrong
    }
    sprintf(loc, "/sys/class/gpio/gpio%d/direction", ID);
    if (access(loc, 0)) // if the file doesn't exist
    {
        sprintf(tempcommand, "echo %d >/sys/class/gpio/export", ID);
        system(tempcommand);
    }
    if (direction == 0)
    {
        sprintf(directionstr, "in");
    }
    else if (direction == 1)
    {
        sprintf(directionstr, "out");
    }
    else
    {
        return -2; // direction is wrong
    }
    fd = open(loc, O_WRONLY);
    write(fd, directionstr, sizeof(directionstr));
    close(fd);
    printf("PIN%d set %d\n", ID, direction);
    return 0;
}

// Return value : 0 Execute success
//               -1 ID is wrong
//               -2 value is wrong
int GPIO_WRITE(int ID, int value)
{
    int fd;
    int size;
    char loc[100];
    char valuestr[5];
    if (ID < 10 || ID > 13)
    {
        printf("[GPIO_WRITE ERROR]"
               "ID is wrong! It should be 10 ~ 13, but your input is %d.",
               ID);
        return -1; // ID is wrong
    }
    if (value != 0 && value != 1)
    {
        printf("[GPIO_WRITE ERROR]"
               "value is wrong! It should be 0 or 1, but your input is %d.",
               value);
        return -2; // value is wrong
    }
    sprintf(loc, "/sys/class/gpio/gpio%d/value", ID);
    sprintf(valuestr, "%d", value);
    fd = open(loc, O_WRONLY);
    write(fd, valuestr, sizeof(valuestr));
    close(fd);
    return 0;
}

// Return value : 1 IO value is HIGH
//                0 IO value is LOW
//               -1 ID is wrong
//               -2 other errors
int GPIO_READ(int ID)
{
    int fd, size;
    char loc[100];
    char buffer[5];
    if (ID < 5 || ID > 9)
    {
        printf("[GPIO_READ ERROR]"
               "ID is wrong! It should be 5 ~ 9, but your input is %d.",
               ID);
        return -1; // ID is wrong
    }
    sprintf(loc, "/sys/class/gpio/gpio%d/value", ID);
    fd = open(loc, O_RDONLY);
    size = read(fd, buffer, sizeof(buffer));
    close(fd);
    if (buffer[0] == '1')
    {
        return 1;
    }
    else if (buffer[0] == '0')
    {
        return 0;
    }
    return -2; // other errors
}

#endif /*CSKY_GX6605S_GPIO_H*/
