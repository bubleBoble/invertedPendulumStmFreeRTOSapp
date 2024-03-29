/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * When cart is in freezing zone (see watchdog task), app should 
 * instantly stop it. 
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
#include "LIP_tasks_common.h"
#include "math.h"

/* Defined in LIP_tasks_common.c. */
// extern float cart_position[ 2 ];
extern enum cart_position_zones cart_current_zone;
extern float cart_position_setpoint_cm_cli_raw;
extern uint32_t bounceoff_resumed;

float voltage_step;

#define N_STEPS_UP 20

void bounceoff_task( void *pvParameters )
{
    for( ;; )
    {
        if( cart_current_zone == FREEZING_ZONE_R )
        {
            // dcm_set_output_volatage( 0.0f );

            // voltage_step = 12.0f / N_STEPS_UP;
            // for( int32_t i=0; i<N_STEPS_UP; i++ )
            // {
            //     dcm_set_output_volatage( -i*voltage_step );
            //     vTaskDelay( 5 );
            // }
            // dcm_set_output_volatage( 0.0f );
            cart_position_setpoint_cm_cli_raw = TRACK_LEN_MAX_CM / 2.0f ;
        }
        else if( cart_current_zone == FREEZING_ZONE_L )
        {
            // dcm_set_output_volatage( 0.0f );

            // voltage_step = 12.0f / N_STEPS_UP;
            // for( int32_t i=0; i<N_STEPS_UP; i++ )
            // {
            //     dcm_set_output_volatage( i*voltage_step );
            //     vTaskDelay( 30 );
            // }
            // dcm_set_output_volatage( 0.0f );
            cart_position_setpoint_cm_cli_raw = TRACK_LEN_MAX_CM / 2.0f ;
        }

        bounceoff_resumed = 0;
        /* Suspend this task. */
        vTaskSuspend( NULL );
    }
}



