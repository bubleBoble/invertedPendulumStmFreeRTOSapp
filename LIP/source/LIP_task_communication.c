/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This file provides task that implements uC to PC communication over uart. Data is 
 * sent in form of human readable chars, HAL_UART_Transmit_IT (non-blocking mode) 
 * is used. This way really helpful app can be used to quickly analyze generated data.
 *     serial oscilloscope: https://x-io.co.uk/serial-oscilloscope/
 * 
 * For raw byte transmission, raw_com_task() task is provided.
 * 
 * This task only reads global state and related variables defined in LIP_tasks_common.h.
 * This task shouldn't write to these variables.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#include "LIP_tasks_common.h"

/* These are defined in LIP_tasks_common.c */
extern float pend_angle[ 2 ];
extern float pend_speed_raw[ 2 ];
extern float pend_speed[ 2 ];
extern float cart_position[ 2 ];
extern float cart_speed_raw[ 2 ];
extern float cart_speed[ 2 ];
extern float *cart_position_setpoint_cm;
extern float number_of_pendulumarm_revolutions_dpc;
extern float pendulum_angle_in_base_range_dpc;
extern float number_of_pendulumarm_revolutions_upc;
extern float pendulum_angle_in_base_range_upc;
extern float pendulum_arm_angle_setpoint_rad_dpc;
extern float pendulum_arm_angle_setpoint_rad_upc;

#ifdef COM_SEND_CTRL_DEBUG
    extern float ctrl_xw;
    extern float ctrl_th;
    extern float ctrl_Dx;
    extern float ctrl_Dt;
#endif

void com_task( void *pvParameters )
{
    /* For RTOS vTaskDelayUntil() */
    TickType_t xLastWakeTime = xTaskGetTickCount();

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * For serial osciloscope - testing/debug purposes
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    char msg[ 256 ]; // msg for uart data transfer
    
    /* Task mainloop */
    for (;;)
    {
        #ifdef COM_SEND_CTRL_DEBUG
            sprintf( msg,
                    "%f,%f,0,%f,%f,0\r\n", 
                    ( double ) ctrl_xw,
                    ( double ) ctrl_th,
                    ( double ) ctrl_Dx,
                    ( double ) ctrl_Dt
            );

            /* Serial send */
            com_send( msg, strlen(msg) );

            vTaskDelay( dt_com );
            xLastWakeTime = xTaskGetTickCount();
        #endif /* COM_SEND_CTRL_DEBUG */

        #ifdef COM_SEND_UPC
            sprintf( msg,
                    "%f,%f,%f,%f,%f,%f,%f,%f,%ld\r\n",
                    // for pendulum
                    ( double ) pend_angle[ 0 ], 
                    ( double ) pendulum_arm_angle_setpoint_rad_upc,
                    ( double ) pend_speed[ 0 ],
                    // for cart
                    ( double ) cart_position[ 0 ],
                    ( double ) cart_speed[ 0 ],
                    ( double ) *cart_position_setpoint_cm,
                    //
                    ( double ) dcm_get_output_voltage(),
                    ( double ) number_of_pendulumarm_revolutions_upc,
                    xLastWakeTime
            );
            com_send( msg, strlen(msg) );        
            vTaskDelay( dt_com );
            xLastWakeTime = xTaskGetTickCount();
        #endif /* COM_SEND_UPC */

        #ifdef COM_SEND_DPC
            sprintf( msg,
                    "%f,%f,%f,%f,%f,%f,%f,%f,%f\r\n",
                    // for pendulum
                    ( double ) pend_angle[ 0 ],
                    ( double ) pend_speed[ 0 ],
                    ( double ) pendulum_arm_angle_setpoint_rad_dpc,
                    // for cart
                    ( double ) cart_position[ 0 ],
                    ( double ) cart_speed[ 0 ],
                    ( double ) *cart_position_setpoint_cm,
                    //
                    ( double ) dcm_get_output_voltage(),
                    ( double ) number_of_pendulumarm_revolutions_dpc,
                    xLastWakeTime
            );
            com_send( msg, strlen(msg) );        
            vTaskDelay( dt_com );
            xLastWakeTime = xTaskGetTickCount();
        #endif /* COM_SEND_DPC */

        #ifdef COM_SEND_DEFAULT
            /* Message content */
            sprintf( msg,
                    "%f,%f,0,%f,%f,0,%f,%ld\r\n",
                    // for pendulum
                    ( double ) pend_angle[ 0 ],
                    // ( double ) pend_speed_raw[ 0 ],
                    ( double ) pend_speed[ 0 ],
                    // for cart
                    ( double ) cart_position[ 0 ],
                    // ( double ) cart_speed_raw[ 0 ],
                    ( double ) cart_speed[ 0 ],
                    // ( double ) *cart_position_setpoint_cm,
                    //
                    ( double ) dcm_get_output_voltage(),
                    xLastWakeTime
            );

            /* Serial send */
            com_send( msg, strlen(msg) );
        
            /* Problem with vTaskDelayUntil: 
            vTaskDelayUntil uses xLastWakeTime argument to 
            calculate next wakeup time, it increments its value internally.
            If the task is suspended, value of xLastWakeTime doesn't get 
            incremented, so when task gets resumed, tickCount maybe for eg.1000, 
            and last saved xLastWakeTime might have value 100, 
            with delay tick count of 100, then, vTaskDelayUntil
            has to be called at least 10 times to increment xLastWakeTime to the
            value of current tickCount. */  
            // vTaskDelayUntil( &xLastWakeTime, dt_com );
            
            /* This delay function doesn't guarantee exact tick delay
            eq. When tested with dt_com=50 (ms), messages were received
            with frequency 18Hz (not 20Hz as expected). So use this 
            function only if data logging rate isn't a great concern. */
            vTaskDelay( dt_com );
            xLastWakeTime = xTaskGetTickCount();
        #endif /* COM_SEND_DEFAULT */
    }
}
