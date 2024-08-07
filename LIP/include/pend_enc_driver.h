/*
 * Description: Wrapper for AS5600 magnetic encoder driver functionality
 * 
 * Main driver source: https://github.com/hepingood/as5600
 * 
 * GPIOs used: PB8 for i2c1 scl (alias I2C1_SCL) 
 *             PB9 for i2c1 sda (alias I2C1_SDA)
 *
 */

#include "driver_as5600_interface.h"

#ifndef PEND_ENC_DRIVER
#define PEND_ENC_DRIVER

#define PI  3.1415926536f
#define PI2 6.2831853072f

uint8_t pend_enc_init( void );

uint8_t pend_enc_read_angle_deg( float *angle );
uint8_t pend_enc_read_angle_rad( float *angle );

uint8_t pend_enc_deinit( void );

int32_t pend_enc_get_cumulative_count( void );

int32_t pend_enc_get_base_count( void );

/* Get number of full pendulum revolutions, negative number indicates negative revolution. */
int32_t get_num_of_revolutions( void );

#endif // PEND_ENC_DRIVER