# Projeto ATmega328P - Código Modularizado

## 📁 Estrutura do Projeto

O código foi refatorado para uma arquitetura modular, facilitando manutenção e compreensão:

```
251022-185405-uno/
├── include/
│   ├── config.h              # Macros e definições de pinos
│   ├── timer_utils.h         # Sistema de temporização
│   ├── modulo_leds.h         # Interface do módulo de LEDs
│   ├── modulo_displays.h     # Interface dos displays 7 segmentos
│   └── modulo_botoes.h       # Interface do módulo de botões
├── src/
│   ├── main.cpp              # Arquivo principal (setup/loop)
│   ├── timer_utils.cpp       # Implementação do Timer1
│   ├── modulo_leds.cpp       # Implementação dos exercícios de LEDs
│   ├── modulo_displays.cpp   # Implementação dos displays
│   └── modulo_botoes.cpp     # Implementação dos exercícios de botões
└── platformio.ini
```

## 🎯 Benefícios da Modularização

1. **Código Principal Enxuto**: `main.cpp` agora tem apenas ~100 linhas (era ~1000)
2. **Organização Clara**: Cada módulo em seu próprio arquivo
3. **Manutenção Facilitada**: Mudanças isoladas em cada módulo
4. **Reutilização**: Funções podem ser facilmente reutilizadas
5. **Compilação Eficiente**: Apenas arquivos modificados são recompilados

## 📊 Comparação de Tamanho

| Arquivo          | Antes (monolítico) | Depois (modular) | Redução |
|------------------|--------------------|------------------|---------|
| main.cpp         | ~1000 linhas       | ~100 linhas      | 90%     |

## 🔧 Como Usar

### Selecionar Exercício

Edite `src/main.cpp` na função `loop()` e descomente o exercício desejado:

```cpp
void loop() {
    ler_botoes();
    
    // MÓDULO 1: LEDs
    modulo1_ex1();      // Ativo
    // modulo1_ex2a();  // Inativo
    
    // MÓDULO 2: Displays
    atualizar_displays();
    
    // MÓDULO 3: Botões
    // modulo3_ex1();
}
```

### Compilar e Gravar

```bash
# Compilar
platformio run

# Compilar e gravar
platformio run --target upload
```

## 📦 Módulos Disponíveis

### 1. Timer Utils (`timer_utils.cpp/h`)
- `timer1_init()`: Inicializa Timer1 (1ms)
- `millis_custom()`: Retorna tempo em ms
- `delay_ms(ms)`: Delay bloqueante

### 2. Módulo LEDs (`modulo_leds.cpp/h`)
- `modulo1_ex1()` até `modulo1_ex2i()`: Exercícios de LEDs e bargraph

### 3. Módulo Displays (`modulo_displays.cpp/h`)
- `atualizar_displays()`: Multiplexação e contagem hex

### 4. Módulo Botões (`modulo_botoes.cpp/h`)
- `ler_botoes()`: Leitura com debounce
- `modulo3_ex1()` até `modulo3_ex10()`: Exercícios com botões

### 5. Config (`config.h`)
- Macros: `SET_BIT`, `CLR_BIT`, `TGL_BIT`, `READ_BIT`
- Definições de pinos
- Tabela hexadecimal para displays

## 🔌 Conexões de Hardware

### LED de Teste
- `PC5` → LED + resistor 220Ω

### Bargraph (8 LEDs)
- `PD0-PD7` → LEDs + resistores 220Ω

### Displays 7 Segmentos
- `PB0-PB7` → Segmentos a-g + dp
- `PC0-PC1` → Seleção de displays (transistores)

### Botões
- `PC2-PC4` → Botões (pull-up interno)
- `PD3-PD5` → LEDs dos exercícios de botões

## 📝 Notas Técnicas

- **Sem funções Arduino**: Usa apenas registradores AVR
- **Timer1**: Configurado para interrupção a cada 1ms
- **Debounce**: 50ms para todos os botões
- **Multiplexação**: 5ms entre displays
- **Flash usado**: ~1.4KB (4.4%)
- **RAM usado**: 72 bytes (3.5%)

## 🚀 Próximos Passos

Para adicionar novos exercícios:

1. Crie a função no módulo apropriado (`.cpp`)
2. Declare o protótipo no header (`.h`)
3. Chame a função em `main.cpp` → `loop()`

## 📄 Licença

Projeto Acadêmico - Microcontroladores e Microprocessadores
Outubro 2025
