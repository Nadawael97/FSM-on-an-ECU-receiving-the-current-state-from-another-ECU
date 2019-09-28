#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "buttons.h"
#include "LEDs.h"
#include "switching.h"
#include "state_machine.h"


#ifdef state11

extern void 
determine_the_state(void)
{
	volatile uint32_t ui32Loop;
	uint32_t current_state;
	uint32_t input;
	uint32_t temp;
	
	current_state = WHITE_GPIO_PIN;
	LED_ON(state[current_state].out,state[current_state].out);
	
	for(ui32Loop = 0; ui32Loop < 500; ui32Loop++)
	{
	}
	
	temp = Switch();
	
	if (temp & 0x00)
	{
		input = NO_BUTTONS_INDEX;
	}
	else if (temp & LEFT_BUTTON)
	{
		input = LEFT_BUTTON_INDEX;
	}
	else if (temp & RIGHT_BUTTON)
	{
		input = RIGHT_BUTTON_INDEX;
	}
	else if (temp & ALL_BUTTONS)
	{
		input = BOTH_BUTTONS_INDEX;
	}
	
	current_state = state[current_state].Next[input];
}

#endif