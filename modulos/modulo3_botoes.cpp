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

// Display 7 Segmentos (cátodo comum)
#define SEG_A   PC0  // Pino 23
#define SEG_B   PC1  // Pino 24
#define SEG_C   PC5  // Pino 28
#define SEG_D   PD5  // Pino 9
#define SEG_E   PD6  // Pino 10
#define SEG_F   PD7  // Pino 11
#define SEG_G   PB2  // Pino 14

// ================================================================================
// VARIÁVEIS GLOBAIS
// ================================================================================
volatile unsigned long timer_millis = 0;
uint8_t exercicio_atual = 2;  // Ex 3.2 para testar

// Debounce melhorado
uint8_t btn_last[3] = {0, 0, 0};
uint8_t btn_click[3] = {0, 0, 0};
unsigned long btn_last_time[3] = {0, 0, 0};  // Timestamp de última mudança

// Ex 3.1
uint8_t ex31_state = 0;

// Ex 3.2
uint8_t ex32_mode = 0;

// Ex 3.3
uint8_t ex33_running = 0;  // Controla se sequência está rodando

// Ex 3.4
uint8_t ex34_btn_state = 0;

// Ex 3.5
uint8_t ex35_freq_level = 0;

// ================================================================================
// TABELA DE DÍGITOS - DISPLAY 7 SEGMENTOS
// ================================================================================
// Padrão para cada dígito (bits: GFEDCBA)
// Bit 0=A, Bit 1=B, Bit 2=C, Bit 3=D, Bit 4=E, Bit 5=F, Bit 6=G
const uint8_t DIGIT_7SEG[10] = {
    0b00111111,  // 0: A B C D E F
    0b00000110,  // 1: B C
    0b01011011,  // 2: A B D E G
    0b01001111,  // 3: A B C D G
    0b01100110,  // 4: B C F G
    0b01101101,  // 5: A C D F G
    0b01111101,  // 6: A C D E F G
    0b00000111,  // 7: A B C
    0b01111111,  // 8: A B C D E F G
    0b01101111   // 9: A B C D F G
};

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
// LEITURA DE BOTÕES - COM DEBOUNCE POR TEMPO
// ================================================================================
void ler_botoes() {
    uint8_t btn_pins[3] = {BTN1, BTN2, BTN3};
    unsigned long now = millis_custom();
    
    for (uint8_t i = 0; i < 3; i++) {
        // Leitura direta: 1 = solto (pull-up), 0 = pressionado
        uint8_t reading = READ_BIT(PINC, btn_pins[i]);
        
        // Detecta borda de descida (1→0 = pressionado)
        if (reading == 0 && btn_last[i] == 1) {
            // Debounce: só aceita se passou 50ms desde última mudança
            if ((now - btn_last_time[i]) > 50) {
                btn_click[i] = 1;  // Marca clique
                btn_last_time[i] = now;
            }
        }
        
        // Atualiza tempo se houve mudança
        if (reading != btn_last[i]) {
            btn_last_time[i] = now;
        }
        
        btn_last[i] = reading;
    }
}

// ================================================================================
// FUNÇÃO AUXILIAR - ATUALIZA DISPLAY 7 SEGMENTOS
// ================================================================================
void atualizar_display(uint8_t digito) {
    if (digito > 9) digito = 0;  // Limita a 0-9
    
    uint8_t pattern = DIGIT_7SEG[digito];
    
    // Atualiza cada segmento individualmente
    if (pattern & (1 << 0)) SET_BIT(PORTC, SEG_A); else CLR_BIT(PORTC, SEG_A);
    if (pattern & (1 << 1)) SET_BIT(PORTC, SEG_B); else CLR_BIT(PORTC, SEG_B);
    if (pattern & (1 << 2)) SET_BIT(PORTC, SEG_C); else CLR_BIT(PORTC, SEG_C);
    if (pattern & (1 << 3)) SET_BIT(PORTD, SEG_D); else CLR_BIT(PORTD, SEG_D);
    if (pattern & (1 << 4)) SET_BIT(PORTD, SEG_E); else CLR_BIT(PORTD, SEG_E);
    if (pattern & (1 << 5)) SET_BIT(PORTD, SEG_F); else CLR_BIT(PORTD, SEG_F);
    if (pattern & (1 << 6)) SET_BIT(PORTB, SEG_G); else CLR_BIT(PORTB, SEG_G);
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
// EXERCÍCIO 3.2 - LED PISCA RAPIDAMENTE
// ================================================================================
void ex3_2() {
    static unsigned long last_blink = 0;
    
    // Pisca LED1 a cada 200ms (rápido)
    if (millis_custom() - last_blink >= 200) {
        last_blink = millis_custom();
        TGL_BIT(PORTD, LED1);  // Alterna entre ON e OFF
    }
}

// ================================================================================
// EXERCÍCIO 3.3 - SEQUÊNCIA 1-2-3 / 3-2-1 (COMEÇA COM BOTÃO)
// ================================================================================
void ex3_3() {
    static uint8_t direction = 0;  // 0 = 1-2-3, 1 = 3-2-1
    static uint8_t index = 0;      // 0, 1 ou 2
    static unsigned long last_update = 0;
    
    // Clique BTN1 = inicia ou inverte
    if (btn_click[0]) {
        btn_click[0] = 0;
        
        if (ex33_running == 0) {
            // Começa a sequência
            ex33_running = 1;
            direction = 0;
            index = 0;
            last_update = millis_custom();
        } else {
            // Inverte direção
            direction = !direction;
            index = 0;
            last_update = millis_custom();
        }
    }
    
    // Só executa se estiver rodando
    if (ex33_running == 0) {
        // Apaga todos os LEDs
        CLR_BIT(PORTD, LED1);
        CLR_BIT(PORTD, LED2);
        CLR_BIT(PORTB, LED3);
        return;
    }
    
    // Avança sequência a cada 150ms (bem rápido)
    if (millis_custom() - last_update >= 150) {
        last_update = millis_custom();
        index++;
        if (index >= 3) index = 0;
    }
    
    // Apaga todos os LEDs primeiro
    CLR_BIT(PORTD, LED1);
    CLR_BIT(PORTD, LED2);
    CLR_BIT(PORTB, LED3);
    
    // Acende LED correto baseado na direção
    if (direction == 0) {
        // Sequência 1-2-3 (normal)
        if (index == 0) SET_BIT(PORTD, LED1);
        else if (index == 1) SET_BIT(PORTD, LED2);
        else if (index == 2) SET_BIT(PORTB, LED3);
    } else {
        // Sequência 3-2-1 (invertida)
        if (index == 0) SET_BIT(PORTB, LED3);
        else if (index == 1) SET_BIT(PORTD, LED2);
        else if (index == 2) SET_BIT(PORTD, LED1);
    }
}

// ================================================================================
// EXERCÍCIO 3.4 - FREQUÊNCIA CRESCENTE ENQUANTO PRESSIONADO
// ================================================================================
void ex3_4() {
    static unsigned long last_toggle = 0;
    static uint16_t interval = 500;  // Começa em 500ms
    static unsigned long last_decrease = 0;
    
    // Lê estado do botão (1 = solto, 0 = pressionado com pull-up)
    uint8_t btn_pressed = READ_BIT(PINC, BTN1) == 0;  // 1 = pressionado
    
    if (btn_pressed) {
        // Diminui intervalo a cada 200ms (mais rápido)
        if (millis_custom() - last_decrease >= 200) {
            last_decrease = millis_custom();
            if (interval > 20) {
                interval -= 50;  // Diminui mais rapidamente
            }
        }
        
        // Pisca LED com intervalo atual
        if (interval <= 0) {
            // Frequência máxima = aceso fixo
            SET_BIT(PORTD, LED1);
        } else {
            if (millis_custom() - last_toggle >= interval) {
                last_toggle = millis_custom();
                TGL_BIT(PORTD, LED1);
            }
        }
    } else {
        // Botão solto = apaga e reseta
        CLR_BIT(PORTD, LED1);
        interval = 500;  // Reseta intervalo
        last_decrease = millis_custom();
    }
}

// ================================================================================
// EXERCÍCIO 3.5 - CLIQUE AUMENTA FREQUÊNCIA, SEGURAR 5S APAGA
// ================================================================================
void ex3_5() {
    static unsigned long last_toggle = 0;
    static uint8_t freq_level = 0;  // 0-5 (0 = apagado, 5 = aceso fixo)
    static unsigned long btn_press_start = 0;
    static uint8_t btn_was_pressed = 0;
    
    // Intervalos para cada nível de frequência (ms)
    const uint16_t intervals[6] = {0, 500, 250, 125, 62, 31};
    
    // Lê estado do botão (1 = solto, 0 = pressionado com pull-up)
    uint8_t btn_pressed = READ_BIT(PINC, BTN1) == 0;
    
    // Detecta quando começou a pressionar
    if (btn_pressed && !btn_was_pressed) {
        btn_press_start = millis_custom();
    }
    
    // Detecta clique (soltou após pressionar)
    if (!btn_pressed && btn_was_pressed) {
        unsigned long press_duration = millis_custom() - btn_press_start;
        
        // Se foi clique curto (< 500ms), aumenta frequência
        if (press_duration < 500) {
            freq_level++;
            if (freq_level > 5) freq_level = 0;  // Volta ao início
            last_toggle = millis_custom();
        }
    }
    
    // Se segurar por 5 segundos, apaga
    if (btn_pressed && (millis_custom() - btn_press_start >= 5000)) {
        freq_level = 0;
    }
    
    btn_was_pressed = btn_pressed;
    
    // Controla LED baseado no nível de frequência
    if (freq_level == 0) {
        // Apagado
        CLR_BIT(PORTD, LED1);
    } else if (freq_level == 5) {
        // Aceso fixo
        SET_BIT(PORTD, LED1);
    } else {
        // Pisca com intervalo correspondente
        if (millis_custom() - last_toggle >= intervals[freq_level]) {
            last_toggle = millis_custom();
            TGL_BIT(PORTD, LED1);
        }
    }
}

// ================================================================================
// EXERCÍCIO 3.6 - DOIS BOTÕES + 1 LED
// LED acende se qualquer um for pressionado; apaga se ambos forem pressionados
// ================================================================================
void ex3_6() {
    // Lê estado dos botões (1 = solto, 0 = pressionado com pull-up)
    uint8_t btn1_pressed = READ_BIT(PINC, BTN1) == 0;
    uint8_t btn2_pressed = READ_BIT(PINC, BTN2) == 0;
    
    if (btn1_pressed && btn2_pressed) {
        // Ambos pressionados = apaga
        CLR_BIT(PORTD, LED1);
    } else if (btn1_pressed || btn2_pressed) {
        // Qualquer um pressionado = acende
        SET_BIT(PORTD, LED1);
    } else {
        // Nenhum pressionado = apaga
        CLR_BIT(PORTD, LED1);
    }
}

// ================================================================================
// EXERCÍCIO 3.7 - DOIS BOTÕES + 2 LEDs
// Botão 1 → LED1 aceso, LED2 piscando
// Botão 2 → inverte funções
// Ambos → apaga
// ================================================================================
void ex3_7() {
    static unsigned long last_blink = 0;
    static uint8_t modo = 2;  // 2 = inativo, 0 = modo botão 1, 1 = modo botão 2
    
    // Detecta clique nos botões para trocar modo
    if (btn_click[0]) {
        btn_click[0] = 0;
        modo = 0;  // Modo botão 1
    }
    
    if (btn_click[1]) {
        btn_click[1] = 0;
        modo = 1;  // Modo botão 2
    }
    
    // Lê estado dos botões
    uint8_t btn1_pressed = READ_BIT(PINC, BTN1) == 0;
    uint8_t btn2_pressed = READ_BIT(PINC, BTN2) == 0;
    
    if (btn1_pressed && btn2_pressed) {
        // Ambos pressionados = apaga tudo
        CLR_BIT(PORTD, LED1);
        CLR_BIT(PORTD, LED2);
    } else if (modo != 2) {
        // Executa modo atual (apenas se foi selecionado)
        if (modo == 0) {
            // Modo botão 1: LED1 aceso, LED2 piscando
            SET_BIT(PORTD, LED1);
            
            if (millis_custom() - last_blink >= 150) {
                last_blink = millis_custom();
                TGL_BIT(PORTD, LED2);
            }
        } else {
            // Modo botão 2: LED2 aceso, LED1 piscando
            SET_BIT(PORTD, LED2);
            
            if (millis_custom() - last_blink >= 150) {
                last_blink = millis_custom();
                TGL_BIT(PORTD, LED1);
            }
        }
    } else {
        // Nenhum modo selecionado = apaga tudo
        CLR_BIT(PORTD, LED1);
        CLR_BIT(PORTD, LED2);
    }
}

// ================================================================================
// EXERCÍCIO 3.8 - DOIS BOTÕES + 3 LEDs (SEQUÊNCIA)
// Botão 1 → sequência 1-2-3
// Botão 2 → sequência 3-2-1
// Ambos → apagam
// ================================================================================
void ex3_8() {
    static unsigned long last_update = 0;
    static uint8_t index = 0;
    
    // Lê estado dos botões (1 = solto, 0 = pressionado com pull-up)
    uint8_t btn1_pressed = READ_BIT(PINC, BTN1) == 0;
    uint8_t btn2_pressed = READ_BIT(PINC, BTN2) == 0;
    
    // Verifica combinações
    if (btn1_pressed && btn2_pressed) {
        // Ambos pressionados = apaga tudo
        CLR_BIT(PORTD, LED1);
        CLR_BIT(PORTD, LED2);
        CLR_BIT(PORTB, LED3);
        index = 0;
        return;
    }
    
    if (!btn1_pressed && !btn2_pressed) {
        // Nenhum botão = apaga tudo
        CLR_BIT(PORTD, LED1);
        CLR_BIT(PORTD, LED2);
        CLR_BIT(PORTB, LED3);
        index = 0;
        return;
    }
    
    // Avança sequência a cada 150ms
    if (millis_custom() - last_update >= 150) {
        last_update = millis_custom();
        
        // Apaga todos os LEDs
        CLR_BIT(PORTD, LED1);
        CLR_BIT(PORTD, LED2);
        CLR_BIT(PORTB, LED3);
        
        if (btn1_pressed) {
            // Sequência 1-2-3
            if (index == 0) SET_BIT(PORTD, LED1);
            else if (index == 1) SET_BIT(PORTD, LED2);
            else if (index == 2) SET_BIT(PORTB, LED3);
        } else if (btn2_pressed) {
            // Sequência 3-2-1
            if (index == 0) SET_BIT(PORTB, LED3);
            else if (index == 1) SET_BIT(PORTD, LED2);
            else if (index == 2) SET_BIT(PORTD, LED1);
        }
        
        // Avança índice
        index++;
        if (index >= 3) index = 0;
    }
}

// ================================================================================
// EXERCÍCIO 3.9 - TRÊS BOTÕES + 4 LEDs
// Botão 1 → todos acesos (LED1, LED2, LED3, LED4)
// Botão 2 → apenas LED1 e LED2
// Botão 3 → apenas LED3 e LED4
// Botão 1 + Botão 3 → todos apagam
// ================================================================================
void ex3_9() {
    // Lê estado dos botões (1 = solto, 0 = pressionado com pull-up)
    uint8_t btn1_pressed = READ_BIT(PINC, BTN1) == 0;
    uint8_t btn2_pressed = READ_BIT(PINC, BTN2) == 0;
    uint8_t btn3_pressed = READ_BIT(PINC, BTN3) == 0;
    
    // Verifica combinações de botões (ordem importa!)
    if (btn1_pressed && btn3_pressed) {
        // Botão 1 + Botão 3 = todos apagam
        CLR_BIT(PORTD, LED1);
        CLR_BIT(PORTD, LED2);
        CLR_BIT(PORTB, LED3);
        CLR_BIT(PORTB, LED4);
    } 
    else if (btn1_pressed) {
        // Botão 1 sozinho = todos acesos
        SET_BIT(PORTD, LED1);
        SET_BIT(PORTD, LED2);
        SET_BIT(PORTB, LED3);
        SET_BIT(PORTB, LED4);
    } 
    else if (btn2_pressed) {
        // Botão 2 = apenas LED1 e LED2
        SET_BIT(PORTD, LED1);
        SET_BIT(PORTD, LED2);
        CLR_BIT(PORTB, LED3);
        CLR_BIT(PORTB, LED4);
    } 
    else if (btn3_pressed) {
        // Botão 3 = apenas LED3 e LED4
        CLR_BIT(PORTD, LED1);
        CLR_BIT(PORTD, LED2);
        SET_BIT(PORTB, LED3);
        SET_BIT(PORTB, LED4);
    } 
    else {
        // Nenhum botão pressionado = todos apagam
        CLR_BIT(PORTD, LED1);
        CLR_BIT(PORTD, LED2);
        CLR_BIT(PORTB, LED3);
        CLR_BIT(PORTB, LED4);
    }
}

// ================================================================================
// EXERCÍCIO 3.10 - 3 BOTÕES + 3 LEDs + DISPLAY 7 SEGMENTOS
// Botão 1 → display "1"; LEDs: [1-ON, 2-OFF, 3-piscando]
// Botão 2 → display "2"; LEDs: [1-piscando, 2-ON, 3-ON]
// Botão 3 → display "3"; LEDs: [1-OFF, 2-piscando, 3-OFF]
// ================================================================================
void ex3_10() {
    static unsigned long last_blink = 0;
    static uint8_t modo = 0;  // 0 = nenhum, 1/2/3 = modos
    
    // Detecta cliques nos botões
    if (btn_click[0]) {
        btn_click[0] = 0;
        modo = 1;
        last_blink = millis_custom();
    }
    if (btn_click[1]) {
        btn_click[1] = 0;
        modo = 2;
        last_blink = millis_custom();
    }
    if (btn_click[2]) {
        btn_click[2] = 0;
        modo = 3;
        last_blink = millis_custom();
    }
    
    // Atualiza display com o modo atual
    atualizar_display(modo);
    
    // Controla LEDs baseado no modo
    switch (modo) {
        case 1:  // Botão 1: LED1=ON, LED2=OFF, LED3=piscando
            SET_BIT(PORTD, LED1);
            CLR_BIT(PORTD, LED2);
            
            if (millis_custom() - last_blink >= 150) {
                last_blink = millis_custom();
                TGL_BIT(PORTB, LED3);
            }
            break;
            
        case 2:  // Botão 2: LED1=piscando, LED2=ON, LED3=ON
            SET_BIT(PORTD, LED2);
            SET_BIT(PORTB, LED3);
            
            if (millis_custom() - last_blink >= 150) {
                last_blink = millis_custom();
                TGL_BIT(PORTD, LED1);
            }
            break;
            
        case 3:  // Botão 3: LED1=OFF, LED2=piscando, LED3=OFF
            CLR_BIT(PORTD, LED1);
            CLR_BIT(PORTB, LED3);
            
            if (millis_custom() - last_blink >= 150) {
                last_blink = millis_custom();
                TGL_BIT(PORTD, LED2);
            }
            break;
            
        default:  // Nenhum modo = tudo apagado
            CLR_BIT(PORTD, LED1);
            CLR_BIT(PORTD, LED2);
            CLR_BIT(PORTB, LED3);
            atualizar_display(0);  // Display apagado
            break;
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
    
    // ========================================
    // CONFIGURA PINOS DO DISPLAY COMO SAÍDA
    // ========================================
    // PORTC: SEG_A (PC0), SEG_B (PC1), SEG_C (PC5)
    SET_BIT(DDRC, SEG_A);
    SET_BIT(DDRC, SEG_B);
    SET_BIT(DDRC, SEG_C);
    CLR_BIT(PORTC, SEG_A);
    CLR_BIT(PORTC, SEG_B);
    CLR_BIT(PORTC, SEG_C);
    
    // PORTD: SEG_D (PD5), SEG_E (PD6), SEG_F (PD7) - além de LED1 e LED2
    SET_BIT(DDRD, SEG_D);
    SET_BIT(DDRD, SEG_E);
    SET_BIT(DDRD, SEG_F);
    CLR_BIT(PORTD, SEG_D);
    CLR_BIT(PORTD, SEG_E);
    CLR_BIT(PORTD, SEG_F);
    
    // PORTB: SEG_G (PB2) - além de LED3 e LED4
    SET_BIT(DDRB, SEG_G);
    CLR_BIT(PORTB, SEG_G);
    
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
    exercicio_atual = 10;  // Ex 3.10 - Display 7 Segmentos + Botões + LEDs
}

void loop() {
    ler_botoes();
    
    switch (exercicio_atual) {
        case 1:  ex3_1();  break;
        case 2:  ex3_2();  break;
        case 3:  ex3_3();  break;
        case 4:  ex3_4();  break;
        case 5:  ex3_5();  break;
        case 6:  ex3_6();  break;
        case 7:  ex3_7();  break;
        case 8:  ex3_8();  break;
        case 9:  ex3_9();  break;
        case 10: ex3_10(); break;
        default: ex3_2();  break;
    }
}
