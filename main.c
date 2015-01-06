//-----------------------------------------------------------------------------
// Micro's Micros
//-----------------------------------------------------------------------------
// Copyright 2014 - Hugo Serrat and Vianney Tran
//
// Program Description:
//
// This software allows to turn any table into a touch interface
// You need 2 micros
// Pinout:
//
// P0.0 - Micro 1
// P0.1 - Micro 2
// P3.3 - Status LED (for tests)
//
//
// Target:         C8051F31x
// Tool chain:     Keil

//-----------------------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------------------

#include "c8051F310.h"
#include <stdio.h>

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

sfr16 TMR2RL   = 0xca;                    // Timer2 reload value
sfr16 TMR2     = 0xcc;                    // Timer2 counter

#define SYSCLK          24500000    // Clock speed in Hz

sbit LED = P3^3;

// Available modes are:
// 1 - Test first MIC (P0.0)
// 2 - Test MIC2 (P0.1)
// 10 - Run (Normal mode)
// 11 - Set coeff

int PROG_MODE = 10;

int MIC_FLAG = 0;
// 0 when none has listen
// 1 when MIC1 have smth
// 2 when MIC2 have smth
// 3 when both have smth and MIC1 first
// 4 when both have smth and MIC2 first
int DIFF_TIME = 0;

/*
int MIC0_FLAG = 0; // Set to 1 when find something on MIC0
int MIC0_LOW = 0; // Low timer
int MIC0_HIGH = 0; // High timer
int MIC1_FLAG = 0; // Set to 1 when find something on MIC1
int MIC1_LOW = 0; // Low timer
int MIC1_HIGH = 0; // High timer
*/

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void Oscillator_Init (void);           // Configure the system clock
void Port_Init (void);                 // Configure the Crossbar and GPIO
void Ext_Interrupt_Init (void);        // Configure External Interrupts
void UART_Init (void);
void Timer2_Init (int);
void Timer2_ISR (void);
void wait100(void);

void Put_char_ (unsigned char);

//-----------------------------------------------------------------------------
// MAIN Routine
//-----------------------------------------------------------------------------
void main (void)
{
	PCA0MD &= ~0x40;                    // Disable Watchdog timer

	Oscillator_Init();                  // Initialize the system clock
	Port_Init ();                       // Initialize crossbar and GPIO
	Ext_Interrupt_Init();               // Initialize External Interrupts
	UART_Init ();
	Timer2_Init(0);

	EA = 1;
	LED = 1; // Put the LED On
	while(1)
	{
		// Wait for an input
		if (RI0)
		{
			RI0 = 0; // Reset Input
			if (SBUF0 == 97) // Letter "a"
			{
				PROG_MODE = 1;
				printf("Test mode MIC1\r\n");
			}
			if (SBUF0 == 98) // "b"
			{
				PROG_MODE = 2;
				printf("Test mode MIC2\r\n");
			}
			if (SBUF0 == 114) // "r"
			{
				PROG_MODE = 10;
				printf("Run Mode\r\n");
			}
		}
		
		// Print value
		if (PROG_MODE == 10)
		{
			if ((MIC_FLAG == 3) || (MIC_FLAG == 4))
			{
				if (MIC_FLAG == 3)
				{
					printf("Micro 1 first with diff: %u\r\n", DIFF_TIME);
				}
				if (MIC_FLAG == 4)
				{
					printf("Micro 2 first with diff: %u\r\n", DIFF_TIME);
				}
				//printf(" %d - ", MIC0_HIGH);
				//printf("%d and ", MIC0_LOW);
				//printf(" and ");
				//printf(" %d - ", MIC1_HIGH);
				//printf("%d\r\n", MIC1_LOW);
				//printf("\r\n");
				wait100();
				MIC_FLAG = 0;
			}
		}
		MIC_FLAG = 0;
		wait100();
		
   }
}

//-----------------------------------------------------------------------------
// Initialization Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Oscillator_Init
//-----------------------------------------------------------------------------

void Oscillator_Init (void)
{
   OSCICN = 0xC3;                      // Set internal oscillator to run
   RSTSRC = 0x04;                      // at its maximum frequency
}

//-----------------------------------------------------------------------------
// Port_Init
//-----------------------------------------------------------------------------

void Port_Init (void)
{
	XBR0 = 0x01; // Enable UART
	XBR1 = 0x40;
	P0SKIP = 0x03;
	P0MDOUT |= 0x10;

    P3MDOUT = 0x08;
}

void UART_Init(void)
{
	TH1 = -213;
	TMOD |= 0x20;
	CKCON |= 0x08;
	TR1 = 1;
	REN0 = 1;
	SBUF0 = '\r';
}

//-----------------------------------------------------------------------------
// Ext_Interrupt_Init
//-----------------------------------------------------------------------------

void Ext_Interrupt_Init (void)
{
	TCON = 0x05;                        // /INT 0 and /INT 1 are edge triggered
	IT01CF = 0x89;	// INT0 is P0.0 and INT1 is P0.1       

	EX0 = 1;                            // Enable /INT0 interrupts
	EX1 = 1;                            // Enable /INT1 interrupts
	IE = 0xA5;
	//IE = 0xFF;

}

void Timer2_Init (int counts)
{
   TMR2CN  = 0x00;                        // Stop Timer2; Clear TF2;
                                          // use SYSCLK/12 as timebase
   CKCON  &= ~0x60;                       // Timer2 clocked based on T2XCLK;

   TMR2RL  = -counts;                     // Init reload values
   TMR2    = 0xffff;                      // set to reload immediately
   ET2     = 1;                           // enable Timer2 interrupts
   TR2     = 1;                           // start Timer2
}
//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// /INT0 ISR
//-----------------------------------------------------------------------------
//
// Whenever a negative edge appears on P0.0, LED1 is toggled.
// The interrupt pending flag is automatically cleared by vectoring to the ISR
//
//-----------------------------------------------------------------------------
void INT0_ISR (void) interrupt 0
{
	if (PROG_MODE == 10)
	{
		if (MIC_FLAG == 0)
		{
			TMR2 = 0xffff; // Reset counter
			MIC_FLAG = 1;
		}
		if (MIC_FLAG == 2)
		{
			//DIFF_TIME = TMR2;
			DIFF_TIME = TMR2;
			MIC_FLAG = 4;
		}
	}
	if (PROG_MODE == 1)
	{
		LED = ~LED;
	}
}

void INT1_ISR (void) interrupt 2
{
	if (PROG_MODE == 10)
	{
		if (MIC_FLAG == 0)
		{
			TMR2 = 0xffff; // Reset counter
			MIC_FLAG = 2;
		}
		if (MIC_FLAG == 1)
		{
			//DIFF_TIME = TMR2;
			DIFF_TIME = TMR2;
			MIC_FLAG = 3;
		}
	}
	if (PROG_MODE == 2)
	{
		LED = ~LED;
	}
}

void Timer2_ISR (void) interrupt 5
{
	static unsigned int counter = 0;
	counter += 1;
	TF2H = 0; // clear Timer2 interrupt flag
	if (counter >= 50)
	{
		counter = 0;
	}
}

//-----------------------------------------------------------------------------
// User Functions
//-----------------------------------------------------------------------------

void Put_char_(unsigned char c)
{
	// Wait available output
	while (TI0 == 0) {}
	TI0 = 0;
	// Send a char
	SBUF0 = c;
}

void wait100(void)
{
	unsigned int i = 0;
	for (i = 0; i < 60000; i++)
	{
		unsigned int j = 0;
	}
}

