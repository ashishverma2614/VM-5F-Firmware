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
#include "driver/gpio.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include "vm-5f.c"
    
/*
 * - Port: UART2
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: on
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below
 */
#define TXD_PIN          17
#define RXD_PIN          16
#define GPIO_OUTPUT_IO_0 5                                                          //EN Pin for the RFID Reader.
#define GPIO_OUTPUT_IO_1 18                                                         //EN Pin for the Relay to control the EM Door.
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0  19                                                         //STOP Switch(pull down) to unlock the Door.
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_0)
#define ESP_INTR_FLAG_DEFAULT 0

#define TAG_LENGTH 12                                                               //Length of EPC Number of 1 UHF RFID Tag.
#define RELAY_TRIGGER_TIME 4000                                                     //Delay time before switching OFF the EM Lock/Relay.

static const int RX_BUF_SIZE = 512;
static xQueueHandle tag_found_queue = NULL;

//Intializing UART
void init() 
{                         
    /*****UART Config*****/                                  
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_2, RX_BUF_SIZE * 2 ,0 , 0, NULL, 0);
}

//Setting up GPIOs
void gpio_setup()
{
        /*****GPIO Config*****/
    
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //bit mask of the pins that you want to set as output, GPIO5/18 here
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //able pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //Disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //bit mask of the pins, use GPIO19 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

//Function to get Data from RFID Reader in RData_packet structure.
unsigned char getData()
{
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    int j = 0;
    struct RData_packet * dpack2;
    dpack2 = (struct RData_packet*)data;

     const int rxBytes = uart_read_bytes(UART_NUM_2, data, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        if(rxBytes > 0) 
        {
            data[rxBytes] = 0;
            j = (rxBytes - 5);          //here, j = length of Data[] received(head, len, add, cmd, checksum excluded).
            //printf("rx bytes:   %d\n", rxBytes);
            //printf("rx head:    %02x \n", dpack2->head);
            //printf("rx length:  %02x \n", dpack2->len);
            //printf("rx address: %02x \n", dpack2->add);
            printf("rx command: %02x \n", dpack2->cmd);
            printf("rx data:    ");
            for(int i=0; i<j; i++)              
            {
                printf("%02x ", dpack2->data[i]);
            }
            printf("\n");
            printf("rx checksum: %02x \n", dpack2->check);  
        }
        free(data);
        uart_flush(UART_NUM_2);
        return rxBytes;       
}

//Function for Tag Detection.
unsigned char TagDetect()
{
    uint8_t* tagdata = (uint8_t*) malloc(RX_BUF_SIZE+1);
    int j = 0;
    uint32_t tag_id;
    struct RData_packet * dpack3;
    dpack3 = (struct RData_packet*)tagdata;

    const int rxBytes = uart_read_bytes(UART_NUM_2, tagdata, RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
        if(rxBytes > 0) 
        {
            tagdata[rxBytes] = 0;
            j = (rxBytes - 5);          //here, j = length of Data[] received(head, len, add, cmd, checksum excluded).
            
            printf("rx command: %02x \n", dpack3->cmd);
            if(j > 10)
            {
                printf("\t\t TAG FOUND!!! \n");
                printf("Freq Ant: %02x \n", dpack3->data[0]);
                printf("PC Bytes: %02x  %02x \n", dpack3->data[1], dpack3->data[2]);
                
                printf("RFID Tag EPC No: ");
                for(int i=3; i<(TAG_LENGTH + 3); i++)              
                {
                    printf("%02x ", dpack3->data[i]);
                }
                printf("\n");
                printf("\nRFID Tag other data: ");
                for(int i=(TAG_LENGTH + 4); i<(j - 3); i++)              
                {
                    printf("%02x", dpack3->data[i]);
                }
                printf("\n");
                tag_id = (uint32_t)&dpack3->data;    
                xQueueSend(tag_found_queue, &tag_id, portMAX_DELAY);
            }
            else
            {
                printf("rx data:");
                for(int i=0; i<j; i++)              
                {
                    printf(" %02x", dpack3->data[i]);
                }
                printf("\n");
            }
            printf("rx checksum: %02x \n", dpack3->check);
        }
    free(tagdata);
    uart_flush(UART_NUM_2);
    return rxBytes;
}

//GPIO interrupt handling task function
static void gpio_task(void* arg)
{
    uint32_t tag_id;
    
    gpio_set_level(GPIO_OUTPUT_IO_0, 1);                //set EN pin for RFID Reader High/On
    //gpio_set_level(GPIO_OUTPUT_IO_1, 1);                //keep the Relay OFF, it's Active Low trigger

    while(1) 
    {
        if(xQueueReceive(tag_found_queue, &tag_id, portMAX_DELAY))
        {
            gpio_set_level(GPIO_OUTPUT_IO_1, 0);        //Turn Relay ON, it's an Active Low trigger.
            printf("\n\t\tDOOR LOCKED!!!\n");
            //printf("\nRFID Tag: %d", tag_id);
            while(gpio_get_level(GPIO_INPUT_IO_0) != 0)
            {
                printf("\n \tPress STOP to Unlock Door.\n");
                vTaskDelay(100 / portTICK_RATE_MS);
            }
            vTaskDelay(RELAY_TRIGGER_TIME / portTICK_RATE_MS);
            gpio_set_level(GPIO_OUTPUT_IO_1, 1);
        }
    }
}

//RFID UART communication task function.
static void rfid_task()
{   
            //resetVM_5F();
            //getData();
            //vTaskDelay(100 / portTICK_PERIOD_MS);
            setFreqRegion();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            getData();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            
            setBaudRate();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            getData();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            
            setDRM();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            getData();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            
            setAntDetect();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            getData();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            
            setWorkAntenna();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            getData();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            
            setOutputPower();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            getData();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            
            getFirmware();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            getData();
            vTaskDelay(100 / portTICK_PERIOD_MS);

            printf("\n\t***SYSTEM READY***\n\n");
            
    while(1)
    {
        real_time_inventory();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        TagDetect();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        
        setWorkAntenna();
        vTaskDelay(100 / portTICK_PERIOD_MS);
        getData();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    init();
    gpio_setup();
    //create a queue to handle gpio event from isr
    tag_found_queue = xQueueCreate(10, sizeof(uint32_t)); 
    //start gpio task
    xTaskCreate(gpio_task, "gpio_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
    //start uart task
    xTaskCreate(rfid_task, "uart_rfid_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}