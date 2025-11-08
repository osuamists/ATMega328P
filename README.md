# Projeto ATmega328P - Controle de LEDs com Proteus

## 📋 Visão Geral

Projeto educacional desenvolvido para ATmega328P @ 16MHz com simulação em **Proteus**. Implementa 3 módulos completos com 21 exercícios de controle de LEDs, displays 7-segmentos e botões.

---

## 🏗️ Estrutura do Projeto

```
ATMega328P/
├── src/
│   └── main.cpp           (Arquivo principal - integra os módulos)
├── modulos/
│   ├── modulo1_leds.cpp   (9 exercícios de controle de LEDs)
│   ├── modulo2_displays.cpp (2 displays 7-segmentos)
│   └── modulo3_botoes.cpp (10 exercícios com botões)
├── proteus/
│   ├── modulo1.pdsprj     (Simulação Proteus - Módulo 1)
│   ├── modulo2.pdsprj     (Simulação Proteus - Módulo 2)
│   └── modulo3.pdsprj     (Simulação Proteus - Módulo 3)
└── README.md              (Este arquivo)
```

---

## 🔧 Hardware Utilizado

### Microcontrolador
- **ATmega328P** @ 16MHz com cristal externo

### Módulo 1 - Controle de LEDs
| Componente | Pino AVR | Quantidade | Descrição |
|-----------|----------|-----------|-----------|
| Bargraph LEDs (D1-D8) | PB0-PB7 | 8 | LEDs em série com resistor 220Ω |
| LED Teste | PC5 | 1 | LED individual para Ex 1.1 |
| LED D7 | PC0 | 1 | LED adicional (9º LED) |

### Módulo 2 - Displays 7-Segmentos
| Componente | Pinos AVR | Quantidade | Descrição |
|-----------|----------|-----------|-----------|
| Display 1 (Cátodo Comum) | PC0-PC1, PC5, PD5-PD7, PB2 | 7 segmentos | Crescente/Decrescente |
| Display 2 (Cátodo Comum) | Compartilhados | 7 segmentos | Sincronizado com Display 1 |

### Módulo 3 - Botões e LEDs
| Componente | Pino AVR | Quantidade | Descrição |
|-----------|----------|-----------|-----------|
| Botões | PC2, PC3, PC4 | 3 | BTN1, BTN2, BTN3 (pull-up interno) |
| LEDs | PD3, PD4, PB0, PB1 | 4 | LED1-LED4 |

---

## 📚 Módulo 1: Controle de LEDs

### ✨ Exercícios Implementados

| ID | Exercício | Descrição | Timing |
|-------|-----------|-----------|--------|
| **1** | **LED Teste** | PC5 pisca: 3x rápido (200ms) + 3x devagar (500ms) | Infinito |
| **2a** | **Acender L→R** | Bargraph acende esquerda para direita (mantém acesos) | 2 ciclos |
| **2b** | **Acender R→L** | Bargraph acende direita para esquerda (mantém acesos) | 2 ciclos |
| **2c** | **1 LED por vez** | Apenas 1 LED aceso por vez, L→R | 2 ciclos |
| **2d** | **Ping-pong** | 1 LED "salta" de um lado para o outro | 2 ciclos |
| **2e** | **Apagar 1x1** | Todos acesos, apaga 1 por vez (vai e volta) | 2 ciclos |
| **2f** | **L→R + Piscar** | Acende L→R, depois pisca 2x, apaga | 2 ciclos |
| **2g** | **Direita-Esquerda** | ⭐ **EXERCÍCIO PRINCIPAL**: R→L acende, apaga 200ms, L→R acende | 2 ciclos |
| **2h** | **Contagem 0→255** | Bargraph em contagem binária crescente | 2 ciclos |
| **2i** | **Contagem 255→0** | Bargraph em contagem binária decrescente | 2 ciclos |

### 📌 Como Testar o Módulo 1

#### **Abrir no Proteus:**
1. Abra `proteus/modulo1.pdsprj`
2. Localize o arquivo `modulos/modulo1_leds.cpp` no projeto PlatformIO

#### **Selecionar um Exercício:**
```cpp
// Em modulo1_leds.cpp, na função setup(), altere:
exercicio_atual = 7;  // ← ESCOLHA AQUI (1-9 ou 0)
```

**Mapeamento de valores:**
```
1 = Ex 1.1  (LED Teste)
2 = Ex 1.2a (Acender L→R)
3 = Ex 1.2b (Acender R→L)
4 = Ex 1.2c (1 LED por vez)
5 = Ex 1.2d (Ping-pong)
6 = Ex 1.2e (Apagar 1x1)
7 = Ex 1.2f (L→R + Piscar)
8 = Ex 1.2g (Direita-Esquerda) ← PRINCIPAL
9 = Ex 1.2h (Contagem 0→255)
0 = Ex 1.2i (Contagem 255→0)
```


#### **Sequência Automática (TESTE COMPLETO):**
O código está configurado para executar **todos os exercícios automaticamente**:
- Cada exercício roda por ~7 segundos
- Após 7 segundos, transiciona automaticamente para o próximo
- Ciclo completo: ~70 segundos

---

## 🚨 PROBLEMAS CONHECIDOS - LED D7

### ⚠️ Problema Principal: LED D7 Acende Sempre

#### **O que acontece:**
- LED D7 (conectado a PC0) **fica aceso permanentemente** após inicialização
- Afeta visualmente o exercício 1.2g (exercício principal)
- Não afeta a funcionalidade dos outros 8 LEDs (PORTB)

#### **Possíveis Causas:**

1. **Inicialização Conflitante:**
   - Problema: `DDRC` e `PORTC` são inicializados como outputs no `setup()`
   - O código tenta configurar PC5 (LED_TESTE) e PC0 (LED_D7)
   - Possível conflito na sequência de escrita

2. **Estado do Loop:**
   - O `loop()` contém um `do-while(1)` infinito
   - Cada exercício tem variáveis `static` que mantêm estado
   - Transição automática entre exercícios pode deixar D7 em estado inconsistente

3. **Possível Pull-up Residual:**
   - `MCUCR |= (1 << PUD)` desativa pull-ups globalmente
   - Mas pode haver capacitância parasita em PC0

## ✅ Soluções Tentadas (e Parcialmente Implementadas)

### ✔️ Solução 1: Reset de Portas Entre Exercícios
```cpp
// Adicionado em loop():
if (millis_custom() - exercise_start_time >= exercise_duration) {
    PORTB = 0x00;
    PORTC = 0x00;      // Apaga todos os pinos
    DDRB = 0x00;
    DDRC = 0x00;       // Coloca como entrada temporariamente
    delay_ms(700);     // Delay para estabilização
    DDRB = 0xFF;
    DDRC = 0xFF;       // Volta como saída
    // ...
}
```
**Status:** ✔️ Implementado | ⚠️ Melhora parcial

### ✔️ Solução 2: Função `update_d7()`
```cpp
void update_d7() {
    if (PORTB != 0) {
        SET_BIT(PORTC, LED_D7_PIN);  // D7 aceso se houver LED aceso em PORTB
    } else {
        CLR_BIT(PORTC, LED_D7_PIN);  // D7 apagado se todos PORTB apagados
    }
}
```
**Status:** ✔️ Implementado | ⚠️ Controle melhorado

### ❌ Solução 3 (Não Implementada): Usar PD0/PD1 em vez de PC0
- PC0 pode estar compartilhado com SPI ou outra funcionalidade
- PD0/PD1 estariam disponíveis (PD5/PD6 são do cristal)
- Requereria rewiring no Proteus

---

## 📊 Módulo 2: Displays 7-Segmentos

### ✨ Exercícios Implementados

| ID | Exercício | Descrição |
|----|-----------|-----------|
| **2.1** | **Crescente** | Display conta 0→9 continuamente |
| **2.2** | **Decrescente** | Display conta 9→0 continuamente |

### 📌 Como Testar o Módulo 2

1. Abra `proteus/modulo2.pdsprj`
2. Altere `exercicio_atual` em `modulo2_displays.cpp`:
   ```cpp
   exercicio_atual = 1;  // 1 = Crescente, 2 = Decrescente
   ```
3. Compile e simule

**Características:**
- ✅ Anti-flicker: Atualiza a cada 500ms
- ✅ Multiplexing otimizado
- ✅ Compartilha segmentos entre displays

---

## 🎮 Módulo 3: Botões e LEDs

### ✨ Exercícios Implementados

| ID | Exercício | Descrição |
|----|-----------|-----------|
| **3.1** | **Liga/Desliga** | Botão controla LED (toggle) |
| **3.2** | **Contador LED** | Botão incrementa contador em LED |
| **3.3** | **Sequência** | Botão inicia sequência automática |
| **3.4-3.10** | **Variações** | Diferentes padrões com botões |

### 📌 Como Testar o Módulo 3

1. Abra `proteus/modulo3.pdsprj`
2. Altere `exercicio_atual` em `modulo3_botoes.cpp`:
   ```cpp
   exercicio_atual = 1;  // 1-10 (vários exercícios disponíveis)
   ```
3. Simule clicando nos botões virtuais no Proteus


## 📝 Resumo Técnico

### Timer1 Configuration
- **Modo:** CTC (Clear Timer on Compare)
- **Prescaler:** 64
- **OCR1A:** 249 (para 1ms @ 16MHz)
- **Interrupção:** TIMER1_COMPA

### Variáveis Globais Críticas
```cpp
volatile unsigned long timer_millis = 0;  // Contador de milissegundos
uint8_t exercicio_atual = 7;              // Exercício a executar
unsigned long exercise_start_time = 0;    // Tempo de início do exercício
unsigned long exercise_duration = 7000;   // 7 segundos por exercício
```

### Pinos Utilizados

| Porta | Pinos | Função |
|-------|-------|--------|
| PORTB | PB0-PB7 | Bargraph (8 LEDs) |
| PORTC | PC0, PC5 | LED D7, LED Teste |
| PORTC | PC2-PC4 | Botões (Módulo 3) |
| PORTD | PD3-PD4, PD7 | LEDs/Segmentos (Módulo 3/2) |
| PORTD | PD5-PD6 | Cristal 16MHz (RESERVADO) |

## 👨‍💻 Autores


Desenvolvido por: Luís Guilherme Busaglo Lopes, Marcos Vinícius Morais Rios, Matheus Machado Santos Patrick Melo Albuquerque e Suamí Gomes Santos


 Finalidade: Projeto educacional para ATmega328P com Proteus.
