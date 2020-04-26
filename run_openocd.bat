@echo off
rem connect to debugger command line by telnet
start telnet localhost 4444
rem start gdb target remote localhost:3333

openocd -f openocd.cfg -c "init" -c "reset halt" -c "flash write_image erase "out/sensors.bin" 0x08000000" -c "reset run"
rem  -c "exit"

rem -c "program out/sensors.bin 0x08000000 verify reset "

