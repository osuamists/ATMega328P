/*
 * ================================================================================
 * MÓDULO 2 - DISPLAYS DE 7 SEGMENTOS
 * ================================================================================
 * Microcontrolador: ATmega328P @ 16MHz
 * 
 * DESCRIÇÃO:
 * Controle de 2 displays de 7 segmentos multiplexados
 * - Display 1: Contagem crescente 0→F (hexadecimal)
 * - Display 2: Contagem decrescente F→0 (hexadecimal)
 * - Multiplexação a cada 5ms para evitar flickering
 * - Atualização de valores a cada 500ms
 * 
 * CONEXÕES DE HARDWARE:
 * - SEGMENTOS (PORTB - catodo comum, 1=aceso):
 *   * PB0 (pino 14) → Segmento A
 *   * PB1 (pino 15) → Segmento B
 *   * PB2 (pino 16) → Segmento C
 *   * PB3 (pino 17) → Segmento D
 *   * PB4 (pino 18) → Segmento E
 *   * PB5 (pino 19) → Segmento F
 *   * PB6 (pino 9)  → Segmento G (⚠️ compartilha com XTAL1 - cuidado!)
 *   * PB7 (pino 10) → Segmento DP (⚠️ compartilha com XTAL2 - cuidado!)
 * 
 * - SELEÇÃO DE DISPLAYS (PORTC - transistores):
 *   * PC0 (pino 23) → Seleção Display 1 (transistor NPN)
 *   * PC1 (pino 24) → Seleção Display 2 (transistor NPN)
 * 
 * NOTA: Se usar cristal externo, PB6/PB7 não estarão disponíveis!
 *       Neste caso, use displays de 7 segmentos sem os segmentos G e DP,
 *       ou mova a conexão dos segmentos para outra porta.
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
// Segmentos em PORTB
#define SEG_A   PB0
#define SEG_B   PB1
#define SEG_C   PB2
#define SEG_D   PB3
#define SEG_E   PB4
#define SEG_F   PB5
#define SEG_G   PB6  // ⚠️ XTAL1 - conflito com cristal!
#define SEG_DP  PB7  // ⚠️ XTAL2 - conflito com cristal!

// Seleção de displays em PORTC
#define SEL_DISP1   PC0
#define SEL_DISP2   PC1

// ================================================================================
// VARIÁVEIS GLOBAIS
// ================================================================================
volatile unsigned long timer_millis = 0;

// Tabela de conversão hexadecimal para 7 segmentos (catodo comum - 1=aceso)
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

uint8_t display1_value = 0;      // Valor display 1 (0-15)
uint8_t display2_value = 15;     // Valor display 2 (15-0)
uint8_t display_atual = 0;       // Display ativo (0 ou 1)

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

// ================================================================================
// FUNÇÃO DE ATUALIZAÇÃO DOS DISPLAYS
// ================================================================================
void atualizar_displays() {
    static unsigned long last_multiplex = 0;
    static unsigned long last_count = 0;
    
    // Multiplexação a cada 5ms
    if (millis_custom() - last_multiplex >= 5) {
        last_multiplex = millis_custom();
        
        if (display_atual == 0) {
            // Desliga display 2, atualiza segmentos, liga display 1
            CLR_BIT(PORTC, SEL_DISP2);
            PORTB = HEX_TABLE[display1_value];
            SET_BIT(PORTC, SEL_DISP1);
            display_atual = 1;
        } else {
            // Desliga display 1, atualiza segmentos, liga display 2
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
// SETUP E LOOP
// ================================================================================
void setup() {
    // Configura PORTB como saída (segmentos)
    DDRB = 0xFF;
    PORTB = 0x00;
    
    // Configura PC0 e PC1 como saída (seleção de displays)
    SET_BIT(DDRC, SEL_DISP1);
    SET_BIT(DDRC, SEL_DISP2);
    CLR_BIT(PORTC, SEL_DISP1);  // Desliga ambos inicialmente
    CLR_BIT(PORTC, SEL_DISP2);
    
    // Inicializa Timer1
    timer1_init();
}

void loop() {
    atualizar_displays();
}

/*
 * ================================================================================
 * NOTAS DE IMPLEMENTAÇÃO NO PROTEUS
 * ================================================================================
 * 
 * 1. CIRCUITO DE MULTIPLEXAÇÃO:
 *    - Use transistores NPN (BC547 ou similar) para controlar cada display
 *    - Base do transistor conectada via resistor 1kΩ a PC0/PC1
 *    - Coletor conectado ao catodo comum do display
 *    - Emissor conectado ao GND
 * 
 * 2. RESISTORES:
 *    - Coloque resistores de 220Ω em cada segmento (A-G e DP)
 *    - Total: 8 resistores por display (ou 1 resistor array)
 * 
 * 3. CONFLITO COM CRISTAL:
 *    - Se usar cristal externo em PB6/PB7 (XTAL1/XTAL2):
 *      * Segmentos G e DP não funcionarão corretamente
 *      * Solução 1: Use displays sem esses segmentos (números 0-9 funcionam)
 *      * Solução 2: Mova segmentos G e DP para outra porta (PORTD)
 *      * Solução 3: Use cristal interno (não recomendado para precisão)
 * 
 * 4. TESTE DE FUNCIONAMENTO:
 *    - Display 1 deve mostrar: 0→1→2→...→E→F→0 (loop)
 *    - Display 2 deve mostrar: F→E→D→...→1→0→F (loop)
 *    - Frequência de atualização: 2 Hz (500ms por número)
 * ================================================================================
 */
