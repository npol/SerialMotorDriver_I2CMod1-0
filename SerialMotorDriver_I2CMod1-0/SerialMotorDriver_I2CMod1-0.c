/*
 * SerialMotorDriver_I2CMod1_0.c
 *
 * Created: 6/3/2014 5:49:55 PM
 *  Author: Nishant Pol
 */

#include "SerialMotorDriver_I2CMod1-0.h"

static inline void motor_init(void);
static inline void M1_Forward(void);
static inline void M1_Reverse(void);
static inline void M2_Forward(void);
static inline void M2_Reverse(void);
static inline void M1_Speed(uint8_t speed);
static inline void M2_Speed(uint8_t speed);

int main(void)
{
    while(1)
    {
        if(new_cmd_flag){
            process_cmd(cmd_buf);
        }
    }
}

ISR(TWI_vect){
    uint8_t TWI_Status = TWSR & 0xf8;               //Get TWI status code from 5MSB in TWSR
    switch(TWI_Status){
        /* Slave Receive Mode
         * When a master addresses the device, an ACK will automatically be sent.  Then the ISR will execute the code
         * in case 0x60 to prepare for reading data (but no data has been received yet).  Once the master sends data,
         * the ISR will execute code in case 0x80, where the slave must read the data and send an ACK to continue reading data.
         * If the master stops the transmission when the slave just sent an ACK, the ISR will execute code in case 0xA0 to
         * reset for a new transmission.  Alternatively, if the slave sends a NACK (to signal an error)
         * to request the end of a transmission, the ISR will execute code in case 0x88.  The base code does not have error handling
         * so the slave always ACK's received data
        */
        case 0x60:                                  //Receive transfer initiated, prepare to receive data
            cmd_buf_ptr = 0;
            TWACK();
            break;

        case 0x80:                                  //Receive transfer in progress, get data and send (N)ACK
            cmd_buf[cmd_buf_ptr] = TWDR;
            if(cmd_buf_ptr == 0)                    //If this is the start of a command, get command length in # of bytes
                get_cmd_size(cmd_buf[cmd_buf_ptr]); //Put command size into global cmd_size
            cmd_buf_ptr++;
            if(cmd_buf_ptr == cmd_size)
                new_cmd_flag = 1;                   //If command is complete, tell main() to process command
            TWACK();
            break;

        case 0x88:                                  //write mode transfer data received, returned NACK
            TWRESET();
            break;

        /* Slave transmit mode
         * After the master sends the slave address and the TWI hardware automatically sends an ACK,
         * the ISR will execute code in case 0xA8 to send the first byte of data.  If the master ACK's
         * the data, the ISR will execute code in case 0xB8 to load the next data byte, otherwise
         * case 0xC0 to allow the master to stop the transmission
         */
        case 0xA8:                              //Master addressed device, slave automatically sent ACK.  Slave must load first data byte
            out_buf_ptr = 0;
            TWDR = out_buf[out_buf_ptr];
            out_buf_ptr++;
            TWACK();
            break;

        case 0xB8:                              //Master ACK'd last data byte sent.  Slave must load next data byte
            TWDR = out_buf[out_buf_ptr];
            out_buf_ptr++;
            TWACK();
            break;

        case 0xC0:                              //Master NACK'd last data byte sent. Slave must reset for new transmission
            TWRESET();
            break;

        default:                                //Error cases
            TWRESET();
            break;
    }
}

ISR(ADC_vect){
    if(ADMUX == I_M1){
        currentIm1 = ADCH;                      //Load 8MSB into current global
        ADMUX = I_M2;                           //Switch ADC mux to other motor
    } else if(ADMUX == I_M2){
        currentIm2 = ADCH;
        ADMUX = I_M1;
    }
    PORTC &= ~LED2;                             //Remove both lines if using LEDs for debugging
    PORTC &= ~LED3;
    if(currentIm1 > CURRENT_THRESHOLD){
        PORTC |= LED2;                          //Indicate overcurrent condition
        M1_Speed(0);
    }
    if(currentIm2 > CURRENT_THRESHOLD){
        PORTC |= LED3;
        M2_Speed(0);
    }
}

void get_cmd_size(uint8_t cmd){
    switch(cmd){
        case 'e':
            cmd_size = 1;
            break;
        case '1':
            cmd_size = 3;
            break;
        case '2':
            cmd_size = 3;
            break;
        case 'i':
        case 'I':
            cmd_size = 1;
            break;
        case 'l':
        case 'L':
            cmd_size = 2;
            break;
        default:
            cmd_size = 0;
            break;
    }
    return;
}

void process_cmd(uint8_t *cmd){
    uint8_t dir;
    uint8_t speed;
    switch(cmd[0]){
        case 'e':
            out_buf[0] = 'e';
            out_buf[1] = 'c';
            out_buf[2] = 'h';
            out_buf[3] = 'o';
            break;

        case '1':                               //Assign speed and direction of motor 1
            dir = cmd[1];
            speed = cmd[2];
            if(dir == 'f' || dir == 'F'){
                M1_Forward();
            } else if(dir == 'r' || dir == 'R'){
                M1_Reverse();
            } else
                load_error_code();
            (void)speed;
            break;

        case '2':                               //Assign speed and direction of motor 2
            dir = cmd[1];
            speed = cmd[2];
            if(dir == 'f' || dir == 'F'){
                M1_Forward();
                } else if(dir == 'r' || dir == 'R'){
                M1_Reverse();
            } else
                load_error_code();
            break;
        case 'i':                               //Get current of motors
        case 'I':                               //Fallthrough on purpose
            out_buf[0] = 0x33;                  //Motor 1 current
            out_buf[1] = 0x55;                  //Motor 2 current
            break;
        case 'l':                               //Turn on or off status LEDs
        case 'L':
            if(cmd[1] == '2'){
                if(cmd[2] == '1' || cmd[2] == 1)
                    PORTC |= LED2;
                else if(cmd[2] == '0' || cmd[2] == 0)
                    PORTC &= ~LED2;
                else
                    load_error_code();
            } else if(cmd[1] == '3'){
                if(cmd[2] == '1' || cmd[2] == 1)
                    PORTC |= LED3;
                else if(cmd[2] == '2' || cmd[2] == 0)
                    PORTC &= ~LED3;
            }
            break;

        default:
            load_error_code();
            break;


    }
}

void load_error_code(void){
    for(int i = 0; i < OUT_BUF_SIZE; i++)
        out_buf[i] = 0x4f;
}

/** Motor Control Functions **/
/* Initialize motor control pins */
static inline void motor_init(void){
    /* Initialize IO pins */
    DDRB |= (1<<PWM1) | (1<<PWM2);              //Motor PWM outputs
    DDRC |= (1<<LED2)|(1<<LED3);                //Make LED pins outputs
    DDRD |= (1<<M1_P)|(1<<M1_N)|                //Make motor direction pins outputs
            (1<<M2_P)|(1<<M2_N);
    
    /* Initialize timer for motor pwm */
    TCCR1A = (1<<COM1A1)|(1<<COM1B1)|(1<<WGM10)|(1<<WGM11);
    TCCR1B = (1<<WGM12);
    OCR1A = 0;
    OCR1B = 0;
    TCCR1B |= (1<<CS12);
    
    /* Initialize ADC for current sensing */
    ADMUX = I_M1;
    ADCSRA = (1<<ADEN)|(1<<ADATE)|(1<<ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
    ADCSRA |= (1<<ADSC);

    DDRC = 0xff;
}

/* Drive motor 1 forward by making M1_P high and M1_N low */
static inline void M1_Forward(void){
    PORTD |= (1<<M1_P);
    PORTD &= ~(1<<M1_N);
    return;
}

/* Drive motor 1 reverse by making M1_P low and M1_N high */
static inline void M1_Reverse(void){
    PORTD &= ~(1<<M1_P);
    PORTD |= (1<<M1_N);
    return;
}

/* Drive motor 2 forward by making M2_P high and M2_N low */
static inline void M2_Forward(void){
    PORTD |= (1<<M2_P);
    PORTD &= ~(1<<M2_N);
    return;
}

/* Drive motor 2 reverse by making M2_P low and M1_N high */
static inline void M2_Reverse(void){
    PORTD &= ~(1<<M2_P);
    PORTD |= (1<<M2_N);
}

static inline void M1_Speed(uint8_t speed){
    OCR1A = SPEED_UNIT*speed;
}

static inline void M2_Speed(uint8_t speed){
    OCR1B = SPEED_UNIT*speed;
}
