#include "stdint.h"
#include "stdio.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "driverlib/interrupt.h"
#include "driverlib/can.h"
#include "inc/hw_can.h"

#include "buttons.h"
#include "LEDs.h"
#include "switching.h" 
#include "state_machine.h"

#undef buttons_test 
#undef state_machine_test
#define Can_test

volatile uint32_t g_ui32MsgCount = 0;

//*****************************************************************************
//
// A flag to indicate that some transmission error occurred.
//
//*****************************************************************************

volatile bool g_bErrFlag = 0;

#define NO_BUTTONS_INDEX 0
#define RIGHT_BUTTON_INDEX 1
#define LEFT_BUTTON_INDEX 2
#define BOTH_BUTTONS_INDEX 3

typedef struct State_machine
{ 
			uint8_t out;
			uint32_t time;
			uint8_t Next[4];
}
State_t;

typedef struct State_machine State_t; 

State_t state[4]=
{
	{WHITE_GPIO_PIN, 2000000 , {WHITE_GPIO_PIN, GREEN_GPIO_PIN, RED_GPIO_PIN, RED_GPIO_PIN}},
	{RED_GPIO_PIN, 2000000, {RED_GPIO_PIN, WHITE_GPIO_PIN, BLUE_GPIO_PIN, (WHITE_GPIO_PIN | BLUE_GPIO_PIN)}},
	{BLUE_GPIO_PIN, 2000000, {BLUE_GPIO_PIN, RED_GPIO_PIN, GREEN_GPIO_PIN, (RED_GPIO_PIN | GREEN_GPIO_PIN)}},
	{GREEN_GPIO_PIN, 2000000, {GREEN_GPIO_PIN, BLUE_GPIO_PIN, WHITE_GPIO_PIN, (BLUE_GPIO_PIN | WHITE_GPIO_PIN)}}
};

void
CANIntHandler(void)
{
    uint32_t ui32Status;

    //
    // Read the CAN interrupt status to find the cause of the interrupt
    //
    ui32Status = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);

    //
    // If the cause is a controller status interrupt, then get the status
    //
    if(ui32Status == CAN_INT_INTID_STATUS)
    {
        //
        // Read the controller status.  This will return a field of status
        // error bits that can indicate various errors.  Error processing
        // is not done in this example for simplicity.  Refer to the
        // API documentation for details about the error status bits.
        // The act of reading this status will clear the interrupt.  If the
        // CAN peripheral is not connected to a CAN bus with other CAN devices
        // present, then errors will occur and will be indicated in the
        // controller status.
        //
        ui32Status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);

        //
        // Set a flag to indicate some errors may have occurred.
        //
        g_bErrFlag = 1;
    }

    //
    // Check if the cause is message object 1, which what we are using for
    // sending messages.
    //
    else if(ui32Status == 1)
    {
        //
        // Getting to this point means that the TX interrupt occurred on
        // message object 1, and the message TX is complete.  Clear the
        // message object interrupt.
        //
        CANIntClear(CAN0_BASE, 1);

        //
        // Increment a counter to keep track of how many messages have been
        // sent.  In a real application this could be used to set flags to
        // indicate when a message is sent.
        //
        g_ui32MsgCount++;

        //
        // Since the message was sent, clear any error flags.
        //
        g_bErrFlag = 0;
    }

    //
    // Otherwise, something unexpected caused the interrupt.  This should
    // never happen.
    //
    else
    {
        //
        // Spurious interrupt handling can go here.
        //
    }
}

void
CAN_Init(void)
{
	//
    // Configure the GPIO pin muxing to select CAN0 functions for these pins.
    // This step selects which alternate function is available for these pins.
    // This is necessary if your part supports GPIO pin function muxing.
    // Consult the data sheet to see which functions are allocated per pin.
    // TODO: change this to select the port/pin you are using
    //
    GPIOPinConfigure(GPIO_PB4_CAN0RX);
    GPIOPinConfigure(GPIO_PB5_CAN0TX);

    //
    // Enable the alternate function on the GPIO pins.  The above step selects
    // which alternate function is available.  This step actually enables the
    // alternate function instead of GPIO for these pins.
    // TODO: change this to match the port/pin you are using
    //
    GPIOPinTypeCAN(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    //
    // The GPIO port and pins have been set up for CAN.  The CAN peripheral
    // must be enabled.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);

    //
    // Initialize the CAN controller
    //
    CANInit(CAN0_BASE);

}

void CANIntSetup(void)
{
	//
    // Enable interrupts on the CAN peripheral.  This example uses static
    // allocation of interrupt handlers which means the name of the handler
    // is in the vector table of startup code.  If you want to use dynamic
    // allocation of the vector table, then you must also call CANIntRegister()
    // here.
    //
    // CANIntRegister(CAN0_BASE, CANIntHandler); // if using dynamic vectors
    //
		
		CANIntRegister(CAN0_BASE ,CANIntHandler);
    CANIntEnable(CAN0_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);

    //
    // Enable the CAN interrupt on the processor (NVIC).
    //
   
		IntEnable(INT_CAN0);
    //
    // Enable the CAN for operation.
    //
    CANEnable(CAN0_BASE);

}

void
InitConsole(void)
{
    //
    // Enable GPIO port A which is used for UART0 pins.
    // TODO: change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Configure the pin muxing for UART0 functions on port A0 and A1.
    // This step is not necessary if your part does not support pin muxing.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    //
    // Enable UART0 so that we can configure the clock.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Select the alternate (UART) function for these pins.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

int main(void)
{
	#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
    defined(TARGET_IS_TM4C129_RA1) ||                                         \
    defined(TARGET_IS_TM4C129_RA2)
    uint32_t ui32SysClock;
	#endif
	#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
    defined(TARGET_IS_TM4C129_RA1) ||                                         \
    defined(TARGET_IS_TM4C129_RA2)
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                       SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_OSC)
                                       25000000);
	#else
			SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
										 SYSCTL_XTAL_16MHZ);
	#endif
	
	#ifdef Can_test
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
		InitConsole();
		CAN_Init();
		
		ButtonsInit();
	#endif
	
	#ifdef state_machine_test
		LED_Init();
		ButtonsInit();
	#endif
	
	
	#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
    defined(TARGET_IS_TM4C129_RA1) ||                                         \
    defined(TARGET_IS_TM4C129_RA2)
    CANBitRateSet(CAN0_BASE, ui32SysClock, 500000);
	#else
			CANBitRateSet(CAN0_BASE, SysCtlClockGet(), 500000);
	#endif
	
	CANIntSetup();
	
	tCANMsgObject sCANMessage;
	uint32_t ui32MsgData;
	uint8_t pui8MsgData;

	uint8_t button;
	volatile uint32_t ui32Loop;
	uint8_t current_state;
	uint8_t input;
	
	uint8_t temp =0;
	uint8_t current = 0;
	
	pui8MsgData = (uint8_t)&ui32MsgData;
	current_state = NO_BUTTONS_INDEX;
	
	
	
	while(1)
	{
		
		#ifdef buttons_test
			LED_ON(GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0x00);
			button = Switch();
			UARTprintf("ay7aga");
			if (button == LEFT_BUTTON)
			{
				LED_ON(state[2].out, state[2].out);
			}
			else if (button == RIGHT_BUTTON)
			{
				LED_ON(state[3].out, state[3].out);
			}
		#endif
		
		#ifdef state_machine_test
			
		UARTprintf(" current led out %u\n", state[current_state].out);
		LED_ON(WHITE_GPIO_PIN,(state[current_state ].out ));
		
		SysCtlDelay(state[(current_state)].time);
		
		
		temp = Switch();
		
		
		if (temp == 0X00)
		{
			UARTprintf("1st");
			input =  NO_BUTTONS_INDEX;
		}
		else if (temp == (LEFT_BUTTON))
		{
			UARTprintf("2nd");
			input = LEFT_BUTTON_INDEX;
		}
		else if (temp == (RIGHT_BUTTON ))
		{
			UARTprintf("3rd");
			input = RIGHT_BUTTON_INDEX;
		}
		else if (temp == (ALL_BUTTONS ))
		{
			UARTprintf("4th");
			input = BOTH_BUTTONS_INDEX;
		}
		current = state[current_state].Next[input];
		
		for(int i = 0; i < 4; i++)
		{
			if(current == state[i].out)
			{
				current_state = i;
				UARTprintf("entry success");
			}
		}
		
		UARTprintf(" current state %u\n", current_state);
		
		#endif
		
		#ifdef Can_test
		
			volatile uint32_t ui32Loop;
			
			temp = Switch();
			
			SysCtlDelay(2000000);
			
			if (temp == 0X00)
			{
				UARTprintf("1st");
				pui8MsgData =  NO_BUTTONS_INDEX;
			}
			else if (temp == (LEFT_BUTTON))
			{
				UARTprintf("2nd");
				pui8MsgData = LEFT_BUTTON_INDEX;
			}
			else if (temp == (RIGHT_BUTTON ))
			{
				UARTprintf("3rd");
				pui8MsgData = RIGHT_BUTTON_INDEX;
			}
			
			UARTprintf("the nn message is %x /n", pui8MsgData);
			sCANMessage.ui32MsgIDMask = 0;
			sCANMessage.ui32Flags = MSG_OBJ_TX_INT_ENABLE;
			sCANMessage.ui32MsgLen = sizeof(pui8MsgData);
			sCANMessage.pui8MsgData = pui8MsgData;
			
			UARTprintf("the message is %d", sCANMessage.pui8MsgData);
			
			CANMessageSet(CAN0_BASE, 1, &sCANMessage, MSG_OBJ_TYPE_TX);
			
			if(g_bErrFlag)
        {
            UARTprintf(" error - cable connected?\n");
        }
        else
        {
            //
            // If no errors then print the count of message sent
            //
            UARTprintf(" total count = %u\n", g_ui32MsgCount);
        }
			
		#endif
		
		}
	
	
	return 0;
}