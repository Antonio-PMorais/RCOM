// RCOM 2026/2027
//
// Link layer protocol implementation

#include "link_layer.h"
#include "serial_port.h"

#include <signal.h>
#include <stdlib.h>

#include <stdio.h>
#include <unistd.h>

#include <errno.h>


// MISC
#define _POSIX_SOURCE 1 // POSIX compliant source
#define BUF_SIZE 256
#define FLAG 0x7E


int alarmEnabled = FALSE;
int alarmCount = 0;

// Alarm function handler.
// This function will run whenever the signal SIGALRM is received.
void alarmHandler(int signal)
{
    alarmEnabled = FALSE;
    alarmCount++;

    printf("Alarm #%d received\n", alarmCount);
}

////////////////////////////////////////////////
// LLOPEN
////////////////////////////////////////////////
int llOpenTx(LinkLayer llParameters)
{
    // ----------------------------------------------------
    // This example code shows how to open the serial port and send a string.
    // TODO: Adapt and extend this code according to the specifications of the project.
    // ----------------------------------------------------

    if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0)
    {
        perror("openSerialPort");
        return -1;
    }

    printf("Serial port %s opened\n", llParameters.serialPort);


    // Set alarm function handler.
    // Install the function signal to be automatically invoked when the timer expires,
    // invoking in its turn the user function alarmHandler
    struct sigaction act = {0};
    act.sa_handler = &alarmHandler;
    if (sigaction(SIGALRM, &act, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("Alarm configured\n");



    // Create string to send
    unsigned char sendFrame[5] = {0};
    unsigned char receiveFrame[5] = {0};
    unsigned char buf[BUF_SIZE] = {0};
    unsigned char receiveByte;

    sendFrame[0] = FLAG; // Start flag
    sendFrame[1] = 0x03; // Control field 
    sendFrame[2] = 0x03; // SET 
    sendFrame[3] = sendFrame[1] ^ sendFrame[2]; // BCC field
    sendFrame[4] = FLAG; // Control field

    writeBytesSerialPort(sendFrame, 5);
 

    alarmCount = 0;
    alarmEnabled = FALSE;
    while (alarmCount < 4)
    {
        if (alarmEnabled == FALSE)
        {
            printf("set\n");
            alarm(3); // Set alarm to be triggered in 3s
            alarmEnabled = TRUE;
            
            for (int i = 0; i < 5; i++) {
                printf("flag1\n");
                readByteSerialPort(&receiveByte);
    
                receiveFrame[i] = receiveByte;
                printf("flag2\n");
            }
            printf("flag3\n");
            if (receiveFrame[0] == FLAG && receiveFrame[1] == 0x01 && receiveFrame[2] == 0x07 && receiveFrame[3] == 0x06 && receiveFrame[4] == FLAG)
            {
                printf("Received UA frame\n");
                break;
            }
            else
            {
                printf("Received invalid frame\n");
                for (int i = 0; i < 5; i++) {
                    printf("0x%02X\n", receiveFrame[i]);
                }
                continue;
            }
        }
    }
    if (alarmCount >= 4) {
        printf("Frame missing after 9 seconds");
        return -1;
    }

    printf("Ending program\n");





    for (int i = 0; i < BUF_SIZE; i++)
    {
        buf[i] = 'a' + i % 26;
    }

    int bytes = writeBytesSerialPort(buf, BUF_SIZE);
    printf("%d bytes written to serial port\n", bytes);

    // Wait until all bytes have been written to the serial port
    sleep(1);

    // Close serial port
    if (closeSerialPort() < 0)
    {
        perror("closeSerialPort");
        return -1;
    }

    printf("Serial port %s closed\n", llParameters.serialPort);

    return 0;
}

int llOpenRx(LinkLayer llParameters)
{
    // ----------------------------------------------------
    // This example code shows how to open the serial port and receive a string.
    // TODO: Adapt and extend this code according to the specifications of the project.
    // ----------------------------------------------------

    if (openSerialPort(llParameters.serialPort, llParameters.baudRate) < 0)
    {
        perror("openSerialPort");
        return -1;
    }

    printf("Serial port %s opened\n", llParameters.serialPort);

    int FRAMED = FALSE;
    int nBytesBuf = 0;
    unsigned char receiveFrame[5] = {0};

    while (FRAMED == FALSE) {
        unsigned char byte;
        int bytes = readByteSerialPort(&byte);
        nBytesBuf += bytes;

        printf("var = 0x%02X\n", byte);
        receiveFrame[nBytesBuf - 1] = byte;

        if (nBytesBuf == 5) {
            FRAMED = TRUE;
        }
    }

    if (receiveFrame[0] == FLAG && receiveFrame[1] == 0x03 && receiveFrame[2] == 0x03 && receiveFrame[3] == 0x00 && receiveFrame[4] == FLAG)
    {
        printf("Frame is correct\n");
        FRAMED = TRUE;
    }
    else
    {
        printf("Frame is incorrect\n");
        if (closeSerialPort() < 0)
            {
                perror("closeSerialPort");
                return -1;
            }

    }

    unsigned char sendFrame[5] = {0};

    sendFrame[0] = FLAG; // Start flag
    sendFrame[1] = 0x01; // Control field 
    sendFrame[2] = 0x07; // SET 
    sendFrame[3] = sendFrame[1] ^ sendFrame[2]; // BCC field
    sendFrame[4] = FLAG; // Control field

    sleep(4);

    writeBytesSerialPort(sendFrame, 5);

    // Read from serial port until the 'z' char is received.

    // NOTE: This while() cycle is a simple example showing how to read from the serial port.
    // It must be changed in order to respect the specifications of the protocol indicated in the Lab guide.

    // TODO: Save the received bytes in a buffer array and print it at the end of the program.
    volatile int STOP = FALSE;

    while (STOP == FALSE)
    {
        // Read one byte from serial port.
        // NOTE: You must check how many bytes were actually read by reading the return value.
        // In this example, we assume that the byte is always read, which may not be true.
        unsigned char byte;
        int bytes = readByteSerialPort(&byte);
        nBytesBuf += bytes;

        printf("Byte received: %c\n", byte);

        if (byte == 'z')
        {
            printf("Received 'z' char. Stop reading from serial port.\n");
            STOP = TRUE;
        }
    }

    printf("Total bytes received: %d\n", nBytesBuf);

    // Close serial port
    if (closeSerialPort() < 0)
    {
        perror("closeSerialPort");
        return -1;
    }

    printf("Serial port %s closed\n", llParameters.serialPort);

    return 0;
}

////////////////////////////////////////////////
// LLSEND
////////////////////////////////////////////////
int llSend(const unsigned char *buf, int bufSize)
{
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLRECEIVE
////////////////////////////////////////////////
int llReceive(unsigned char *packet)
{
    // TODO: Implement this function

    return 0;
}

////////////////////////////////////////////////
// LLCLOSE
////////////////////////////////////////////////
int llCloseTx()
{
    // TODO: Implement this function

    return 0;
}

int llCloseRx()
{
    // TODO: Implement this function

    return 0;
}
