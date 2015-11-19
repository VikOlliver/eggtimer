# eggtimer
Arduino-based egg timer with optional relay output for alternative projects. Drives 4-digit TM1637 LED display

Best
Universal
Gadget for
Getting
Eggs
Right

Two switches (D4, D5) control start/change and set/move in the normal manner of digial watch setting. The TM1637 LED display is connected to D2 and D3.

This is intended as a general purpose program for teaching kids to assemble an Arduino project on a breadboard. Consequently in the power-up mode it flashes LEDs on D13 and D12 in antiphase, and sends audio pulses to D11 suitable for driving a piezo speaker. These cease when either switch is pressed.

The default time is 5 minutes - the perfect time for a soft-boiled egg. At the end of this time it will let you know that your egg has exploded (LED displays "BANG"), pulses audio tones on D10, and brings D6 high for 10 seconds.

Once set, time is stored in EEPROM.
