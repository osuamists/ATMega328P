/*
 * ================================================================================
 * MÓDULO 3 - CONTROLE DE BOTÕES E LEDs
 * ================================================================================
 * Microcontrolador: ATmega328P @ 16MHz
 * 
 * DESCRIÇÃO:
 * 10 exercícios de interação com botões e LEDs com debounce
 * - Ex 3.1: Toggle LED com botão
 * - Ex 3.2: Ciclo de modos (ON → Pisca → Rápido → OFF)
 * - Ex 3.3: Sequência 1-2-3 / 3-2-1 invertida com botão
 * - Ex 3.4: Frequência crescente enquanto pressionado
 * - Ex 3.5: Clique aumenta frequência, segurar 5s apaga
 * - Ex 3.6: 2 botões + 1 LED (qualquer um acende, ambos apaga)
 * - Ex 3.7: 2 botões + 2 LEDs alternados
 * - Ex 3.8: 2 botões + 3 LEDs sequência
 * - Ex 3.9: 3 botões + 4 LEDs combinações
 * - Ex 3.10: 3 botões + 3 LEDs + display
 * 
 * CONEXÕES DE HARDWARE:
 * - BOTÕES (PORTC - entrada com pull-up):
 *   * PC2 (pino 25) → Botão 1 (conectar ao GND quando pressionado)
 *   * PC3 (pino 26) → Botão 2 (conectar ao GND quando pressionado)
 *   * PC4 (pino 27) → Botão 3 (conectar ao GND quando pressionado)
 * 
 * - LEDs (PORTB - saída):
 *   * PB0 (pino 14) → LED 1 com resistor de 220Ω
 *   * PB1 (pino 15) → LED 2 com resistor de 220Ω
 *   * PB2 (pino 16) → LED 3 com resistor de 220Ω
 *   * PB3 (pino 17) → LED 4 com resistor de 220Ω (para ex 3.9)
 * 
 * - DISPLAY (Opcional - para Ex 3.10):
 *   * PB4-PB7 para segmentos
 *   * PC0-PC1 para seleção
 * 
 * SELEÇÃO DE EXERCÍCIO:
 * Altere a variável 'exercicio_atual' na função setup() para escolher (1-10)
 * ================================================================================
 */

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
// Botões em PORTC
#define BTN1    PC2
#define BTN2    PC3
#define BTN3    PC4

// LEDs em PORTB
#define LED1    PB0
#define LED2    PB1
#define LED3    PB2
#define LED4    PB3

// Display (para Ex 3.10)
#define SEG_A   PB4
#define SEG_B   PB5
#define SEG_C   PB6
#define SEG_D   PB7
#define SEL_DISP1   PC0
#define SEL_DISP2   PC1

// ================================================================================
// VARIÁVEIS GLOBAIS
// ================================================================================
volatile unsigned long timer_millis = 0;
uint8_t exercicio_atual = 1;  // 1-10

// Sistema de debounce
unsigned long btn_last_debounce[3] = {0, 0, 0};
uint8_t btn_state[3] = {0, 0, 0};          // Estado atual (debounced)
uint8_t btn_last_state[3] = {0, 0, 0};     // Estado anterior
uint8_t btn_pressed[3] = {0, 0, 0};        // Flag de botão pressionado
unsigned long btn_press_time[3] = {0, 0, 0};

// Variáveis específicas dos exercícios
uint8_t ex31_led_state = 0;
uint8_t ex32_mode = 0;
uint8_t ex33_direction = 0;
uint8_t ex33_led_index = 0;
unsigned long ex33_last_update = 0;
uint8_t ex35_freq = 0;

// Display (Ex 3.10)
const uint8_t DIGIT_TABLE[10] = {
    0b11110000,  // 0 (simplificado - 4 segmentos)
    0b01100000,  // 1
    0b11010000,  // 2
    0b11110000,  // 3
    0b01100000,  // 4
    0b10110000,  // 5
    0b10110000,  // 6
    0b11100000,  // 7
    0b11110000,  // 8
    0b11110000   // 9
};
uint8_t display_value = 0;

// ================================================================================
// SISTEMA DE TIMER (Timer1 - 1ms)
// ================================================================================
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

// ================================================================================
// FUNÇÃO DE LEITURA DE BOTÕES COM DEBOUNCE
// ================================================================================
void ler_botoes() {
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t btn_pin = BTN1 + i;
        uint8_t reading = !READ_BIT(PINC, btn_pin);  // Invertido (pull-up)
        
        if (reading != btn_last_state[i]) {
            btn_last_debounce[i] = millis_custom();
        }
        
        if ((millis_custom() - btn_last_debounce[i]) > 50) {
            if (reading != btn_state[i]) {
                btn_state[i] = reading;
                
                if (btn_state[i] == 1) {
                    btn_pressed[i] = 1;
                    btn_press_time[i] = millis_custom();
                }
            }
        }
        
        btn_last_state[i] = reading;
    }
}

// ================================================================================
// EXERCÍCIO 3.1 - 1 botão + 1 LED (toggle)
// ================================================================================
void modulo3_ex1() {
    if (btn_pressed[0]) {
        btn_pressed[0] = 0;
        ex31_led_state = !ex31_led_state;
    }
    
    if (ex31_led_state) {
        SET_BIT(PORTB, LED1);
    } else {
        CLR_BIT(PORTB, LED1);
    }
}

// ================================================================================
// EXERCÍCIO 3.2 - 1 botão + 1 LED (ciclo de modos)
// ON → Piscar → Rápido → OFF
// ================================================================================
void modulo3_ex2() {
    static unsigned long last_blink = 0;
    
    if (btn_pressed[0]) {
        btn_pressed[0] = 0;
        ex32_mode++;
        if (ex32_mode > 3) ex32_mode = 0;
    }
    
    switch (ex32_mode) {
        case 0:  // OFF
            CLR_BIT(PORTB, LED1);
            break;
        case 1:  // ON
            SET_BIT(PORTB, LED1);
            break;
        case 2:  // Piscar normal (500ms)
            if (millis_custom() - last_blink >= 500) {
                last_blink = millis_custom();
                TGL_BIT(PORTB, LED1);
            }
            break;
        case 3:  // Piscar rápido (100ms)
            if (millis_custom() - last_blink >= 100) {
                last_blink = millis_custom();
                TGL_BIT(PORTB, LED1);
            }
            break;
    }
}

// ================================================================================
// EXERCÍCIO 3.3 - 1 botão + 3 LEDs (sequência 1-2-3 / 3-2-1)
// ================================================================================
void modulo3_ex3() {
    if (btn_pressed[0]) {
        btn_pressed[0] = 0;
        ex33_direction = !ex33_direction;
        ex33_led_index = 0;
    }
    
    if (millis_custom() - ex33_last_update >= 500) {
        ex33_last_update = millis_custom();
        
        // Apaga LEDs 1-3
        PORTB &= 0xF8;
        
        if (ex33_direction == 0) {
            SET_BIT(PORTB, LED1 + ex33_led_index);
        } else {
            SET_BIT(PORTB, LED1 + (2 - ex33_led_index));
        }
        
        ex33_led_index++;
        if (ex33_led_index >= 3) ex33_led_index = 0;
    }
}

// ================================================================================
// EXERCÍCIO 3.4 - 1 botão + 1 LED (frequência crescente)
// ================================================================================
void modulo3_ex4() {
    static unsigned long last_toggle = 0;
    static uint16_t interval = 500;
    
    if (btn_state[0]) {
        if (interval > 10) {
            interval -= 5;
        } else {
            interval = 0;
        }
        
        if (interval == 0) {
            SET_BIT(PORTB, LED1);
        } else if (millis_custom() - last_toggle >= interval) {
            last_toggle = millis_custom();
            TGL_BIT(PORTB, LED1);
        }
    } else {
        CLR_BIT(PORTB, LED1);
        interval = 500;
    }
}

// ================================================================================
// EXERCÍCIO 3.5 - 1 botão + 1 LED (clique aumenta freq, segurar 5s apaga)
// ================================================================================
void modulo3_ex5() {
    static unsigned long last_toggle = 0;
    static uint16_t intervals[] = {1000, 500, 250, 100, 50};
    
    // Clique curto
    if (btn_pressed[0] && !btn_state[0]) {
        btn_pressed[0] = 0;
        ex35_freq++;
        if (ex35_freq >= 5) ex35_freq = 0;
    }
    
    // Segurar 5s
    if (btn_state[0] && (millis_custom() - btn_press_time[0] >= 5000)) {
        ex35_freq = 0;
        btn_pressed[0] = 0;
    }
    
    // Pisca LED
    if (ex35_freq > 0) {
        if (millis_custom() - last_toggle >= intervals[ex35_freq - 1]) {
            last_toggle = millis_custom();
            TGL_BIT(PORTB, LED1);
        }
    } else {
        CLR_BIT(PORTB, LED1);
    }
}

// ================================================================================
// EXERCÍCIO 3.6 - 2 botões + 1 LED
// ================================================================================
void modulo3_ex6() {
    if (btn_state[0] && btn_state[1]) {
        CLR_BIT(PORTB, LED1);
    } else if (btn_state[0] || btn_state[1]) {
        SET_BIT(PORTB, LED1);
    } else {
        CLR_BIT(PORTB, LED1);
    }
}

// ================================================================================
// EXERCÍCIO 3.7 - 2 botões + 2 LEDs
// ================================================================================
void modulo3_ex7() {
    static unsigned long last_blink = 0;
    static uint8_t modo = 0;
    
    if (btn_state[0] && btn_state[1]) {
        CLR_BIT(PORTB, LED1);
        CLR_BIT(PORTB, LED2);
    } else {
        if (btn_pressed[0]) {
            btn_pressed[0] = 0;
            modo = 0;
        }
        if (btn_pressed[1]) {
            btn_pressed[1] = 0;
            modo = 1;
        }
        
        if (modo == 0) {
            SET_BIT(PORTB, LED1);
            if (millis_custom() - last_blink >= 300) {
                last_blink = millis_custom();
                TGL_BIT(PORTB, LED2);
            }
        } else {
            SET_BIT(PORTB, LED2);
            if (millis_custom() - last_blink >= 300) {
                last_blink = millis_custom();
                TGL_BIT(PORTB, LED1);
            }
        }
    }
}

// ================================================================================
// EXERCÍCIO 3.8 - 2 botões + 3 LEDs (sequência)
// ================================================================================
void modulo3_ex8() {
    static unsigned long last_update = 0;
    static uint8_t index = 0;
    static uint8_t modo = 0;
    
    if (btn_state[0] && btn_state[1]) {
        PORTB &= 0xF8;
        modo = 0;
    } else if (btn_state[0]) {
        modo = 1;
    } else if (btn_state[1]) {
        modo = 2;
    }
    
    if (modo != 0 && millis_custom() - last_update >= 400) {
        last_update = millis_custom();
        
        PORTB &= 0xF8;
        
        if (modo == 1) {
            SET_BIT(PORTB, LED1 + index);
        } else {
            SET_BIT(PORTB, LED1 + (2 - index));
        }
        
        index++;
        if (index >= 3) index = 0;
    }
}

// ================================================================================
// EXERCÍCIO 3.9 - 3 botões + 4 LEDs
// ================================================================================
void modulo3_ex9() {
    if (btn_state[0] && btn_state[2]) {
        PORTB &= 0xF0;
    } else if (btn_state[0]) {
        PORTB = (PORTB & 0xF0) | 0x0F;
    } else if (btn_state[1]) {
        PORTB = (PORTB & 0xF0) | 0x03;
    } else if (btn_state[2]) {
        PORTB = (PORTB & 0xF0) | 0x0C;
    }
}

// ================================================================================
// EXERCÍCIO 3.10 - 3 botões + 3 LEDs + display
// ================================================================================
void modulo3_ex10() {
    static unsigned long last_blink = 0;
    
    if (btn_pressed[0]) {
        btn_pressed[0] = 0;
        display_value = 1;
    }
    if (btn_pressed[1]) {
        btn_pressed[1] = 0;
        display_value = 2;
    }
    if (btn_pressed[2]) {
        btn_pressed[2] = 0;
        display_value = 3;
    }
    
    // Atualiza segmentos do display (simplificado)
    PORTB = (PORTB & 0x0F) | DIGIT_TABLE[display_value];
    
    // Atualiza LEDs
    switch (display_value) {
        case 1:
            SET_BIT(PORTB, LED1);
            CLR_BIT(PORTB, LED2);
            if (millis_custom() - last_blink >= 300) {
                last_blink = millis_custom();
                TGL_BIT(PORTB, LED3);
            }
            break;
        case 2:
            CLR_BIT(PORTB, LED1);
            SET_BIT(PORTB, LED2);
            SET_BIT(PORTB, LED3);
            break;
        case 3:
            CLR_BIT(PORTB, LED3);
            if (millis_custom() - last_blink >= 300) {
                last_blink = millis_custom();
                TGL_BIT(PORTB, LED1);
                TGL_BIT(PORTB, LED2);
            }
            break;
        default:
            PORTB &= 0xF8;
            break;
    }
}

// ================================================================================
// SETUP E LOOP
// ================================================================================
void setup() {
    // Configura PORTB como saída (LEDs)
    DDRB = 0xFF;
    PORTB = 0x00;
    
    // Configura botões em PORTC como entrada com pull-up
    CLR_BIT(DDRC, BTN1);
    CLR_BIT(DDRC, BTN2);
    CLR_BIT(DDRC, BTN3);
    SET_BIT(PORTC, BTN1);
    SET_BIT(PORTC, BTN2);
    SET_BIT(PORTC, BTN3);
    
    // Configura seleção de display (Ex 3.10)
    SET_BIT(DDRC, SEL_DISP1);
    SET_BIT(DDRC, SEL_DISP2);
    
    // Inicializa Timer1
    timer1_init();
    
    // ========================================
    // SELECIONE O EXERCÍCIO AQUI (1-10):
    // ========================================
    exercicio_atual = 1;  // 1=Ex3.1, 2=Ex3.2, ..., 10=Ex3.10
}

void loop() {
    ler_botoes();
    
    switch (exercicio_atual) {
        case 1:  modulo3_ex1();   break;
        case 2:  modulo3_ex2();   break;
        case 3:  modulo3_ex3();   break;
        case 4:  modulo3_ex4();   break;
        case 5:  modulo3_ex5();   break;
        case 6:  modulo3_ex6();   break;
        case 7:  modulo3_ex7();   break;
        case 8:  modulo3_ex8();   break;
        case 9:  modulo3_ex9();   break;
        case 10: modulo3_ex10();  break;
        default: modulo3_ex1();   break;
    }
}
