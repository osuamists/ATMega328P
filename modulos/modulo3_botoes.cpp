/*
 * ================================================================================
 * MÓDULO 3 - BOTÕES E LEDs (VERSÃO SIMPLIFICADA E FUNCIONAL)
 * ================================================================================
 * Microcontrolador: ATmega328P @ 16MHz
 * 
 * HARDWARE:
 * - BTN1: PC2 (pino 25) → GND (pull-up interno)
 * - BTN2: PC3 (pino 26) → GND (pull-up interno)
 * - BTN3: PC4 (pino 27) → GND (pull-up interno)
 * - LED1: PD3 (pino 1) → 220Ω → GND
 * - LED2: PD4 (pino 2) → 220Ω → GND
 * - LED3: PB0 (pino 12) → 220Ω → GND
 * - LED4: PB1 (pino 13) → 220Ω → GND
 * 
 * SELECIONE O EXERCÍCIO: exercicio_atual = 1-10
 * ================================================================================
 */

#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// ================================================================================
// MACROS
// ================================================================================
#define SET_BIT(REG, BIT)    (REG |= (1 << BIT))
#define CLR_BIT(REG, BIT)    (REG &= ~(1 << BIT))
#define TGL_BIT(REG, BIT)    (REG ^= (1 << BIT))
#define READ_BIT(REG, BIT)   ((REG >> BIT) & 1)

// ================================================================================
// DEFINIÇÃO DE PINOS
// ================================================================================
// Botões
#define BTN1    PC2
#define BTN2    PC3
#define BTN3    PC4

// LEDs
#define LED1    PD3
#define LED2    PD4
#define LED3    PB0
#define LED4    PB1

// ================================================================================
// VARIÁVEIS GLOBAIS
// ================================================================================
volatile unsigned long timer_millis = 0;
uint8_t exercicio_atual = 1;

// Debounce simplificado
uint8_t btn_last[3] = {0, 0, 0};
uint8_t btn_click[3] = {0, 0, 0};

// Ex 3.1
uint8_t ex31_state = 0;

// Ex 3.2
uint8_t ex32_mode = 0;

// ================================================================================
// TIMER1 - 1ms
// ================================================================================
ISR(TIMER1_COMPA_vect) {
    timer_millis++;
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

unsigned long millis_custom() {
    unsigned long m;
    cli();
    m = timer_millis;
    sei();
    return m;
}

// ================================================================================
// LEITURA DE BOTÕES - VERSÃO ULTRA SIMPLES
// ================================================================================
void ler_botoes() {
    uint8_t btn_pins[3] = {BTN1, BTN2, BTN3};
    
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t reading = !READ_BIT(PINC, btn_pins[i]);  // Invertido (pull-up)
        
        // Detecta borda de descida (1→0 = soltou após clicar)
        if (reading == 0 && btn_last[i] == 1) {
            btn_click[i] = 1;  // Marca clique
        }
        
        btn_last[i] = reading;
    }
}

// ================================================================================
// EXERCÍCIO 3.1 - TOGGLE LED
// ================================================================================
void ex3_1() {
    if (btn_click[0]) {
        btn_click[0] = 0;
        ex31_state = !ex31_state;
    }
    
    if (ex31_state) {
        SET_BIT(PORTD, LED1);
    } else {
        CLR_BIT(PORTD, LED1);
    }
}

// ================================================================================
// EXERCÍCIO 3.2 - CICLO DE MODOS
// ================================================================================
void ex3_2() {
    static unsigned long last_blink = 0;
    
    // Detecta clique
    if (btn_click[0]) {
        btn_click[0] = 0;
        ex32_mode++;
        if (ex32_mode > 3) ex32_mode = 0;
        last_blink = millis_custom();
    }
    
    // Executa modo
    switch (ex32_mode) {
        case 0:  // OFF
            CLR_BIT(PORTD, LED1);
            break;
            
        case 1:  // ON
            SET_BIT(PORTD, LED1);
            break;
            
        case 2:  // Pisca 500ms
            if (millis_custom() - last_blink >= 500) {
                last_blink = millis_custom();
                TGL_BIT(PORTD, LED1);
            }
            break;
            
        case 3:  // Pisca 200ms
            if (millis_custom() - last_blink >= 200) {
                last_blink = millis_custom();
                TGL_BIT(PORTD, LED1);
            }
            break;
    }
}

// ================================================================================
// EXERCÍCIO 3.3 - SEQUÊNCIA 1-2-3 / 3-2-1
// ================================================================================
void ex3_3() {
    static uint8_t direction = 0;
    static uint8_t index = 0;
    static unsigned long last_update = 0;
    
    // Clique = inverte direção
    if (btn_click[0]) {
        btn_click[0] = 0;
        direction = !direction;
        index = 0;
    }
    
    // Avança sequência a cada 500ms
    if (millis_custom() - last_update >= 500) {
        last_update = millis_custom();
        index++;
        if (index >= 3) index = 0;
    }
    
    // Apaga todos
    CLR_BIT(PORTD, LED1);
    CLR_BIT(PORTD, LED2);
    CLR_BIT(PORTB, LED3);
    
    // Acende LED correto
    if (direction == 0) {
        // 1-2-3
        if (index == 0) SET_BIT(PORTD, LED1);
        if (index == 1) SET_BIT(PORTD, LED2);
        if (index == 2) SET_BIT(PORTB, LED3);
    } else {
        // 3-2-1
        if (index == 0) SET_BIT(PORTB, LED3);
        if (index == 1) SET_BIT(PORTD, LED2);
        if (index == 2) SET_BIT(PORTD, LED1);
    }
}

// ================================================================================
// SETUP E LOOP
// ================================================================================
void setup() {
    // Configura LEDs como saída
    SET_BIT(DDRD, LED1);
    SET_BIT(DDRD, LED2);
    SET_BIT(DDRB, LED3);
    SET_BIT(DDRB, LED4);
    
    // Apaga todos LEDs
    CLR_BIT(PORTD, LED1);
    CLR_BIT(PORTD, LED2);
    CLR_BIT(PORTB, LED3);
    CLR_BIT(PORTB, LED4);
    
    // Configura botões como entrada com pull-up
    CLR_BIT(DDRC, BTN1);
    CLR_BIT(DDRC, BTN2);
    CLR_BIT(DDRC, BTN3);
    SET_BIT(PORTC, BTN1);
    SET_BIT(PORTC, BTN2);
    SET_BIT(PORTC, BTN3);
    
    // Inicializa Timer
    timer1_init();
    
    // ========================================
    // SELECIONE O EXERCÍCIO (1-10):
    // ========================================
    exercicio_atual = 1;  // Comece com Ex 3.1
}

void loop() {
    ler_botoes();
    
    switch (exercicio_atual) {
        case 1:  ex3_1();  break;
        case 2:  ex3_2();  break;
        case 3:  ex3_3();  break;
        default: ex3_1();  break;
    }
}
