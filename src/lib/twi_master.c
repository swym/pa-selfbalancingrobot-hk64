#include "twi_master.h"

#include <avr/io.h>


#include "../common.h"
#include "../timer.h"


//----------- Global-Variables ----------//
volatile uint8_t twi_receive_buffer[BUFFER_SIZE];
volatile uint8_t twi_send_buffer[BUFFER_SIZE];

volatile uint8_t timer_twi_ready_timeout;
//----------- Modul-Variables -----------//
volatile uint8_t transmitter;
volatile uint8_t rindx;
volatile uint8_t sindx;
volatile uint8_t number_of_bytes;
volatile uint8_t ready;
volatile uint8_t sla;

/**
\brief Setup the twi module.

This function sets up the twi module for master operation. Therefore we have to write proper values to the following registers:<p>
- TWBR      Two Wire Bitrate Register
- TWSR      Two Wire Status Register
- TWCR      Two Wire Control Register
<p>
TWBR and TWSR are used to configure the bus clock speed.
The TWI specifications allows us to choose between two different clock speed modes: one below 100kHz and one up to 400kHz.<br>
The following equation shows how to set up the clock speed:<p>
SCL frequency = CPU clock / 16 + 2 * TWBR * (4 ^ TWPS)<p>
According to the equation we choosed TWBR = 18 and TWSR = 1 for 100kHz operations.<br>
\code
TWBR = TWI_TWBR_VALUE_100;
TWSR &= 0b11111100;
TWSR |= 0x01;
\endcode<br>
As we can see in this tiny code fragment, the TWPS value is encoded by the two last bits of the TWSR register.
*/
void twi_master_init()
{


	TWBR = TWI_TWBR_VALUE_100;
	TWSR &= 0b11111100;
	TWSR |= 0x01;
	TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWIE);

}

/**
\brief Method to send data to a slave.
\param slave contains the slave adress
\param anz_bytes contains the amount of bytes that have to be transmitted

The data to be transmitted is stored in the global variable send_buffer. send_buffer[] is an array of uint8_t types.
It's size is defined by the constant BUF_SIZE. The declaration of send_buffer is located in the twi_master.h file.
<p>
If one wants to transmit data to a slave, he has to write the data to the send_buffer. After calling the send_data() method,
all bytes from send_buffer[0] to send_buffer[anz_bytes - 1] will be transmitted to the slave without interruption.

\code
extern void send_data(uint8_t slave, uint8_t anz_bytes)
{
	ready = false;
	transmitter = true;
	sla = slave;
	number_of_bytes = anz_bytes;
	sindx = 0;
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
	while(!ready){}
}
\endcode
Code explanation in detail:
<p>
\code
ready = false;
\endcode
The <b>ready</b> variable is used to guarantee, that the whole transmission of every bytes is done without interruption.
The program flow within the main method of this master controller is halted until all bytes have been transmitted.
<p>
\code
transmitter = true;
\endcode
By setting the variable <b>transmitter</b> to <i>TRUE</i>, the ISR of the master controller can determine wether Master Transmitter
or Master Receiver mode is active.
<p>
\code
sla = slave;
\endcode
Store the slave adress in the global variable sla to make it visible for the ISR.
Same with anz_bytes.
<p>
\code
sindx = 0;
\endcode
Set the send_buffer index to zero.
<p>
\code
TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
\endcode
Here we can see how to manipulate the Two Wire Control Register.
\n
TWINT causes the TWI to trigger an interrupt so that the ISR executes.
\n
TWSTA cause the TWI to put a <i> start signal </i> on the TWI bus. All attached slave will recognize the
start signal.
\n
TWEN enables the TWI.
\n
TWIE Two Wire Interrupt Enable allows the TWI to trigger interrupts.
<p>
\code
while(!ready){}
The ready flag is set within the ISR when the TWI reaches an end state that indicates that the transmission
has finished.
\endcode
*/
void twi_send_data(uint8_t slave, uint8_t anz_bytes)
{
	PORT_SCOPE |= _BV(5);
	ready = false;
	transmitter = true;
	sla = slave;
	PORT_SCOPE &= ~_BV(5);

	PORT_SCOPE |= _BV(6);
	number_of_bytes = anz_bytes;
	sindx = 0;
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
	PORT_SCOPE &= ~_BV(6);

	PORT_SCOPE |= _BV(7);
	/**
	 * Wait for ready bit.
	 * HACK: break if timeout runs out.
	 */
	//while(!ready){}
	timer_twi_ready_timeout = TIMER_TWI_READY_TIMEOUT_MS;
	for(;;) {

		if(ready) {
			PORT_LED ^= _BV(0);
			break;
		}

		if(!timer_twi_ready_timeout) {
			PORT_LED |= 0xFF;
			ready = true;
			break;
		}
	}

	PORT_SCOPE &= ~_BV(7);
}

/**
\brief
Corresponding to send_data this method receives data the same way.
\param slave contains the slave adress
\param anz_bytes contains the amount of bytes that have to be transmitted

A global receive_buffer is used to store data that is received by TWI.
\code
extern void receive_data(uint8_t slave, uint8_t anz_bytes)
{
    ready = FALSE;
    transmitter = FALSE;
    sla = slave;
    number_of_bytes = anz_bytes;
    rindx = 0;
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
    while(!ready){}
}
\endcode
The only differences:
\n
<b>transmitter</b> is set to FALSE
\n
Instead of sindex we set rindex to zero.
*/
void twi_receive_data(uint8_t slave, uint8_t anz_bytes)
{
	ready = false;
	transmitter = false;
	sla = slave;
	number_of_bytes = anz_bytes;
	rindx = 0;
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN) | (1<<TWIE);
	while(!ready){}
}

void twi_master_set_ready()
{
	ready = true;
}

/**
\brief This Interrupt Service Routine handles all possible status conditions of the TWI hardware.

Everytime a bus operation has finished, the TWI modules triggers a TWI interrupt. In this case the
ISR(TWI_vect) is executed. By determing the twi status by reading the TWSR code, we can see in which
state of communication we are and take proper actions to continue the transmission.

A more detailed description can be found here: \ref isr_master
*/
ISR (TWI_vect)
{
	switch(TW_STATUS)
	{
		/*********************************************************
		CHECK TWO WIRE STATUS REGISTER AND TAKE APROPRIATE ACTIONS
		*********************************************************/
		case TW_START:	//SART CONDITION SENT
		case TW_REP_START:	//REPEATED START SENT
			if(transmitter)
			{
				//MASTER TRANSMITTER MODE
				TWDR = (sla<<1) | TW_WRITE;
				TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
			}
			else
			{
				//MASTER RECEIVER MODE
				TWDR = (sla<<1) | TW_READ;
				TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
			}
			break;



		/****************************
		BEGIN MASTER TRANSMITTER MODE
		*****************************/
		case TW_MT_SLA_ACK:
			if(number_of_bytes == 0)
			{
				//Nothing to transmit. Send Stop.
				TWCR = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
				ready = true;
			}
			else
			{
				TWDR = twi_send_buffer[sindx];
				TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
				number_of_bytes--;
				sindx++;
			}
			break;

		case TW_MT_SLA_NACK:

			//SLAVE ADR WAS NOT ACKNOWLEDGED
			if(number_of_bytes)
			{
				//STILL DATA TO BE SENT -> execute repeated start:
				TWCR = (1<<TWSTA) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);

			}
			else
			{
				//NOTHING TO SEND -> execute stop:
				TWCR = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
				ready = true;
			}
			break;

		case TW_MT_DATA_ACK:

			if(number_of_bytes == 0)
			{
				//Nothing to transmit. Send Stop.
				TWCR = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
				ready = true;
			}
			else
			{
				TWDR = twi_send_buffer[sindx];
				TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
				number_of_bytes--;
				sindx++;
			}
			break;

		case TW_MT_DATA_NACK:
			TWCR = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
			ready = true;
			break;

		case TW_MT_ARB_LOST:
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA) | (1<<TWSTA);
			break;
		/**************************
		END MASTER TRANSMITTER MODE
		***************************/

		/*************************
		BEGIN MASTER RECEIVER MODE
		**************************/
		case TW_MR_SLA_ACK:
			if(number_of_bytes <= 1)
			{
				TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
			}
			else
			{
				TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
			}
			break;

		case TW_MR_SLA_NACK:
			TWCR = (1<<TWSTA) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
			break;

		case TW_MR_DATA_ACK:
			twi_receive_buffer[rindx] = TWDR;
			number_of_bytes--;
			rindx++;
			if(number_of_bytes == 1)
			{
				TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
			}
			else
			{
				TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
			}
			break;

		case TW_MR_DATA_NACK:
			twi_receive_buffer[rindx] = TWDR;
			TWCR = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
			ready = true;
			break;

		case TW_BUS_ERROR:
				TWCR = (1<<TWSTO) | (1<<TWINT) | (1<<TWEN) | (1<<TWIE);
				ready = true;
			break;

		case TW_NO_INFO:
			break;

		default:
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA) | (1<<TWSTO);
			ready = true;
			break;
		/***********************
		END MASTER RECEIVER MODE
		************************/
	}
}


/* ****************
 *  HELPERFUNCTIONS
 * **************** */

// TODO: implement helperfunctions for byte-arrays

/**
 * Reads a full 8-bit register of a specifed slave
 * @param slave_adress
 * @param register_adress
 * @return read value
 */
uint8_t twi_master_read_register(uint8_t slave_address,
								 uint8_t register_address)
{
	uint8_t value;

	twi_send_buffer[0] = register_address;

	twi_master_set_ready();
	twi_send_data(slave_address, 1);
	twi_receive_data(slave_address, 1);

	value = twi_receive_buffer[0];

	return value;
}

/**
 * Writes a register value to a specifed slave
 * @param slave_address
 * @param register_address
 * @param value
 */
void twi_master_write_register(uint8_t slave_address,
							   uint8_t register_address,
							   uint8_t value)
{
	twi_send_buffer[0] = register_address;
	twi_send_buffer[1] = value;

	twi_master_set_ready();
	twi_send_data(slave_address, 2);
}


/**
 *
 * @param slave_address
 * @param register_address
 * @param bit
 * @return
 */
bool twi_master_read_register_bit(uint8_t slave_address,
								  uint8_t register_address,
								  uint8_t bit)
{
	uint8_t register_value;

	register_value = twi_master_read_register(slave_address, register_address);

	if(register_value & (1<<bit)) {
		return true;
	} else {
		return false;
	}
}


/**
 *
 * @param slave_address
 * @param register_address
 * @param bit
 * @param enable
 */
void twi_master_write_register_bit(uint8_t  slave_address,
							   uint8_t register_address,
							   uint8_t bit,
							   bool    enable)
{
	uint8_t register_value;

	/* read register and delete data_int*/
	register_value = twi_master_read_register(slave_address, register_address);
	register_value &= ~(1<<bit);

	if(enable) {
		register_value |= (1<<bit);
	}

	//set register
	twi_master_write_register(slave_address, register_address, register_value);
}
