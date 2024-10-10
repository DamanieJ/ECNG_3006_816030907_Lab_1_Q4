#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h> // ask about this

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_sleep.h"

#include "driver/gpio.h"
#include "driver/hw_timer.h"

#include "driver/uart.h"


#define SLOTX       4
#define CYCLEX      5
#define SLOT_T      5000
#define BUF_SIZE    1024

#define TEST_ONE_SHOT    false        // testing will be done without auto reload (one-shot)
#define TEST_RELOAD      true         // testing will be done with auto reload


int count = 0;
int tps, cycle = 0, slot = 0;
//now is the current time. then is the previous time
int now, then1, bstart;
// TickType_t  now, then;
struct time_store {
    int min;
    int second;
    int millisec;

};
struct time_store current_time;


void hw_timer_callback1(void* arg)
{

    count++;
    if (count == 10) {
        current_time.millisec++;
        if (current_time.millisec == 1000) {
            current_time.second++;
            current_time.millisec = 0;
            if (current_time.second == 60) {
                current_time.min++;
                current_time.second = 0;
            }
        }
        count = 0;
    }



}


void cfg_serial() {

    uint8_t* data = (uint8_t*)malloc(BUF_SIZE);


    uart_config_t config_uart;

    config_uart.baud_rate = 9600;

    config_uart.data_bits = UART_DATA_8_BITS;

    config_uart.parity = UART_PARITY_DISABLE;

    config_uart.stop_bits = UART_STOP_BITS_1;

    config_uart.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;


    uart_param_config(UART_NUM_0, &config_uart);

    uart_driver_install(UART_NUM_0, BUF_SIZE * 2, 0, 0, NULL, 0);

}



void sleep_func(uint32_t how_long) {
    esp_sleep_enable_timer_wakeup(how_long);
    esp_light_sleep_start();
}

void one() {
    uint8_t* data = (uint8_t*)malloc(BUF_SIZE);

    const char* message = "Task one running \n";

    //uart_write_bytes(UART_NUM_0, data, strlen(data));//ask abt log

    uart_write_bytes(UART_NUM_0, message, strlen(message));
    sleep_func(1000000);
}

void two() {
    uint8_t* data = (uint8_t*)malloc(BUF_SIZE);

    const char* message = "Task two running \n";

    //uart_write_bytes(UART_NUM_0, data, strlen(data));//ask abt log

    uart_write_bytes(UART_NUM_0, message, strlen(message));


    sleep_func(1000000);


}

void three() {
    uint8_t* data = (uint8_t*)malloc(BUF_SIZE);

    const char* message = "Task three running \n";

    //uart_write_bytes(UART_NUM_0, data, strlen(data));//ask abt log

    uart_write_bytes(UART_NUM_0, message, strlen(message));
    sleep_func(1000000);

}

void four() {

    uint8_t* data = (uint8_t*)malloc(BUF_SIZE);

    const char* message = "Task four running \n";

    //uart_write_bytes(UART_NUM_0, data, strlen(data));//ask abt log

    uart_write_bytes(UART_NUM_0, message, strlen(message));
    sleep_func(1000000);

}

int time( struct time_store times) {       //store current time as integer as struct cannot be used in minus calc

    return times.second;
}

void burn() {
    uint8_t* data = (uint8_t*)malloc(BUF_SIZE);

    const char* message = "I am stuck in burn \n";
    uart_write_bytes(UART_NUM_0, message, strlen(message));


    now = time(current_time);
    while ((now = time(current_time) - then1) < 5) { //current time-previous time until is 5s
        /* burn time here */
    }
    printf("burn time = %2.2ds\n\n", (time(current_time) - now));
    then1 = now;
    cycle = CYCLEX;

}



void (*ttable[SLOTX][CYCLEX])() = {
        {one, two, burn, burn, burn},
        {one, three, burn, burn, burn},
        {one, four, burn, burn, burn},
        {burn, burn, burn, burn, burn}
};

void app_main() {
    tps = 1000;
    then1 = 0;
    current_time.millisec = 0;
    current_time.second = 0;
    current_time.min = 0;
    hw_timer_init(hw_timer_callback1, NULL);
    hw_timer_alarm_us(100, TEST_RELOAD);
    hw_timer_get_intr_type(1);
    hw_timer_set_reload(1);
    hw_timer_set_load_data(100);
    cfg_serial();

    tps = 10000; // number of clock ticks per second 
    printf("clock ticks/sec = %d\n\n", tps);
    while (1) {
        for (slot = 0; slot < SLOTX; slot++)
            for (cycle = 0; cycle < CYCLEX; cycle++)
                (*ttable[slot][cycle])();
    }
} // dispatch next task

