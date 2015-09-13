#ifndef _USART_H_
#define _USART_H_

/*
USART read and write functions. Use for XBee
Requires MCU library eg PIC16F877A.h for the register names TXSTA. TXIE etc
*/

//unsigned char recieve; //Interrupt recieve byte

void USART_init(long int baud_rate, unsigned char rxie);
void send_byte(unsigned char byte);
unsigned char receive_byte();


void USART_init(long int baud_rate, unsigned char rxie){
    TRISC6 = 0;
    TRISC7 = 1;
    TXSTA = 36;
    if(baud_rate == 9600){
        SPBRG = 129;
    }
    else if(baud_rate == 19200){
        SPBRG = 64;
    }
    else if(baud_rate == 115200){
        SPBRG = 10;
    }
    RCSTA = 144;
    TXIE = 0;
    if(rxie == 1){
        RCIE = 1;
        GIE = 1;
        PEIE = 1;
    }
    else{
        RCIE = 0;
    }
}

void send_byte(unsigned char byte){
    while(!TXIF);
    TXREG = byte;
}

unsigned char receive_byte(){
    if(OERR){
        CREN = 0;
        CREN = 1;
    }
    while(!RCIF);
    return RCREG;
}

/*void interrupt ISR(){ // Copy into main c program
    if(RCIF){
        //receive = RCREG;//Un-comment if using recieve interrupts, if RCIE = 1
        if(OERR){
            CREN = 0;
            CREN = 1;
        }
    }
}*/

#endif
