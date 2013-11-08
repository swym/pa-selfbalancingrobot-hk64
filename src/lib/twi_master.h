#define TWI_TWBR_VALUE_400 3 /**<Defines the TWBR value for 400 kHz TWI operation*/
#define TWI_TWBR_VALUE_100 18/**<Defines the TWBR value for 100 kHz TWI operation*/
#define TWI_TWBR_VALUE_25 78
#define TWI_TWPS_VALUE 1/**<The prescaler is always 1 correspondig to the 
							following equation:\n SCL frequency = CPU clock 
							/ 16 + 2 * TWBR * (4 ^ TWPS)*/

#define BUFFER_SIZE 10

#include <avr/io.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>

extern volatile uint8_t twi_receive_buffer[BUFFER_SIZE];
extern volatile uint8_t twi_send_buffer[BUFFER_SIZE];


extern void twi_master_init();
extern void twi_send_data(uint8_t slave, uint8_t anz_bytes);
extern void twi_receive_data(uint8_t slave, uint8_t anz_bytes);
extern void twi_master_set_ready();

extern uint8_t twi_master_read_register(uint8_t, uint8_t); /*uint8_t slave_address, uint8_t register_address*/
extern void twi_master_write_register(uint8_t, uint8_t, uint8_t); /* uint8_t slave_address, uint8_t register_address, uint8_t value */
extern bool twi_master_read_register_bit(uint8_t, uint8_t, uint8_t); /* uint8_t slave_address, uint8_t register_address, uint8_t bit */
extern void twi_master_write_register_bit(uint8_t, uint8_t, uint8_t, bool); /* uint8_t slave_address, uint8_t register_address, uint8_t bit, bool enable */
