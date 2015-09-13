#ifndef _LCD_H_
#define _LCD_H_

#define RS RB3
#define EN RB2
#define DB0 RB1
#define DB1 RB0
#define DB2 RD7
#define DB3 RD6
#define DB4 RD5
#define DB5 RD4
#define DB6 RD3
#define DB7 RD2

void data_write(unsigned char a);
void first_row();
void second_row();
void write_text(const char *s);
void clear_screen();
void write_int(int n);
void lcd_init();

void data_write(unsigned char a){
    unsigned char bin[8], i;
    for(i = 0; i < 8; i++){
        bin[i] = a % 2;
        a /= 2;
    }
    DB0 = bin[0];
    DB1 = bin[1];
    DB2 = bin[2];
    DB3 = bin[3];
    DB4 = bin[4];
    DB5 = bin[5];
    DB6 = bin[6];
    DB7 = bin[7];
    EN = 1;
    __delay_ms(5);
    EN = 0;
}

void first_row(){
    RS = 0;
    data_write(128);
}

void second_row(){
    RS = 0;
    data_write(168);
}

void write_text(const char * s){
    RS = 1;
    while(*s != '\0'){
        data_write(*s);
        s++;
    }
}

void clear_screen(){
    RS = 0;
    data_write(1);
}

void write_int(int n){ 
    RS = 1;
    if(n < 0){
        write_text("-");
        n *= -1;
    }
    else{
        write_text("+");
    }
    data_write(((n / 1000) % 10) + 48);
    data_write(((n / 100) % 10) + 48);
    data_write(((n / 10) % 10) + 48);
    data_write((n % 10) + 48);
}

void lcd_init(){
    RS = 0;
    data_write(56);   
    data_write(12);   
    data_write(1);    
    data_write(6); 
}

#endif
