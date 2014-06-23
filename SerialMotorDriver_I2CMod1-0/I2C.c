/*
 * I2C.c
 *
 * Created: 6/4/2014 7:09:26 PM
 *  Author: Nishant Pol
 */ 

#include "I2C.h"

/* Initialize TWI Module */
void init_TWI(void){
    TWSR &= ~((1<<TWPS1)|(1<<TWPS0));           //TWI prescaler = 1
    TWBR = 0x00;                                //No bit rate required for slave
    TWAR = (SLAVE_ADDRESS << TWA0);             //Load slave address, no general call
    TWCR = (1<<TWEN)|                           //Enable TWI module
           (1<<TWEA)|                           //Enable aknowledgement of own address
           (0<<TWSTA)|                          //Not master, so do not generate a start condition
           (0<<TWSTO)|                          //Not master, so do not generate a stop condition
           (1<<TWIE);                           //Enable TWI Interrupt
    return;
}

/* Send Acknowledge on next data byte received (normal operation),
 * Or expect Acknowledge from master on next byte sent
 */
void TWACK(void){
    TWCR = (1<<TWEN)|                           //Enable TWI hardware
           (1<<TWINT)|                          //Clear TWI Interrupt flag
           (1<<TWIE)|                           //Enable TWI interrupt
           (1<<TWEA);                           //Send ACK on next transmission
}

/* Send Not-Acknowledge on next data byte received (error handling),
 * Or expect Not Acknowledge from master on next byte sent
 */
void TWNACK(void){
    TWCR = (1<<TWEN)|                           //Enable TWI hardware
           (1<<TWINT)|                                 //Clear TWI Interrupt flag
           (1<<TWIE)|                                  //Enable TWI interrupt
           (0<<TWEA);                                  //Send NACK on next transmission
}

/* Reset TWI for new transmission */
void TWRESET(void){
    TWCR = (1<<TWEN)|                           //Enable TWI hardware
           (1<<TWINT)|                          //Clear TWI Interrupt flag
           (1<<TWEA)|                                  //Send ACK when addressed with own address
           (1<<TWSTO)|                          //Release I2C bus
           (1<<TWIE);                                  //Enable TWI interrupt
}
