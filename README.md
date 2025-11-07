# Projeto de Sistemas Embarcados com ATmega328P

Este repositório contém uma série de projetos modulares para o microcontrolador **ATmega328P**, desenvolvidos em C/C++ com manipulação direta de registradores e simulados no Proteus. O objetivo é demonstrar o controle de periféricos como LEDs, botões e displays de 7 segmentos.

## 🚀 Estrutura do Projeto

O projeto é dividido em três módulos independentes, cada um com seu próprio arquivo de simulação no Proteus e código-fonte.

- **Módulo 1:** Controle de LEDs
- **Módulo 2:** Displays de 7 Segmentos Multiplexados
- **Módulo 3:** Interação com Botões e LEDs

## 🛠️ Ferramentas Utilizadas

- **Microcontrolador:** ATmega328P @ 16MHz
- **IDE:** Visual Studio Code com PlatformIO
- **Simulador:** Proteus 9
- **Linguagem:** C/C++ (AVR-libc)

---

##  Módulo 1: Controle de LEDs

Este módulo foca no controle básico de 8 LEDs conectados ao microcontrolador, explorando diferentes padrões de acionamento.

### 📋 Funcionalidades
- **Acionamento individual:** Liga e desliga cada LED separadamente.
- **Sequências:** Cria animações como "Knight Rider" (carro fantástico) e sequências binárias.
- **Controle de brilho:** Implementa PWM (Pulse-Width Modulation) via software para variar a intensidade dos LEDs.

### 🔌 Conexões (Hardware)
| Componente | Conexão no ATmega328P |
|------------|-----------------------|
| 8 LEDs (D1-D8) | PORTD (PD0-PD7) |
| 8 Resistores | 220Ω (um para cada LED) |
| Cristal | 16MHz (entre PB6 e PB7) |

### 💻 Código
O código do Módulo 1 demonstra:
- Configuração de pinos como saída (`DDRD`).
- Manipulação de portas lógicas (`PORTD`).
- Uso de delays para criar temporizações (`_delay_ms()`).

---

## Módulo 2: Displays de 7 Segmentos Multiplexados

Este módulo implementa o controle de dois displays de 7 segmentos usando a técnica de multiplexação para economizar pinos.

### 📋 Funcionalidades
- **Display 1:** Contagem crescente em hexadecimal (0, 1, 2, ..., E, F).
- **Display 2:** Contagem decrescente em hexadecimal (F, E, D, ..., 1, 0).
- **Multiplexação:** Alternância rápida entre os displays para criar a ilusão de que ambos estão acesos simultaneamente.

### 🔌 Conexões (Hardware)
| Componente | Conexão no ATmega328P |
|---|---|
| **Segmentos A-G** | PB0-PB6 (compartilhados entre os 2 displays) |
| **Seleção Display 1** | PC0 (pino de controle do cátodo comum) |
| **Seleção Display 2**| PC1 (pino de controle do cátodo comum) |
| **7 Resistores**| 220Ω (um para cada barramento de segmento) |

### 💻 Código
O código do Módulo 2 aborda:
- **Tabela de conversão** hexadecimal para padrão de 7 segmentos.
- **Multiplexação** com delays em microssegundos (`_delay_us()`) para evitar piscamento (flickering).
- **Contadores** que operam de forma independente.

---

## Módulo 3: Interação com Botões e LEDs

Este módulo combina o uso de LEDs e botões para criar sistemas interativos, aplicando técnicas de debouncing e detecção de estado.

### 📋 Funcionalidades
- **Exercício 3.1:** Toggle - um clique liga o LED, outro desliga.
- **Exercício 3.2:** Ciclo de modos (ON → Piscar → Piscar Rápido → OFF).
- **Exercício 3.3:** Sequências de LEDs (1-2-3 e 3-2-1) controladas por botão.
- **Exercício 3.8:** Sequências direcionais controladas por dois botões.
- **Exercício 3.10:** Controle de LEDs e display de 7 segmentos com três botões.

### 🔌 Conexões (Hardware)
| Componente | Conexão no ATmega328P |
|---|---|
| **LEDs (D1-D4)** | PD3, PD4, PB0, PB1 |
| **Botões (BTN1-BTN3)**| PC2, PC3, PC4 (com pull-up interno ativado) |
| **Display 7 Segmentos**| Mapeado em pinos livres (PORTC e PORTD) |

### 💻 Código
O código do Módulo 3 explora:
- **Leitura de entradas digitais** (`PINC`).
- **Debouncing de botões** para evitar múltiplas leituras.
- **Máquinas de estado** para controlar o comportamento do sistema com base nos cliques.
- **Combinação de periféricos** (LEDs, botões e display) em um único sistema.

---

## 🚀 Como Compilar e Simular

1. **Abra o Projeto:** Abra a pasta de cada módulo (`modulo1`, `modulo2`, `modulo3`) no Visual Studio Code com o PlatformIO.
2. **Compile:** Use o comando `pio run` no terminal ou o botão "Build" do PlatformIO.
3. **Localize o .hex:** O arquivo compilado estará em `.pio/build/ATmega328P/firmware.hex`.
4. **Carregue no Proteus:**
   - Abra o arquivo `.pdsprj` correspondente ao módulo.
   - Clique com o botão direito no ATmega328P e vá
