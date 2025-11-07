/*
 * MÓDULO 1 - CONTROLE DE LEDs
 * ATmega328P @ 16MHz
 * 
 */

#include <Arduino.h> 
#include <avr/io.h>
#include <avr/interrupt.h>

#define SET_BIT(REG, BIT)   (REG |= (1 << BIT))
#define CLR_BIT(REG, BIT)   (REG &= ~(1 << BIT))
#define TGL_BIT(REG, BIT)   (REG ^= (1 << BIT))
#define READ_BIT(REG, BIT)  ((REG >> BIT) & 1)

#define LED_TESTE_PIN   5
#define LED_D7_PIN      0

volatile unsigned long timer_millis = 0;
uint8_t exercicio_atual = 0;
unsigned long exercise_start_time = 0;
unsigned long exercise_duration = 2000;

// Função auxiliar para atualizar D7 baseado em PORTB
inline void update_d7() {
    if (READ_BIT(PORTB, 7)) SET_BIT(PORTC, LED_D7_PIN);
    else CLR_BIT(PORTC, LED_D7_PIN);
}

void timer1_init() {
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 249;
    SET_BIT(TCCR1B, WGM12);
    SET_BIT(TCCR1B, CS11);
    SET_BIT(TCCR1B, CS10);
    SET_BIT(TIMSK1, OCIE1A);
    sei();
}

ISR(TIMER1_COMPA_vect) {
    timer_millis++;
}

unsigned long millis_custom() {
    unsigned long m;
    cli();
    m = timer_millis;
    sei();
    return m;
}

void delay_ms(unsigned long ms) {
    unsigned long start = millis_custom();
    while ((millis_custom() - start) < ms);
}

void modulo1_ex1() {
    static unsigned long last_toggle = 0;
    static uint8_t fase = 0;
    unsigned long intervalo = (fase < 6) ? 200 : 500;
    
    PORTB = 0x00;
    CLR_BIT(PORTC, LED_D7_PIN);
    
    if (millis_custom() - last_toggle >= intervalo) {
        last_toggle = millis_custom();
        TGL_BIT(PORTC, LED_TESTE_PIN);
        fase++;
        if (fase >= 12) fase = 0;
    }
}

void modulo1_ex2a() {
    static unsigned long last_update = 0;
    static uint8_t step = 0;
    static uint8_t leds = 0;
    static uint8_t repeats = 0;
    static uint8_t initialized = 0;
    if (!initialized) {
        PORTB = 0;
        CLR_BIT(PORTC, LED_D7_PIN);
        step = 0;
        leds = 0;
        repeats = 0;
        last_update = millis_custom();
        initialized = 1;
    }
    if (millis_custom() - last_update >= 100) {
        last_update = millis_custom();
        if (step < 8) {
            SET_BIT(leds, step);
            PORTB = leds;
            update_d7();
            step++;
        } else if (step == 8) {
            step++;
        } else if (step == 9) {
            leds = 0;
            PORTB = 0;
            CLR_BIT(PORTC, LED_D7_PIN);
            step = 0;
            repeats++;
            if (repeats >= 2) {
                repeats = 0;
                step = 10;
                initialized = 0;
            }
        }
    }
}

void modulo1_ex2b() {
    static unsigned long last_update = 0;
    static uint8_t step = 0;
    static uint8_t leds = 0;
    static uint8_t repeats = 0;
    static uint8_t initialized = 0;
    if (!initialized) {
        PORTB = 0;
        CLR_BIT(PORTC, LED_D7_PIN);
        step = 0;
        leds = 0;
        repeats = 0;
        last_update = millis_custom();
        initialized = 1;
    }
    if (millis_custom() - last_update >= 100) {
        last_update = millis_custom();
        if (step < 8) {
            SET_BIT(leds, (7 - step));
            PORTB = leds;
            update_d7();
            step++;
        } else if (step == 8) {
            step++;
        } else if (step == 9) {
            leds = 0;
            PORTB = 0;
            CLR_BIT(PORTC, LED_D7_PIN);
            step = 0;
            repeats++;
            if (repeats >= 2) {
                repeats = 0;
                step = 10;
                initialized = 0;
            }
        }
    }
}

void modulo1_ex2c() {
    static unsigned long last_update = 0;
    static uint8_t position = 0;
    static uint8_t repeats = 0;
    static uint8_t initialized = 0;
    if (!initialized) {
        PORTB = 0;
        CLR_BIT(PORTC, LED_D7_PIN);
        position = 0;
        repeats = 0;
        last_update = millis_custom();
        initialized = 1;
    }
    if (millis_custom() - last_update >= 75) {
        last_update = millis_custom();
        PORTB = 1 << position;
        update_d7();
        position++;
        if (position >= 8) {
            position = 0;
            repeats++;
            if (repeats >= 2) {
                repeats = 0;
                PORTB = 0;
                CLR_BIT(PORTC, LED_D7_PIN);
                initialized = 0;
            }
        }
    }
}

void modulo1_ex2d() {
    static unsigned long last_update = 0;
    static uint8_t position = 0;
    static int8_t direction = 1;
    static uint8_t repeats = 0;
    static uint8_t initialized = 0;
    if (!initialized) {
        PORTB = 0;
        CLR_BIT(PORTC, LED_D7_PIN);
        position = 0;
        direction = 1;
        repeats = 0;
        last_update = millis_custom();
        initialized = 1;
    }
    if (millis_custom() - last_update >= 75) {
        last_update = millis_custom();
        PORTB = 1 << position;
        update_d7();
        position += direction;
        if (position >= 7 && direction > 0) {
            direction = -1;
        } else if (position <= 0 && direction < 0) {
            direction = 1;
            repeats++;
            if (repeats >= 2) {
                repeats = 0;
                PORTB = 0;
                CLR_BIT(PORTC, LED_D7_PIN);
                initialized = 0;
            }
        }
    }
}

void modulo1_ex2e() {
    static unsigned long last_update = 0;
    static uint8_t position = 0;
    static int8_t direction = 1;
    static uint8_t leds = 0xFF;
    static uint8_t repeats = 0;
    static uint8_t initialized = 0;
    static uint8_t show_all_time = 0;
    if (!initialized) {
        leds = 0xFF;
        PORTB = 0xFF;
        SET_BIT(PORTC, LED_D7_PIN);
        position = 0;
        direction = 1;
        repeats = 0;
        show_all_time = 150;
        last_update = millis_custom();
        initialized = 1;
    }
    if (millis_custom() - last_update >= 75) {
        last_update = millis_custom();
        if (show_all_time > 0) {
            show_all_time -= 75;
            PORTB = 0xFF;
            SET_BIT(PORTC, LED_D7_PIN);
        } else {
            CLR_BIT(leds, position);
            PORTB = leds;
            update_d7();
            if (direction > 0) {
                if (position < 7) {
                    position++;
                } else {
                    direction = -1;
                    position--;
                }
            } else {
                if (position > 0) {
                    position--;
                } else {
                    direction = 1;
                    leds = 0xFF;
                    repeats++;
                    show_all_time = 150;
                    if (repeats >= 2) {
                        repeats = 0;
                        initialized = 0;
                        PORTB = 0;
                        CLR_BIT(PORTC, LED_D7_PIN);
                    }
                }
            }
        }
    }
}

void modulo1_ex2f() {
    static unsigned long last_update = 0;
    static uint8_t step = 0;
    static uint8_t blink_counter = 0;
    static uint8_t leds = 0;
    static uint8_t initialized = 0;
    if (!initialized) {
        PORTB = 0;
        CLR_BIT(PORTC, LED_D7_PIN);
        step = 0;
        blink_counter = 0;
        leds = 0;
        last_update = millis_custom();
        initialized = 1;
    }
    if (step < 8) {
        if (millis_custom() - last_update >= 100) {
            last_update = millis_custom();
            SET_BIT(leds, step);
            PORTB = leds;
            update_d7();
            step++;
        }
    } else if (step < 14) {
        if (millis_custom() - last_update >= 150) {
            last_update = millis_custom();
            leds = (leds == 0xFF) ? 0x00 : 0xFF;
            PORTB = leds;
            update_d7();
            blink_counter++;
            if (blink_counter >= 4) {
                step = 14;
                blink_counter = 0;
            } else {
                step++;
            }
        }
    } else {
        PORTB = 0;
        CLR_BIT(PORTC, LED_D7_PIN);
        step = 0;
        leds = 0;
        initialized = 0;
    }
}

void modulo1_ex2g() {
    static unsigned long last_update = 0;
    static uint8_t step = 0;
    static uint8_t leds = 0;
    static uint8_t repeats = 0;
    static uint8_t initialized = 0;
    if (!initialized) {
        PORTB = 0;
        CLR_BIT(PORTC, LED_D7_PIN);
        step = 0;
        leds = 0;
        repeats = 0;
        last_update = millis_custom();
        initialized = 1;
    }
    if (step < 8) {
        if (millis_custom() - last_update >= 100) {
            last_update = millis_custom();
            SET_BIT(leds, (7 - step));  
            PORTB = leds;
            update_d7();
            step++;
        }
    } 
    else if (step == 8) {
        if (millis_custom() - last_update >= 200) {
            last_update = millis_custom();
            leds = 0;
            PORTB = 0;
            CLR_BIT(PORTC, LED_D7_PIN);
            step++;
        }
    } 
    else if (step < 17) {
        if (millis_custom() - last_update >= 100) {
            last_update = millis_custom();
            SET_BIT(leds, (step - 9));  
            PORTB = leds;
            update_d7();
            step++;
        }
    } 
    else {
        leds = 0;
        PORTB = 0;
        CLR_BIT(PORTC, LED_D7_PIN);
        repeats++;
        if (repeats >= 2) {
            repeats = 0;
            step = 100;
            initialized = 0;
        } else {
            step = 0;
        }
    }
}

void modulo1_ex2h() {
    static unsigned long last_update = 0;
    static uint8_t counter = 0;
    static uint8_t repeats = 0;
    static uint8_t initialized = 0;
    if (!initialized) {
        PORTB = 0;
        CLR_BIT(PORTC, LED_D7_PIN);
        counter = 0;
        repeats = 0;
        last_update = millis_custom();
        initialized = 1;
    }
    if (millis_custom() - last_update >= 150) {
        last_update = millis_custom();
        PORTB = counter;
        update_d7();
        counter++;
        if (counter == 0) {
            repeats++;
            if (repeats >= 2) {
                repeats = 0;
                PORTB = 0;
                CLR_BIT(PORTC, LED_D7_PIN);
                initialized = 0;
            }
        }
    }
}

void modulo1_ex2i() {
    static unsigned long last_update = 0;
    static uint8_t counter = 255;
    static uint8_t repeats = 0;
    static uint8_t initialized = 0;
    if (!initialized) {
        PORTB = 255;
        SET_BIT(PORTC, LED_D7_PIN);
        counter = 255;
        repeats = 0;
        last_update = millis_custom();
        initialized = 1;
    }
    if (millis_custom() - last_update >= 150) {
        last_update = millis_custom();
        PORTB = counter;
        update_d7();
        counter--;
        if (counter == 255) {
            repeats++;
            if (repeats >= 2) {
                repeats = 0;
                initialized = 0;
                PORTB = 0;
                CLR_BIT(PORTC, LED_D7_PIN);
            } else {
                counter = 255;
            }
        }
    }
}

void setup() {
    MCUCR |= (1 << PUD);
    DDRB = 0xFF;
    PORTB = 0x00;
    DDRC = 0xFF;
    PORTC = 0x00;
    SET_BIT(DDRC, LED_TESTE_PIN);
    CLR_BIT(PORTC, LED_TESTE_PIN);
    SET_BIT(DDRC, LED_D7_PIN);
    CLR_BIT(PORTC, LED_D7_PIN);
    
    timer1_init();
    
    // ═══════════════════════════════════════════════════════════════════
    // ⚙️ CONFIGURE AQUI O EXERCÍCIO QUE DESEJA EXECUTAR (0 a 9):
    // ═══════════════════════════════════════════════════════════════════
    exercicio_atual = 0;  // ← MUDE ESTE NÚMERO
    // ═══════════════════════════════════════════════════════════════════
    
    exercise_start_time = millis_custom();
}

void loop() {
    if (millis_custom() - exercise_start_time >= exercise_duration) {
        PORTB = 0x00;
        PORTC = 0x00;
        DDRB = 0x00;
        DDRC = 0x00;
        delay_ms(700);
        DDRB = 0xFF;
        DDRC = 0xFF;
        PORTB = 0x00;
        PORTC = 0x00;
        
        exercicio_atual++;
        if (exercicio_atual > 9) exercicio_atual = 0;
        exercise_start_time = millis_custom();
    }
    
    switch (exercicio_atual) {
        case 0:  modulo1_ex1();   break;
        case 1:  modulo1_ex2a();  break;
        case 2:  modulo1_ex2b();  break;
        case 3:  modulo1_ex2c();  break;
        case 4:  modulo1_ex2d();  break;
        case 5:  modulo1_ex2e();  break;
        case 6:  modulo1_ex2f();  break;
        case 7:  modulo1_ex2g();  break;
        case 8:  modulo1_ex2h();  break;
        case 9:  modulo1_ex2i();  break;
        default: modulo1_ex1();   break;
    }
}
