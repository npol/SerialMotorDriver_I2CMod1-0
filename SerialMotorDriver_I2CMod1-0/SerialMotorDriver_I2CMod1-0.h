/*
 * SerialMotorDriver_I2CMod1_0.h
 *
 * Created: 6/4/2014 7:09:05 PM
 *  Author: Nishant Pol
 */

#define F_CPU 16_000_000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include "I2C.h"

/* Pin Assignments */
#define SENSE_1 PINC0
#define SENSE_2 PINC1

#define RX PIND0
#define TX PIND1
#define M1_P PIND2
#define M1_N PIND3
#define M2_P PIND4
#define M2_N PIND7

#define PWM1 PINB1
#define PWM2 PINB2

#define LED2 PINC3
#define LED3 PINC2

/* I2C Interface */
#define MAX_CMD_SIZE 8
volatile uint8_t cmd_buf[MAX_CMD_SIZE];
volatile uint8_t cmd_buf_ptr = 0;
volatile uint8_t cmd_size = 0;
volatile uint8_t new_cmd_flag = 0;

#define OUT_BUF_SIZE 8
volatile uint8_t out_buf[OUT_BUF_SIZE];
volatile uint8_t out_buf_ptr = 0;

#define SPEED_UNIT 0x100
#define AVERAGE 16

#define CURRENT_THRESHOLD 0x80               //I motor threshold for 1.5A

#define I_M1 PINC0
#define I_M2 PINC1

volatile uint8_t currentIm1 = 0;
volatile uint8_t currentIm2 = 0;

void get_cmd_size(uint8_t cmd);
void process_cmd(volatile uint8_t *cmd);
void load_error_code(void);

