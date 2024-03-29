/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This file provides task that is only active when command "zero" or "home" is called.
 * Its purpose is to move the cart without any controller.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "LIP_tasks_common.h"
#include "limits.h"

/* Voltage value used to move cart without any controller. */
#define IDLE_MOVEMENTS_VOLTAGE 1.65f

/* Defined in LIP_tasks_common.c */
extern enum lip_app_states app_current_state;
extern float cart_position[ 2 ];
extern float *cart_position_setpoint_cm;
extern float cart_position_setpoint_cm_cli_raw;
extern uint32_t reset_home;

extern TaskHandle_t cartworker_TaskHandle;

void set_output_voltage_nobounce( float voltage );

void cart_worker_task( void * pvParameters )
{
    /* Holds value retrieved from task notification. */
    uint32_t notif_value_received;

    for ( ;; )
    {   
        /* Wait for notification at index 0. */
        xTaskNotifyWaitIndexed( 0,                     /* Notification index */
                                0x00,                  /* Bits to clear on entry (before save to third arg). */ 
                                ULONG_MAX,             /* Bits to clear on exit (after save to). */
                                &notif_value_received, /* Value of received notification. Can be set to NULL if not used. */
                                portMAX_DELAY );       /* Block time (while waiting for notification). */
                
        /* notif_value_received possible values: GO_RIGHT, GO_RIGHT+GO_LEFT, GO_LEFT, SP_HOME
           cart_position_calibrated*/
        if( notif_value_received == GO_RIGHT )
        {
            /* App is uninitialized state and cart is already in zero position OR
            app is in default state (cart position calibrated). 
            Move cart to the right until track center reached. */
            
            /* Debouncing. */
            set_output_voltage_nobounce( IDLE_MOVEMENTS_VOLTAGE );
            
            while( cart_position[ 0 ] < TRACK_LEN_MAX_CM/2 * 0.98f )
            {
                vTaskDelay( dt_cartworker );
            }
            dcm_set_output_volatage( 0.0f );

            /* Change app state from UNINITIALIZED to DEFAULT right after zero position is reached. */
            app_current_state = DEFAULT;
        }
        else if ( notif_value_received == GO_LEFT )
        {
            /* App is in default state(cart position calibrated).
            Move cart to the left until center position reached. */
            
            /* Debouncing. */
            set_output_voltage_nobounce( -IDLE_MOVEMENTS_VOLTAGE );
            
            while( cart_position[ 0 ] > TRACK_LEN_MAX_CM/2 * 1.02f )
            {
                vTaskDelay( dt_cartworker );
            }
            dcm_set_output_volatage( 0.0f );
        }
        else if( notif_value_received == GO_LEFT+GO_RIGHT )
        {
            /* App is in uninitialized state and cart position has to be calibrated. */
            
            /* Debouncing. */
            set_output_voltage_nobounce( -IDLE_MOVEMENTS_VOLTAGE );
            
            while( ! READ_ZERO_POSITION_REACHED )
            {
                /* Go left until zero position reached. */
                vTaskDelay( dt_cartworker );
            }
            app_current_state = DEFAULT;
            
            /* Debouncing. */
            set_output_voltage_nobounce( IDLE_MOVEMENTS_VOLTAGE );

            while( cart_position[ 0 ] < TRACK_LEN_MAX_CM/2.0f )
            {
                /* Go to track center. */
                vTaskDelay( dt_cartworker );
                // vTaskDelay( 10 );
            }
            dcm_set_output_volatage( 0.0f );
        }
        else if( notif_value_received == SP_HOME )
        {
            /* App is in UPC or DPC state. Change setpoint to home postion. Write new setpoint to
            *_raw cli setpoint (unfiltered). cart_position_setpoint_cm_cli_raw acts as input to 
            cart_position_setpoint_cm_cli low-pass filter. Low-pass filter is used for both setpoint 
            sources to smooth out discontinous input. */
            cart_position_setpoint_cm_cli_raw = TRACK_LEN_MAX_CM/2.0f;
        }

        /* Task delay.
        This task wont run all the time, so vTaskDelayUntil can't be used. */
        vTaskDelay( dt_cartworker );
    }
}

void set_output_voltage_nobounce( float voltage )
{
    /* Not elegant way of doing debouncing but for now it works. */
    dcm_set_output_volatage( voltage );
    vTaskDelay(10);
    dcm_set_output_volatage( voltage );
    vTaskDelay(10);
    dcm_set_output_volatage( voltage );
    vTaskDelay(10);
    dcm_set_output_volatage( voltage );
    vTaskDelay(10);
    dcm_set_output_volatage( voltage );
    vTaskDelay(10);
    dcm_set_output_volatage( voltage );
}