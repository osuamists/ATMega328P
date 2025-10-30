/*
 * ================================================================================
 * MÓDULO 2 - DISPLAYS DE 7 SEGMENTOS MULTIPLEXADOS (OTIMIZADO)
 * ================================================================================
 * Microcontrolador: ATmega328P @ 16MHz
 * 
 * DESCRIÇÃO:
 * Controle de 2 displays de 7 segmentos multiplexados (cátodo comum)
 * - Display 1 (esquerdo): Contagem crescente 0→F (hexadecimal)
 * - Display 2 (direito): Contagem decrescente F→0 (hexadecimal)
 * - Multiplexação ultra-rápida: 200µs por display (SEM piscamento!)
 * - Atualização: ~200ms entre mudanças de número
 * 
 * CONEXÕES DE HARDWARE (ATmega328P):
 * - SEGMENTOS (PORTB - cátodo comum):
 *   * PB0-PB6: Segmentos A-G (via resistores 220Ω)
 * 
 * - SELEÇÃO DE DISPLAYS (PORTC):
 *   * PC0: Display 1 (via transistor NPN + R1kΩ)
 *   * PC1: Display 2 (via transistor NPN + R1kΩ)
 * ================================================================================
 */

#include <avr/io.h>
#include <util/delay.h>

// ================================================================================
// TABELA DE CONVERSÃO HEXADECIMAL → 7 SEGMENTOS (CÁTODO COMUM)
// ================================================================================
// Formato: 0bGFEDCBA (7 bits = 7 segmentos)
const uint8_t hexa[16] = {
    0b00111111,  // 0: A B C D E F
    0b00000110,  // 1: B C
    0b01011011,  // 2: A B D E G
    0b01001111,  // 3: A B C D G
    0b01100110,  // 4: B C F G
    0b01101101,  // 5: A C D F G
    0b01111101,  // 6: A C D E F G
    0b00000111,  // 7: A B C
    0b01111111,  // 8: A B C D E F G (todos)
    0b01101111,  // 9: A B C D F G
    0b01110111,  // A: A B C E F G
    0b01111100,  // b: C D E F G
    0b00111001,  // C: A D E F
    0b01011110,  // d: B C D E G
    0b01111001,  // E: A D E F G
    0b01110001   // F: A E F G
};

// ================================================================================
// FUNÇÃO MAIN
// ================================================================================
int main(void) {
    // Configura PORTB como saída (segmentos A-G)
    DDRB = 0b01111111;  // PB0-PB6 como saída
    PORTB = 0x00;      // Inicialmente apagado
    
    // Configura PORTC como saída (seleção de displays)
    DDRC = (1 << PC0) | (1 << PC1);  // PC0 e PC1 como saída
    PORTC = 0x00;                     // Inicialmente apagado
    
    // Contadores
    uint8_t contador_crescente = 0;   // Display 1: 0→F
    uint8_t contador_decrescente = 15; // Display 2: F→0
    
    // Loop infinito
    while (1) {
        // Multiplexação ULTRA-RÁPIDA: 500 ciclos × 0.4ms = ~200ms por número
        for (uint16_t i = 0; i < 500; i++) {
            
            // ========== DISPLAY 1 (Esquerdo) ==========
            PORTB = hexa[contador_crescente];  // Carrega padrão
            PORTC = (1 << PC0);                // Ativa Display 1
            _delay_us(200);                    // 0.2ms (ultra-rápido, sem piscamento!)
            
            // ========== DISPLAY 2 (Direito) ==========
            PORTB = hexa[contador_decrescente]; // Carrega padrão
            PORTC = (1 << PC1);                 // Ativa Display 2
            _delay_us(200);                     // 0.2ms (ultra-rápido, sem piscamento!)
        }
        
        // ========== ATUALIZA CONTADORES ==========
        // Incrementa Display 1 (0→F→0)
        contador_crescente++;
        if (contador_crescente > 15) {
            contador_crescente = 0;
        }
        
        // Decrementa Display 2 (F→0→F)
        if (contador_decrescente == 0) {
            contador_decrescente = 15;
        } else {
            contador_decrescente--;
        }
    }
    
    return 0;
}
