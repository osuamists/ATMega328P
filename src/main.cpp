/*
 * ================================================================================
 * PROJETO ATMEGA328P - Microcontroladores e Microprocessadores
 * ================================================================================
 * Autor: Projeto Acadêmico
 * Data: Outubro 2025
 * Microcontrolador: ATmega328P @ 16MHz
 * 
 * DESCRIÇÃO:
 * Projeto completo demonstrando manipulação direta de registradores AVR para:
 * - Controle de LEDs individuais e em bargraph (8 LEDs)
 * - Displays de 7 segmentos multiplexados
 * - Leitura de botões com debounce
 * - Sistema de temporização baseado em Timer1 (1ms)
 * 
 * IMPORTANTE: Este código NÃO usa funções Arduino (digitalWrite, delay, etc.)
 * Utiliza apenas manipulação direta de registradores AVR.
 * ================================================================================
 */

// ================================================================================
// SEÇÃO 1: INCLUDES E BIBLIOTECAS
// ================================================================================
#include <Arduino.h> 
#include <avr/io.h>
#include <avr/interrupt.h>

// ================================================================================
// SEÇÃO 2: MACROS PARA MANIPULAÇÃO DE BITS
// ================================================================================
#define SET_BIT(REG, BIT)    (REG |= (1 << BIT))      // Seta bit (1)
#define CLR_BIT(REG, BIT)    (REG &= ~(1 << BIT))     // Limpa bit (0)
#define TGL_BIT(REG, BIT)    (REG ^= (1 << BIT))      // Inverte bit (toggle)
#define READ_BIT(REG, BIT)   ((REG >> BIT) & 1)       // Lê bit (retorna 0 ou 1)

// ================================================================================
// SEÇÃO 3: DEFINIÇÕES DE PINOS
// ================================================================================

// MÓDULO 1 - LEDs
// NOTA: LED_TESTE movido para PC5 para evitar conflito com displays em PORTB
#define LED_TESTE        PC5    // LED de teste (alterado de PB5)

// BARGRAPH - 8 LEDs em PORTD (PD0-PD7)
// PD0=LED0, PD1=LED1, ... PD7=LED7

// MÓDULO 2 - DISPLAYS DE 7 SEGMENTOS
// Segmentos em PORTB (PB0-PB7): a,b,c,d,e,f,g,dp
// PORTB agora é exclusivo para os displays de 7 segmentos
#define SEG_A   PB0
#define SEG_B   PB1
#define SEG_C   PB2
#define SEG_D   PB3
#define SEG_E   PB4
#define SEG_F   PB5
#define SEG_G   PB6
#define SEG_DP  PB7

// Seleção de displays em PORTC (PC0, PC1)
#define SEL_DISP1   PC0
#define SEL_DISP2   PC1

// MÓDULO 3 - BOTÕES
// Configuração flexível para diferentes exercícios
#define BTN1    PC2
#define BTN2    PC3
#define BTN3    PC4

// LEDs para exercícios com botões
// NOTA: Usando PD3-PD5 para evitar conflito com LED_TESTE em PC5
// e com displays em PC0-PC1
#define LED_BTN1    PD3
#define LED_BTN2    PD4
#define LED_BTN3    PD5

// ================================================================================
// SEÇÃO 4: VARIÁVEIS GLOBAIS
// ================================================================================

// Sistema de Timer
volatile unsigned long timer_millis = 0;

// Displays de 7 Segmentos
// Tabela de conversão para dígitos hexadecimais (catodo comum - 1=aceso)
const uint8_t HEX_TABLE[16] = {
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111,  // 9
    0b01110111,  // A
    0b01111100,  // b
    0b00111001,  // C
    0b01011110,  // d
    0b01111001,  // E
    0b01110001   // F
};

uint8_t display1_value = 0;      // Valor do display 1 (0-15)
uint8_t display2_value = 15;     // Valor do display 2 (15-0)
uint8_t display_atual = 0;       // Qual display está ativo (0 ou 1)
unsigned long display_last_update = 0;
unsigned long display_last_multiplex = 0;

// Variáveis para MÓDULO 1 - LEDs
uint8_t modulo1_estado = 0;      // Qual exercício está rodando
unsigned long led_last_update = 0;
uint8_t led_count = 0;
uint8_t led_pattern_step = 0;
int8_t led_direction = 1;        // Direção do movimento (-1 ou 1)
uint8_t blink_count = 0;

// Variáveis para MÓDULO 3 - Botões
uint8_t modulo3_estado = 0;      // Qual exercício está rodando
unsigned long btn_last_debounce[3] = {0, 0, 0};
uint8_t btn_state[3] = {0, 0, 0};          // Estado atual (debounced)
uint8_t btn_last_state[3] = {0, 0, 0};     // Estado anterior
uint8_t btn_pressed[3] = {0, 0, 0};        // Flag de botão pressionado
unsigned long btn_press_time[3] = {0, 0, 0}; // Tempo que botão foi pressionado

// Variáveis específicas dos exercícios de botões
uint8_t ex31_led_state = 0;       // 3.1: LED ligado/desligado
uint8_t ex32_mode = 0;            // 3.2: modo (0=OFF, 1=ON, 2=PISCA, 3=PISCA RÁPIDO)
uint8_t ex33_direction = 0;       // 3.3: direção (0=1-2-3, 1=3-2-1)
uint8_t ex33_led_index = 0;
unsigned long ex33_last_update = 0;
uint8_t ex34_freq_level = 0;      // 3.4: nível de frequência
unsigned long ex34_last_toggle = 0;
uint8_t ex35_freq = 0;            // 3.5: frequência atual
unsigned long ex35_press_start = 0;

// ================================================================================
// SEÇÃO 5: SISTEMA DE TIMER (Timer1 configurado para 1ms)
// ================================================================================

/*
 * Configuração do Timer1 em modo CTC (Clear Timer on Compare Match)
 * Clock: 16MHz
 * Prescaler: 64
 * Frequência do timer: 16MHz / 64 = 250kHz
 * Para interrupção a cada 1ms: OCR1A = 250 - 1 = 249
 */
void timer1_init() {
    // Desabilita interrupções globais durante configuração
    cli();
    
    // Configura Timer1 em modo CTC (WGM12=1, WGM13=0, WGM11=0, WGM10=0)
    TCCR1A = 0;                    // Limpa registrador A
    TCCR1B = 0;                    // Limpa registrador B
    TCNT1 = 0;                     // Zera contador
    
    // Configura valor de comparação para 1ms
    OCR1A = 249;                   // (16MHz / 64 / 1000Hz) - 1
    
    // Ativa modo CTC (WGM12 = 1)
    SET_BIT(TCCR1B, WGM12);
    
    // Configura prescaler 64 (CS11=1, CS10=1)
    SET_BIT(TCCR1B, CS11);
    SET_BIT(TCCR1B, CS10);
    
    // Habilita interrupção de comparação A
    SET_BIT(TIMSK1, OCIE1A);
    
    // Habilita interrupções globais
    sei();
}

/*
 * ISR - Rotina de Interrupção do Timer1
 * Executada a cada 1ms
 */
ISR(TIMER1_COMPA_vect) {
    timer_millis++;
}

/*
 * Função millis_custom() - Retorna tempo em milissegundos
 * Similar ao millis() do Arduino, mas implementado com Timer1
 */
unsigned long millis_custom() {
    unsigned long m;
    cli();                         // Desabilita interrupções
    m = timer_millis;              // Lê valor
    sei();                         // Habilita interrupções
    return m;
}

/*
 * Função delay_ms() - Delay bloqueante baseado em millis_custom()
 * USO: Apenas quando delay bloqueante é necessário
 */
void delay_ms(unsigned long ms) {
    unsigned long start = millis_custom();
    while ((millis_custom() - start) < ms) {
        // Aguarda
    }
}

// ================================================================================
// SEÇÃO 6: MÓDULO 1 - CONTROLE DE LEDs
// ================================================================================

/*
 * EXERCÍCIO 1.1 - Piscar LED (PC5)
 * 3x rápido (200ms) e 3x devagar (500ms), repetir eternamente
 */
void modulo1_ex1() {
    static unsigned long last_toggle = 0;
    static uint8_t fase = 0;  // 0-5: rápido, 6-11: devagar
    unsigned long intervalo;
    
    if (fase < 6) {
        intervalo = 200;  // Rápido
    } else {
        intervalo = 500;  // Devagar
    }
    
    if (millis_custom() - last_toggle >= intervalo) {
        last_toggle = millis_custom();
        TGL_BIT(PORTC, LED_TESTE);
        
        fase++;
        if (fase >= 12) {
            fase = 0;
        }
    }
}

/*
 * EXERCÍCIO 1.2a - Bargraph: Acender da direita para esquerda
 * Mantém acesos, apaga todos, repete
 */
void modulo1_ex2a() {
    static unsigned long last_update = 0;
    static uint8_t step = 0;
    
    if (millis_custom() - last_update >= 200) {
        last_update = millis_custom();
        
        if (step < 8) {
            // Acende LED da direita para esquerda
            SET_BIT(PORTD, step);
        } else if (step == 8) {
            // Mantém acesos por 500ms
            delay_ms(500);
        } else if (step == 9) {
            // Apaga todos
            PORTD = 0x00;
            delay_ms(300);
        }
        
        step++;
        if (step >= 10) {
            step = 0;
        }
    }
}

/*
 * EXERCÍCIO 1.2b - Bargraph: Acender da esquerda para direita
 * Mantém acesos, apaga todos, repete
 */
void modulo1_ex2b() {
    static unsigned long last_update = 0;
    static uint8_t step = 0;
    
    if (millis_custom() - last_update >= 200) {
        last_update = millis_custom();
        
        if (step < 8) {
            // Acende LED da esquerda para direita (7 até 0)
            SET_BIT(PORTD, 7 - step);
        } else if (step == 8) {
            delay_ms(500);
        } else if (step == 9) {
            PORTD = 0x00;
            delay_ms(300);
        }
        
        step++;
        if (step >= 10) {
            step = 0;
        }
    }
}

/*
 * EXERCÍCIO 1.2c - Bargraph: Apenas 1 LED aceso por vez
 * Da direita para esquerda
 */
void modulo1_ex2c() {
    static unsigned long last_update = 0;
    static uint8_t position = 0;
    
    if (millis_custom() - last_update >= 150) {
        last_update = millis_custom();
        
        // Apaga todos e acende apenas o LED atual
        PORTD = (1 << position);
        
        position++;
        if (position >= 8) {
            position = 0;
        }
    }
}

/*
 * EXERCÍCIO 1.2d - Bargraph: Ping-pong
 * 1 LED aceso por vez, vai e volta
 */
void modulo1_ex2d() {
    static unsigned long last_update = 0;
    static uint8_t position = 0;
    static int8_t direction = 1;
    
    if (millis_custom() - last_update >= 100) {
        last_update = millis_custom();
        
        PORTD = (1 << position);
        
        position += direction;
        
        // Inverte direção nas extremidades
        if (position >= 7 && direction > 0) {
            direction = -1;
        } else if (position <= 0 && direction < 0) {
            direction = 1;
        }
    }
}

/*
 * EXERCÍCIO 1.2e - Bargraph: Todos acesos, apagar 1 por vez em vai e volta
 */
void modulo1_ex2e() {
    static unsigned long last_update = 0;
    static uint8_t position = 0;
    static int8_t direction = 1;
    static uint8_t leds = 0xFF;  // Todos acesos
    
    if (millis_custom() - last_update >= 150) {
        last_update = millis_custom();
        
        // Apaga o LED na posição atual
        CLR_BIT(leds, position);
        PORTD = leds;
        
        position += direction;
        
        // Inverte direção e reacende todos quando chega no fim
        if (position >= 7 && direction > 0) {
            direction = -1;
        } else if (position <= 0 && direction < 0) {
            direction = 1;
            leds = 0xFF;  // Reacende todos
        }
    }
}

/*
 * EXERCÍCIO 1.2f - Bargraph: Esquerda para direita mantendo acesos
 * Piscar todos 5x, apagar todos
 */
void modulo1_ex2f() {
    static unsigned long last_update = 0;
    static uint8_t step = 0;
    static uint8_t blink_counter = 0;
    
    if (step < 8) {
        // Acende da esquerda para direita
        if (millis_custom() - last_update >= 200) {
            last_update = millis_custom();
            SET_BIT(PORTD, 7 - step);
            step++;
        }
    } else if (step < 18) {
        // Pisca 5 vezes (10 transições)
        if (millis_custom() - last_update >= 200) {
            last_update = millis_custom();
            if (PORTD == 0xFF) {
                PORTD = 0x00;
            } else {
                PORTD = 0xFF;
            }
            blink_counter++;
            if (blink_counter >= 10) {
                step = 18;
                blink_counter = 0;
            } else {
                step++;
            }
        }
    } else {
        // Apaga todos e reinicia
        PORTD = 0x00;
        delay_ms(500);
        step = 0;
    }
}

/*
 * EXERCÍCIO 1.2g - Bargraph: Direita para esquerda mantendo acesos, apagar
 * Depois esquerda para direita
 */
void modulo1_ex2g() {
    static unsigned long last_update = 0;
    static uint8_t step = 0;
    
    if (step < 8) {
        // Direita para esquerda
        if (millis_custom() - last_update >= 200) {
            last_update = millis_custom();
            SET_BIT(PORTD, step);
            step++;
        }
    } else if (step == 8) {
        // Pausa
        if (millis_custom() - last_update >= 500) {
            last_update = millis_custom();
            PORTD = 0x00;
            step++;
        }
    } else if (step < 17) {
        // Esquerda para direita
        if (millis_custom() - last_update >= 200) {
            last_update = millis_custom();
            SET_BIT(PORTD, 7 - (step - 9));
            step++;
        }
    } else {
        // Apaga e reinicia
        delay_ms(500);
        PORTD = 0x00;
        delay_ms(300);
        step = 0;
    }
}

/*
 * EXERCÍCIO 1.2h - Bargraph: Contagem binária crescente 0-255
 * Passo de 250ms
 */
void modulo1_ex2h() {
    static unsigned long last_update = 0;
    static uint8_t counter = 0;
    
    if (millis_custom() - last_update >= 250) {
        last_update = millis_custom();
        PORTD = counter;
        counter++;
    }
}

/*
 * EXERCÍCIO 1.2i - Bargraph: Contagem binária decrescente 255-0
 * Passo de 250ms
 */
void modulo1_ex2i() {
    static unsigned long last_update = 0;
    static uint8_t counter = 255;
    
    if (millis_custom() - last_update >= 250) {
        last_update = millis_custom();
        PORTD = counter;
        counter--;
    }
}

// ================================================================================
// SEÇÃO 7: MÓDULO 2 - DISPLAYS DE 7 SEGMENTOS
// ================================================================================

/*
 * Atualiza display de 7 segmentos
 * Multiplexação entre os 2 displays
 */
void atualizar_displays() {
    static unsigned long last_multiplex = 0;
    static unsigned long last_count = 0;
    
    // Multiplexação a cada 5ms
    if (millis_custom() - last_multiplex >= 5) {
        last_multiplex = millis_custom();
        
        if (display_atual == 0) {
            // Desliga display 2, liga display 1
            CLR_BIT(PORTC, SEL_DISP2);
            PORTB = HEX_TABLE[display1_value];
            SET_BIT(PORTC, SEL_DISP1);
            display_atual = 1;
        } else {
            // Desliga display 1, liga display 2
            CLR_BIT(PORTC, SEL_DISP1);
            PORTB = HEX_TABLE[display2_value];
            SET_BIT(PORTC, SEL_DISP2);
            display_atual = 0;
        }
    }
    
    // Atualiza contagem a cada 500ms
    if (millis_custom() - last_count >= 500) {
        last_count = millis_custom();
        
        // Display 1: crescente 0→F
        display1_value++;
        if (display1_value > 15) {
            display1_value = 0;
        }
        
        // Display 2: decrescente F→0
        if (display2_value == 0) {
            display2_value = 15;
        } else {
            display2_value--;
        }
    }
}

// ================================================================================
// SEÇÃO 8: MÓDULO 3 - LEITURA DE BOTÕES COM DEBOUNCE
// ================================================================================

/*
 * Lê estado dos botões com debounce de 50ms
 * Atualiza arrays btn_state[] e btn_pressed[]
 */
void ler_botoes() {
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t btn_pin = BTN1 + i;
        uint8_t reading = !READ_BIT(PINC, btn_pin);  // Botão com pull-up (invertido)
        
        // Se mudou o estado, reseta o debounce
        if (reading != btn_last_state[i]) {
            btn_last_debounce[i] = millis_custom();
        }
        
        // Se passou o tempo de debounce
        if ((millis_custom() - btn_last_debounce[i]) > 50) {
            // Se o estado mudou
            if (reading != btn_state[i]) {
                btn_state[i] = reading;
                
                // Se foi pressionado (transição 0->1)
                if (btn_state[i] == 1) {
                    btn_pressed[i] = 1;
                    btn_press_time[i] = millis_custom();
                }
            }
        }
        
        btn_last_state[i] = reading;
    }
}

/*
 * EXERCÍCIO 3.1 - 1 botão + 1 LED
 * 1º clique acende, 2º apaga
 */
void modulo3_ex1() {
    if (btn_pressed[0]) {
        btn_pressed[0] = 0;
        ex31_led_state = !ex31_led_state;
    }
    
    if (ex31_led_state) {
        SET_BIT(PORTD, LED_BTN1);
    } else {
        CLR_BIT(PORTD, LED_BTN1);
    }
}

/*
 * EXERCÍCIO 3.2 - 1 botão + 1 LED
 * ON → Piscar → Piscar rápido → OFF (ciclo)
 */
void modulo3_ex2() {
    static unsigned long last_blink = 0;
    
    if (btn_pressed[0]) {
        btn_pressed[0] = 0;
        ex32_mode++;
        if (ex32_mode > 3) {
            ex32_mode = 0;
        }
    }
    
    switch (ex32_mode) {
        case 0:  // OFF
            CLR_BIT(PORTD, LED_BTN1);
            break;
        case 1:  // ON
            SET_BIT(PORTD, LED_BTN1);
            break;
        case 2:  // Piscar normal (500ms)
            if (millis_custom() - last_blink >= 500) {
                last_blink = millis_custom();
                TGL_BIT(PORTD, LED_BTN1);
            }
            break;
        case 3:  // Piscar rápido (100ms)
            if (millis_custom() - last_blink >= 100) {
                last_blink = millis_custom();
                TGL_BIT(PORTD, LED_BTN1);
            }
            break;
    }
}

/*
 * EXERCÍCIO 3.3 - 1 botão + 3 LEDs
 * Sequência 1-2-3, apertar inverte para 3-2-1
 */
void modulo3_ex3() {
    static unsigned long last_update = 0;
    
    if (btn_pressed[0]) {
        btn_pressed[0] = 0;
        ex33_direction = !ex33_direction;
        ex33_led_index = 0;
    }
    
    if (millis_custom() - last_update >= 500) {
        last_update = millis_custom();
        
        // Apaga todos os LEDs
        PORTD &= 0xF8;  // Limpa bits 0-2
        
        // Acende LED correto
        if (ex33_direction == 0) {
            // Sequência 1-2-3 (PD0, PD1, PD2)
            SET_BIT(PORTD, ex33_led_index);
        } else {
            // Sequência 3-2-1
            SET_BIT(PORTD, 2 - ex33_led_index);
        }
        
        ex33_led_index++;
        if (ex33_led_index >= 3) {
            ex33_led_index = 0;
        }
    }
}

/*
 * EXERCÍCIO 3.4 - 1 botão + 1 LED
 * Enquanto pressionado, LED pisca com frequência crescente até fixo
 */
void modulo3_ex4() {
    static unsigned long last_toggle = 0;
    static uint16_t interval = 500;
    
    if (btn_state[0]) {
        // Botão pressionado - aumenta frequência
        if (interval > 10) {
            interval -= 5;
        } else {
            interval = 0;  // LED fixo
        }
        
        if (interval == 0) {
            SET_BIT(PORTD, LED_BTN1);
        } else if (millis_custom() - last_toggle >= interval) {
            last_toggle = millis_custom();
            TGL_BIT(PORTD, LED_BTN1);
        }
    } else {
        // Botão solto - reseta
        CLR_BIT(PORTD, LED_BTN1);
        interval = 500;
    }
}

/*
 * EXERCÍCIO 3.5 - 1 botão + 1 LED
 * Cada clique aumenta frequência, segurar 5s apaga
 */
void modulo3_ex5() {
    static unsigned long last_toggle = 0;
    static uint16_t intervals[] = {1000, 500, 250, 100, 50};
    
    // Detecta clique curto
    if (btn_pressed[0] && !btn_state[0]) {
        btn_pressed[0] = 0;
        ex35_freq++;
        if (ex35_freq >= 5) {
            ex35_freq = 0;
        }
    }
    
    // Detecta segurar por 5s
    if (btn_state[0] && (millis_custom() - btn_press_time[0] >= 5000)) {
        ex35_freq = 0;
        btn_pressed[0] = 0;
    }
    
    // Pisca LED conforme frequência
    if (ex35_freq > 0) {
        if (millis_custom() - last_toggle >= intervals[ex35_freq - 1]) {
            last_toggle = millis_custom();
            TGL_BIT(PORTD, LED_BTN1);
        }
    } else {
        CLR_BIT(PORTD, LED_BTN1);
    }
}

/*
 * EXERCÍCIO 3.6 - 2 botões + 1 LED
 * LED acende se qualquer um pressionado, apaga se ambos
 */
void modulo3_ex6() {
    if (btn_state[0] && btn_state[1]) {
        // Ambos pressionados - apaga
        CLR_BIT(PORTD, LED_BTN1);
    } else if (btn_state[0] || btn_state[1]) {
        // Qualquer um pressionado - acende
        SET_BIT(PORTD, LED_BTN1);
    } else {
        // Nenhum pressionado - apaga
        CLR_BIT(PORTD, LED_BTN1);
    }
}

/*
 * EXERCÍCIO 3.7 - 2 botões + 2 LEDs
 * Botão1→LED1 aceso/LED2 piscando
 * Botão2→inverter
 * Ambos→apagar
 */
void modulo3_ex7() {
    static unsigned long last_blink = 0;
    static uint8_t modo = 0;  // 0=LED1 aceso/LED2 pisca, 1=LED2 aceso/LED1 pisca
    
    if (btn_state[0] && btn_state[1]) {
        // Ambos - apaga
        CLR_BIT(PORTD, LED_BTN1);
        CLR_BIT(PORTD, LED_BTN2);
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
            SET_BIT(PORTD, LED_BTN1);
            if (millis_custom() - last_blink >= 300) {
                last_blink = millis_custom();
                TGL_BIT(PORTD, LED_BTN2);
            }
        } else {
            SET_BIT(PORTD, LED_BTN2);
            if (millis_custom() - last_blink >= 300) {
                last_blink = millis_custom();
                TGL_BIT(PORTD, LED_BTN1);
            }
        }
    }
}

/*
 * EXERCÍCIO 3.8 - 2 botões + 3 LEDs
 * Botão1→sequência 1-2-3
 * Botão2→3-2-1
 * Ambos→apagar
 */
void modulo3_ex8() {
    static unsigned long last_update = 0;
    static uint8_t index = 0;
    static uint8_t modo = 0;  // 0=parado, 1=crescente, 2=decrescente
    
    if (btn_state[0] && btn_state[1]) {
        // Ambos - apaga tudo
        PORTD &= 0xF8;
        modo = 0;
    } else if (btn_state[0]) {
        modo = 1;  // Crescente
    } else if (btn_state[1]) {
        modo = 2;  // Decrescente
    }
    
    if (modo != 0 && millis_custom() - last_update >= 400) {
        last_update = millis_custom();
        
        PORTD &= 0xF8;  // Apaga LEDs 0-2
        
        if (modo == 1) {
            SET_BIT(PORTD, index);
        } else {
            SET_BIT(PORTD, 2 - index);
        }
        
        index++;
        if (index >= 3) {
            index = 0;
        }
    }
}

/*
 * EXERCÍCIO 3.9 - 3 botões + 4 LEDs
 * Botão1→todos acesos
 * Botão2→LED1+2
 * Botão3→LED3+4
 * Botão1+3→apagar
 */
void modulo3_ex9() {
    if (btn_state[0] && btn_state[2]) {
        // Botão 1 + 3 - apaga todos
        PORTD &= 0xF0;
    } else if (btn_state[0]) {
        // Botão 1 - todos acesos
        PORTD = (PORTD & 0xF0) | 0x0F;
    } else if (btn_state[1]) {
        // Botão 2 - LED 0 e 1
        PORTD = (PORTD & 0xF0) | 0x03;
    } else if (btn_state[2]) {
        // Botão 3 - LED 2 e 3
        PORTD = (PORTD & 0xF0) | 0x0C;
    }
}

/*
 * EXERCÍCIO 3.10 - 3 botões + 3 LEDs + display
 * Botão1→display mostra "1"/LEDs:[ON,OFF,PISCANDO]
 * Botão2→display mostra "2"/LEDs:[OFF,ON,ON]
 * Botão3→display mostra "3"/LEDs:[PISCANDO,PISCANDO,OFF]
 */
void modulo3_ex10() {
    static unsigned long last_blink = 0;
    static uint8_t modo_atual = 0;
    
    if (btn_pressed[0]) {
        btn_pressed[0] = 0;
        modo_atual = 1;
    }
    if (btn_pressed[1]) {
        btn_pressed[1] = 0;
        modo_atual = 2;
    }
    if (btn_pressed[2]) {
        btn_pressed[2] = 0;
        modo_atual = 3;
    }
    
    // Atualiza display
    display1_value = modo_atual;
    
    // Atualiza LEDs
    switch (modo_atual) {
        case 1:
            // LED0=ON, LED1=OFF, LED2=PISCANDO
            SET_BIT(PORTD, 0);
            CLR_BIT(PORTD, 1);
            if (millis_custom() - last_blink >= 300) {
                last_blink = millis_custom();
                TGL_BIT(PORTD, 2);
            }
            break;
        case 2:
            // LED0=OFF, LED1=ON, LED2=ON
            CLR_BIT(PORTD, 0);
            SET_BIT(PORTD, 1);
            SET_BIT(PORTD, 2);
            break;
        case 3:
            // LED0=PISCANDO, LED1=PISCANDO, LED2=OFF
            CLR_BIT(PORTD, 2);
            if (millis_custom() - last_blink >= 300) {
                last_blink = millis_custom();
                TGL_BIT(PORTD, 0);
                TGL_BIT(PORTD, 1);
            }
            break;
        default:
            PORTD &= 0xF8;
            break;
    }
}

// ================================================================================
// SEÇÃO 9: FUNÇÕES SETUP E LOOP
// ================================================================================

void setup() {
    // PORTD inteiro como saída (bargraph + LEDs de botões em PD3-PD5)
    DDRD = 0xFF;
    PORTD = 0x00;
    
    // PORTB como saída (segmentos dos displays - uso exclusivo)
    DDRB = 0xFF;
    PORTB = 0x00;
    
    // PORTC: 
    // PC0-PC1 como saída (seleção de displays)
    // PC2-PC4 como entrada com pull-up (botões)
    // PC5 como saída (LED_TESTE)
    SET_BIT(DDRC, SEL_DISP1);
    SET_BIT(DDRC, SEL_DISP2);
    SET_BIT(DDRC, LED_TESTE);   // LED_TESTE em PC5
    
    CLR_BIT(DDRC, BTN1);        // Botões como entrada
    CLR_BIT(DDRC, BTN2);
    CLR_BIT(DDRC, BTN3);
    SET_BIT(PORTC, BTN1);       // Ativa pull-up
    SET_BIT(PORTC, BTN2);
    SET_BIT(PORTC, BTN3);
    
    // LEDs de botões já configurados em DDRD = 0xFF (PD3-PD5)
    
    // Inicializa Timer1
    timer1_init();
    
    // Estado inicial
    modulo1_estado = 0;  // Altere para selecionar exercício (0-9)
    modulo3_estado = 0;  // Altere para selecionar exercício (0-9)
}

void loop() {
    // Atualiza leitura de botões
    ler_botoes();
    
    // ========================================
    // SELEÇÃO DE MÓDULOS - ALTERE AQUI
    // ========================================
    
    // DESCOMENTE O MÓDULO QUE DESEJA EXECUTAR:
    
    // --- MÓDULO 1: LEDs ---
    modulo1_ex1();      // 1.1 - Piscar LED 3x rápido, 3x devagar
    // modulo1_ex2a();     // 1.2a - Direita→Esquerda mantendo
    // modulo1_ex2b();     // 1.2b - Esquerda→Direita mantendo
    // modulo1_ex2c();     // 1.2c - 1 LED por vez D→E
    // modulo1_ex2d();     // 1.2d - Ping-pong
    // modulo1_ex2e();     // 1.2e - Todos acesos, apagar em vai-volta
    // modulo1_ex2f();     // 1.2f - E→D mantendo, piscar 5x
    // modulo1_ex2g();     // 1.2g - D→E mantendo, apagar, E→D
    // modulo1_ex2h();     // 1.2h - Contagem binária 0-255
    // modulo1_ex2i();     // 1.2i - Contagem binária 255-0
    
    // --- MÓDULO 2: Displays ---
    // atualizar_displays();  // Displays contadores hex
    
    // --- MÓDULO 3: Botões ---
    // modulo3_ex1();      // 3.1 - 1 botão toggle LED
    // modulo3_ex2();      // 3.2 - Ciclo ON/Pisca/Rápido/OFF
    // modulo3_ex3();      // 3.3 - Sequência 1-2-3 / 3-2-1
    // modulo3_ex4();      // 3.4 - Frequência crescente
    // modulo3_ex5();      // 3.5 - Clique aumenta freq, segurar apaga
    // modulo3_ex6();      // 3.6 - 2 botões, LED se um pressionado
    // modulo3_ex7();      // 3.7 - 2 botões, 2 LEDs alternados
    // modulo3_ex8();      // 3.8 - 2 botões, sequência LEDs
    // modulo3_ex9();      // 3.9 - 3 botões, 4 LEDs combinações
    modulo3_ex10();     // 3.10 - 3 botões, 3 LEDs, display
    
    // Para testar displays junto com outros módulos, descomente:
    atualizar_displays();
}

/*
 * ================================================================================
 * FIM DO CÓDIGO
 * ================================================================================
 * 
 * INSTRUÇÕES DE USO:
 * 
 * 1. No arquivo platformio.ini, certifique-se de ter:
 *    [env:uno]
 *    platform = atmelavr
 *    board = uno
 *    framework = arduino
 *    
 * 2. Para selecionar qual exercício executar, vá até a função loop() e
 *    descomente a função correspondente.
 *    
 * 3. Compile e faça upload para o Arduino Uno.
 * 
 * 4. CONEXÕES DE HARDWARE SUGERIDAS:
 *    - LED_TESTE: PC5 com resistor de 220Ω
 *    - MÓDULO 1 (Bargraph): LEDs em PD0-PD7 com resistores de 220Ω
 *    - MÓDULO 2 (Displays): 
 *        * Segmentos a-g+dp em PB0-PB7 (PORTB exclusivo para displays)
 *        * Seleção de displays em PC0-PC1 (transistores para multiplexação)
 *    - MÓDULO 3 (Botões):
 *        * Botões em PC2-PC4 com pull-up interno habilitado
 *        * LEDs de botões em PD3-PD5 com resistores de 220Ω
 * 
 * ================================================================================
 */
