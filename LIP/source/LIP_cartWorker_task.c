/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This file provides task that is only active when command "zero" or "home" is called.
 * Its purpose is to move the cart without any controller.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "LIP_tasks_common.h"
#include "limits.h"

/* Keeps track of current app state. */
extern enum lip_app_states app_current_state;

/* Global cart position variable, defined in LIP_tasks_common.c */
extern float cart_position[ 2 ];

/* Cart position setpoint, independent of currenlty selected setpoint source. 
defined in LIP_tasks_common.c */
extern float *cart_position_setpoint_cm;

void cartWorkerTask( void * pvParameters )
{
    /* Hold value retrieved from task notification. */
    uint32_t notif_value_received;

    for ( ;; )
    {   
        /* Wait for notification at index 0. */
        xTaskNotifyWaitIndexed( 0,                     /* Notification index */
                                0x00,                  /* Bits to clear on entry (before save to third arg). */ 
                                ULONG_MAX,             /* Bits to clear on exit (after save to third arg). */
                                &notif_value_received, /* Value of received notification. Can be set to NULL if not used. */
                                portMAX_DELAY );       /* Block time (while waiting for notification). */

        /* notif_value_received possible values: GO_RIGHT, GO_RIGHT+GO_LEFT, GO_LEFT, SP_HOME
           cart_position_calibrated*/
        if( notif_value_received == GO_RIGHT )
        {
            /* App is uninitialized state and cart is already in zero position OR
            app is in default state (cart position calibrated). 
            Move cart to the right until track center reached. */
            dcm_set_output_volatage( 2.0f );
            while( cart_position[ 0 ] < TRACK_LEN_MAX_CM/2 )
            {
                vTaskDelay( dt_cartworker );
            }
            dcm_set_output_volatage( 0.0f );
            app_current_state = DEFAULT;
        }
        else if ( notif_value_received == GO_LEFT )
        {
            /* App is in default state(cart position calibrated).
            Move cart to the left until center position reached. */
            dcm_set_output_volatage( -2.0f );
            while( cart_position[ 0 ] > TRACK_LEN_MAX_CM/2 )
            {
                vTaskDelay( dt_cartworker );
            }
            dcm_set_output_volatage( 0.0f );
        }
        else if( notif_value_received == GO_LEFT+GO_RIGHT )
        {
            /* App is in uninitialized state and cart position has to be calibrated. */
            dcm_set_output_volatage( -2.0f );
            while( ! READ_ZERO_POSITION_REACHED )
            {
                /* Go left until zero position reached. */
                vTaskDelay( dt_cartworker );
                // vTaskDelay( 10 );
            }
            dcm_set_output_volatage( 2.0f );
            while( cart_position[ 0 ] < TRACK_LEN_MAX_CM/2.0f )
            {
                /* Go to track center. */
                vTaskDelay( dt_cartworker );
                // vTaskDelay( 10 );
            }
            dcm_set_output_volatage( 0.0f );
            app_current_state = DEFAULT;
        }
        else if( notif_value_received == SP_HOME )
        {
            /* App is in UPC or DPC state. Change setpoint to home postion. 
            cart_position_setpoint_cm should point to setpoint set by cli command
            "spcli", otherwise cart behaviour maybe unexpected. This condition is
            handled in "spcli" command callback function. */
            *cart_position_setpoint_cm = TRACK_LEN_MAX_CM/2.0f;
        }

        /* Task delay */
        /* This task wont run all the time, so vTaskDelayUntil can't be used. */
        vTaskDelay( dt_cartworker );
    }
}