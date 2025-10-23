/*
 * ================================================================================
 * MÓDULO 1 - CONTROLE DE LEDs (BARGRAPH)
 * ================================================================================
 * Microcontrolador: ATmega328P @ 16MHz
 * 
 * DESCRIÇÃO:
 * 9 exercícios de controle de LEDs usando manipulação direta de registradores AVR:
 * - Ex 1.1: Piscar LED 3x rápido, 3x devagar
 * - Ex 1.2a-i: Diversos padrões com bargraph de 8 LEDs
 * 
 * CONEXÕES DE HARDWARE:
 * - LED_TESTE: PC5 com resistor de 220Ω
 * - BARGRAPH (8 LEDs completos em PORTB):
 *   * PB0 (pino 12) → LED0 com resistor de 220Ω
 *   * PB1 (pino 13) → LED1 com resistor de 220Ω
 *   * PB2 (pino 14) → LED2 com resistor de 220Ω
 *   * PB3 (pino 15) → LED3 com resistor de 220Ω
 *   * PB4 (pino 16) → LED4 com resistor de 220Ω
 *   * PB5 (pino 17) → LED5 com resistor de 220Ω
 *   * PB6 (pino 7)  → LED6 com resistor de 220Ω
 *   * PB7 (pino 8)  → LED7 com resistor de 220Ω
 *   ✅ PORTD livre - PD5/PD6 reservados para o cristal de 16MHz
8*/


#include <Arduino.h> 
#include <avr/io.h>
#include <avr/interrupt.h>

// ================================================================================
// MACROS PARA MANIPULAÇÃO DE BITS
// ================================================================================
#define SET_BIT(REG, BIT)    (REG |= (1 << BIT))
#define CLR_BIT(REG, BIT)    (REG &= ~(1 << BIT))
#define TGL_BIT(REG, BIT)    (REG ^= (1 << BIT))
#define READ_BIT(REG, BIT)   ((REG >> BIT) & 1)

// ================================================================================
// DEFINIÇÕES DE PINOS
// ================================================================================
#define LED_TESTE   PC5    // LED de teste individual

// BARGRAPH - Usando PORTB completo (8 LEDs)
// PORTB: PB0-PB7 = 8 LEDs para bargraph
// PORTD: Livre, PD5/PD6 reservados para cristal de 16MHz

// ================================================================================
// VARIÁVEIS GLOBAIS
// ================================================================================
volatile unsigned long timer_millis = 0;
uint8_t exercicio_atual = 0;  // 0=Ex1.1, 1=Ex1.2a, 2=Ex1.2b, etc.
unsigned long exercise_start_time = 0;
unsigned long exercise_duration = 2000;  // 2 segundos por exercício

// ================================================================================
// SISTEMA DE TIMER (Timer1 - 1ms)
// ================================================================================
void timer1_init() {
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 249;  // (16MHz / 64 / 1000Hz) - 1
    SET_BIT(TCCR1B, WGM12);  // Modo CTC
    SET_BIT(TCCR1B, CS11);   // Prescaler 64
    SET_BIT(TCCR1B, CS10);
    SET_BIT(TIMSK1, OCIE1A); // Interrupção
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

// ================================================================================
// EXERCÍCIO 1.1 - Piscar LED (PC5)
// 3x rápido (200ms) e 3x devagar (500ms), repetir eternamente
// ================================================================================
void modulo1_ex1() {
    static unsigned long last_toggle = 0;
    static uint8_t fase = 0;  // 0-5: rápido, 6-11: devagar
    unsigned long intervalo = (fase < 6) ? 200 : 500;
    
    if (millis_custom() - last_toggle >= intervalo) {
        last_toggle = millis_custom();
        TGL_BIT(PORTC, LED_TESTE);
        fase++;
        if (fase >= 12) fase = 0;
    }
}

// ================================================================================
// EXERCÍCIO 1.2a - Bargraph: Acender da esquerda para direita
// Mantém acesos, apaga todos, repete (2x)
// ================================================================================
void modulo1_ex2a() {
    static unsigned long last_update = 0;
    static uint8_t step = 0;
    static uint8_t leds = 0;
    static uint8_t repeats = 0;
    
    if (millis_custom() - last_update >= 100) {
        last_update = millis_custom();
        
        if (step < 8) {
            SET_BIT(leds, step);
            PORTB = leds;
            step++;
        } else if (step == 8) {
            step++;
        } else if (step == 9) {
            leds = 0;
            PORTB = 0;
            step = 0;
            repeats++;
            if (repeats >= 2) {
                repeats = 0;
                step = 10;  // Força saída
            }
        }
    }
}

// ================================================================================
// EXERCÍCIO 1.2b - Bargraph: Acender da direita para esquerda
// Mantém acesos, apaga todos, repete (2x)
// ================================================================================
void modulo1_ex2b() {
    static unsigned long last_update = 0;
    static uint8_t step = 0;
    static uint8_t leds = 0;
    static uint8_t repeats = 0;
    
    if (millis_custom() - last_update >= 100) {
        last_update = millis_custom();
        
        if (step < 8) {
            SET_BIT(leds, (7 - step));
            PORTB = leds;
            step++;
        } else if (step == 8) {
            step++;
        } else if (step == 9) {
            leds = 0;
            PORTB = 0;
            step = 0;
            repeats++;
            if (repeats >= 2) {
                repeats = 0;
                step = 10;  // Força saída
            }
        }
    }
}

// ================================================================================
// EXERCÍCIO 1.2c - Bargraph: Apenas 1 LED aceso por vez
// Da esquerda para a direita (2x)
// ================================================================================
void modulo1_ex2c() {
    static unsigned long last_update = 0;
    static uint8_t position = 0;
    static uint8_t repeats = 0;
    
    if (millis_custom() - last_update >= 75) {
        last_update = millis_custom();
        PORTB = 1 << position;
        position++;
        if (position >= 8) {
            position = 0;
            repeats++;
            if (repeats >= 2) {
                repeats = 0;
                PORTB = 0;
            }
        }
    }
}

// ================================================================================
// EXERCÍCIO 1.2d - Bargraph: Ping-pong
// 1 LED aceso por vez, vai e volta (2x)
// ================================================================================
void modulo1_ex2d() {
    static unsigned long last_update = 0;
    static uint8_t position = 0;
    static int8_t direction = 1;
    static uint8_t repeats = 0;
    
    if (millis_custom() - last_update >= 75) {
        last_update = millis_custom();
        PORTB = 1 << position;
        
        position += direction;
        
        if (position >= 7 && direction > 0) {
            direction = -1;
        } else if (position <= 0 && direction < 0) {
            direction = 1;
            repeats++;
            if (repeats >= 2) {
                repeats = 0;
                PORTB = 0;
            }
        }
    }
}

// ================================================================================
// EXERCÍCIO 1.2e - Bargraph: Todos acesos, apagar 1 por vez em vai e volta (2x)
// ================================================================================
void modulo1_ex2e() {
    static unsigned long last_update = 0;
    static uint8_t position = 0;
    static int8_t direction = 1;
    static uint8_t leds = 0xFF;
    static uint8_t repeats = 0;
    static uint8_t initialized = 0;
    static uint8_t show_all_time = 0;  // Contador para mostrar todos acesos
    
    // Inicializa todos os LEDs acesos
    if (!initialized) {
        leds = 0xFF;
        PORTB = 0xFF;
        position = 0;
        direction = 1;
        initialized = 1;
        show_all_time = 150;  // Mostra todos acesos por 150ms
    }
    
    if (millis_custom() - last_update >= 75) {
        last_update = millis_custom();
        
        if (show_all_time > 0) {
            // Mostra todos os LEDs acesos no início
            show_all_time -= 75;
            PORTB = 0xFF;
        } else {
            // Começa a apagar
            CLR_BIT(leds, position);
            PORTB = leds;
            
            // Move para o próximo LED
            if (direction > 0) {
                if (position < 7) {
                    position++;
                } else {
                    // Chegou ao final, volta
                    direction = -1;
                    position--;
                }
            } else {
                if (position > 0) {
                    position--;
                } else {
                    // Voltou ao início, próximo ciclo
                    direction = 1;
                    leds = 0xFF;  // Reacende todos
                    repeats++;
                    show_all_time = 150;  // Mostra todos acesos novamente
                    
                    if (repeats >= 2) {
                        repeats = 0;
                        initialized = 0;
                        PORTB = 0;
                    }
                }
            }
        }
    }
}

// ================================================================================
// EXERCÍCIO 1.2f - Bargraph: Esquerda para direita mantendo acesos
// Piscar todos 2x, apagar todos
// ================================================================================
void modulo1_ex2f() {
    static unsigned long last_update = 0;
    static uint8_t step = 0;
    static uint8_t blink_counter = 0;
    static uint8_t leds = 0;
    
    if (step < 8) {
        if (millis_custom() - last_update >= 100) {
            last_update = millis_custom();
            SET_BIT(leds, (7 - step));
            PORTB = leds;
            step++;
        }
    } else if (step < 14) {
        if (millis_custom() - last_update >= 150) {
            last_update = millis_custom();
            if (leds == 0xFF) {
                leds = 0x00;
            } else {
                leds = 0xFF;
            }
            PORTB = leds;
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
        step = 0;
        leds = 0;
    }
}

// ================================================================================
// EXERCÍCIO 1.2g - Bargraph: Direita para esquerda mantendo acesos, apagar
// Depois esquerda para direita (2x)
// ================================================================================
void modulo1_ex2g() {
    static unsigned long last_update = 0;
    static uint8_t step = 0;
    static uint8_t leds = 0;
    static uint8_t repeats = 0;
    
    if (step < 8) {
        if (millis_custom() - last_update >= 100) {
            last_update = millis_custom();
            SET_BIT(leds, step);
            PORTB = leds;
            step++;
        }
    } else if (step == 8) {
        if (millis_custom() - last_update >= 200) {
            last_update = millis_custom();
            leds = 0;
            PORTB = 0;
            step++;
        }
    } else if (step < 17) {
        if (millis_custom() - last_update >= 100) {
            last_update = millis_custom();
            SET_BIT(leds, (7 - (step - 9)));
            PORTB = leds;
            step++;
        }
    } else {
        leds = 0;
        PORTB = 0;
        repeats++;
        if (repeats >= 2) {
            repeats = 0;
            step = 100;  // Força saída
        } else {
            step = 0;
        }
    }
}

// ================================================================================
// EXERCÍCIO 1.2h - Bargraph: Contagem binária crescente 0-255 (2x)
// ================================================================================
void modulo1_ex2h() {
    static unsigned long last_update = 0;
    static uint8_t counter = 0;
    static uint8_t repeats = 0;
    
    if (millis_custom() - last_update >= 150) {
        last_update = millis_custom();
        PORTB = counter;
        counter++;
        if (counter == 0) {  // Transbordou (passou de 255)
            repeats++;
            if (repeats >= 2) {
                repeats = 0;
                PORTB = 0;
            }
        }
    }
}

// ================================================================================
// EXERCÍCIO 1.2i - Bargraph: Contagem binária decrescente 255-0 (2x)
// ================================================================================
void modulo1_ex2i() {
    static unsigned long last_update = 0;
    static uint8_t counter = 255;
    static uint8_t repeats = 0;
    static uint8_t initialized = 0;
    
    if (!initialized) {
        PORTB = 255;
        initialized = 1;
    }
    
    if (millis_custom() - last_update >= 150) {
        last_update = millis_custom();
        PORTB = counter;
        counter--;
        if (counter == 255) {  // Transbordou (passou de 0)
            repeats++;
            if (repeats >= 2) {
                repeats = 0;
                initialized = 0;
                PORTB = 0;
            } else {
                counter = 255;
            }
        }
    }
}

// ================================================================================
// SETUP E LOOP
// ================================================================================
void setup() {
    // Configura PORTB (LEDs do bargraph) como saída
    DDRB = 0xFF;  // PB0-PB7 como saída
    PORTB = 0;
    
    // Configura LED_TESTE em PC5
    SET_BIT(DDRC, LED_TESTE);
    CLR_BIT(PORTC, LED_TESTE);
    
    // Inicializa Timer1
    timer1_init();
    
    // ========================================
    // SELECIONE O EXERCÍCIO AQUI (0-9):
    // ========================================
    exercicio_atual = 1;  // Começa em 1.2a
    exercise_start_time = millis_custom();
}

void loop() {
    // Verifica se tempo do exercício atual expirou
    if (millis_custom() - exercise_start_time >= exercise_duration) {
        exercicio_atual++;
        if (exercicio_atual > 9) exercicio_atual = 1;  // Volta para 1.2a após 1.2i
        exercise_start_time = millis_custom();
        PORTB = 0;  // Apaga todos os LEDs na transição
        delay_ms(500);  // Pausa entre exercícios
    }
    
    switch (exercicio_atual) {
        case 0:  modulo1_ex1();   break;  // 1.1 - Piscar LED 3x rápido/devagar
        case 1:  modulo1_ex2a();  break;  // 1.2a - Direita→Esquerda mantendo
        case 2:  modulo1_ex2b();  break;  // 1.2b - Esquerda→Direita mantendo
        case 3:  modulo1_ex2c();  break;  // 1.2c - 1 LED por vez D→E
        case 4:  modulo1_ex2d();  break;  // 1.2d - Ping-pong
        case 5:  modulo1_ex2e();  break;  // 1.2e - Apagar 1 por vez vai-volta
        case 6:  modulo1_ex2f();  break;  // 1.2f - E→D mantendo, piscar 5x
        case 7:  modulo1_ex2g();  break;  // 1.2g - D→E, apagar, E→D
        case 8:  modulo1_ex2h();  break;  // 1.2h - Contagem binária 0-255
        case 9:  modulo1_ex2i();  break;  // 1.2i - Contagem binária 255-0
        default: modulo1_ex1();   break;
    }
}
