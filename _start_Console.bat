@echo off
:: Display task menu
echo --------------------------------------------------
echo Please choose a task:
echo 1. Build (BUILD)
echo 2. Flash (FLASH)
echo 3. Build and Flash (BUILD&FLASH)
echo 4. Clean (CLEAN)
echo --------------------------------------------------
set /p choice=Enter your choice (1/2/3/4): 

:: Execute the chosen task
if "%choice%"=="1" (
    echo Running build task...
    make
    echo Build completed!
) else if "%choice%"=="2" (
    echo Running flash task...
    openocd ^
        -f C:/openocd-0.10.0/scripts/interface/cmsis-dap.cfg ^
        -f C:/openocd-0.10.0/scripts/target/stm32f1x.cfg ^
        -c "init" ^
        -c "reset init" ^
        -c "program build/CAN.elf verify" ^
        -c "reset run" ^
        -c "shutdown"
    echo Flash completed!
) else if "%choice%"=="3" (
    echo Running build and flash tasks...
    echo Building...
    make
    if errorlevel 1 exit /b 1
    echo Build completed!

    echo Flashing...
    openocd ^
        -f C:/openocd-0.10.0/scripts/interface/cmsis-dap.cfg ^
        -f C:/openocd-0.10.0/scripts/target/stm32f1x.cfg ^
        -c "init" ^
        -c "reset init" ^
        -c "program build/CAN.elf verify" ^
        -c "reset run" ^
        -c "shutdown"
    echo Flash completed!
) else if "%choice%"=="4" (
    echo Running clean task...
    del /q build\*
    echo Clean completed! All files in the build folder have been deleted.
) else (
    echo Invalid choice. Please enter 1, 2, 3, or 4.
)

pause
