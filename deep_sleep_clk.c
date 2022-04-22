/*
* Copyright (C) 2019 Center for Industry 4.0
* All Rights Reserved
*
* Center_for_Industry_4.0_LICENSE_PLACEHOLDER
* Desarrolladores: Enrique Germany, Luciano Radrigan
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "soc/rtc.h"
#include "deep_sleep_clk.h"

// Codigo provisto por el profesor //

void deep_sleep_clk(float operation_time,int32_t Time_to_wake_up){
    uint64_t wakeup_time_sec;

    printf("Time_to_wake_up %d \n", Time_to_wake_up);
    printf("operation_time %f \n", operation_time);
    if((Time_to_wake_up-operation_time)>0){
        wakeup_time_sec = (uint64_t)(Time_to_wake_up-operation_time);  
    }
    else{
        wakeup_time_sec = 60;
        printf("tiempo de ejecucion");
    }
    
    //const int wakeup_time_sec2 = Time_to_wakeup_sec2-operation_time;
    printf("Enabling timer wakeup, %lld\n", wakeup_time_sec); // wakeup_time_sec2 para pruebas cada 30 segundos [definida en .h]
    esp_sleep_enable_timer_wakeup(wakeup_time_sec*1000000); //* 1.000.000
    printf("goin to sleep for clk...");
    esp_deep_sleep_start();
}
