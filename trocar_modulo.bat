@echo off
:: ================================================================================
:: Script para Trocar Entre Módulos
:: ================================================================================
:: 
:: USO: trocar_modulo.bat <numero>
:: 
:: Exemplo: trocar_modulo.bat 1   (carrega Módulo 1 - LEDs)
::          trocar_modulo.bat 2   (carrega Módulo 2 - Displays)
::          trocar_modulo.bat 3   (carrega Módulo 3 - Botões)
::          trocar_modulo.bat 0   (carrega Código Completo)
:: ================================================================================

if "%1"=="" (
    echo.
    echo [ERRO] Você precisa especificar o número do módulo!
    echo.
    echo USO: trocar_modulo.bat ^<numero^>
    echo.
    echo Opções:
    echo   0 - Código Completo (todos os módulos)
    echo   1 - Módulo 1 (LEDs - 9 exercícios)
    echo   2 - Módulo 2 (Displays de 7 segmentos)
    echo   3 - Módulo 3 (Botões - 10 exercícios)
    echo.
    exit /b 1
)

if not "%1"=="0" if not "%1"=="1" if not "%1"=="2" if not "%1"=="3" (
    echo.
    echo [ERRO] Número de módulo inválido! Use 0, 1, 2 ou 3
    echo.
    exit /b 1
)

:: Remove main.cpp atual
if exist "src\main.cpp" (
    del /f /q "src\main.cpp" >nul 2>&1
    echo [✓] Removido main.cpp anterior
)

:: Copia o módulo selecionado
if "%1"=="0" (
    copy /y "modulos\main_completo.cpp" "src\main.cpp" >nul 2>&1
    echo.
    echo [✓] MÓDULO COMPLETO carregado!
    echo    Arquivo: main_completo.cpp
    echo    Todos os módulos integrados (1, 2 e 3^)
)

if "%1"=="1" (
    copy /y "modulos\modulo1_leds.cpp" "src\main.cpp" >nul 2>&1
    echo.
    echo [✓] MÓDULO 1 - LEDs carregado!
    echo    Arquivo: modulo1_leds.cpp
    echo    9 exercícios de controle de LEDs
    echo    Altere 'exercicio_atual' no setup() para escolher (0-9^)
)

if "%1"=="2" (
    copy /y "modulos\modulo2_displays.cpp" "src\main.cpp" >nul 2>&1
    echo.
    echo [✓] MÓDULO 2 - Displays carregado!
    echo    Arquivo: modulo2_displays.cpp
    echo    Displays de 7 segmentos multiplexados
    echo    [!] Atenção: PB6/PB7 conflitam com cristal!
)

if "%1"=="3" (
    copy /y "modulos\modulo3_botoes.cpp" "src\main.cpp" >nul 2>&1
    echo.
    echo [✓] MÓDULO 3 - Botões carregado!
    echo    Arquivo: modulo3_botoes.cpp
    echo    10 exercícios com botões e LEDs
    echo    Altere 'exercicio_atual' no setup() para escolher (1-10^)
)

echo.
echo Próximo passo:
echo   pio run
echo.
echo Arquivo HEX será gerado em:
echo   .pio\build\uno\firmware.hex
echo.
