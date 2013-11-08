/*
 * bma020.c
 *
 *  Created on: 16.10.2013
 *      Author: alexandermertens
 */

/* *** INCLUDES ************************************************************* */
#include "bma020.h"

/* system headers              */

/* local headers               */
#include "lib/twi_master.h"


/* *** DEFINES ************************************************************* */
#define BMA020_TWI_ADDRESS			(0x70>>1)

/* *** REGISTER NAMING *** */
#define BMA020_REGISTER_CHIPID									0x00
#define BMA020_REGISTER_VERSION									0x01
#define BMA020_REGISTER_VALUE_X_LSB								0x02
#define BMA020_REGISTER_VALUE_X_MSB								0x03
#define BMA020_REGISTER_VALUE_Y_LSB								0x04
#define BMA020_REGISTER_VALUE_Y_MSB								0x05
#define BMA020_REGISTER_VALUE_Z_LSB								0x06
#define BMA020_REGISTER_VALUE_Z_MSB								0x07
#define BMA020_REGISTER_UNUSED									0x08
#define BMA020_REGISTER_STATUS									0x09
#define BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP				0x0A
#define BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG	0x0B
#define BMA020_REGISTER_LG_THRESHOLD							0x0C
#define BMA020_REGISTER_LG_DURATION								0x0D
#define BMA020_REGISTER_HG_THRESHOLD							0x0E
#define BMA020_REGISTER_HG_DURATION								0x0F
#define	BMA020_REGISTER_ANY_MOTION_THRESHOLD					0x10
#define BMA020_REGISTER_AMDUR_HGHYS_LGHYS						0x11
#define BMA020_REGISTER_CUSTOMER_1								0x12
#define BMA020_REGISTER_CUSTOMER_2								0x13
#define BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH					0x14
#define BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP			0x15

/* *** BIT NAMING *** */
/* BMA020_REGISTER_STATUS - 0x09 */
#define BMA020_BIT_STATUS_HG				0
#define BMA020_BIT_STATUS_LG				1
#define BMA020_BIT_HG_LATCHED				2
#define BMA020_BIT_LG_LATCHED				3
#define BMA020_BIT_ALERT_PHASE				4
#define BMA020_BIT_SELF_TEST_RESULT			7

/* BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP - 0x0A */
#define	BMA020_BIT_SLEEP					0
#define BMA020_BIT_SOFT_RESET				1
#define	BMA020_BIT_SELF_TEST_0				2
#define BMA020_BIT_SELF_TEST_1				3
#define BMA020_BIT_RESET_INT				6

/* BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG - 0x0B */
#define BMA020_BIT_ENABLE_LG				0
#define BMA020_BIT_ENABLE_HG				1
#define BMA020_BIT_COUNTER_LG_0				2
#define BMA020_BIT_COUNTER_LG_1				3
#define BMA020_BIT_COUNTER_HG_0				4
#define BMA020_BIT_COUNTER_HG_1				5
#define	BMA020_BIT_ANY_MOTION				6
#define BMA020_BIT_ALTERT					7

/* BMA020_REGISTER_AMDUR_HGHYS_LGHYS - 0x11 */
#define BMA020_BIT_LG_HYSTERESIS_0			0
#define BMA020_BIT_LG_HYSTERESIS_1			1
#define BMA020_BIT_LG_HYSTERESIS_2			2
#define BMA020_BIT_HG_HYSTERESIS_0			3
#define BMA020_BIT_HG_HYSTERESIS_1			4
#define BMA020_BIT_HG_HYSTERESIS_2			5
#define BMA020_BIT_ANY_MOTION_DURATION_0	6
#define BMA020_BIT_ANY_MOTION_DURATION_1	7

/* BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH - 0x14 */
#define BMA020_BIT_BANDWIDTH_0				0
#define BMA020_BIT_BANDWIDTH_1				1
#define BMA020_BIT_BANDWIDTH_2				2
#define BMA020_BIT_RANGE_0					3
#define BMA020_BIT_RANGE_1					4

/* BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP - 0x15 */
#define BMA020_BIT_WAKE_UP					0
#define BMA020_BIT_WAKE_UP_PAUSE_0			1
#define BMA020_BIT_WAKE_UP_PAUSE_1			2
#define BMA020_BIT_SHADOW_DIS				3
#define BMA020_BIT_LATCH_INT				4
#define BMA020_BIT_NEW_DATA_INT				5
#define BMA020_BIT_ADVANCED_INT				6
#define BMA020_BIT_SPI4						7


/* *** DECLARATIONS ********************************************************* */

/* local type and constants     */

/* local function declarations  */


/* *** FUNCTION DEFINITIONS ************************************************* */

/**
 * The Chip ID is used by customer to be able to recognize BMA020. This code is
 * fixed to 010b.
 * @return Chip ID
 */

uint8_t bma020_get_chip_id()
{
	uint8_t register_value;
	register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
											  BMA020_REGISTER_VERSION);
	return (0x07 & register_value);
}


/**
 * ML Version is used with AL Version to identify the chip revision
 * @return ML Version
 */

uint8_t bma020_get_ml_version()
{
	uint8_t register_value;

	register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
											  BMA020_REGISTER_VERSION);

	return (0x0F & register_value);
}

/**
 * AL Version is used with ML Version to identify the chip revision
 * @return AL Version
 */

uint8_t bma020_get_ai_version()
{
	uint8_t register_value;
	register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
											  BMA020_REGISTER_VERSION);
	return (register_value >> 4);
}

/** TODO: complete behavior description
 * This function reads the acceleration registers of the acceleration sensor.
 * The sensor delivers a 10-bit raw value per axis. This function leftshifts it
 * so it writes a 16-bit value to the struct. That has also the effect hat small
 * changes results a bigger change of the value. This should filterd and smoothed
 * by higher funtions. But so the read operation is much easier.
 *
 * The digital signal of x, y, z is 2's complement.
 *
 * Depending on the range the maxia value is +/-2g, +/-4g or +/-8g.
 *
 *	Example: range = 2g;
 *
 * TODO: Check table!
 *     G            | Bit pattern
 * :--------------: | : -----------------------------:
 * -2.000 g         | 1000 0000 0000 0000
 * -1.996 g         | 1000 0000 0100 0000
 * -1.992 g         | 1000 0000 1000 0000
 *  ...             |  ...
 * -0.004 g         | 1111 1111 1111 1111
 *  0.000 g         | 0000 0000 0000 0000
 * +0.004 g         | 0000 0000 0100 0000
 * ...              | ...
 * +1.988 g         | 0111 1111 1111 1101
 * +1.992 g         | 0111 1111 1111 1110
 * +1.996 g         | 0111 1111 1111 1111
 *
 * @param raw_a_vector a acceleration script, which stores the acceleration values
 */

void bma020_read_raw_acceleration(acceleration_t* raw_a_vector)
{
	uint8_t temp_data;

	twi_send_buffer[0] = BMA020_REGISTER_VALUE_X_LSB;

	twi_master_set_ready();
	twi_send_data(BMA020_TWI_ADDRESS, 1);
	twi_receive_data(BMA020_TWI_ADDRESS, 6);

	temp_data = twi_receive_buffer[0];
	raw_a_vector->x = (uint16_t)((temp_data & 0xC0));
	temp_data = twi_receive_buffer[1];
	raw_a_vector->x = (uint16_t)(raw_a_vector->x | temp_data << 8);

	temp_data = twi_receive_buffer[2];
	raw_a_vector->y = (uint16_t)((temp_data & 0xC0));
	temp_data = twi_receive_buffer[3];
	raw_a_vector->y = (uint16_t)(raw_a_vector->y | temp_data << 8);

	temp_data = twi_receive_buffer[4];
	raw_a_vector->z = (uint16_t)((temp_data & 0xC0));
	temp_data = twi_receive_buffer[5];
	raw_a_vector->z = (uint16_t)(raw_a_vector->z | temp_data << 8);
}


/**
 * This function is true of the acceleration registers have been updated.
 *
 * Valid values for axis are 'x', 'y' or 'z'.
 *
 * @param axis
 * @return true, if the axis value is new
 */

bool bma020_is_new_data(char axis)
{
	uint8_t register_adress;

	if(axis == 'x') {
		register_adress = BMA020_REGISTER_VALUE_X_LSB;
	} else if(axis == 'y') {
		register_adress = BMA020_REGISTER_VALUE_Y_LSB;
	} else if(axis == 'z') {
		register_adress = BMA020_REGISTER_VALUE_Z_LSB;
	} else {
		return false;
	}

	return twi_master_read_register_bit(BMA020_TWI_ADDRESS, register_adress, 0);
}


/**
 * bma020_get_status_hg() returns true, if the  when the corresponding criteria
 * have been issued; they are automatically reset by BMA020 when the criteria
 * disappear.
 * @return
 */

bool bma020_get_status_hg(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_STATUS,
										BMA020_BIT_STATUS_HG);
}

/**
 * bma020_get_status_lg() returns true, if the  when the corresponding criteria
 * have been issued; they are automatically reset by BMA020 when the criteria
 * disappear.
 * @return
 */

bool bma020_get_status_lg(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_STATUS,
										BMA020_BIT_STATUS_LG);
}


/**
 * These status bit is set when the corresponding criteria have been issued.
 * They are latched and thus only the microcontroller can reset them. When both
 * high acceleration and low acceleration thresholds are enabled, these bits can
 * be used by microprocessor to detect which criteria generated the interrupt.
 * @return true if, lg criteria is fullified
 */

bool bma020_get_lg_latched(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_STATUS,
										BMA020_BIT_LG_LATCHED);
}


/**
 * Resets the LG Criteria Bit.
 */
void bma020_reset_lg_latched(void)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_STATUS,
								  BMA020_BIT_LG_LATCHED,
								  false);
}

/**
 * These status bit is set when the corresponding criteria have been issued.
 * They are latched and thus only the microcontroller can reset them. When both
 * high acceleration and low acceleration thresholds are enabled, these bits can
 * be used by microprocessor to detect which criteria generated the interrupt.
 * @return true if, hg criteria is fullified
 */
bool bma020_get_hg_latched(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_STATUS,
										BMA020_BIT_HG_LATCHED);
}

/**
 * Resets the HG Criteria Bit.
 */
void bma020_reset_hg_latched(void)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_STATUS,
								  BMA020_BIT_HG_LATCHED,
								  false);
}


/**
 * This function returns ture, if alert mode is activated and an any motion
 * criterion has been detected. During alert phase, HG_dur and LG_dur variables
 * are decreased to have a smaller reaction time when HG_thres and LG_thres
 * thresholds are crossed; the decrease rate is by 1 ms per ms.The alert mode is
 * reset when an interrupt generated due to a high threshold or a low threshold
 * event or when both HG_dur and LG_dur variables are at 0. When alert is reset,
 * HG_dur and LG_dur variables come back to their original values stored in
 * image registers.
 * @return true if alare criterias are fullified
 */


bool bma020_get_alert_phase(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_STATUS,
										BMA020_BIT_ALERT_PHASE);
}


/**
 * It can be used together with selftest_0 control bit (section 3.3.4). After
 * selftest_0 has been set, self-test procedure starts. At the end selftest_0 is
 * written to 0 and microcontroller can react by reading st_result bit. When
 * st_result=1 the self test passed successfully.The result of the st_result can
 * be taken into account to evaluate the basic function of the sensor. Note:
 * Evaluation of the st_result bit should only be understood as one part of a
 * wider functionality test. It should not be taken into consideration as the
 * only criterion.
 * @return true if self test passed
 */

bool bma020_get_self_test_result(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_STATUS,
										BMA020_BIT_SELF_TEST_RESULT);
}


/**
 * This function turns the sensor IC in sleep mode. Control and image registers
 * are not cleared. When BMA020 is in sleep mode no operation can be performed
 * but wake-up the sensor IC by setting sleep=0 or soft_reset. As a consequence all
 * write and read operations are forbidden when the sensor IC is in sleep mode
 * except command used to wake up the device or soft_reset command. After sleep
 * mode removal, it takes 1ms to obtain stable acceleration values (>99% data
 * integrity).
 */

void bma020_sleep(void)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP,
								  BMA020_BIT_SLEEP,
								  true);
}


/**
 * BMA020 is reset each time this bit (address 0Ah, bit 1) is written to 1. The
 * effect is identical to power-on reset. Control, status and image registers
 * are reset to values stored in the default setting registers (see also memory
 * map). After soft_reset or power-on reset BMA020 comes up in normal mode or
 * wake-up mode. It is not possible to boot BMA020 to sleep mode.
 * No serial transaction should occur within 10 microsec. after soft_reset cmd..
 * The soft_reset procedure may overwrite the SPI4 setting (section 3.1.1). Thus
 * the correct interface configuration may have to be updated.
 */
void bma020_soft_reset(void)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP,
								  BMA020_BIT_SOFT_RESET,
								  true);
}


/**
 * The self-test command (address 0Ah, bit 2) uses electrostatic forces to move
 * the MEMS common electrode. The result from selftest can be verified by
 * reading st_result (section 3.4.1). During the self-test procedure no external
 * change of the acceleration should be generated.
 *
 */


void bma020_perform_self_test_0(void)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP,
								  BMA020_BIT_SELF_TEST_0,
								  true);
}

/**
 * This self test bit (address 0Ah, bit3) does not generate any electrostatic
 * force in the MEMS element but is used to verify the interrupt function is
 * working correctly and that microprocessor is able to react to the interrupts.
 * 0g acceleration is emulated at ADC input and the user can detect the whole
 * logic path for interrupt, including the PCB path integrity. The LG_thres
 * register must be set to about 0.4g while LG_dur = 0 to generate a low-g
 * interrupt
 *
 */

void bma020_perform_self_test_1(void)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP,
								  BMA020_BIT_SELF_TEST_1,
								  true);
}

/**
 * The SPI4 bit ((address 15h, bit 7) is used to select the correct SPI protocol
 * (three-wire or four- wire, SPI-mode 3). The default value is SPI4=1
 * (four-wire SPI is default value). After power on reset or soft reset it is
 * recommended to set the SPI4-bit to the correct value.
 * This first writing is possible because only CSB, SCK and SDI are required for
 * a write sequence and the 3 bit timing diagrams are identical in three-wire
 * and four-wire configuration.
 *
 * Recommended procedure: Set SPI4 to the correct value (SPI4=0 for SPI
 * three-wire, SPI4=1 for SPI four-wire (=default)) every time after power on
 * reset or soft reset.
 * @param enable
 */

void bma020_enable_spi4(bool enable)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP,
								  BMA020_BIT_SPI4,
								  enable);
}

/**
 * returns true if 4 wire SPI is activated.
 * @return
 */

bool bma020_is_spi4(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP,
										BMA020_BIT_SPI4);
}

/**
 *
 * BMA020 provides the possibility to block the update of data MSB while LSB are
 * read out. This avoids a potential mixing of LSB and MSB of successive
 * conversion cycles. When this bit (address 15h, bit 3) is at 1, the blocking
 * procedure for MSB is not realized and MSB only reading is possible.
 * @param disable
 */
void bma020_disable_shadow(bool disable)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP,
								  BMA020_BIT_SHADOW_DIS,
								  disable);
}

/**
 * returns true, if value shadowing is disabled.
 * @return
 */
bool bma020_is_shadow_disabled(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP,
										BMA020_BIT_SHADOW_DIS);
}


/**
 * This interrupt (address 0Ah, bit 6) is reset (interrupt pad goes to low) each
 * time this bit is written to 1.
 */
void bma020_reset_interrupt(void)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_TESTS_RESET_SLEEP,
								  BMA020_BIT_RESET_INT,
								  true);
}

/**
 * If this bit (address 15h, bit 5) is set to 1, an interrupt will be generated
 * when all three axes acceleration values are new, i.e. BMA020 updated all
 * acceleration values after latest serial read-out. Interrupt generated from
 * new data detection is a latched one; microcontroller has to write reset_INT
 * at 1 after interrupt has been detected high (section 3.3.1). This interrupt
 * is also reset by any acceleration byte read procedure (read access to address
 * 02h to 07h).
 * New data interrupt always occurs at the end of the Z-axis value update in the
 * output register (3kHz rate). Following figure shows two examples of X-axis
 * read out and the corresponding interrupt generation.
 * @param enable
 */
void bma020_enable_new_data_int(bool enable)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP,
								  BMA020_BIT_NEW_DATA_INT,
								  enable);
}

/**
 * returns the current state of new_data_int. enabled == true, disabled == false.
 * @return returns the current state of new_data_int
 */
bool bma020_is_new_data_int(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP,
										BMA020_BIT_NEW_DATA_INT);
}

/**
 * This bit (address 0Bh, bit 0) enables the LG_thres criteria to generate an
 * interrupt.
 */
void bma020_enable_lg(bool enable)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG,
								  BMA020_BIT_ENABLE_LG,
								  enable);
}

/**
 * returns true if the lg interrupt is enabled.
 * @return
 */

bool bma020_is_enable_lg(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG,
										BMA020_BIT_ENABLE_LG);
}


/**
 * This bit (address 0Bh, bit 1) enables the HG_thres criteria to generate an
 * interrupt.
 * @param enable
 */


void bma020_enable_hg(bool enable)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG,
								  BMA020_BIT_ENABLE_HG,
								  enable);
}

/**
 * returns true if the lg interrupt is enabled.
 * @return
 */


bool bma020_is_enable_hg(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG,
										BMA020_BIT_ENABLE_HG);
}

/**
 *
 * This bit (address 15h, bit 6) is used to disable advanced interrupt control bits
 * (any_motion, alert). If enable_adv_INT=0, writing to these bits has no effect on
 * sensor IC function.
 * @param enable
 */

void bma020_enable_advanced_int(bool enable)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP,
								  BMA020_BIT_ADVANCED_INT,
								  enable);
}

/**
 * returns true if the advanced interrupt is enabled.
 * @return
 */

bool bma020_is_advanced_int(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP,
										BMA020_BIT_ADVANCED_INT);
}


/**
 * This bit ((address 0Bh, bit 6)enables the any motion criteria to generate
 * directly an interrupt. It can not be turned on simultaneously with alert.
 * This bit can be masked by enable_ adv_INT, the value of this bit is ignored
 * when enable_adv_INT=0 (section 3.2.3).
 *
 * @param enable
 */

void bma020_enable_any_motion(bool enable)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
							      BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG,
							      BMA020_BIT_ANY_MOTION,
							      enable);
}

/**
 * returns true if the any motion interrupt is enabled.
 * @return
 */

bool bma020_is_enable_any_motion(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG,
										BMA020_BIT_ANY_MOTION);
}

/**
 * f this bit (address 0Bh, bit 7) is at 1, the any_motion criterion will set
 * BMA020 into alert mode (section 3.2.9). This bit can be masked by
 * enable_adv_INT, the value of this bit is ignored when enable_adv_INT=0
 * (section 3.2.3).
 *
 * @param enable
 */

void bma020_enable_motion_alert(bool enable)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG,
								  BMA020_BIT_ALTERT,
								  enable);
}

/**
 * returns true if the motion alert interrupt is enabled.
 * @return
 */


bool bma020_is_enable_motion_alert(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG,
										BMA020_BIT_ALTERT);
}

/**
 * If this bit (address 15h, bit 4) is at 1, interrupts are latched. The INT pad
 * stays high until microprocessor detects it and writes reset_INT control bit
 * to 1 (section 3.3.1). When this bit is at 0, interrupts are set and reset
 * directly by BMA020 according to programmable criteria (sections 3.2.7 and
 * 3.2.8).
 * @param enable
 */

void bma020_enable_latched_int(bool enable)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP,
								  BMA020_BIT_LATCH_INT,
								  enable);
}

/**
 * returns true if latched interrupt is enabled.
 * @return
 */

bool bma020_is_latched_int(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP,
										BMA020_BIT_LATCH_INT);
}



/**
 * LG_thres (address 0C, bits 7-0 / low-g threshold) and LG_hyst (address 11h,
 * bits 2-0 / low-g threshold hysteresis) can basically (although not
 * recommended with BMA020) be used to detect a free fall. The threshold and
 * duration codes define one criterion for interrupt generation when absolute
 * value of acceleration is low for long enough duration.
 *
 * Data format is unsigned integer.
 *
 * Condition                          |   Result
 * :--------------------------------: | : ----------------------------------------------------------------------------------------------------------------:
 * LG_thres criterion_x is true if    | \f$ |acc_x | <= LG_thres / 255 * range\f$
 * LG_thres interrupt is set if       | \f$ (LG_thres criterion_x AND LG_thres criterion_y AND LG_thres criterion_z) AND interrupt counter = (LG_dur+1)\f$
 * LG_thres criterion_x is false if   | \f$ |acc_x | > (LG_thres + 32*LG_hyst) / 255 * range\f$
 * LG_thres interrupt is reset if     | \f$ NOT(LG_thres criterion_x AND LG_thres criterion_y AND LG_thres criterion_z)\f$
 *
 * LG_thres and LG_hyst codes must be chosen to have (LG_thres + 32*LG_hyst) < 511.
 *
 * When LG_thres criterion becomes active, an interrupt counter is incremented
 * by 1 LSB/ms. When the low-g interrupt counter value equals (LG_dur+1), an
 * interrupt is generated. Depending on counter_LG (address 0Bh, bit 3 and 2)
 * register, the counter could also be reset or count down when LG_thres
 * criterion is false.
 *
 * If latch_INT=0, the interrupt is not a latched interrupt and then it is reset
 * as soon as LG_thres criteria becomes false. When interrupt occurs, the
 * interrupt counter is reset.
 *
 * Remark: The LG_thres criteria is set with an AND condition on all three axes
 * to be used for free fall detection. However, please note due to the
 * relatively wide sensitivity tolerance of the BMA020 the absolute threshold
 * values for low-g and high-g interrupt can only provide a rough estimation of
 * the motion profile.
 *
 * @param threshold
 */

void bma020_set_lg_threshold(uint8_t threshold)
{
	twi_master_write_register(BMA020_TWI_ADDRESS,
							  BMA020_REGISTER_LG_THRESHOLD,
							  threshold);
}

/**
 * returns the current set lg_threshold
 * @return
 */

uint8_t bma020_get_lg_threshold()
{
	return twi_master_read_register(BMA020_TWI_ADDRESS,
									BMA020_REGISTER_LG_THRESHOLD);
}

/**
 * sets lg_duration
 * @param duration
 */

void bma020_set_lg_duration(uint8_t duration)
{
	twi_master_write_register(BMA020_TWI_ADDRESS,
							  BMA020_REGISTER_LG_DURATION,
							  duration);
}

/**
 * returns the current set lg_duration
 * @return
 */

uint8_t bma020_get_lg_duration()
{
	return twi_master_read_register(BMA020_TWI_ADDRESS,
									BMA020_REGISTER_LG_DURATION);
}


/**
 * sets counter lg
 * valid params: 0, 1, 2, 3
 * @param counter
 * @return
 */

bool bma020_set_counter_lg(uint8_t counter)
{
	if(counter == 0  || counter == 1 || counter == 2 || counter == 3) {

		uint8_t register_value;

		/* read register and delete old value */
		register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
												  BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG);
		register_value &= ~(1<<BMA020_BIT_COUNTER_LG_1 |
							1<<BMA020_BIT_COUNTER_LG_0); /* (value &= 0b11110011;)*/

		/* set new value (shift counter value on correct position */
		register_value |= counter<<BMA020_BIT_COUNTER_LG_0;

		/*write new value to register */
		twi_master_write_register(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG,
								  register_value);

		return true;
	} else {
		return false;
	}
}


/**
 * returns current counter_lg
 * @return
 */

uint8_t bma020_get_counter_lg(void)
{
	uint8_t register_value;

	/* read register and delete all non counter lg bits */
	register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
											  BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG);
	register_value &= (1<<BMA020_BIT_COUNTER_LG_1 |
					   1<<BMA020_BIT_COUNTER_LG_0);

	return (register_value >> BMA020_BIT_COUNTER_LG_0);
}

/**
 * Sets the hysterese
 * @param hysterese
 */

void bma020_set_lg_hysterese(uint8_t hysterese)
{
	uint8_t register_value;

	/* only the lowest three bits are valid */
	hysterese &= 0x07;

	/* read register and delete old value */
	register_value  = twi_master_read_register(BMA020_TWI_ADDRESS,
											   BMA020_REGISTER_AMDUR_HGHYS_LGHYS);
	register_value &= ~(BMA020_BIT_LG_HYSTERESIS_0 |
						BMA020_BIT_LG_HYSTERESIS_1 |
						BMA020_BIT_LG_HYSTERESIS_2); /* value &= 0b00000111; */

	/* set new value */
	register_value |= hysterese;

	/*write back to register */
	twi_master_write_register(BMA020_TWI_ADDRESS,
							  BMA020_REGISTER_AMDUR_HGHYS_LGHYS,
							  register_value);
}

/**
 *
 * @return lg_hysterese
 */

uint8_t bma020_get_lg_hysterese(void)
{
	uint8_t register_value;

	register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
											  BMA020_REGISTER_AMDUR_HGHYS_LGHYS);

	/* only the lowest three bits represents the hyterese */
	return (register_value & 0x07);
}


/**
 * HG_thres (address 0Eh, bits 7-0 / high-g threshold) and HG_hyst (address 11h,
 * bits 5-3 / high-g threshold hysteresis) define the high-G level and its
 * associated hysteresis. HG_dur (high-g threshold qualification duration) and
 * counter_HG (address 0Bh, bits 5 and 4 / high-g counter down register) are
 * used for debouncing the high-g criteria.
 * Threshold and duration codes define a criterion for interrupt generation when
 * absolute value of acceleration is high for long enough duration.
 *
 * Data format is unsigned integer.
 *
 * Condition                            |   Result
 * :----------------------------------: | : ----------------------------------------------------------------------------------------------------------------:
 * HG_threshold criterion_x is true if  | \f$ |acc_x | ≥ HG_thres / 255 * range\f$
 * HG_threshold interrupt is set if     | \f$ (HG_thres criterion_x OR HG_thres criterion_y OR HG_thres criterion_z) AND interrupt counter = (HG_dur+1)\f$
 * HG_threshold criterion_x is false if | \f$ |acc_x | < (HG_thres - 32*HG_hyst) / 255 * range\f$
 * HG_threshold interrupt is reset if   | \f$ NOT(HG_thres criterion_x OR HG_thres criterion_y OR HG_thres criterion_z)\f$
 *
 * HG_thres and HG_hyst codes must be chosen to have (HG_thres - 32*HG_hyst) > 0.
 *
 * When HG_thres criterion becomes active, a counter is incremented by 1 LSB/ms.
 * When the high-g acceleration interrupt counter value equals (HG_dur+1), an
 * interrupt is generated. Depending on counter_HG register value, the counter
 * could also be reset or count down when HG_thres criterion is false.
 *
 * If latch_INT=0, the interrupt is not a latched interrupt and then it is reset
 * as soon as HG_thres criteria becomes false. When interrupt occurs, the
 * interrupt counter is reset.
 *
 * @param threshold
 */


void bma020_set_hg_threshold(uint8_t threshold)
{
	twi_master_write_register(BMA020_TWI_ADDRESS,
							  BMA020_REGISTER_HG_THRESHOLD,
							  threshold);
}

/**
 * returns the current set hg_threshold
 * @return
 */

uint8_t bma020_get_hg_threshold()
{
	return twi_master_read_register(BMA020_TWI_ADDRESS,
									BMA020_REGISTER_HG_THRESHOLD);
}


/**
 * Sets the hg_duration
 * @param duration
 */

void bma020_set_hg_duration(uint8_t duration)
{
	twi_master_write_register(BMA020_TWI_ADDRESS,
							  BMA020_REGISTER_HG_DURATION,
							  duration);
}

/**
 * return the hg_duration
 * @return
 */

uint8_t bma020_get_hg_duration()
{
	return twi_master_read_register(BMA020_TWI_ADDRESS,
									BMA020_REGISTER_HG_DURATION);
}


/**
 * sets counter lg
 * valid params: 0, 1, 2, 3
 * @param counter
 * @return
 */
bool bma020_set_counter_hg(uint8_t counter)
{
	if(counter == 0  || counter == 1 || counter == 2 || counter == 3) {

		uint8_t register_value;

		/* read register and delete old value */
		register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
												  BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG);
		register_value &= ~(1<<BMA020_BIT_COUNTER_HG_1 |
							1<<BMA020_BIT_COUNTER_HG_0); /* (value &= 0b11110011;)*/

		/* set new value (shift counter value on correct position */
		register_value |= counter<<BMA020_BIT_COUNTER_HG_0;

		/*write new value to register */
		twi_master_write_register(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG,
								  register_value);

		return true;
	} else {
		return false;
	}
}

/**
 * returns the current counter
 * @return
 */
uint8_t bma020_get_counter_hg(void)
{
	uint8_t register_value;

	/* read register and delete all non counter lg bits */
	register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
											  BMA020_REGISTER_ALERT_ENANYMOTION_CNTHG_CNTLG_ENHG_ENLG);
	register_value &= (1<<BMA020_BIT_COUNTER_HG_1 |
					   1<<BMA020_BIT_COUNTER_HG_0);

	return (register_value >> BMA020_BIT_COUNTER_HG_0);
}


/**
 * sets the hg_hysterese
 * @param hysterese
 */

void bma020_set_hg_hysterese(uint8_t hysterese)
{
	uint8_t register_value;

	/* only the lowest three bits are valid */
	hysterese &= 0x07;

	/* read register and delete old value */
	register_value  = twi_master_read_register(BMA020_TWI_ADDRESS,
											   BMA020_REGISTER_AMDUR_HGHYS_LGHYS);
	register_value &= ~(BMA020_BIT_HG_HYSTERESIS_0 |
						BMA020_BIT_HG_HYSTERESIS_1 |
						BMA020_BIT_HG_HYSTERESIS_2); /* value &= 0b00111000; */

	/* set new value */
	register_value |= hysterese<<BMA020_BIT_HG_HYSTERESIS_0;

	/*write back to register */
	twi_master_write_register(BMA020_TWI_ADDRESS,
							  BMA020_REGISTER_AMDUR_HGHYS_LGHYS,
							  register_value);
}


/**
 * returns the hg_hysterese
 * @return
 */

uint8_t bma020_get_hg_hysterese(void)
{
	uint8_t register_value;

	register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
											  BMA020_REGISTER_AMDUR_HGHYS_LGHYS);

	/* only the bits 0b00111000 are the hyterese */
	register_value &= 0x38;
	register_value = register_value>>3;
	return register_value;
}

/**
 * For the evaluation using “any motion” criterion successive acceleration data
 * from digital filter output are stored and moving differences for all axes are
 * built. To calculate the difference the acceleration values of all axes at
 * time t0 are compared to values at t0+3/(2*bandwidth). The difference of both
 * values is equal to the difference of two successive moving averages (from
 * three data points).
 * The differential value is compared to a global critical threshold
 * any_motion_thres (address 10h, bits 7-0). Interrupt can be generated when the
 * absolute value of measured difference is higher than the programmed threshold
 * for long enough duration defined by any_motion_dur (address 11h, bits 7 and
 * 6).
 * Any_motion_thres and any_motion_dur data are unsigned integer.
 * Any_motion_thres LSB size corresponds to 15.6mg for +/- 2g range and scales
 * with range selection (section 3.1.2).
 * @param threshold
 */
void bma020_set_any_motion_threshold(uint8_t threshold)
{
	twi_master_write_register(BMA020_TWI_ADDRESS,
							  BMA020_REGISTER_ANY_MOTION_THRESHOLD,
							  threshold);
}

/**
 * returns any motion threshold
 * @return
 */
uint8_t bma020_get_any_motion_threshold(void)
{
	return twi_master_read_register(BMA020_TWI_ADDRESS,
									BMA020_REGISTER_ANY_MOTION_THRESHOLD);
}

/**
 * Any_motion_dur is used to filter the motion profile and also to define a
 * minimum interrupt duration because the reset condition is also filtered.
 * Valid values for duration: 1, 3, 5, 7
 * @param duration
 * @return
 */
bool bma020_set_any_motion_duration(uint8_t duration)
{
	uint8_t register_value;

	if(duration == 1 || duration == 3 || duration == 5 || duration == 7) {


		/* read register and delete old value */
		register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
												  BMA020_REGISTER_AMDUR_HGHYS_LGHYS);
		register_value &= ~(1<<BMA020_BIT_ANY_MOTION_DURATION_1 |
							1<<BMA020_BIT_ANY_MOTION_DURATION_0); /* value &= 0b00111111; */


		if(duration == 1) {
			/* set no bits*/
		} else if(duration == 3) {
			register_value |= 1<<BMA020_BIT_ANY_MOTION_DURATION_0;
		} else if(duration == 5) {
			register_value |= 1<<BMA020_BIT_ANY_MOTION_DURATION_1;
		} else if(duration == 7) {
			register_value |= (1<<BMA020_BIT_ANY_MOTION_DURATION_0 |
							   1<<BMA020_BIT_ANY_MOTION_DURATION_1);
		}

		twi_master_write_register(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_AMDUR_HGHYS_LGHYS,
								  register_value);

		return true;
	} else {
		return false;
	}
}

uint8_t bma020_get_any_motion_duration(void)
{
	uint8_t register_value;
	uint16_t return_value;

	/* read register and delete all non any_motion_duration bits */
	register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
											  BMA020_REGISTER_AMDUR_HGHYS_LGHYS);
	register_value &= (1<<BMA020_BIT_ANY_MOTION_DURATION_0 |
					   1<<BMA020_BIT_ANY_MOTION_DURATION_1);

	/* convert to human readable values*/
	if(register_value == 1<<BMA020_BIT_ANY_MOTION_DURATION_0) {
		return_value = 3;
	} else if(register_value == 1<<BMA020_BIT_ANY_MOTION_DURATION_0) {
		return_value = 5;
	} else if(register_value == (1<<BMA020_BIT_ANY_MOTION_DURATION_0 |
								 1<<BMA020_BIT_ANY_MOTION_DURATION_1)) {
		return_value = 7;
	} else {
		return_value = 1;
	}

	return return_value;
}

/**
 * This bit (address 15h, bit 0) makes BMA020 automatically switching from sleep
 * mode to normal mode after the delay defined by wake_up_pause (section 3.1.5).
 * When the sensor IC goes from sleep to normal mode, it starts acceleration
 * acquisition and performs interrupt verification (section 3.2). The sensor IC
 * automatically switches back from normal to sleep mode again if no fulfilment
 * of programmed interrupt criteria has been detected. The IC wakes-up for a
 * minimum duration which depends on the number of required valid acceleration
 * data to determine if an interrupt should be generated.
 *
 * If a latched interrupt is generated, this can be used to wake-up a
 * microprocessor. The sensor IC will wait for a reset_INT command and restart
 * interrupt verification. BMA020 can not go back to sleep mode if reset_INT is
 * not issued after a latched interrupt.
 *
 * If a not-latched interrupt is generated, the device waits in the normal mode
 * till the interrupt condition disappears. The minimum duration of interrupt
 * activation is 330microsecs. If no interrupt is generated, the sensor IC goes
 * to sleep mode for a defined time (wake_up_pause).
 * For more details on the wake-up functionality, please refer to chapter 7.3
 *
 * @param enable
 */
void bma020_enable_wake_up(bool enable)
{
	twi_master_write_register_bit(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP,
								  BMA020_BIT_WAKE_UP,
								  enable);
}

/**
 * returns true, if wake_uo behavior is enabled.
 * @return
 */

bool bma020_is_wake_up(void)
{
	return twi_master_read_register_bit(BMA020_TWI_ADDRESS,
										BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP,
										BMA020_BIT_WAKE_UP);
}




/**
 * These bits (address 15h, bit 2 and 1) define the sleep phase duration between
 * each automatic wake-up.
 * wake_up_pause<1:0>	Sleep phase duration
 * 00					20 ms
 * 01					80 ms
 * 10					320 ms
 * 11					2560 ms
 *
 * @param wake_up_time
 * @return
 */

bool bma020_set_wake_up_pause(uint16_t wake_up_time)
{
	uint8_t register_value;

	if( wake_up_time == 20   ||
		wake_up_time == 80   ||
		wake_up_time == 320  ||
		wake_up_time == 2560) {

		/* read register and delete old value */
		register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
												  BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP);
		register_value &= ~(1<<BMA020_BIT_WAKE_UP_PAUSE_1 |
							1<<BMA020_BIT_WAKE_UP_PAUSE_0); /* value &= 0b11111001; */


		if(wake_up_time == 20) {
			/* set no bits */
		} else if (wake_up_time == 80) {
			register_value |= (1<<BMA020_BIT_WAKE_UP_PAUSE_0);
		} else if(wake_up_time == 320) {
			register_value |= (1<<BMA020_BIT_WAKE_UP_PAUSE_1);
		} else if(wake_up_time == 2560) {
			register_value |= (1<<BMA020_BIT_WAKE_UP_PAUSE_0 |
							   1<<BMA020_BIT_WAKE_UP_PAUSE_1);
		}

		twi_master_write_register(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH,
								  register_value);

		return true;
	} else {
		return false;
	}
}

uint16_t bma020_get_wake_up_pause(void)
{
	uint8_t register_value;
	uint16_t return_value;


	/* read register and delete all non wake_up_pause */
	register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
											  BMA020_REGISTER_CONTROL_SPI4_INTERRUPT_WAKEUP);
	register_value &= (1<<BMA020_BIT_WAKE_UP_PAUSE_1 |
					   1<<BMA020_BIT_WAKE_UP_PAUSE_0);

	/* convert to human readable values*/
	if(register_value == 1<<BMA020_BIT_WAKE_UP_PAUSE_0) {
		return_value = 80;
	} else if(register_value ==  1<<BMA020_BIT_WAKE_UP_PAUSE_1) {
		return_value = 320;
	} else if(register_value == (1<<BMA020_BIT_WAKE_UP_PAUSE_0 |
								 1<<BMA020_BIT_WAKE_UP_PAUSE_1)) {
		return_value = 2560;
	} else {
		return_value = 20;
	}

	return return_value;
}


/**
 * These three bits (address 14h, bits 2-0) are used to setup the digital
 * filtering of ADC output data to obtain the desired bandwidth. A second order
 * analogue filter defines the max. bandwidth to 1.5kHz. Digital filters can be
 * activated to reduce the bandwidth down to 25Hz in order to reduce signal
 * noise. The digital filters are moving average filters of various length with
 * a refresh rate of 3kHz. Since the bandwidth is reduced by a digital filter
 * for the factor 1/2 , 1/4, ... of the analogue filter frequency of 1.5kHz the
 * mean values of the bandwidth are slightly deviating from the rounded nominal
 * values. Table 4 shows the corresponding data:
 *
 * bandwidth<2:0>	Mean bandwidth[Hz]
 * 000				25
 * 001				50
 * 010				100
 * 011				190
 * 100				375
 * 101				750
 * 110				1500
 * 111				Not authorised code
 *
 * @param bandwidth
 */
bool bma020_set_bandwidth(uint16_t bandwidth)
{
	uint8_t value;

	if( bandwidth == 25   ||
		bandwidth == 50   ||
		bandwidth == 100  ||
		bandwidth == 190  ||
		bandwidth == 375  ||
		bandwidth == 750  ||
		bandwidth == 1500) {


		/* read register and delete old value */
		value = twi_master_read_register(BMA020_TWI_ADDRESS,
										 BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH);
		value &= ~(1<<BMA020_BIT_BANDWIDTH_2 |
				   1<<BMA020_BIT_BANDWIDTH_1 |
				   1<<BMA020_BIT_BANDWIDTH_0); /* value &= 0b11111000; */


		if(bandwidth == 25) {
			/* if bandwidth == 25; set no bits*/
		} else if(bandwidth == 50) {
			value |= (1<<BMA020_BIT_BANDWIDTH_0);
		} else if(bandwidth == 100) {
			value |= (1<<BMA020_BIT_BANDWIDTH_1);
		} else if(bandwidth == 190) {
			value |= (1<<BMA020_BIT_BANDWIDTH_1 | 1<<BMA020_BIT_BANDWIDTH_0);
		} else if(bandwidth == 375) {
			value |= (1<<BMA020_BIT_BANDWIDTH_2);
		} else if(bandwidth == 750) {
			value |= (1<<BMA020_BIT_BANDWIDTH_2 | 1<<BMA020_BIT_BANDWIDTH_0);
		} else if(bandwidth == 1500) {
			value |= ((1<<BMA020_BIT_BANDWIDTH_2) | (1<<BMA020_BIT_BANDWIDTH_1));
		}

		twi_master_write_register(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH,
								  value);

		return true;
	} else {
		return false;
	}
}

/**
 * returns the current bandwidth
 * @return
 */

uint16_t bma020_get_bandwidth(void)
{
	uint8_t register_value;
	uint16_t return_value;

	/* read register and delete all non range-bits*/
	register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
											  BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH);
	register_value &= (1<<BMA020_BIT_BANDWIDTH_2 |
					   1<<BMA020_BIT_BANDWIDTH_1 |
					   1<<BMA020_BIT_BANDWIDTH_0); /* (value &= 0b00000111;)*/

	/*convert to human readable values */
	if(register_value == 1<<BMA020_BIT_BANDWIDTH_0) {
		return_value = 50;
	} else if(register_value ==  1<<BMA020_BIT_BANDWIDTH_1)  {
		return_value = 100;
	} else if(register_value == (1<<BMA020_BIT_BANDWIDTH_1 |
								 1<<BMA020_BIT_BANDWIDTH_0)) {
		return_value = 190;
	} else if(register_value ==  1<<BMA020_BIT_BANDWIDTH_2)  {
		return_value = 375;
	} else if(register_value == (1<<BMA020_BIT_BANDWIDTH_2 |
								 1<<BMA020_BIT_BANDWIDTH_0)) {
		return_value = 750;
	} else if(register_value == (1<<BMA020_BIT_BANDWIDTH_2 |
								 1<<BMA020_BIT_BANDWIDTH_1)) {
		return_value = 1500;
	} else {
		/* if no bit is set; bandwidth == 25*/
		return_value = 25;
	}

	return return_value;
}

/**
 * These two bits (address 14h, bits 4 and 3) are used to select the full scale
 * acceleration range. Directly after changing the full scale range it takes
 * \f$ \frac{1}{2 \cdot bandwidth} \f$ to overwrite the data registers with
 * filtered data according to the selected bandwidth.
 *
 * Valid values for range (uint8_t):
 *
 * range (uint8_t)  | Full scale acceleration range
 * :--------------: | : -----------------------------:
 * 2                | +/- 2g
 * 4                | +/- 4g
 * 8                | +/- 8g
 *
 * @param range - New range value
 * @return boolean - return true, if param is valid and write sequence to the
 * 					 sensor is completed.
 * 					 return false, if param is invalid. Then no write sqquence
 * 					 was performed. so
 */
bool bma020_set_range(uint8_t range)
{
	if(range == 2 || range == 4 || range == 8) {

		uint8_t value;

		/* read register and delete old value */
		value = twi_master_read_register(BMA020_TWI_ADDRESS,
										 BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH);
		value &= ~(1<<BMA020_BIT_RANGE_1 | 1<<BMA020_BIT_RANGE_0); /* (value &= 0b11100111;)*/


		if(range == 2) {
			/* if range == 2g; set no bits*/
		} else if(range == 4) {
			value |= (1<<BMA020_BIT_RANGE_0);
		} else if(range == 8) {
			value |= (1<<BMA020_BIT_RANGE_1);
		}

		twi_master_write_register(BMA020_TWI_ADDRESS,
								  BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH,
								  value);

		return true;
	} else {
		return false;
	}
}

/**
 * returns current range
 * @return
 */

uint8_t bma020_get_range(void)
{
	uint8_t register_value;
	uint8_t return_value;

	/* read register and delete all non range-bits*/
	register_value = twi_master_read_register(BMA020_TWI_ADDRESS,
											  BMA020_REGISTER_CONTROL_RANGE_BANDWIDTH);
	register_value &= (1<<BMA020_BIT_RANGE_1 | 1<<BMA020_BIT_RANGE_0); /* (value &= 0b00011000;)*/

	/*convert to human readable values */
	if(register_value == 1<<BMA020_BIT_RANGE_0) {
		return_value = 4;
	} else if (register_value == 1<<BMA020_BIT_RANGE_1) {
		return_value = 8;
	} else {
		/* if no bit is set; range == 2 */
		return_value = 2;
	}
	return return_value;
}

/**
 *
 * Both bytes (address 12h, bit 7-0 and address 13h, bit 7-0) can be used by
 * customer. Writing or reading of these registers has no effect on the sensor IC
 * functionality.
 */

void bma020_set_customer_reserved_1(uint8_t value)
{
	twi_master_write_register(BMA020_TWI_ADDRESS,
							  BMA020_REGISTER_CUSTOMER_1,
							  value);
}

uint8_t bma020_get_customer_reserved_1()
{
	return twi_master_read_register(BMA020_TWI_ADDRESS,
									BMA020_REGISTER_CUSTOMER_1);
}

void bma020_set_customer_reserved_2(uint8_t value)
{
	twi_master_write_register(BMA020_TWI_ADDRESS,
							  BMA020_REGISTER_CUSTOMER_2,
							  value);
}

uint8_t bma020_get_customer_reserved_2()
{
	return twi_master_read_register(BMA020_TWI_ADDRESS,
									BMA020_REGISTER_CUSTOMER_2);
}

/**
 * Initialize the BMA020 accelerationsensor
 */
void bma020_init(void)
{

}



