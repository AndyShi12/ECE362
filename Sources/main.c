/*
************************************************************************
 ECE 362 - Mini-Project C Source File - Fall 2014����  � ������   �� 
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
void outchar(char x); // for debugging use only
char inchar(void);  // for bonus option (terminal input for setting clock)
void rdisp(void);   // RPM display
void bco(char x);   // SCI buffered character output
void shiftout(char);  // LCD drivers (written previously)
void lcdwait(void);
void send_byte(char);
void send_i(char);
void chgline(void);
void print_c(char);
void pmsglcd(char[]);

/* Variable declarations */
char leftpb = 0;  // left pushbutton flag
char rghtpb = 0;  // right pushbutton flag
char prevpb = 0;  // previous pushbutton state
char runstp = 0;  // motor run/stop flag
char onesec   = 0;  // one second flag
char tenths = 0;  // tenth of a second flag
char tin  = 0;  // SCI transmit display buffer IN pointer
char tout = 0;  // SCI transmit display buffer OUT pointer
int red =0;
int blue = 0;
int green = 0;
int pulscnt   = 0;  // pulse count (read from PA every second)
int colormode = 1;
int color = 1;
int LINE1 = 1;
int LINE2 = 0;
char *mess;
int right = 1;
int left = 1;
unsigned int in0;
unsigned int in1;
unsigned int in2;
unsigned int in3;
int wait;
   	   			 		  			 		       

/* Special ASCII characters */
#define TSIZE 81  // transmit buffer size (80 characters)
char tbuf[TSIZE]; // SCI transmit display buffer

#define CR 0x0D   // ASCII return�
#define LF 0x0A   // ASCII new line�

/* LCD COMMUNICATION BIT MASKS (note - different than previous labs) */
#define RS 0x10   // RS pin mask (PTT[4])
#define RW 0x20   // R/W pin mask (PTT[5])
#define LCDCLK 0x40 // LCD EN/CLK pin mask (PTT[6])

/* LCD INSTRUCTION CHARACTERS */
#define LCDON 0x0F  // LCD initialization command
#define LCDCLR 0x01 // LCD clear display command
#define TWOLINE 0x38  // LCD 2-line enable command
#define CURMOV 0xFE // LCD cursor move instruction
	 	   		

void  initializations(void) {

/* Set the PLL speed (bus clock = 24 MHz) */
  CLKSEL = CLKSEL & 0x80; // disengage PLL from system
  PLLCTL = PLLCTL | 0x40; // turn on PLL
  SYNR = 0x02;            // set PLL multiplier
  REFDV = 0;              // set PLL divider
  while (!(CRGFLG & 0x08)){  }
  CLKSEL = CLKSEL | 0x80; // engage PLL

/* Disable watchdog timer (COPCTL register) */
  COPCTL = 0x40   ; // COP off; RTI and COP stopped in BDM-mode

/* Initialize asynchronous serial port (SCI) for 9600 baud, interrupts off initially */
  SCIBDH =  0x00; //set baud rate to 9600
  SCIBDL =  0x9C; //24,000,000 / 16 / 156 = 9600 (approx)  
  SCICR1 =  0x00; //$9C = 156
  SCICR2 =  0x0C; //initialize SCI for program-driven operation
  DDRB   =  0x10; //set PB4 for output mode
  PORTB  =  0x10; //assert DTR pin on COM port
         
         
/* 
   Initialize TIM Ch 7 (TC7) for periodic interrupts every 10.0 ms  
    - Enable timer subsystem��   ����  � ������   �� 
    - Set channel 7 for output compare
    - Set appropriate pre-scale factor and enable counter reset after OC7
    - Set up channel 7 to generate 10 ms interrupt rate
    - Initially disable TIM Ch 7 interrupts                                 
*/                                  

  TSCR1 = 0x80;
  TSCR2 = 0x0C;
  TIOS = 0x80;
  TIE = 0x00;
  TC7 = 15000;
  TIE_C7I = 0;



/*
 Initialize the PWM unit to produce a signal with the following
 characteristics on PWM output channel 3:
   - sampling frequency of approximately 100 Hz
   - left-aligned, negative polarity
   - period register = $FF (yielding a duty cycle range of 0% to 100%,
     for duty cycle register values of $00 to $FF 
   - duty register = $00 (motor initially stopped)
��   ����  � ������   �� 
 IMPORTANT: Need to set MODRR so that PWM Ch 3 is routed to port pin PT3
*/                                
  
  
  PWME = 0x0F;
  PWMPOL = 0x0F;

  PWMPER0 = 0xFF; 
  PWMDTY0 = 0x00;
  
  PWMPER3 = 0xFF;
  PWMDTY3 = 0x00;
  
  PWMPER2 = 0xFF;
  PWMDTY2 = 0x00;
  
  PWMPER1 = 0xFF;
  PWMDTY1 = 0x00;
  
  PWMCLK_PCLK0 = 1;
  PWMCLK_PCLK1 = 1;
  PWMCLK_PCLK2 = 1;
  PWMCLK_PCLK3 = 1;
  
  PWMPRCLK = 0x33;

  PWMSCLA = 0x3B;
  PWMSCLB = 0x3B;   // ******************************************************************************************************************************************

  MODRR_MODRR0 = 1; 
  MODRR_MODRR1 = 1; 
  MODRR_MODRR2 = 1; 
  MODRR_MODRR3 = 1; 




/* 
 Initialize the ATD to sample a D.C. input voltage (range: 0 to 5V)
 on Channel 0 (connected to a 10K-ohm potentiometer). The ATD should
 be operated in a program-driven (i.e., non-interrupt driven), normal
 flag clear mode using nominal sample time/clock prescaler values,
 8-bit, unsigned, non-FIFO mode.
��   ����  � ������   �� 
 Note: Vrh (the ATD reference high voltage) is connected to 5 VDC and
       Vrl (the reference low voltage) is connected to GND on the 
       9S12C32 kit.  An input of 0v will produce output code $00,
       while an input of 5.00 volts will produce output code $FF
*/                                  
  
  ATDCTL2      = 0x80;
  ATDCTL3      = 0x20;      
  ATDCTL4 = 0x85;

                                  

/* 
  Initialize the pulse accumulator (PA) for event counting mode,
  and to increment on negative edges (no PA interrupts will be utilized,
  since overflow should not occur under normal operating conditions)
*/         



/*
  Initialize the RTI for an 2.048 ms interrupt rate
*/

  CRGINT = 0x80;
  RTICTL = 0x1F;  
  
/*
  Initialize SPI for baud rate of 6 Mbs, MSB first
  (note that R/S, R/W', and LCD clk are on different PTT pins)
*/

  SPICR1 = SPICR1 | 0x58;
  SPIBR_SPPR0 = 0x01;
  SPIBR_SPR0 = 0x00;

/* Initialize digital I/O port pins */

  DDRT = 0x7F;
  DDRM = 0xFF;
  DDRAD = 0;
  //ATDDIEN = 0xC0;
  
/* 
   Initialize the LCD
     - pull LCDCLK high (idle)
     - pull R/W' low (write state)
     - turn on LCD (LCDON instruction)
     - enable two-line mode (TWOLINE instruction)
     - clear LCD (LCDCLR instruction)
     - wait for 2ms so that the LCD can wake up     
*/ 

  PTT_PTT6 = 1;
  PTT_PTT5 = 0;
  
  send_byte(LCDON);
  send_byte(TWOLINE);  
  send_byte(LCDCLR);  
  lcdwait();                                  
        
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

  mess = "Mark";
  pmsglcd(mess);

  TIE_C7I = 1;
  colormode = 2;
  PWMDTY3 = 255;
 for(;;) {


 
 if(colormode == 1){  //////////////////////////
  
  
 ATDCTL5 = 0x10;      
 while((128&ATDSTAT0)==0) 
   {} 
   
 in0 = ATDDR0H;
 in1 = ATDDR1H;
 in2 = ATDDR2H;
 in3 = ATDDR3H;
 
 PWMDTY0 = in0;
 PWMDTY1 = in1;
 PWMDTY2 = in2;
 PWMDTY3 = in3;
 }               ///////////////////////////

  red = PWMDTY3;
  green = PWMDTY2;
  blue = PWMDTY1;

    
    
     
  } /* loop forever */
   
}   /* do not leave main */



/*
***********************************************************************   ����  � ������   �� 
 RTI interrupt service routine: RTI_ISR

 Initialized for 8.192 ms interrupt rate

  Samples state of pushbuttons (PAD7 = left, PAD6 = right)

  If change in state from "high" to "low" detected, set pushbutton flag
     leftpb (for PAD7 H -> L), rghtpb (for PAD6 H -> L)
     Recall that pushbuttons are momentary contact closures to ground 
************************************************************************
*/

interrupt 7 void RTI_ISR(void)

{

    CRGFLG = CRGFLG | 0x80; 

                    
    if (left < PTAD_PTAD7) 
    {      
      leftpb = 1;          
    }

    if ((right < PTAD_PTAD6)) 
    {                      
      rghtpb = 1;                          
    }

    right = PTAD_PTAD6;
    left = PTAD_PTAD7;

}

/*
***********************************************************************   ����  � ������   �� 
  TIM interrupt service routine

  Initialized for 10.0 ms interrupt rate

  Uses variable "tencnt" to track if one-tenth second has accumulated
     and sets "tenths" flag 
��   ����  � ������   �� 
  Uses variable "onecnt" to track if one second has accumulated and
     sets "onesec" flag                         
;***********************************************************************
*/

interrupt 15 void TIM_ISR(void)
{
    // clear TIM CH 7 interrupt flag 
  TFLG1 = TFLG1 | 0x80; 

 
if(colormode == 2)  ////////////////////RAINBOW////////////
{
    
    if(PWMDTY3 == 255 && (PWMDTY2 == PWMDTY1 == PWMDTY0 == 0))
      {
        for(wait = 500; wait>0 ; wait--)
        {
          lcdwait();
        }
      }
        
    
    if (color == 1)
    {///////////  change constant 30 to --> (30 * Poteniometer in pin)/255  to control freq
      PWMDTY2++;
      
      for(wait = 30; wait>0 ; wait--)
        {
          lcdwait();
        }
        
        
      if (PWMDTY2 == 127)
      {
        color++;
      } 
    }///////////
    
    if (color == 2)
      {
        PWMDTY2++;
        
        lcdwait();
        
        if (PWMDTY2 == 255)
          {
            color++;
          }
      }
      
    if (color == 3)
      {
        PWMDTY3--;
        
        if (PWMDTY3 == 0)
          {
            color++;
          }
      }
      
    if (color == 4)
      {
        PWMDTY2--;
        PWMDTY1++;
        
        if (PWMDTY2 == 0)
          {
            color++;
          }
      }
      
    if (color == 5)
      {
        PWMDTY3++;
        PWMDTY1--;
        
        if (PWMDTY3 == 127)    
          {
            color++;
          }
      }
      
    if (color == 6)
      {
        PWMDTY1++;
        
        if (PWMDTY3 < 143)
          {
            PWMDTY3++;
          }
      
      if (PWMDTY1 == 255)
        {
          color++;
        } 
      }
      
    if (color == 7)
      {  //////&&
        if (PWMDTY3 < 255)
          {
            PWMDTY3++;
          }
          
        PWMDTY1--;
        
        if (PWMDTY1 == 0)
          {
            color = 1;
          }
       for(wait = 10; wait>0 ; wait--)
        {
          lcdwait();
        }
        
      } //////&&
} //////////////////////////////POT OF GOLD///////////////////////

/*
if (color == 1)
  {
    PWMDTY1++;
    
    if (PWMDTY1 == 255)
      {
        color == 2;
      }
  }
  
if (color == 2)
  {
    PWMDTY1--;
    
    if (PWMDTY1 == 255)
      {
        color == 2;
      }
  }  
*/



}

/*
***********************************************************************   ����  � ������   �� 
  SCI (transmit section) interrupt service routine
��   ����  � ������   �� 
    - read status register to enable TDR write
    - check status of TBUF: if EMPTY, disable SCI transmit interrupts and exit; else, continue
    - access character from TBUF[TOUT]
    - output character to SCI TDR
    - increment TOUT mod TSIZE  

  NOTE: DO NOT USE OUTCHAR (except for debugging)                   
***********************************************************************
*/

interrupt 20 void SCI_ISR(void)
{
 

}

/*
***********************************************************************   ����   ����  � ������   �� 
  SCI buffered character output routine - bco

  Places character x passed to it into TBUF

   - check TBUF status: if FULL, wait for space; else, continue
   - place character in TBUF[TIN]
   - increment TIN mod TSIZE
   - enable SCI transmit interrupts

  NOTE: DO NOT USE OUTCHAR (except for debugging)
***********************************************************************
*/

void bco(char x)
 {
 

}

/*
***********************************************************************   ����   ����  � ������   �� 
 RPM display routine - rdisp
��   ����  � ������   �� 
 This routine starts by reading (and clearing) the 16-bit PA register.
 It then calculates an estimate of the RPM based on the number of
 pulses accumulated from the 64-hole chopper over a one second integration
 period and divides this value by 28 to estimate the gear head output
 shaft speed. Next, it converts this binary value to a 3-digit binary coded
 decimal (BCD) representation and displays the converted value on the
 terminal as "RPM = NNN" (updated in place).  Finally this RPM value, along
 with a bar graph showing ther percent-of-max of the current RPM value
 are shifted out to the LCD using pmsglcd.

***********************************************************************
*/

void rdisp()
{
 
 
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
  int delay;
  while(!SPISR_SPTEF) {}
  SPIDR = x;
  for(delay = 15; delay > 0; delay--) {}
    
  
  
  // read the SPTEF bit, continue if bit is 1
  // write data to SPI data register
  // wait for 30 cycles for SPI data to shift out 

}

/*
***********************************************************************
  lcdwait: Delay for approx 2 ms
***********************************************************************
*/

void lcdwait()
{
  asm{
    delay:
      pshx
      psha
      pshc
      ldaa #2
      
    loopo:  
      ldx #7995
      
     loopi:
       dbne x,loopi
       
       dbne a,loopo
       
       pulc
       pula
       pulx
  }
}

/*
*********************************************************************** 
  send_byte: writes character x to the LCD
***********************************************************************
*/

void send_byte(char x)
{
     // shift out character
     shiftout(x);
     // pulse LCD clock line low->high->low
     PTT_PTT6 = 0;
     lcdwait();
     PTT_PTT6 = 1;
     lcdwait();
     PTT_PTT6 = 0;
     lcdwait();

     // wait 2 ms for LCD to process data
}

/*
***********************************************************************
  send_i: Sends instruction byte x to LCD  
***********************************************************************
*/

void send_i(char x)
{
        // set the register select line low (instruction data)
     PTT_PTT4 = 0;
     send_byte(x);
        // send byte
}

/*
***********************************************************************
  chgline: Move LCD cursor to position x
  NOTE: Cursor positions are encoded in the LINE1/LINE2 variables
***********************************************************************
*/

void chgline()
{

     if(LINE1 == 1)
     {
        send_i(CURMOV);
        send_i(0xC0);
        LINE1--;
        LINE2++;
     } else 
     {
        send_i(CURMOV);
        send_i(0x80);
        LINE1++;
        LINE2--;
     }
    
}

/*
***********************************************************************
  print_c: Print (single) character x on LCD            
***********************************************************************
*/
 
void print_c(char x)
{
     PTT_PTT4 = 1;
     send_byte(x); //
}

/*
***********************************************************************
  pmsglcd: print character string str[] on LCD
***********************************************************************
*/

void pmsglcd(char str[])
{
     int mark = 0;
     int toprint;
     
     while(str[mark] != 0) 
     {
       toprint = str[mark];
       print_c(toprint);
       mark++;
     }
       
      //
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

