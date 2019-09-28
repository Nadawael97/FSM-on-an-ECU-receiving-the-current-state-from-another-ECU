#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_can.h"
#include "inc/hw_memmap.h"
#include "inc\hw_ints.h"
#include "driverlib/can.h"
#include "driverlib/gpio.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

//#include "buttons.h"
#include "E:\merged_partition_content\MCT\Spring 2019 (FINALLY!!)\Automotive_Embedded\Projects\Buttons_and_LEDs/LEDs.h"
#include "E:\merged_partition_content\MCT\Spring 2019 (FINALLY!!)\Automotive_Embedded\Projects\Buttons_and_LEDs/switching.h" 

volatile uint32_t g_ui32MsgCount = 0;

//*****************************************************************************
//
// A flag for the interrupt handler to indicate that a message was received.
//
//*****************************************************************************
volatile bool g_bRXFlag = 0;

//*****************************************************************************
//
// A flag to indicate that some reception error occurred.
//
//*****************************************************************************
volatile bool g_bErrFlag = 0;

#define NO_BUTTONS_INDEX 0x00
#define RIGHT_BUTTON_INDEX 0x01
#define LEFT_BUTTON_INDEX 0x02
#define BOTH_BUTTONS_INDEX 0x03

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

//*****************************************************************************
//
// This function sets up UART0 to be used for a console to display information
// as the example is running.
//
//*****************************************************************************
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

//*****************************************************************************
//
// This function is the interrupt handler for the CAN peripheral.  It checks
// for the cause of the interrupt, and maintains a count of all messages that
// have been received.
//
//*****************************************************************************
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
        // The act of reading this status will clear the interrupt.
        //
        ui32Status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);

        //
        // Set a flag to indicate some errors may have occurred.
        //
        g_bErrFlag = 1;
    }

    //
    // Check if the cause is message object 1, which what we are using for
    // receiving messages.
    //
    else if(ui32Status == 1)
    {
        //
        // Getting to this point means that the RX interrupt occurred on
        // message object 1, and the message reception is complete.  Clear the
        // message object interrupt.
        //
        CANIntClear(CAN0_BASE, 1);

        //
        // Increment a counter to keep track of how many messages have been
        // received.  In a real application this could be used to set flags to
        // indicate when a message is received.
        //
        g_ui32MsgCount++;

        //
        // Set flag to indicate received message is pending.
        //
        g_bRXFlag = 1;

        //
        // Since a message was received, clear any error flags.
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
    // Configure the GPIO pin muxing to select CAN0 functions for these pins.
    // This step selects which alternate function is available for these pins.
    // This is necessary if your part supports GPIO pin function muxing.
    // Consult the data sheet to see which functions are allocated per pin.
    // TODO: change this to select the port/pin you are using
    //
		
		SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	
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
    CANIntRegister(CAN0_BASE, CANIntHandler); // if using dynamic vectors
    //
    CANIntEnable(CAN0_BASE, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);
		UARTprintf("entermain");
    //
    // Enable the CAN interrupt on the processor (NVIC).
    //
    IntEnable(INT_CAN0);

    //
    // Enable the CAN for operation.
    //
    CANEnable(CAN0_BASE);

}

//*****************************************************************************
//
// Configure the CAN and enter a loop to receive CAN messages.
//
//*****************************************************************************
int
main(void)
{
	
		#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
				defined(TARGET_IS_TM4C129_RA1) ||                                         \
				defined(TARGET_IS_TM4C129_RA2)
				uint32_t ui32SysClock;
		#endif

				tCANMsgObject sCANMessage;
				uint8_t pui8MsgData[8];
				//
				// Set the clocking to run directly from the external crystal/oscillator.
				// TODO: The SYSCTL_XTAL_ value must be changed to match the value of the
				// crystal used on your board.
				//
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

		
    //
    // Set up the serial console to use for displaying messages.  This is
    // just for this example program and is not needed for CAN operation.
    //
    InitConsole();
    CAN_Init();
		CANIntSetup();
		LED_Init();
		
		uint8_t current_state;
		uint8_t current;
		uint8_t input;		
		current_state = NO_BUTTONS_INDEX;

    //
    // For this example CAN0 is used with RX and TX pins on port B4 and B5.
    // The actual port and pins used may be different on your part, consult
    // the data sheet for more information.
    // GPIO port B needs to be enabled so these pins can be used.
    // TODO: change this to whichever GPIO port you are using
    //
   
    
		#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
				defined(TARGET_IS_TM4C129_RA1) ||                                         \
				defined(TARGET_IS_TM4C129_RA2)
				CANBitRateSet(CAN0_BASE, ui32SysClock, 500000);
		#else
				CANBitRateSet(CAN0_BASE, SysCtlClockGet(), 500000);
		#endif

    
    //
		// Initialize a message object to be used for receiving CAN messages with
		// any CAN ID.  In order to receive any CAN ID, the ID and mask must both
		// be set to 0, and the ID filter enabled.
		//
		sCANMessage.ui32MsgID = 0;
		sCANMessage.ui32MsgIDMask = 0;
		sCANMessage.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
		sCANMessage.ui32MsgLen = 8;

		//
		// Now load the message object into the CAN peripheral.  Once loaded the
		// CAN will receive any message on the bus, and an interrupt will occur.
		// Use message object 1 for receiving messages (this is not the same as
		// the CAN ID which can be any value in this example).
		//
		CANMessageSet(CAN0_BASE, 1, &sCANMessage, MSG_OBJ_TYPE_RX);
				

		
    //
    // Enter loop to process received messages.  This loop just checks a flag
    // that is set by the interrupt handler, and if set it reads out the
    // message and displays the contents.  This is not a robust method for
    // processing incoming CAN data and can only handle one messages at a time.
    // If many messages are being received close together, then some messages
    // may be dropped.  In a real application, some other method should be used
    // for queuing received messages in a way to ensure they are not lost.  You
    // can also make use of CAN FIFO mode which will allow messages to be
    // buffered before they are processed.
    //
    for(;;)
    {
        unsigned int uIdx;
				
        //
        // If the flag is set, that means that the RX interrupt occurred and
        // there is a message ready to be read from the CAN
        //
        if(g_bRXFlag)
        {
            //
            // Reuse the same message object that was used earlier to configure
            // the CAN for receiving messages.  A buffer for storing the
            // received data must also be provided, so set the buffer pointer
            // within the message object.
            //
            sCANMessage.pui8MsgData = pui8MsgData;
						
						UARTprintf("CAN message: %x \n",pui8MsgData);
            //
            // Read the message from the CAN.  Message object number 1 is used
            // (which is not the same thing as CAN ID).  The interrupt clearing
            // flag is not set because this interrupt was already cleared in
            // the interrupt handler.
            
            CANMessageGet(CAN0_BASE, 1, &sCANMessage, 0);
						//UARTprintf("CAN message: %d , %x \n", sCANMessage  ,sCANMessage.pui8MsgData);
						
						LED_ON(WHITE_GPIO_PIN,0x00);
            //
            // Clear the pending message flag so that the interrupt handler can
            // set it again when the next message arrives.
            //
            g_bRXFlag = 0;

            //
            // Check to see if there is an indication that some messages were
            // lost.
            //
            if(sCANMessage.ui32Flags & MSG_OBJ_DATA_LOST)
            {
                UARTprintf("CAN message loss detected\n");
            }

            //
            // Print out the contents of the message that was received.
            //
            UARTprintf("Msg ID=0x%08X len=%u data=0x",
                       sCANMessage.ui32MsgID, sCANMessage.ui32MsgLen);
            for(uIdx = 0; uIdx < sCANMessage.ui32MsgLen; uIdx++)
            {
                UARTprintf("%02X \n ", pui8MsgData[uIdx]);
            }
            UARTprintf("total count=%u \n", g_ui32MsgCount);
						
						if ( pui8MsgData[0] == 0X6d)
						{
							UARTprintf("1st");
							input =  NO_BUTTONS_INDEX;
						}
						else if (pui8MsgData[0] == 0x1e)
						{
							UARTprintf("2nd");
							input = LEFT_BUTTON_INDEX;
						}
						else if (pui8MsgData[0] == 0x58)
						{
							UARTprintf("3rd");
							input = RIGHT_BUTTON_INDEX;
						}
						 
						current = state[current_state].Next[input];
						UARTprintf("total count=%u \n", current);
						//current_state = current;
						
						for(int i = 0; i < 4; i++)
						{
							UARTprintf("entry success");
							if(current == state[i].out)
							{
								current_state = i;
								UARTprintf("entry success");
							}
						}
						
						LED_ON(WHITE_GPIO_PIN,(state[current_state].out ));
		
						SysCtlDelay(state[(current_state)].time);
						
        }
    }

    //
    // Return no errors
    //
    return(0);
}
