/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Fall 2014                    
***********************************************************************
	 	   			 		  			 		  		
 Team ID: Team 6

 Project Name: < ? >

 Team Members:

   - Team/Doc Leader: < ? >      Signature: ______________________
   
   - Software Leader: < ? >      Signature: ______________________

   - Interface Leader: < ? >     Signature: ______________________

   - Peripheral Leader: < ? >    Signature: ______________________


 Academic Honesty Statement:  In signing above, we hereby certify that we 
 are the individuals who created this HC(S)12 source file and that we have
 not copied the work of any other student (past or present) while completing 
 it. We understand that if we fail to honor this agreement, we will receive 
 a grade of ZERO and be subject to possible disciplinary action.

***********************************************************************

 The objective of this Mini-Project is to .... < ? >


***********************************************************************

 List of project-specific success criteria (functionality that will be
 demonstrated):

 1.

 2.

 3.

 4.

 5.

***********************************************************************

  Date code started: < ? >

  Update history (add an entry every time a significant change is made):

  Date: < ? >  Name: < ? >   Update: < ? >

  Date: < ? >  Name: < ? >   Update: < ? >

  Date: < ? >  Name: < ? >   Update: < ? >


***********************************************************************
*/

#include <hidef.h>      /* common defines and macros */
#include "derivative.h"      /* derivative-specific definitions */
#include <mc9s12c32.h>

/* All functions after main should be initialized here */
char inchar(void);
void outchar(char x);


/* Variable declarations */

   	   			 		  			 		       

/* Special ASCII characters */
#define CR 0x0D		// ASCII return 
#define LF 0x0A		// ASCII new line 

/* LCD COMMUNICATION BIT MASKS (note - different than previous labs) */
#define RS 0x10		// RS pin mask (PTT[4])
#define RW 0x20		// R/W pin mask (PTT[5])
#define LCDCLK 0x40	// LCD EN/CLK pin mask (PTT[6])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F	// LCD initialization command
#define LCDCLR 0x01	// LCD clear display command
#define TWOLINE 0x38	// LCD 2-line enable command
#define CURMOV 0xFE	// LCD cursor move instruction
#define LINE1 = 0x80	// LCD line 1 cursor position
#define LINE2 = 0xC0	// LCD line 2 cursor position

	 	   		
/*	 	   		
***********************************************************************
 Initializations
***********************************************************************
*/

void  initializations(void) {

/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; //; disengage PLL from system
  PLLCTL = PLLCTL | 0x40; //; turn on PLL
  SYNR = 0x02;            //; set PLL multiplier
  REFDV = 0;              //; set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; //; engage PLL

/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40   ; //COP off; RTI and COP stopped in BDM-mode

/* Initialize asynchronous serial port (SCI) for 9600 baud, interrupts off initially */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port

/* Initialize peripherals */
	 	   		
//***************************
//*  PWM                    *
//***************************
 
/* 
 Initialize TIM Ch 7 (TC7) for periodic interrupts every 10.0 ms  
  - Enable timer subsystem                         
  - Set channel 7 for output compare
  - Set appropriate pre-scale factor and enable counter reset after OC7
  - Set up channel 7 to generate 10 ms interrupt rate
  - Initially disable TIM Ch 7 interrupts	 	   			 		  			 		  		
*/	 	  
  // slide 199 	
  		 		  			 		  		
 	TSCR1 = 0x80;
  TSCR2 = 0x0C;
  TIOS  = 0x80;
  TIE   = 0x80;
  TC7   = 15000;    

/*
 Initialize the PWM unit to produce a signal with the following
 characteristics on PWM output channel 3:
   - sampling frequency of approximately 100 Hz
   - left-aligned, negative polarity
   - period register = $FF (yielding a duty cycle range of 0% to 100%,
     for duty cycle register values of $00 to $FF 
   - duty register = $00 (motor initially stopped)
                         
 IMPORTANT: Need to set MODRR so that PWM Ch 3 is routed to port pin PT3
*/ 	   			 		  			 		  		
  // Initialize the PWM unit to produce a signal with the following characteristics on PWM output channel 3:
  MODRR = 0x08;	    // PT3 used as PWM Ch 3 output
  PWME = 0x08;      	//enable PWM Ch 3
	
	//- sampling frequency of approximately 100 Hz
	PWMPRCLK	= 0x00;   //sampling frequency = CLK/PWMPER0 = 24MHz/250 = 96kHz
  
  //- left-aligned, negative polarity
  PWMPOL	= 0x00;     //active low polarity
 	PWMCAE	= 0x00;     //left-aligned output mode
  
  //- period register = $FF (yielding a duty cycle range of 0% to 100%, for duty cycle register values of $00 to $FF 
  PWMCTL	= 0x00;     //no concatenate (8-bit)
	PWMCLK	= 0x02;     //select Clock A for Ch 0
  
  //- duty register = $00 (motor initially stopped)
 	PWMDTY0	= 0;        //initially clear DUTY register proportional to the amplitude of waveform

 //PWMPER0	= 250;      //set maximum 8-bit period


//***************************
//*  ATD Sampling           *
//***************************
 
/* 
 Initialize the ATD to sample a D.C. input voltage (range: 0 to 5V)
 on Channel 0 (connected to a 10K-ohm potentiometer). The ATD should
 be operated in a program-driven (i.e., non-interrupt driven), normal
 flag clear mode using nominal sample time/clock prescaler values,
 8-bit, unsigned, non-FIFO mode.
                         
 Note: Vrh (the ATD reference high voltage) is connected to 5 VDC and
       Vrl (the reference low voltage) is connected to GND on the 
       9S12C32 kit.  An input of 0v will produce output code $00,
       while an input of 5.00 volts will produce output code $FF
*/	 	   			 		  			 		  		
    ATDCTL2 = 0x80;
    ATDCTL3 = 0x08;
    ATDCTL4 = 0x85;
  			 		  			 		  		

/* 
  Initialize the pulse accumulator (PA) for event counting mode,
  and to increment on negative edges (no PA interrupts will be utilized,
  since overflow should not occur under normal operating conditions)
*/		     
   PACTL = 0x40;

//***************************
//*  RTI, SPI, and I/O      *
//***************************

/*
  Initialize the RTI for an 2.048 ms interrupt rate
*/
  RTICTL = 0x41;
  CRGINT = CRGINT | 0x80;
  
/*
  Initialize SPI for baud rate of 6 Mbs, MSB first
  (note that R/S, R/W', and LCD clk are on different PTT pins)
*/
  SPICR1 = 0x50;
  SPICR2 = 0x00;
  DDRM   = 0xFF;
  SPIBR  = 0x01;

/* Initialize digital I/O port pins */
    DDRAD = 0; 	
    ATDDIEN = 0xC0; 
    DDRT = 0xff;
    
//***************************
//*  Initialize LCD         *
//***************************
/* 
 Initialize the LCD
   - pull LCDCLK high (idle)
   - pull R/W' low (write state)
   - turn on LCD (LCDON instruction)
   - enable two-line mode (TWOLINE instruction)
   - clear LCD (LCDCLR instruction)
   - wait for 2ms so that the LCD can wake up     
*/ 
	PTT_PTT4 = 1;   // pull high
  PTT_PTT3 = 0;   // pull low
  send_i(LCDON);  // turn on LCD
  send_i(TWOLINE);  //enable 2 lines
  send_i(LCDCLR);   //clear LCD 
            
/* Initialize interrupts */
	      
	      
}

	 		  			 		  		
/*	 		  			 		  		
***********************************************************************
Main
***********************************************************************
*/
void main(void) {
  	DisableInterrupts
	initializations(); 		  			 		  		
	EnableInterrupts;

 for(;;) {
  
/* < start of your main loop > */ 
  
  

  
   } /* loop forever */
   
}   /* do not leave main */




/*
***********************************************************************                       
 RTI interrupt service routine: RTI_ISR
************************************************************************
*/

interrupt 7 void RTI_ISR(void)
{
  	// clear RTI interrupt flagt 
  	CRGFLG = CRGFLG | 0x80; 
 

}

/*
***********************************************************************                       
  TIM interrupt service routine	  		
***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{
  	// clear TIM CH 7 interrupt flag 
 	TFLG1 = TFLG1 | 0x80; 
 

}

/*
***********************************************************************                       
  SCI interrupt service routine		 		  		
***********************************************************************
*/

interrupt 20 void SCI_ISR(void)
{
 


}


/*
***********************************************************************                              
display

***********************************************************************
*/

void disp()
{
   /* 
   int hundred;
    int tens;
    int ones;
    
   rpm = pulscnt * 60/64/28;
   send_i(LCDCLR);
   chgline(0x80);
   
   pmsglcd("RPM");
   
   hundred = rpm/100 + 48;
   print_c(hundred);
   tens = (rpm%100)/10+48;
   print_c(tens);
   ones = (rpm%100)%10+48;
   print_c(ones);
   */
}

/*
***********************************************************************
  shiftout: Transmits the character x to external shift 
            register using the SPI.  It should shift MSB first.  
            MISO = PM[4]
            SCK  = PM[5]
***********************************************************************
*/
 
void shiftout(char x)

{
  /*
  // test the SPTEF bit: wait if 0; else, continue
  // write data x to SPI data register
  // wait for 30 cycles for SPI data to shift out 

  //PTT_PTT4 = 1;
  while(!SPISR_SPTEF);
  SPIDR = x;
  while(!SPISR_SPTEF);
  // write data to SPI data register
  lcdwait();
  PTT_PTT4 = 0;
  // wait for 30 cycles for SPI data to shift out
  lcdwait();
  PTT_PTT4 = 1;
  lcdwait();
  */
}

/*
***********************************************************************
  lcdwait: Delay for approx 2 ms
***********************************************************************
*/

void lcdwait()
{
   int n;
   for(n=0; n<4800; n++);
}

/*
*********************************************************************** 
  send_byte: writes character x to the LCD
***********************************************************************
*/

void send_byte(char x)
{
     // shift out character
     // pulse LCD clock line low->high->low
     // wait 2 ms for LCD to process data
     PTT_PTT2 = 1; 
     shiftout(x);
     lcdwait();
}

/*
***********************************************************************
  send_i: Sends instruction byte x to LCD  
***********************************************************************
*/

void send_i(char x)
{
        PTT_PTT2 = 0; 
        shiftout(x);
}

/*
***********************************************************************
  chgline: Move LCD cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/

void chgline(char x)
{
  send_i(CURMOV);
  send_i(x);
}

/*
***********************************************************************
  print_c: Print (single) character x on LCD            
***********************************************************************
*/
 
void print_c(char x)
{
  while(!(SPISR & 0x80));
   send_byte(x);
}

/*
***********************************************************************
  pmsglcd: print character string str[] on LCD
***********************************************************************
*/

void pmsglcd(char str[])
{
   int n = 0;
   int length = strlen(str);
   for(n=0; n< length; n++) {
   print_c(str[n]);
   }
}

/*
***********************************************************************
 Character I/O Library Routines for 9S12C32 
***********************************************************************
 Name:         inchar
 Description:  inputs ASCII character from SCI serial port and returns it
 Example:      char ch1 = inchar();
***********************************************************************
*/

char inchar(void) {
  /* receives character from the terminal channel */
        while (!(SCISR1 & 0x20)); /* wait for input */
    return SCIDRL;
}

/*
***********************************************************************
 Name:         outchar    (use only for DEBUGGING purposes)
 Description:  outputs ASCII character x to SCI serial port
 Example:      outchar('x');
***********************************************************************
*/

void outchar(char x) {
  /* sends a character to the terminal channel */
    while (!(SCISR1 & 0x80));  /* wait for output buffer empty */
    SCIDRL = x;
}