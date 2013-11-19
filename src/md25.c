/*
 * md25.c
 *
 *  Created on: Nov 15, 2013
 *      Author: alexandermertens
 */


/* *** INCLUDES ************************************************************** */
#include "md25.h"

#include "lib/twi_master.h"

/* *** DECLARATIONS ********************************************************** */

/* local type and constants     */

//-------------------Defines--------------------//
#define MD25_REGISTER_MOTOR1_SPEED			0x00
#define MD25_REGISTER_MOTOR2_SPEED			0x01
#define MD25_REGISTER_ENC1A					0x02
#define MD25_REGISTER_ENC1B					0x03
#define MD25_REGISTER_ENC1C					0x04
#define MD25_REGISTER_ENC1D					0x05
#define MD25_REGISTER_ENC2A					0x06
#define MD25_REGISTER_ENC2B					0x07
#define MD25_REGISTER_ENC2C					0x08
#define MD25_REGISTER_ENC2D					0x09
#define MD25_REGISTER_BATTERY_VOLTS			0x0A
#define MD25_REGISTER_MOTOR1_CURRENT		0x0B
#define MD25_REGISTER_MOTOR2_CURRENT		0x0C
#define MD25_REGISTER_SOFTWARE_REVISION		0x0D
#define MD25_REGISTER_ACCELERATION_RATE		0x0E
#define MD25_REGISTER_MODE					0x0F
#define MD25_REGISTER_COMMAND				0x10

#define MD25_CMD_RESET_ENCODERS				0x20
#define MD25_CMD_DISABLE_SPEED_REGULATION	0x30
#define MD25_CMD_ENABLE_SPEED_REGULATION	0x31
#define MD25_CMD_DISABLE_I2C_TIMEOUT		0x32
#define MD25_CMD_ENABLE_I2C_TIMEOUT			0x33

#define MD25_TWI_ADRESS						(0xB0>>1)

/* local function declarations  */


/* *** FUNCTION DEFINITIONS ************************************************** */
void md25_set_speed(int8_t motor1, int8_t motor2)
{
	twi_send_buffer[0] = MD25_REGISTER_MOTOR1_SPEED;
	twi_send_buffer[1] = motor1;
	twi_send_buffer[2] = motor2;

	twi_master_set_ready();
	twi_send_data(MD25_TWI_ADRESS, 3);
}


void md25_set_motor1_speed(int8_t speed)
{
	twi_master_write_register(MD25_TWI_ADRESS,
							  MD25_REGISTER_MOTOR1_SPEED,
							  speed);

}

void md25_set_motor2_speed(int8_t speed)
{
	twi_master_write_register(MD25_TWI_ADRESS,
							  MD25_REGISTER_MOTOR2_SPEED,
							  speed);

}

int8_t md25_get_motor1_speed(void)
{
	return twi_master_read_register(MD25_TWI_ADRESS,
									MD25_REGISTER_MOTOR1_SPEED);
}

int8_t md25_get_motor2_speed(void)
{
	return twi_master_read_register(MD25_TWI_ADRESS,
									MD25_REGISTER_MOTOR2_SPEED);
}

void md25_set_acceleration_rate(uint8_t accerlation)
{
	if(accerlation >= MD25_ACCELERATION_MIN ||
	   accerlation <= MD25_ACCELERATION_MAX) {

		twi_master_write_register(MD25_TWI_ADRESS,
								  MD25_REGISTER_ACCELERATION_RATE,
								  accerlation);
	}
}

uint8_t md25_get_acceleration_rate(void)
{
	return twi_master_read_register(MD25_TWI_ADRESS,
									MD25_REGISTER_ACCELERATION_RATE);
}


void md25_set_mode(uint8_t mode)
{


	//TODO: implement mode 3 and mode 4
	if(mode == MD25_MODE_SIGNED_SPEED   ||
	   mode == MD25_MODE_UNSIGNED_SPEED ||
	   mode == MD25_MODE_SIGNED_TURN    ||
	   mode == MD25_MODE_UNSIGNED_TURN)
	{
		twi_master_write_register(MD25_TWI_ADRESS,
								  MD25_REGISTER_MODE,
								  mode);
	}
}

uint8_t md25_get_mode(void)
{
	return twi_master_read_register(MD25_TWI_ADRESS,
									MD25_REGISTER_MODE);
}

uint32_t md25_get_motor_encoder(uint8_t motor)
{
	uint32_t enc_val;
	uint32_t temp_data;

	if(motor == MD25_MOTOR1) {
		twi_send_buffer[0] = MD25_REGISTER_ENC1A;
	} else {
		twi_send_buffer[0] = MD25_REGISTER_ENC2A;
	}

	twi_master_set_ready();
	twi_send_data(MD25_TWI_ADRESS, 1);
	twi_receive_data(MD25_TWI_ADRESS, 4);

	temp_data  = twi_receive_buffer[0];
	enc_val    = (temp_data << 24);

	temp_data  = twi_receive_buffer[1];
	enc_val   |= (temp_data << 16);

	temp_data  = twi_receive_buffer[2];
	enc_val   |= (temp_data << 8);

	enc_val   |= (uint32_t)(twi_receive_buffer[3]);


	return enc_val;
}

uint8_t md25_get_battery_volts(void)
{
	return twi_master_read_register(MD25_TWI_ADRESS,
									MD25_REGISTER_BATTERY_VOLTS);
}

uint8_t md25_get_motor_current(uint8_t motor)
{
	if(motor == MD25_MOTOR1) {
		return twi_master_read_register(MD25_TWI_ADRESS,
										MD25_REGISTER_MOTOR1_CURRENT);
	} else {
		return twi_master_read_register(MD25_TWI_ADRESS,
										MD25_REGISTER_MOTOR1_CURRENT);
	}
}

uint8_t md25_get_software_revision(void)
{
	return twi_master_read_register(MD25_TWI_ADRESS,
									MD25_REGISTER_SOFTWARE_REVISION);
}

void md25_cmd_reset_encoders(void)
{
	twi_master_write_register(MD25_TWI_ADRESS,
							  MD25_REGISTER_COMMAND,
							  MD25_CMD_RESET_ENCODERS);
}


void md25_cmd_enable_speed_regulation(bool enable)
{
	if(enable) {
		twi_master_write_register(MD25_TWI_ADRESS,
								  MD25_REGISTER_COMMAND,
								  MD25_CMD_ENABLE_SPEED_REGULATION);
	} else {
		twi_master_write_register(MD25_TWI_ADRESS,
								  MD25_REGISTER_COMMAND,
								  MD25_CMD_DISABLE_SPEED_REGULATION);
	}
}


void md25_cmd_enable_i2c_timout(bool enable)
{
	if(enable) {
		twi_master_write_register(MD25_TWI_ADRESS,
								  MD25_REGISTER_COMMAND,
								  MD25_CMD_ENABLE_I2C_TIMEOUT);
	} else {
		twi_master_write_register(MD25_TWI_ADRESS,
								  MD25_REGISTER_COMMAND,
								  MD25_CMD_DISABLE_I2C_TIMEOUT);
	}
}

void md25_init()
{
	md25_set_mode(MD25_MODE_SIGNED_SPEED);

	md25_set_acceleration_rate(MD25_ACCELERATION_MAX);

	md25_set_speed(0,0);
}
