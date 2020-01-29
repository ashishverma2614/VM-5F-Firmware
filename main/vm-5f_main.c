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

#define TXD_PIN (GPIO_NUM_4)
#define RXD_PIN (GPIO_NUM_5)

//Intializing UART and GPIO 
void init() {                                                           
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, BUF_SIZE , BUF_SIZE , 0, NULL, 0);
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

//Send data to the VM-5F RFID Reader.
int sendData(const char* data)
{
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_1, data, len);
    printf("Data send length: %d\n", len);
    return txBytes;
}

//Reset the RFID Reader.
int resetVM_5F()
{
    struct Data_packet *dpack, d2;
    d2.head     = 0x0A;
    d2.len      = 0x03;
    d2.add      = 0xFF;
    d2.cmd      = 0x70;
    d2.data[0]  = 0x00; d2.data[1] = 0x00; d2.data[2] = 0x00; d2.data[3] = 0x00;
    d2.check    = sizeof(d2.head + d2.len + d2.add + d2.cmd + d2.data);

    dpack = &d2;
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_1, (const char*) dpack, len);
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
    d1.data[0]  = 0xEC; d1.data[1] = 0x00; d1.data[2] = 0x00; d1.data[3] = 0x00;
    //d1.data[]   = {0xEC, 0x00, 0x00, 0x00};
    d1.check    = sizeof(d1.head + d1.len + d1.add + d1.cmd + d1.data);

    dpack = &d1;
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_1, (const char*) dpack, len);
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
    d3.data[0]  = 0x04; d3.data[1] = 0x00; d3.data[2] = 0x00; d3.data[3] = 0x00;
    //d3.data     = {0x04, 0x00, 0x00, 0x00};
    d3.check    = sizeof(d3.head + d3.len + d3.add + d3.cmd + d3.data);

    dpack = &d3;
    const int len = sizeof(Data_packet);
    const int txBytes = uart_write_bytes(UART_NUM_1, (const char*) dpack, len);
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
    const int txBytes = uart_write_bytes(UART_NUM_1, (const char*) dpack, len);
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
        //resetVM_5F();
        setBaudRate();
        getFirmware();
        setWorkAntenna();
    while(1)
    {
        
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

//Rx task function.
static void rx_task()
{ 
    uint8_t* data = (uint8_t*) malloc(BUF_SIZE+1);
    struct Data_packet * dpack2;

    while(1)
    {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, BUF_SIZE, 1000 / portTICK_RATE_MS);
        if(rxBytes >= 0) {
            data[rxBytes] = 0;
            dpack2 = &data;
            printf("rx head: %02x \n", dpack2->head);
            printf("rx length: %02x \n", dpack2->len);
            printf("rx address: %02x \n", dpack2->add);
            printf("rx data: %02x \n", dpack2->data[0]);
            printf("rx checksum: %02x \n", dpack2->check);
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
