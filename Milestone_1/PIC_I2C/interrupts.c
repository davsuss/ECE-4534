#include "maindefs.h"
#include "interrupts.h"
#include "user_interrupts.h"
#include "messages.h"

void enable_interrupts() {
    // Peripheral interrupts can have their priority set to high or low
    // enable high-priority interrupts and low-priority interrupts

    // clear all interrupt enables
    PIE1 = 0;
    PIE2 = 0;
    PIE3 = 0;

    // disable interrupts from INTCON
    INTCONbits.INT0IE = 0;
    INTCONbits.TMR0IE = 0;
    INTCONbits.RBIE = 0;

    // disable interrupts from INTCON3
    INTCON3bits.INT1IE = 0;
    INTCON3bits.INT2IE = 0;

    // Timer1 interrupt at low priority
    IPR1bits.TMR1IP = 0;

    // Timer0 interrupt at high priority
    INTCON2bits.TMR0IP = 1;

    // allow interrupts from timer 1
    PIE1bits.TMR1IE = 1;
    
    // I2C Priority
    IPR1bits.SSP1IP = 1 ;

    // I2C Slave Interrupt
    PIE1bits.SSP1IE = 1;

    // allow interrupts from timer 0
    INTCONbits.TMR0IE = 1;

    // allow priority-based interrupt
    RCONbits.IPEN = 1;

    // enable global interrupt low and high priority
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
}

//----------------------------------------------------------------------------
// High priority interrupt routine
// this parcels out interrupts to individual handlers
interrupt void InterruptHandlerHigh() {
    // We need to check the interrupt flag of each enabled high-priority interrupt to
    // see which device generated this interrupt.  Then we can call the correct handler.

    // check to see if we have an interrupt on timer 0
    if (INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0; // clear this interrupt flag
        // call whatever handler you want (this is "user" defined)
        timer0_int_handler();
    }
    //i2c
    if(PIR1bits.SSP1IF){
        PIR1bits.SSP1IF = 0;
        I2CR_int_handler();
    }
}

//----------------------------------------------------------------------------
// Low priority interrupt routine
// this parcels out interrupts to individual handlers
// This works the same way as the "High" interrupt handler
interrupt low_priority void InterruptHandlerLow() {
    // check to see if we have an interrupt on timer 1
    if (PIR1bits.TMR1IF) {
        PIR1bits.TMR1IF = 0; //clear interrupt flag
        timer1_int_handler();
    }

}

