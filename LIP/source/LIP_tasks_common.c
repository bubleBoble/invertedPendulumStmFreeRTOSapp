/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * This file provides default LIPcreateTasks function that creates default tasks.
 * Global variables which are used by more than one task are defined here.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "LIP_tasks_common.h"

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Globals used by all tasks.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
/* Holds data from ADC3 tranfered over DMA, init code is in motor_driver.c/dcm_init(). */
volatile uint16_t adc_data_pot;

/* holds each byte received from console (uart3) */
uint8_t cRxedChar = 0x00;

/* These are made global but only basic_test_task will write to these
Only controller_task should read these
Pendulum magnetic encoder reading. */
float pend_angle[ 2 ] = { 0.0f };   // Angle current & previous sample
float pend_speed[ 2 ] = { 0.0f };   // Angle derivative
IIR_filter low_pass_IIR_pend;

/* These are made global but only basic_test_task will write to them
Only controller_task should read them
DCM encoder reading */
float cart_position[ 2 ] = { 0.0f };
float cart_speed[ 2 ];
IIR_filter low_pass_IIR_cart;

/* Cart position setpoint from adc reading, converetd to [0, 47] range in cm.
[ 0 ] is current, [ 1 ] is previous sample. */
float cart_position_setpoint_cm_pot_raw; // raw read
float cart_position_setpoint_cm_pot;     // low pass filtered
/* Cart position setpoint set by cli command, range [0, 47] in cm.
[ 0 ] is current, [ 1 ] is previous sample. */
float cart_position_setpoint_cm_cli_raw; // raw read
float cart_position_setpoint_cm_cli;     // low pass filtered
/* This variable points to cart position setpoint from selected source, so either
cart_position_setpoint_cm_pot or cart_position_setpoint_cm_cli. This setpoint is used
by controllers. By default it points to converted potentiometer reading. */
float *cart_position_setpoint_cm = &cart_position_setpoint_cm_pot;
/* Pendulum magnetic encoder reading at down position. Can sometimes be different if
the pendulum shaft is forced to rotate inside a bearings. The default pendulum
position is assumed to be down position, from control/model perspective down
position corresponds to PI radians, so PI has to be subtracted from initial reading and
resultant value is offset that has to subtracted from each angle reading */
float pend_init_angle_offset;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* Watchdog task - protection for cart min and max positions and default always running task. */
TaskHandle_t watchdogTaskHandle = NULL;
StackType_t WATCHDOG_STACKBUFFER [ WATCHDOG_STACK_DEPTH ];
StaticTask_t WATCHDOG_TASKBUFFER_TCB;

/* Console task */
TaskHandle_t consoleTaskHandle;
StackType_t console_STACKBUFFER[ CONSOLE_STACKDEPTH ];
StaticTask_t console_TASKBUFFER_TCB;
TickType_t time_at_which_consoleMutex_was_taken;

/* State estimation task */
TaskHandle_t utilTaskHandle = NULL;
StackType_t utilTask_STACKBUFFER [ UTIL_STACK_DEPTH ];
StaticTask_t utilTask_TASKBUFFER_TCB;

/* Communication task. */
TaskHandle_t comTaskHandle = NULL;
StackType_t COM_STACKBUFFER [ COM_STACK_DEPTH ];
StaticTask_t COM_TASKBUFFER_TCB;

/* Communication task - for raw bytes transmission. */
TaskHandle_t rawComTaskHandle = NULL;
StackType_t RAWCOM_STACKBUFFER [ RAWCOM_STACK_DEPTH ];
StaticTask_t RAWCOM_TASKBUFFER_TCB;

/* Controllers test tasks */
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
/* Controller 1 task 
Basic full state feedback down position, output voltage is zero in specified deadzone. */
TaskHandle_t ctrl_FSF_downpos_task_handle = NULL;
StackType_t ctrl_FSF_downpos_STACKBUFFER [ CTRL_FSF_DOWNPOS_STACK_DEPTH ];
StaticTask_t ctrl_FSF_downpos_TASKBUFFER_TCB;

/* Controller 2 task
Full state feedback down position with better deadzone compensation, 
nonlinear cart position gain "hard switching". */
TaskHandle_t ctrl_2_FSF_downpos_task_handle = NULL;
StackType_t ctrl_2_FSF_downpos_STACKBUFFER [ CTRL_2_FSF_DOWNPOS_STACK_DEPTH ];
StaticTask_t ctrl_2_FSF_downpos_TASKBUFFER_TCB;

/* Controller 3 task
Full state feedback down position with better deadzone compensation, 
nonlinear cart position gain "tanh switching". */
TaskHandle_t ctrl_3_FSF_downpos_task_handle = NULL;
StackType_t ctrl_3_FSF_downpos_STACKBUFFER [ CTRL_3_FSF_DOWNPOS_STACK_DEPTH ];
StaticTask_t ctrl_3_FSF_downpos_TASKBUFFER_TCB;

/* Controller 4 task
Full state feedback with integral action on cart position error */
TaskHandle_t ctrl_4_I_FSF_downpos_task_handle = NULL;
StackType_t ctrl_4_I_FSF_downpos_STACKBUFFER [ CTRL_4_I_FSF_DOWNPOS_STACK_DEPTH ];
StaticTask_t ctrl_4_I_FSF_downpos_TASKBUFFER_TCB;

/* Controller 5 task
Full state feedback up position with deadzone compensation, 
nonlinear cart position gain "tanh switching". */
TaskHandle_t ctrl_5_FSF_uppos_task_handle = NULL;
StackType_t ctrl_5_FSF_uppos_STACKBUFFER [ CTRL_5_FSF_UPPOS_STACK_DEPTH ];
StaticTask_t ctrl_5_FSF_uppos_TASKBUFFER_TCB;
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

/* CREATE TASKS
Should it be moved to main_LIP.c ? - yes if it will only create one task. */
void LIPcreateTasks()
{
    watchdogTaskHandle = xTaskCreateStatic( watchdogTask,
                                            (const char*) "WatchdogTask",
                                            WATCHDOG_STACK_DEPTH,
                                            (void *) 0,
                                            tskIDLE_PRIORITY+PRIORITY_WATCHDOG,
                                            WATCHDOG_STACKBUFFER,
                                            &WATCHDOG_TASKBUFFER_TCB );

    /* Task that implements FreeRTOS console functionality */
    consoleTaskHandle = xTaskCreateStatic( vCommandConsoleTask,
                                           (const char*) "ConsoleTask",
                                           CONSOLE_STACKDEPTH,
                                           (void *) 0,
                                           tskIDLE_PRIORITY+PRIORITY_CONSOLE,
                                           console_STACKBUFFER,
                                           &console_TASKBUFFER_TCB );

    /* State variables are calculated here. */
    utilTaskHandle = xTaskCreateStatic( utilTask,
                                        (const char*) "UtilTask",
                                        UTIL_STACK_DEPTH,
                                        (void *) 0,
                                        tskIDLE_PRIORITY+PRIORITY_UTIL,
                                        utilTask_STACKBUFFER,
                                        &utilTask_TASKBUFFER_TCB );


    /* Human readable communication - for serialoscilloscope. */
    comTaskHandle = xTaskCreateStatic( comTask,
                                       (const char*) "CommunicationTask",
                                       COM_STACK_DEPTH,
                                       (void *) 0,
                                       tskIDLE_PRIORITY+PRIORITY_COM,
                                       COM_STACKBUFFER,
                                       &COM_TASKBUFFER_TCB );
    /* Data streaming is suspended right after task creation */
    vTaskSuspend( comTaskHandle );

    /* Down position controller 3
    Full state feedback down position ctrl-er with "tanh switching" deadzone compensation 
    (nonlinear cart position gain). */
    ctrl_3_FSF_downpos_task_handle = xTaskCreateStatic( ctrl_3_FSF_downpos_task,
                                                        (const char*) "Controller3DownPosTask",
                                                        CTRL_3_FSF_DOWNPOS_STACK_DEPTH,
                                                        (void *) 0,
                                                        tskIDLE_PRIORITY+PRIORITY_CTRL,
                                                        ctrl_3_FSF_downpos_STACKBUFFER,
                                                        &ctrl_3_FSF_downpos_TASKBUFFER_TCB );
    /* All controller tasks are suspended right after thier creation */  
    vTaskSuspend( ctrl_3_FSF_downpos_task_handle );

    /* Raw byte communication task. */
    // rawComTaskHandle = xTaskCreateStatic( rawComTask,
    //                                       (const char*) "RawCommunicationTask",
    //                                       RAWCOM_STACK_DEPTH,
    //                                       (void *) 0,
    //                                       tskIDLE_PRIORITY + 2,
    //                                       RAWCOM_STACKBUFFER,
    //                                       &RAWCOM_TASKBUFFER_TCB);

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    /* Down position controllers test tasks */
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    /* Controller 1 task 
    Basic full state feedback down position, output voltage is zero in specified deadzone. */
    // ctrl_FSF_downpos_task_handle = xTaskCreateStatic( ctrl_FSF_downpos_task,
    //                                                   (const char*) "ControllerDownPosTask",
    //                                                   CTRL_FSF_DOWNPOS_STACK_DEPTH,
    //                                                   (void *) 0,
    //                                                   tskIDLE_PRIORITY+3,
    //                                                   ctrl_FSF_downpos_STACKBUFFER,
    //                                                   &ctrl_FSF_downpos_TASKBUFFER_TCB );

    /* Controller 2 task
    Full state feedback down position with better deadzone compensation, 
    nonlinear cart position gain "hard switching". */
    // ctrl_2_FSF_downpos_task_handle = xTaskCreateStatic( ctrl_2_FSF_downpos_task,
    //                                                   (const char*) "Controller2DownPosTask",
    //                                                   CTRL_2_FSF_DOWNPOS_STACK_DEPTH,
    //                                                   (void *) 0,
    //                                                   tskIDLE_PRIORITY+3,
    //                                                   ctrl_2_FSF_downpos_STACKBUFFER,
    //                                                   &ctrl_2_FSF_downpos_TASKBUFFER_TCB );

    /* Controller 3 task
    Full state feedback down position with better deadzone compensation, 
    nonlinear cart position gain "tanh switching". */
    // ctrl_3_FSF_downpos_task_handle = xTaskCreateStatic( ctrl_3_FSF_downpos_task,
    //                                                   (const char*) "Controller3DownPosTask",
    //                                                   CTRL_3_FSF_DOWNPOS_STACK_DEPTH,
    //                                                   (void *) 0,
    //                                                   tskIDLE_PRIORITY+3,
    //                                                   ctrl_3_FSF_downpos_STACKBUFFER,
    //                                                   &ctrl_3_FSF_downpos_TASKBUFFER_TCB );
    // /* Controller task is suspended right after task creation */  
    // vTaskSuspend( ctrl_3_FSF_downpos_task_handle );

    /* Controller 4 task
    Full state feedback with integral action on cart position error */
    // ctrl_4_I_FSF_downpos_task_handle = xTaskCreateStatic( ctrl_4_I_FSF_downpos_task,
    //                                                       (const char*) "Controller4DownPosTask",
    //                                                       CTRL_4_I_FSF_DOWNPOS_STACK_DEPTH,
    //                                                       (void *) 0,
    //                                                       tskIDLE_PRIORITY+3,
    //                                                       ctrl_4_I_FSF_downpos_STACKBUFFER,
    //                                                       &ctrl_4_I_FSF_downpos_TASKBUFFER_TCB );

    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */    
    /* UP position controllers test tasks */
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    /* Controller 5 task
    Full state feedback up position with deadzone compensation, 
    nonlinear cart position gain "tanh switching". */
    // ctrl_5_FSF_uppos_task_handle = xTaskCreateStatic( ctrl_5_FSF_uppos_task,
    //                                                   (const char*) "Controller5UpPosTask",
    //                                                   CTRL_5_FSF_UPPOS_STACK_DEPTH,
    //                                                   (void *) 0,
    //                                                   tskIDLE_PRIORITY+3,
    //                                                   ctrl_5_FSF_uppos_STACKBUFFER,
    //                                                   &ctrl_5_FSF_uppos_TASKBUFFER_TCB );
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
}