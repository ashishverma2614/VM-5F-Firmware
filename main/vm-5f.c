/*
    Includes functions for setting up and getting data from VM-5F RFID Reader.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "driver/gpio.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"

//Response Data Packet Definition Structure.
typedef struct RData_packet                                  
{
    unsigned char head;
    unsigned char len;
    unsigned char add;
    unsigned char cmd;
    unsigned char data[27];
    unsigned char check;
        
}RData_packet;

//Host Command Data Packet Definition Array.
unsigned char DPacket_send[8];

//Reset the RFID Reader.
int resetVM_5F()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x03;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x70;
    DPacket_send[4] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3]) * 0xFF) & 0xFF);
    
    const int len = 5;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[4]);
    return txBytes;
}

//Get VM-5F RFID Reader Firmware Version.
unsigned char getFirmware()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x03;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x72;             //get Firmware version cmd 0x72.
    DPacket_send[4] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3]) * 0xFF) & 0xFF);
    
    const int len = 5;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[4]);
    return txBytes;
}

//Set up RF Frequency Spectrum(we will use ETSI Spectrum Regulation, with Freq Range: (0x00)865.00 MHz to (0x06)868.00 MHz).
unsigned char setFreqRegion()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x06;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x78;         //set freq region cmd 0x78
    DPacket_send[4] = 0x02;         //Spectrum Regulation: ETSI
    DPacket_send[5] = 0x00;         //Start Freq - 865.00 MHz
    DPacket_send[6] = 0x06;         //Stop Freq - 868.00 MHz
    DPacket_send[7] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3] + DPacket_send[4] + DPacket_send[5] + DPacket_send[6]) * 0xFF) & 0xFF);
    
    const int len = 8;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[7]);
    return txBytes;
}

//Get RF Frequency Spectrum
unsigned char getFreqRegion()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x03;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x79;         //get freq region cmd 0x79
    DPacket_send[4] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3]) * 0xFF) & 0xFF);
    
    const int len = 5;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[4]);
    return txBytes;
}

//Set up VM-5F RFID Reader UART baud rate to 115200 bps (0x71, 0x04).
unsigned char setBaudRate()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x04;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x71;         //set Baud Rate cmd 0x71
    DPacket_send[4] = 0x04;         //Baud rate set to 115200 bps.
    DPacket_send[5] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3] + DPacket_send[4]) * 0xFF) & 0xFF);
    
    const int len = 6;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]/*dpack->cmd*/);
    printf("Checksum: %02x \n", DPacket_send[5]/*dpack->check*/);
    return txBytes;
}

//Set up working Antenna.
unsigned char setWorkAntenna()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x04;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x74;         //set working antenna cmd 0x74
    DPacket_send[4] = 0x00;         //Antenna ID: Antenna 1 - 0x00.
    DPacket_send[5] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3] + DPacket_send[4]) * 0xFF) & 0xFF);
    
    const int len = 6;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    //printf("Command send: %02x \n", DPacket_send[3]);
    //printf("Checksum: %02x \n", DPacket_send[5]);
    return txBytes;
}

//Get query about working Antenna.
unsigned char getWorkAntenna()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x03;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x75;         //get working antenna cmd 0x75
    DPacket_send[4] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3]) * 0xFF) & 0xFF);
    
    const int len = 5;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[4]);
    return txBytes;
}

//Set up RF Output Power(from 20 to 33 dBm, 0x14 to 0x21, resp).
unsigned char setOutputPower()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x04;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x76;         //set RF output power cmd 0x76
    DPacket_send[4] = 0x1A;         //set Power at: 26 dBm.
    DPacket_send[5] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3] + DPacket_send[4]) * 0xFF) & 0xFF);
    
    const int len = 6;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[5]);
    return txBytes;
}

//Get RF Output Power(from 20 to 33 dBm, 0x14 to 0x21, resp).
unsigned char getOutputPower()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x03;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x77;         //set RF output power cmd 0x77
    DPacket_send[4] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3]) * 0xFF) & 0xFF);
    
    const int len = 5;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[4]);
    return txBytes;
}

//Set DRM Mode.
unsigned char setDRM()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x04;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x7C;         //set DRM Mode cmd 0x7C
    DPacket_send[4] = 0x01;         //set DRM mode to OPEN DRM.
    DPacket_send[5] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3] + DPacket_send[4]) * 0xFF) & 0xFF);
    
    const int len = 6;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[5]);
    return txBytes;
}

//Get DRM Mode.
unsigned char getDRM()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x03;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x7D;         //get DRM Mode status cmd 0x7D
    DPacket_send[4] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3]) * 0xFF) & 0xFF);
    
    const int len = 5;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[4]);
    return txBytes;
}

//Set Antenna Detection On/Off.
unsigned char setAntDetect()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x03;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x62;         //set Antenna detection mode cmd 0x62
    DPacket_send[4] = 0x01;         //Open Antenna detector connection
    DPacket_send[5] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3] + DPacket_send[4]) * 0xFF) & 0xFF);
    
    const int len = 6;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[5]);
    return txBytes;
}

//Get Antenna Detection Mode status.
unsigned char getAntDetect()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x03;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x63;         //get Antenna detection mode status cmd 0x63
    DPacket_send[4] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3]) * 0xFF) & 0xFF);
    
    const int len = 5;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[4]);
    return txBytes;
}

//Set Reader Address.
unsigned char setReadAddress()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x04;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x73;         //set Reader's Address cmd 0x73
    DPacket_send[4] = 0x00;         //Address can be set from 0 to 254.
    DPacket_send[5] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3] + DPacket_send[4]) * 0xFF) & 0xFF);
    
    const int len = 6;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[5]);
    return txBytes;
}

/***************************** EPC C1 Gen2 Commands ***************************************/
//Real Time Inventory detecting/reading tags.
unsigned char real_time_inventory()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x04;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x89;         //Read realtime tags cmd 0x89
    DPacket_send[4] = 0x01;         //1 RF carrier freq hopping channel used per inventory round.
    DPacket_send[5] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3] + DPacket_send[4]) * 0xFF) & 0xFF);
    
    const int len = 6;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    //printf("Command send: %02x \n", DPacket_send[3]);
    //printf("Checksum: %02x \n", DPacket_send[5]);
    return txBytes;
}

//Name Inventory detecting/reading tags.
unsigned char name_inventory()
{
    DPacket_send[0] = 0xA0;
    DPacket_send[1] = 0x04;
    DPacket_send[2] = 0xFF;
    DPacket_send[3] = 0x80;         //Read tags cmd 0x80
    DPacket_send[4] = 0x01;         //1 RF carrier freq hopping channel used per inventory round.
    DPacket_send[5] = (((DPacket_send[0] + DPacket_send[1] + DPacket_send[2] + DPacket_send[3] + DPacket_send[4]) * 0xFF) & 0xFF);
    
    const int len = 6;
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) &DPacket_send[0], len);
    //printf("len = %d \n", len);
    printf("Command send: %02x \n", DPacket_send[3]);
    printf("Checksum: %02x \n", DPacket_send[5]);
    return txBytes;
}