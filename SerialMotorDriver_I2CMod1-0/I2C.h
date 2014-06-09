/*
 * I2C.h
 *
 * Created: 6/4/2014 7:09:38 PM
 *  Author: Nishant Pol
 */

#include <avr/io.h>
#define SDA PC4
#define SCL PC5

#define SLAVE_ADDRESS 0x10

void init_TWI(void);
void TWACK(void);
void TWNACK(void);
void TWRESET(void);

