#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "LEDs.h"
#include "utils/uartstdio.h"

void
LED_Init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }
		
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_GPIO_PIN | BLUE_GPIO_PIN | GREEN_GPIO_PIN );

}

void 
LED_ON(uint32_t LED, uint32_t LED_Colour)
{
	GPIOPinWrite(GPIO_PORTF_BASE, LED, LED_Colour);
}