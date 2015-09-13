#define _XTAL_FREQ 20000000
#include <xc.h>
#include <pic16f877a.h>
#include <math.h>
#include "LCD.h"
#include "USART.h"
#include "I2C.h"
#include "MPU6050.h"

#define button_A RC2
#define button_B RC1
#define button_X RD0
#define button_Y RC0

#define button_L1 RD1
#define button_L2 RC5
#define button_R1 RE1
#define button_R2 RE2

#define button_L3 RE1
#define button_R3 RA4

#define button_up RB4
#define button_down RB6
#define button_left RB5
#define button_right RB7

#define right RA2

#define mode_dpad 0
#define mode_analog 1
#define mode_receiver 2
#define mode_quad 3

#pragma config FOSC = HS     
#pragma config WDTE = OFF
#pragma config PWRTE = OFF
#pragma config BOREN = OFF
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

unsigned char cursor, c, mode_quad_counter;
int throttle, direction, send[4], x1, x2, y1, y2, x1_offset, x2_offset, y1_offset, y2_offset, receive;

void timer_init(){
    T0CS = 0;
    T0SE = 0;
    GIE = 1;
    PSA = 0;
    PS2 = 1;
    PS1 = 1;
    PS0 = 0;
    TMR0 = 0;
    TMR0IE = 1;
}

int get_adc(unsigned char port){
    ADCON0 = port * 8 + 1;
    GO_DONE = 1;
    while(GO_DONE);
    return ((ADRESH  * 256) + ADRESL);
}

void get_adc_values(unsigned char a){
    int i;
    x1 = 0;
    y1 = 0;
    for(i = 0; i < 10; i++){
        x1 += get_adc(1);
        __delay_us(50);
    }
    x1 /= 10;
    for(i = 0; i < 10; i++){
        y1 += get_adc(2);
        __delay_us(50);
    }
    y1 /= 10;
    if(a){
        x2 = 0;
        for(i = 0; i < 10; i++){
            x2 += get_adc(0);
            __delay_us(50);
        }
        x2 /= 10;
        y2 = 0;
        for(i = 0; i < 10; i++){
            y2 += get_adc(3);
            __delay_us(50);
        }
        y2 /= 10;
    }
}

void calibrate_adc(){
    double tx1 = 0, tx2 = 0, ty1 = 0, ty2 = 0;
    int i;
    for(i = 0; i < 250; i++){
        tx1 += get_adc(0);
        __delay_ms(1);
    }
    for(i = 0; i < 250; i++){
        tx2 += get_adc(1);
        __delay_ms(1);
    }
    for(i = 0; i < 250; i++){
        tx2 += get_adc(2);
        __delay_ms(1);
    }
    for(i = 0; i < 250; i++){
        tx2 += get_adc(3);
        __delay_ms(1);
    }
    x1_offset = (int)(tx1 / 250);
    x2_offset = (int)(tx2 / 250);
    y1_offset = (int)(ty1 / 250);
    y2_offset = (int)(ty2 / 250);
}

void interrupt ISR(){
    if(RCIF){
        receive = RCREG;
        if(OERR){
            CREN = 0;
            CREN = 1;
        }
    }
    if(TMR0IF){
        TMR0IF = 0;
        if(cursor == mode_dpad){
            send_byte(send[0]);
        }
        if(cursor == mode_quad){
            if(button_Y == 0){
                throttle = 0;
            }
            else if(button_R2 == 0){
                throttle++;
                if(throttle > 255) throttle = 255;
            }
            else if(button_L2 == 0){
                throttle--;
                if(throttle < 0) throttle = 0;
            }
            if(mode_quad_counter == 0){
                send_byte(throttle / 2);
                mode_quad_counter++;
            }
            else{
                mode_quad_counter = 0;
                send_byte(send[1]);
            }
        }
    }
}

void init(){
    TRISA = 0b00111111;
    TRISB = 0b11110000;
    TRISC = 0b00100111;
    TRISD = 0b00000011;
    TRISE = 0b00000111;
    ADCON0 = 0b00000001;
    ADCON1 = 0b10000010;
    i2c_init();
    USART_init(9600, 0);
}

void main(){
    float x_angle, y_angle;
    signed char t;
    receive = 0;
    init();
    __delay_ms(20);
    MPU6050_init();
    lcd_init();
    cursor = 0;
    first_row();
    write_text("Property of");
    second_row();
    write_text("Shivam Sharma");
    __delay_ms(1000);
    clear_screen();
    do{
        get_adc_values(0);
        if(cursor == mode_dpad){
            if(button_down == 0 || y1 > 700) cursor = 2;
            else if(button_right == 0 || x1 > 700) cursor = 1;
        }      
        else if(cursor == mode_analog){
            if(button_down == 0 || y1 > 700) cursor = 3;
            else if(button_left == 0 || x1 < 200) cursor = 0;
        }
        else if(cursor == mode_receiver){
            if(button_up == 0 || y1 < 200) cursor = 0;
            else if(button_right == 0 || x1 > 700) cursor = 3;
        }
        else if(cursor == mode_quad){
            if(button_up == 0 || y1 < 200) cursor = 1;
            else if(button_left == 0 || x1 < 200) cursor = 2;
        }
        first_row();
        if(cursor == mode_dpad)write_text("->");
        else write_text("  ");
        write_text("D-Pad");
        if(cursor == mode_analog)write_text("->");
        else write_text("  ");
        write_text("Analog");
        second_row();
        if(cursor == mode_receiver)write_text("->");
        else write_text("  ");
        write_text("Receiver");
        if(cursor == mode_quad)write_text("->");
        else write_text("  ");
        write_text("Quad");
    }while(button_A);    
    clear_screen();
    if(cursor == mode_dpad){
        RCIE = 1;
        timer_init();
        while(1){
            if(button_up == 0 || button_down == 0 || button_left == 0 || button_right == 0){
                if(button_up == 0 && button_down == 1 && button_left == 1 && button_right == 1) direction = 1;
                else if(button_up == 0 && button_down == 1 && button_left == 1 && button_right == 0) direction = 2;
                else if(button_up == 1 && button_down == 1 && button_left == 1 && button_right == 0) direction = 3;
                else if(button_up == 1 && button_down == 0 && button_left == 1 && button_right == 0) direction = 4;
                else if(button_up == 1 && button_down == 0 && button_left == 1 && button_right == 1) direction = 5;
                else if(button_up == 1 && button_down == 0 && button_left == 0 && button_right == 1) direction = 6;
                else if(button_up == 1 && button_down == 1 && button_left == 0 && button_right == 1) direction = 7;
                else if(button_up == 0 && button_down == 1 && button_left == 0 && button_right == 1) direction = 8;
                else direction = 0;
            }
            else{
                get_adc_values(0);
                if(x1 < 700 && x1 > 200 && y1 < 200) direction = 1;
                else if(x1 > 600 && y1 < 300) direction = 2;
                else if(x1 > 700 && y1 < 700 && y1 > 200) direction = 3;
                else if(x1 > 600 && y1 > 600) direction = 4;
                else if(x1 < 700 && x1 > 200 && y1 > 700) direction = 5;
                else if(x1 < 300 && y1 > 600) direction = 6;
                else if(x1 < 200 && y1 < 700 && y1 > 200) direction = 7;
                else if(x1 < 300 && y1 < 300) direction = 8;
                else direction = 0;
            }
            if(!button_A){
                direction += 16;
            }
            if(!button_B){
                direction += 32;
            }
            if(!button_X){
                direction += 64;
            }
            if(!button_Y){
                direction += 128;
            }
            send[0] = direction;
            first_row();
            if((send[0] % 16) == 1) write_text("Up         ");
            else if((send[0] % 16) == 2) write_text("Up-Right   ");
            else if((send[0] % 16) == 3) write_text("Right      ");
            else if((send[0] % 16) == 4) write_text("Down-Right ");
            else if((send[0] % 16) == 5) write_text("Down       ");
            else if((send[0] % 16) == 6) write_text("Down-Left  ");
            else if((send[0] % 16) == 7) write_text("Left       ");
            else if((send[0] % 16) == 8) write_text("Up-Left    ");
            else write_text("           ");
            if(((send[0] / 16) % 2) == 1) write_text("A");
            else  write_text(" ");
            if(((send[0] / 32) % 2) == 1) write_text("B");
            else write_text(" ");
            if(((send[0] / 64) % 2) == 1) write_text("X");
            else  write_text(" ");
            if(((send[0] / 128) % 2) == 1) write_text("Y");
            else write_text(" ");
            second_row();
            write_int(receive);
        }
    }
    else if(cursor == mode_analog){
        RCIE = 1;
        timer_init();
        while(1){
            get_adc_values(1);
            first_row();
            write_int(x1);
            write_text(" ");
            write_int(x2);
            second_row();
            write_int(y1);
            write_text(" ");
            write_int(y2);
        }
    }
    else if(cursor == mode_receiver){
        while(1){
            t = receive_byte();
            first_row();
            write_int(t);
        }
    }
    else if(cursor == mode_quad){
        throttle = 0;
        c = 0;
        mode_quad_counter = 0;
        RCIE = 1;
        timer_init();
        while(1){
            if(button_up == 0 || button_down == 0 || button_left == 0 || button_right == 0){
                if(button_up == 0 && button_down == 1 && button_left == 1 && button_right == 1) direction = 1;
                else if(button_up == 0 && button_down == 1 && button_left == 1 && button_right == 0) direction = 2;
                else if(button_up == 1 && button_down == 1 && button_left == 1 && button_right == 0) direction = 3;
                else if(button_up == 1 && button_down == 0 && button_left == 1 && button_right == 0) direction = 4;
                else if(button_up == 1 && button_down == 0 && button_left == 1 && button_right == 1) direction = 5;
                else if(button_up == 1 && button_down == 0 && button_left == 0 && button_right == 1) direction = 6;
                else if(button_up == 1 && button_down == 1 && button_left == 0 && button_right == 1) direction = 7;
                else if(button_up == 0 && button_down == 1 && button_left == 0 && button_right == 1) direction = 8;
                else direction = 0;
            }
            else{
                get_adc_values(0);
                if(x1 < 700 && x1 > 200 && y1 < 200) direction = 1;
                else if(x1 > 600 && y1 < 300) direction = 2;
                else if(x1 > 700 && y1 < 700 && y1 > 200) direction = 3;
                else if(x1 > 600 && y1 > 600) direction = 4;
                else if(x1 < 700 && x1 > 200 && y1 > 700) direction = 5;
                else if(x1 < 300 && y1 > 600) direction = 6;
                else if(x1 < 200 && y1 < 700 && y1 > 200) direction = 7;
                else if(x1 < 300 && y1 < 300) direction = 8;
                else direction = 0;
            }
            if(!button_A){
                direction += 16;
            }
            if(!button_B){
                direction += 32;
            }
            if(!button_X){
                direction += 64;
                throttle = 0;
            }
            send[1] = direction + 128;
            first_row();
            write_text("Throttle: ");
            data_write(((throttle / 100) % 10) + 48);
            data_write(((throttle / 10) % 10) + 48);
            data_write((throttle % 10) + 48);
            second_row();
            if((send[1] % 16) == 1) write_text("Up         ");
            else if((send[1] % 16) == 2) write_text("Up-Right   ");
            else if((send[1] % 16) == 3) write_text("Right      ");
            else if((send[1] % 16) == 4) write_text("Down-Right ");
            else if((send[1] % 16) == 5) write_text("Down       ");
            else if((send[1] % 16) == 6) write_text("Down-Left  ");
            else if((send[1] % 16) == 7) write_text("Left       ");
            else if((send[1] % 16) == 8) write_text("Up-Left    ");
            else write_text("           ");
            if(((send[1] / 16) % 2) == 1) write_text("A");
            else  write_text(" ");
            if(((send[1] / 32) % 2) == 1) write_text("B");
            else write_text(" ");
            if(((send[1] / 64) % 2) == 1) write_text("X");
            else  write_text(" ");
        }
    }
}

