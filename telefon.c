#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "uart.h"

void setup(void);
void loop(void);

int main(void){
    setup();
    for(;;) loop();
}

void setup(){
    uart_init(UART_BAUD_SELECT_DOUBLE_SPEED(115201, F_CPU));
    PORTB |= 0x38;
    sei();
}

int parseHex(char* buf){
    int result = 0;
    int len = 2;
    for(int i=0; i<len; i++){
        char c = buf[len-i];
        int v = 0;
        if(c>='0' && c<='9'){
            v=(c-'0');
        }else if(c>='a' && c<= 'f'){
            v=(c-'a'+10);
        }else if(c>='A' && c<= 'F'){
            v=(c-'A'+10);
        }
        result |= v<<(4*i);
    }
    return result;
}

void loop(){ //one frame
    uint16_t receive_status = 1;
    do{ //Always empty the receive buffer since there are _delay_xxs in the following code and thus this might not run all that often.
        receive_status = uart_getc();
        char c = receive_status&0xFF;
        receive_status &= 0xFF00;
        if(!receive_status){
        }
    }while(!receive_status);
    //Get switch states
    static uint8_t dial_counter = 0;
    static uint8_t state_impulse = 0;
    if(state_impulse <= 1){
        if(PINB & 0x20){
            if(state_impulse == 0){
                state_impulse = 0x40;
                dial_counter++;
            }
        }else{
            state_impulse = 0;
        }
    }else{
        state_impulse--;
    }
    static uint8_t state_dialing = 0;
    if(state_dialing <= 1){
        if(PINB & 0x10){
            if(state_dialing == 0){
                state_dialing = 0xFF;
                if(dial_counter == 10){
                    dial_counter = 0;
                }
                uart_putc('0'+dial_counter);
                dial_counter = 0;
                uart_putc('\n');
            }
        }else{
            state_dialing = 0;
        }
    }else{
        state_dialing--;
    }
    static uint8_t state_hangup = 0;
    if(state_hangup <= 1){
        if(PINB & 0x08){
            if(state_hangup == 0){
                state_hangup = 0xFF;
                uart_putc('h');
                uart_putc('\n');
            }
        }else{
            state_hangup = 0;
        }
    }else{
        state_hangup--;
    }
    PORTB |= 0x08;
    _delay_us(100);
}
