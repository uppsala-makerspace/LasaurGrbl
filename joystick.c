#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

//
// This array is used for storing the data read from the ADC FIFO. It
// must be as large as the FIFO for the sequencer in use.  This example
// uses sequence 3 which has a FIFO depth of 1.  If another sequence
// was used with a deeper FIFO, then the array size must be changed.
//
static unsigned long joystick_x[1];
static unsigned long joystick_y[1];

static void x_handler(void)
{
    //
    // Clear the ADC interrupt flag.
    //
    ADCIntClear(ADC0_BASE, 0);

    //
    // Read ADC Value.
    //
    ADCSequenceDataGet(ADC0_BASE, 0, joystick_x);
}

static void y_handler(void)
{
    //
    // Clear the ADC interrupt flag.
    //
    ADCIntClear(ADC0_BASE, 1);

    //
    // Read ADC Value.
    //
    ADCSequenceDataGet(ADC0_BASE, 1, joystick_y);
}


void init_joystick(void)
{
    //
    // The ADC0 peripheral must be enabled for use.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    //
    // Select the analog ADC function for these pins.
    //
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);
    GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_3);

    // Use sequences 0 and 1 for x and y.
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

    // Single ended sample on CH3 (X) and CH4 (Y).
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH4 | ADC_CTL_IE | ADC_CTL_END);

    // Enable the sequences.
    ADCSequenceEnable(ADC0_BASE, 0);
    ADCSequenceEnable(ADC0_BASE, 1);

    // Register ISRs.
    ADCIntRegister(ADC0_BASE, 0, x_handler);
    ADCIntRegister(ADC0_BASE, 1, y_handler);

    ADCIntEnable(ADC0_BASE, 0);
    ADCIntEnable(ADC0_BASE, 1);
}

void trigger_joystick(void)
{
    //
    // Trigger the ADC conversion.
    //
    ADCProcessorTrigger(ADC0_BASE, 0);
    ADCProcessorTrigger(ADC0_BASE, 1);
}

unsigned long joystick_x_get(void)
{
	return joystick_x[0];
}

unsigned long joystick_y_get(void)
{
	return joystick_y[0];
}

