#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "buttons.h"
#include "utils/uartstdio.h"

uint8_t  ui8PrevButtonState = 0; 
uint8_t ui8CurButtonState = 0;
uint8_t ui8Message;


extern uint8_t
Switch(void)
{		ui8Message = 0;
    uint8_t g_ui8ButtonStates = (GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS));
	
		ui8CurButtonState = (~g_ui8ButtonStates);
				
		//
		// Check if previous debounced state is equal to the current state.
		//
	
		if(ui8CurButtonState != ui8PrevButtonState)
		{
				ui8PrevButtonState = ui8CurButtonState;

				//
				// Check to make sure the change in state is due to button press
				// and not due to button release.
				//
				UARTprintf("if statements entry.\n");
				UARTprintf("anding  is  %x \n" , (ui8CurButtonState & ALL_BUTTONS));
				//check if the button pressed or not
				if((ui8CurButtonState & ALL_BUTTONS) != 0)
				{
						//check if the button pressed is left or right
						if((ui8CurButtonState & ALL_BUTTONS) == LEFT_BUTTON)
						{
								ui8Message = LEFT_BUTTON;
						}
						
						else if((ui8CurButtonState & ALL_BUTTONS) == RIGHT_BUTTON)
						{
							ui8Message = RIGHT_BUTTON;
							UARTprintf("Right Button is pressed.\n");
								
						}
						else if((ui8CurButtonState & ALL_BUTTONS) == ALL_BUTTONS)
						{
							ui8Message = ALL_BUTTONS;
							
						}

				}
		}
		
		return ui8Message;
		
}


//*****************************************************************************
//
// Initializes the switch task.
//
//*****************************************************************************
uint32_t
SwitchTaskInit(void)
{
    //
    // Unlock the GPIO LOCK register for Right button to work.
    //
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0xFF;

    //
    // Initialize the buttons
    //
    ButtonsInit();
		
}
