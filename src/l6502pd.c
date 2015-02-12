/*
 * l6502pd.c
 *
 *  Created on: Feb 6, 2015
 *      Author: alexandermertens
 */

#include <avr/io.h>
#include <avr/iom64.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#include "uart.h"

// PORT declaration
#define DELAY_MS		125

#define DDR_LED			DDRA
#define PORT_LED		PORTA

#define DDR_MCTRL		DDRC
#define PORT_MCTRL		PORTC

#define	DDR_MOTOR_PWM	DDRB
#define	PORT_MOTOR_PWM	PORTB

//#define PWM_M1_ORC		OCR1B
//#define PWM_M2_ORC		OCR1C

#define PWM_M1_OCR_ADDR	0x78
#define PWM_M2_OCR_ADDR	0x48

#define	DDR_MOTOR_AN	DDRF
#define	PORT_MOTOR_AN	PORTF

// PIN declaration
#define LED0			PA0
#define LED1			PA1
#define LED2			PA2
#define LED3			PA3
#define LED4			PA4
#define LED5			PA5
#define LED6			PA6
#define LED7			PA7

#define MCTRL_M11		PC4
#define MCTRL_M12		PC5
#define MCTRL_M21		PC6
#define MCTRL_M22		PC7

#define PWM_M1			PB6
#define PWM_M2			PB7

// MOTOR Symbol definitions

#define MOTOR_1			0
#define MOTOR_2			1

#define MCTRL_MODE_LEFT				0
#define MCTRL_MODE_RIGHT			1
#define MCTRL_MODE_HALT				2
#define MCTRL_MODE_STALL			3
#define MCTRL_MODE_FORWARD			6
#define MCTRL_MODE_BACKWARD			7

#define MOTOR_DEFAULT_RAMP_ACCEL	2

#define MOTOR_SPEED_MIN		(-255)
#define MOTOR_SPEED_MAX		255

// typedefs

typedef uint8_t motor_id_t;

typedef struct {
	int16_t speed_current;
	int16_t speed_setpoint;
	uint8_t ramp_acceleration;
	uint8_t mode;
	uint8_t mctrl_in1;
	uint8_t mctrl_in2;
	volatile uint16_t * pwm_ocr_ptr;
} motor_t;


// module variables

motor_t motor_1;
motor_t motor_2;

volatile uint8_t led_value = 0;


void init_pwm_timer(void)
{
	// Phase Correct PWM with Timer1 on PWM3 PB6/OC1B) and PWM4 (PB7/OC1C)

	TCCR1A |= _BV(WGM10);				// Mode Select: 1 - PWM, Phase Correct; TOP is 0x00FF

	TCCR1A |= _BV(COM1B1);				// Compare Output Mode for PWM3: Set on upcountung
	TCCR1A |= _BV(COM1C1);				// Compare Output Mode for PWM4: Set on upcountung

	TCCR1B |= _BV(CS10);				// Clock Select: clk/1

	OCR1B   = 0;						// initial compare value
	OCR1C	= 0;
}

void init_irq_timer(void)
{
	// IRQ Timer with Timer2 as "system ticker" (~16 ms)

	TCCR2  = _BV(WGM21);				// Mode Select: CTC (Clear Timer on Compare)

	TCCR2 |= _BV(CS21);					// Clock Select: clk/8

	TIMSK |= _BV(OCIE2);				// Enable CTC Match Interrupt

	OCR2  = 249;						// Set Output Compare Register value
}


void motor_set_speed(motor_t * m, int16_t new_speed)
{
	if(m != NULL) {

		if(new_speed > MOTOR_SPEED_MAX) {
			new_speed = MOTOR_SPEED_MAX;
		}
		if(new_speed < MOTOR_SPEED_MIN) {
			new_speed = MOTOR_SPEED_MIN;
		}

		m->speed_setpoint = new_speed;
	}
}

void test_print_motor(motor_t * m)
{
	if(m != NULL) {
		printf("m:0x%X cur:%4d set:%4d a:%u in1:%u in2:%u ocr:0x%X\n",
				(uint16_t)(m),
				m->speed_current,
				m->speed_setpoint,
				m->ramp_acceleration,
				m->mctrl_in1,
				m->mctrl_in2,
				(uint16_t)(m->pwm_ocr_ptr));
	}
}

void motor_update_pwm(motor_t * m)
{
	uint16_t speed_diff;
	uint8_t  ramp_step;
	uint8_t  mctrl_tmp;

	if(m != NULL) {

		////  determine actual ramp step value
		//calculate difference between setpoint and current
		speed_diff = abs(m->speed_setpoint - m->speed_current);

		//decide size of step
		if(speed_diff > m->ramp_acceleration) {
			//prepare a normal ramp step
			ramp_step = m->ramp_acceleration;
		} else {
			//prepare the last step to reach speed_setpoint
			ramp_step = speed_diff;
		}

		//calculate one ramp step and refresh speed_current
		if(m->speed_setpoint > m->speed_current) {		// positive acceleration
			m->speed_current += ramp_step;
		} else {										// negative acceleration
			m->speed_current -= ramp_step;
		}


		//decide turning direction:
		//save old mctrl state, delete and set valid control bits of current motor
		mctrl_tmp  = PORT_MCTRL;
		mctrl_tmp &= ~(_BV(m->mctrl_in1) | _BV(m->mctrl_in2));
		if(m->speed_current >= 0) {
			mctrl_tmp |= _BV(m->mctrl_in1);
		} else {
			mctrl_tmp |= _BV(m->mctrl_in2);
		}
		PORT_MCTRL = mctrl_tmp;

		//write new pwm duty to motor driver
		if( m->speed_current > 0) {
			*m->pwm_ocr_ptr =  m->speed_current;
		} else {
			*m->pwm_ocr_ptr = -m->speed_current;
		}
	}
}

void init_motors(void)
{
	// set Motor Ctrl Pins as output
	DDR_MCTRL = _BV(MCTRL_M11) | _BV(MCTRL_M12) | _BV(MCTRL_M21) | _BV(MCTRL_M22);
	//DDRC = 0xFF;

	// set M1 (PWM3) and M2 (PWM4) as output
	DDR_MOTOR_PWM = _BV(PWM_M1) | _BV(PWM_M2);

	motor_1.speed_current = 0;
	motor_1.speed_setpoint = 0;
	motor_1.ramp_acceleration = MOTOR_DEFAULT_RAMP_ACCEL;
	motor_1.mode = 0;
	motor_1.mctrl_in1 = MCTRL_M11;
	motor_1.mctrl_in2 = MCTRL_M12;
	motor_1.pwm_ocr_ptr = &OCR1B;

	motor_2.speed_current = 0;
	motor_2.speed_setpoint = 0;
	motor_2.ramp_acceleration = MOTOR_DEFAULT_RAMP_ACCEL;
	motor_2.mode = 0;
	motor_2.mctrl_in1 = MCTRL_M21;
	motor_2.mctrl_in2 = MCTRL_M22;
	motor_2.pwm_ocr_ptr = &OCR1C;
}


void init_leds(void)
{
	//set LED Pins as output
	DDR_LED = _BV(LED0) | _BV(LED1) |
			  _BV(LED2) | _BV(LED3) |
			  _BV(LED4) | _BV(LED5) |
			  _BV(LED6) | _BV(LED7);
}

void init(void)
{
	init_leds();

	init_pwm_timer();
	init_irq_timer();

	init_motors();

	UART_init(38400);

	printf("inited...");

	sei();				//enable global IRQs

}

motor_t motor_test;

void test_write_motor_struct(void)
{
	//init motor_test
	motor_test.speed_current = 0xAA;
	motor_test.pwm_ocr_ptr = &OCR1C;
	PORT_LED = motor_test.speed_current;

	_delay_ms(1000);

	motor_t * m_ptr;
	m_ptr = &motor_test;

	*m_ptr->pwm_ocr_ptr = m_ptr->speed_current;

	for(;;) {

		*m_ptr->pwm_ocr_ptr = *m_ptr->pwm_ocr_ptr + 1;

		PORT_LED = *m_ptr->pwm_ocr_ptr;

		_delay_ms(500);
	}
}

typedef struct {
	uint8_t value;
	volatile uint16_t * reg_ptr;
} struct_reg_t;


void test_write_register_ptr(void)
{
	struct_reg_t reg;							//konkretes struct mit daten und ziel_pointer
	struct_reg_t * struct_ptr;					//pointer auf ein struct

	struct_ptr = &reg;							//init pointer mit adresse des structs

	reg.value = 0x42;							//init wert direkt im struct
	reg.reg_ptr = &OCR1B;						//stetze pointer für temp-register auf ein konkretes register

	PORT_LED = 0xFF; 							// alle LED an
	_delay_ms(1000);

		//ENTWEDER
	//PORT_LED = *reg.reg_ptr; //LEDs = valueof(reg_ptr)	// schreibe aktuellen Wert des temp-registers auf LEDs

		//ODER
	PORT_LED = *(*struct_ptr).reg_ptr;			// schreibe aktuellen Wert des temp-registers auf LEDs

	_delay_ms(1000);

		//ENTWEDER
	//*reg.reg_ptr = reg.value;
	//*(*struct_ptr).reg_ptr = (*struct_ptr).value; //schreibe initial-wert aus struct in temp-register

	*struct_ptr->reg_ptr = (*struct_ptr).value;


	PORT_LED = *((*struct_ptr).reg_ptr);		// schreibe aktuellen Wert des temp-registers auf LEDs


	for(;;) {
		//*reg.reg_ptr = *reg.reg_ptr + 1;		// inc temp-register

		*struct_ptr->reg_ptr = *struct_ptr->reg_ptr + 1;

		PORT_LED = *(*struct_ptr).reg_ptr;		// schreibe aktuellen Wert des temp-registers auf LEDs
		_delay_ms(500);
	}

	_delay_ms(1000);

//	*reg.reg_ptr = value;

//	uint8_t value = 0xAA;
//	volatile uint16_t * reg_ptr;

//	reg_ptr = &OCR1A;
//	*reg_ptr = value;
//
//	PORTA = *reg_ptr;

//	for(;;) {
//		*reg_ptr = *reg_ptr + 1;
//		PORTA = *reg_ptr;
//		_delay_ms(500);
//	}
}



void test_update_motor_pwm(motor_t * m, int16_t speed)
{
	printf("--START--\n");

	printf("new speed_setpoint: %4d\n", speed);
	motor_set_speed(m, speed);

	_delay_ms(2000);

	while(m->speed_setpoint != m->speed_current) {
		motor_update_pwm(m);
		test_print_motor(m);

		_delay_ms(5);
	}

	printf("--COMPLETE--\n\n");
	_delay_ms(2000);
}

void run_test_motor_ramp(void)
{
	printf("--run_test_motor_ramp()--\n");

	test_update_motor_pwm(&motor_1, -255);
	test_update_motor_pwm(&motor_1, 255);
	test_update_motor_pwm(&motor_2, 255);
	test_update_motor_pwm(&motor_2, 0);
	//test_update_motor_pwm(&motor_1, -100);
/*


	motor_set_speed(&motor_1, 255);
	test_print_motor(&motor_1);
	_delay_ms(DELAY_MS);

	motor_set_speed(&motor_1, -255);
	test_print_motor(&motor_1);
	_delay_ms(DELAY_MS);

	motor_set_speed(&motor_1, 1000);
	test_print_motor(&motor_1);
	_delay_ms(DELAY_MS);

	motor_set_speed(&motor_1, -1000);
	test_print_motor(&motor_1);
	_delay_ms(DELAY_MS);

	motor_set_speed(&motor_1, 50);
	test_print_motor(&motor_1);
	_delay_ms(DELAY_MS);

	motor_set_speed(&motor_1, -50);
	test_print_motor(&motor_1);
	_delay_ms(DELAY_MS);

	motor_set_speed(&motor_1, 0);
	test_print_motor(&motor_1);
	_delay_ms(DELAY_MS);
*/
}

void run_test_motor_update(void)
{

	motor_set_speed(&motor_1, 255);
	motor_set_speed(&motor_2, -255);
	_delay_ms(2000);

	motor_set_speed(&motor_1, -255);
	motor_set_speed(&motor_2, 255);
	_delay_ms(2000);


	motor_set_speed(&motor_1, 0);
	motor_set_speed(&motor_2, 0);
	_delay_ms(2000);
}



void run(void)
{
    for(;;) {
    	run_test_motor_update();
    }
}

int main(void)
{
	init();
	run();
    return 0;
}

ISR(TIMER2_COMP_vect)
{
	motor_update_pwm(&motor_1);
	motor_update_pwm(&motor_2);
}
