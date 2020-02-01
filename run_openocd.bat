@rem connect to debugger command line by telnet
start telnet localhost 4444

start /min openocd  -f "C:\Program Files (x86)\OpenOCD-20191029-0.10.0\share\openocd\scripts\interface\stlink.cfg"^
         -f "C:\Program Files (x86)\OpenOCD-20191029-0.10.0\share\openocd\scripts\target\stm32f1x.cfg"^
         -s "C:\Program Files (x86)\OpenOCD-20191029-0.10.0\share\openocd\scripts"
