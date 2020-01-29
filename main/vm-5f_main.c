/* 
    
    VM-5F UHF RFID Reader communication with ESP-32 via UART to read RFID tags, 
    and use GPIOs to control a relay, a STOP button, an LED, and a Buzzer.

*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
    
/*
 * - Port: UART1
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: on
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below
 */

static const int BUF_SIZE = 256;

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)

//Intializing UART and GPIO 
void init() {                                                           
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_2, BUF_SIZE , BUF_SIZE , 0, NULL, 0);
}

//Host Command Data Packet Definition.
typedef struct Data_packet                                  
{
    unsigned char head;
    unsigned char len;
    unsigned char add;
    unsigned char cmd;
    unsigned char data[4];
    unsigned char check;
        
}Data_packet;
/*
//Send data to the VM-5F RFID Reader.
int sendData(const char* data)
{
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_2, data, len);
    printf("Data send length: %d\n", len);
    return txBytes;
}
*/
//Reset the RFID Reader.
int resetVM_5F()
{
    struct Data_packet *dpack, d2;
    d2.head     = 0x0A;
    d2.len      = 0x03;
    d2.add      = 0xFF;
    d2.cmd      = 0x70;
    //d2.data[0]  = 0x00; d2.data[1] = 0x00; d2.data[2] = 0x00; d2.data[3] = 0x00;
    d2.check    = sizeof(d2.head + d2.len + d2.add + d2.cmd + d2.data);

    dpack = &d2;
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) dpack, len);
    printf("Command send: %02x \n", dpack->cmd);
    return txBytes;
}

//Get VM-5F RFID Reader Firmware Version.
int getFirmware()
{
    struct Data_packet *dpack, d1;
    d1.head     = 0x0A;
    d1.len      = 0x03;
    d1.add      = 0x01;
    d1.cmd      = 0x72;
    //d1.data[0]  = 0xEC; d1.data[1] = 0x00; d1.data[2] = 0x00; d1.data[3] = 0x00;
    //d1.data[]   = {0xEC, 0x00, 0x00, 0x00};
    d1.check    = sizeof(d1.head + d1.len + d1.add + d1.cmd + d1.data);

    dpack = &d1;
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) dpack, len);
    printf("Command send: %02x \n", dpack->cmd);
    return txBytes;
}

//Set up VM-5F RFID Reader UART baud rate to 115200 bps (0x71, 0x04).
int setBaudRate()
{
    struct Data_packet *dpack, d3;
    d3.head     = 0x0A;
    d3.len      = 0x03;
    d3.add      = 0x01;
    d3.cmd      = 0x71;
    d3.data[0]  = 0x04; //d3.data[1] = 0x00; d3.data[2] = 0x00; d3.data[3] = 0x00;
    //d3.data     = {0x04, 0x00, 0x00, 0x00};
    d3.check    = sizeof(d3.head + d3.len + d3.add + d3.cmd + d3.data);

    dpack = &d3;
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) dpack, len);
    printf("Command send: %02x \n", dpack->cmd);
    //printf("txBytes sent: %d \n", sizeof(txBytes));
    return txBytes;
}

//Set up working Antenna.
int setWorkAntenna()
{
    struct Data_packet *dpack, d4;
    d4.head     = 0x0A;
    d4.len      = 0x04;
    d4.add      = 0xFF;
    d4.cmd      = 0x74;
    d4.data[0]  = 0x00; //d4.data[1] = 0x00; d4.data[2] = 0x00; d4.data[3] = 0x00;
    //d3.data     = {0x04, 0x00, 0x00, 0x00};
    d4.check    = sizeof(d4.head + d4.len + d4.add + d4.cmd + d4.data);

    dpack = &d4;
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) dpack, len);
    printf("Command send: %02x \n", dpack->cmd);
    //printf("txBytes sent: %d \n", sizeof(txBytes));
    return txBytes;
}

//Set up RF Output Power(from 20 to 33 dBm, 0x14 to 0x21, resp).
int setOutputPower()
{
    struct Data_packet *dpack, d5;
    d5.head     = 0x0A;
    d5.len      = 0x04;
    d5.add      = 0xFF;
    d5.cmd      = 0x76;
    d5.data[0]  = 0x20; //d4.data[1] = 0x00; d4.data[2] = 0x00; d4.data[3] = 0x00;
    //d3.data     = {0x04, 0x00, 0x00, 0x00};
    d5.check    = sizeof(d5.head + d5.len + d5.add + d5.cmd + d5.data);

    dpack = &d5;
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) dpack, len);
    printf("Command send: %02x \n", dpack->cmd);
    //printf("txBytes sent: %d \n", sizeof(txBytes));
    return txBytes;
}

//Set up RF Frequency Spectrum(we will use ETSI Spectrum Regulation, with Freq Range: (0x00)865.00 MHz to (0x06)868.00 MHz).
int setFreqRegion()
{
    struct Data_packet *dpack, d6;
    d6.head     = 0x0A;
    d6.len      = 0x04;
    d6.add      = 0xFF;
    d6.cmd      = 0x78;
    d6.data[0]  = 0x02; d6.data[1] = 0x00; d6.data[2] = 0x06; //d6.data[3] = 0x00;
    //d3.data     = {0x04, 0x00, 0x00, 0x00};
    d6.check    = sizeof(d6.head + d6.len + d6.add + d6.cmd + d6.data);

    dpack = &d6;
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) dpack, len);
    printf("Command send: %02x \n", dpack->cmd);
    //printf("txBytes sent: %d \n", sizeof(txBytes));
    return txBytes;
}

//Set DRM Mode.
int setDRM()
{
    struct Data_packet *dpack, d7;
    d7.head     = 0x0A;
    d7.len      = 0x04;
    d7.add      = 0xFF;
    d7.cmd      = 0x7C;
    d7.data[0]  = 0x01; //d7.data[1] = 0x06; d7.data[2] = 0x00; d7.data[3] = 0x00;
    //d3.data     = {0x04, 0x00, 0x00, 0x00};
    d7.check    = sizeof(d7.head + d7.len + d7.add + d7.cmd + d7.data);
    
    dpack = &d7;
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) dpack, len);
    printf("Command send: %02x \n", dpack->cmd);
    //printf("txBytes sent: %d \n", sizeof(txBytes));
    return txBytes;
}

//Set Antenna Detection On/Off.
int setAntDetect()
{
    struct Data_packet *dpack, d8;
    d8.head     = 0x0A;
    d8.len      = 0x04;
    d8.add      = 0xFF;
    d8.cmd      = 0x62;
    d8.data[0]  = 0x01; //d8.data[1] = 0x06; d8.data[2] = 0x00; d8.data[3] = 0x00;
    //d3.data     = {0x04, 0x00, 0x00, 0x00};
    d8.check    = sizeof(d8.head + d8.len + d8.add + d8.cmd + d8.data);
    
    dpack = &d8;
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) dpack, len);
    printf("Command send: %02x \n", dpack->cmd);
    //printf("txBytes sent: %d \n", sizeof(txBytes));
    return txBytes;
}

//Set Reader Address.
int setReadAddress()
{
    struct Data_packet *dpack, d9;
    d9.head     = 0x0A;
    d9.len      = 0x04;
    d9.add      = 0xFF;
    d9.cmd      = 0x62;
    d9.data[0]  = 0x00; d9.data[1] = 0xFF; //d9.data[2] = 0xFF; d9.data[3] = 0xFF;
    //d3.data     = {0x04, 0x00, 0x00, 0x00};
    d9.check    = sizeof(d9.head + d9.len + d9.add + d9.cmd + d9.data);
    
    dpack = &d9;
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_2, (const char*) dpack, len);
    printf("Command send: %02x \n", dpack->cmd);
    //printf("txBytes sent: %d \n", sizeof(txBytes));
    return txBytes;
}

//Tx task function.
static void tx_task()
{   
        //sendData( (const char*) dpack);                                                             //type casting pointer data types.            
        //printf("Cmd send: %d\n", dpack->cmd);
        //printf("Checksum send: %d\n\n\n", dpack->check);
    int i = 0;
    while(1)
    {
        if(i<1)
        {
            resetVM_5F();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            setBaudRate();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            setReadAddress();
            i++;
        }
    
            getFirmware();
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            setWorkAntenna();
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            setOutputPower();
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            setFreqRegion();
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            setDRM();
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            setAntDetect();
            vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

//Rx task function.
static void rx_task()
{ 
    uint8_t* data = (uint8_t*) malloc(BUF_SIZE+1);
    struct Data_packet *dpack2;

    while(1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_2, data, BUF_SIZE, 1000 / portTICK_RATE_MS);
        dpack2 = (struct Data_packet*)data;
        if(rxBytes >= 0) {
            data[rxBytes] = 0;
            printf("%d\n", rxBytes);
            printf("rx head: %02x \n", dpack2->head);
            printf("rx length: %02x \n", dpack2->len);
            printf("rx address: %02x \n", dpack2->add);
            printf("rx data: %02x \n", dpack2->data[0]);
            printf("rx checksum: %02x \n", dpack2->check);            
            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
    free(data);
}

void app_main()
{
    init();
    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}