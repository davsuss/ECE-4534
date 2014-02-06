#include <xc.h>
#include <stdlib.h>
#include <plib/adc.h>

//#define _XTAL_FREQ 8000000
#define USE_AND_MASKS

#pragma config XINST = OFF

int ADCValue = 0;

void ADCInit() {

    //Configure ADC
    OpenADC(ADC_FOSC_2 & ADC_RIGHT_JUST & ADC_20_TAD, ADC_CH0 & ADC_INT_ON & ADC_VREFPLUS_VDD & ADC_VREFMINUS_VSS, ADC_0ANA);

    // Enable ADC interrupts
    ADC_INT_ENABLE(); 

    // Interrupt master switch on
    ei();       

	/// Note: Do the stuff below in the timer interrupt
	
    //Start ADC
    ConvertADC();

    while(1) //infinite loop
    {
    }
}

void interrupt ADCInterrupt()
{
    //check if the interrupt is caused by ADC
    if(PIR1bits.ADIF == 1)
    {
        ADCValue = ReadADC();
/////////////////////////////////
        // Do stuff
//////////////////////////////////
        
        //Reset interrupt flag and start conversion again
        ADIF = 0;
        ConvertADC();
    }
}
