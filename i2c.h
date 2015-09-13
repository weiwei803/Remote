//i2c library

#ifndef _I2C_H_
#define _I2C_H_

void i2c_init();
void i2c_start();
void i2c_stop();
void i2c_ack();
void i2c_nak();
unsigned char i2c_read();
void i2c_restart();
void i2c_send(unsigned char dat);
void i2c_wait();

void i2c_init(){
    TRISC3 = 1;      // SDA and SCL as input pin
    TRISC4 = 1;      // these pins can be configured either i/p or o/p
    SSPSTAT &= 0x7F; // Slew rate enabled
    SSPCON = 0x28;   // SSPEN = 1, I2C Master mode, clock = FOSC/(4 * (SSPADD + 1))
    SSPADD = 0x0B;   // 400Khz @ 20Mhz Fosc
}

void i2c_start(){
    SEN = 1;     // Start condition enabled
    while(SEN);  // automatically cleared by hardware, wait for start condition to finish
}

void i2c_stop(){
    PEN = 1;     // Stop condition enabled
    while(PEN);  // Wait for stop condition to finish, PEN automatically cleared by hardware
}

void i2c_restart(){
    RSEN = 1;     // Repeated start enabled
    while(RSEN);  // wait for condition to finish
}

void i2c_ack(){
    ACKDT = 0;      // Acknowledge data bit, 0 = ACK
    ACKEN = 1;      // Ack data enabled
    while(ACKEN);   // wait for ack data to send on bus
}

void i2c_nak(){
    ACKDT = 1;      // Acknowledge data bit, 1 = NAK
    ACKEN = 1;      // Ack data enabled
    while(ACKEN);   // wait for ack data to send on bus
}

void i2c_wait(){
    while ((SSPCON2 & 0x1F ) || ( SSPSTAT & 0x04 ) );  // wait for any pending transfer
}

void i2c_send(unsigned char dat){
    SSPBUF = dat;    // Move data to SSPBUF
    while(BF);       // wait till complete data is sent from buffer
    i2c_wait();      // wait for any pending transfer
}

unsigned char i2c_read(){
    unsigned char temp;  // Reception works if transfer is initiated in read mode
    RCEN = 1;            // Enable data reception
    while(!BF);          // wait for buffer full
    temp = SSPBUF;       // Read serial buffer and store in temp register
    i2c_wait();          // wait to check any pending transfer
    return temp;         // Return the read data from bus
}

#endif
