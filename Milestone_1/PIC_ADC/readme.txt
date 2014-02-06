ADC class files are broken and don't work

ADCmain needs to be added to the main.c for A/D conversion to work

Call ADCInit() to initialize the module.
The last two lines of ADCInit() are only there for testing purposes atm and need to be placed in the timer polling interrupt.
Currently, the way it is set up, the ADC will constantly be polling and interrupting

If you want me to insert the stuff myself I can, just tell me to.