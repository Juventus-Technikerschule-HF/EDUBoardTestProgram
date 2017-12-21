/****************************************************************************
 Title	:   C include file for the KS0066U LCD library (lcd_ks0066_4bit_io.c)
 Author:    Original von Peter Fleury, anpassung Jürgen Oelkoch j.oe@gmx.de
 Software:  AVR-GCC 4.3.2
 Hardware:  any AVR device

 DESCRIPTION
       Basic routines for interfacing a KS006U-based text lcd display
       in 4-bit-io-mode

 USAGE
       See the C include lcd_ks0066_4bit_io.h file for a description of each
       function
       
*****************************************************************************/
/*****************************************************************************
 Modification by Martin Burger to support ATXmega without delay stuff
*****************************************************************************/

#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "lcd_ks0066_4bit_io.h"

/****************************************************************************
** constants/macros 
*****************************************************************************/



// #define DDR(x) (*(&x - 1))      /* address of data direction register of port x */
// #if defined(__AVR_ATmega64__) || defined(__AVR_ATmega128__)
//     /* on ATmega64/128 PINF is on port 0x00 and not 0x60 */
//     #define PIN(x) ( &PORTF==&(x) ? _SFR_IO8(0x00) : (*(&x - 2)) )
// #else
// 	#define PIN(x) (*(&x - 2))    /* address of input register of port x          */
// #endif

#define lcd_e_delay()   __asm__ __volatile__( "rjmp 1f\n 1:" );
#define lcd_e_high()    LCD_E_PORT.OUT  |=  (1<<LCD_E_PIN);
#define lcd_e_low()     LCD_E_PORT.OUT  &= ~(1<<LCD_E_PIN);
#define lcd_e_toggle()  toggle_e()
#define lcd_rw_high()   LCD_RW_PORT.OUT |=  (1<<LCD_RW_PIN)
#define lcd_rw_low()    LCD_RW_PORT.OUT &= ~(1<<LCD_RW_PIN)
#define lcd_rs_high()   LCD_RS_PORT.OUT |=  (1<<LCD_RS_PIN)
#define lcd_rs_low()    LCD_RS_PORT.OUT &= ~(1<<LCD_RS_PIN)

#if LCD_LINES==1 /* number of lines; defined in lcd_ks0066_4bit_io.h */
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_1LINE 
#else
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_2LINES 
#endif

/****************************************************************************
** function prototypes 
*****************************************************************************/
static void toggle_e(void);

/****************************************************************************
** local functions
*****************************************************************************/

uint32_t usCounter = 0;

void _delay_us(uint32_t us) {
	TCF0.INTCTRLA = 0x02;
	usCounter = us;
	while(usCounter>0);
	TCF0.INTCTRLA = 0x00;
}
ISR(TCF0_OVF_vect) {
	if(usCounter > 0) {
		usCounter--;
	}	
}
void _delay_ms(uint32_t ms) {
	TCF0.INTCTRLA = 0x02;
	usCounter = ms*1000;
	while(usCounter>0);
	TCF0.INTCTRLA = 0x00;
}

/* toggle Enable Pin to initiate write */
static void toggle_e(void)
{
    lcd_e_high();
    //lcd_e_delay();
	_delay_us(5);
    lcd_e_low();
}

/*************************************************************************
Low-level function to write byte to LCD controller
Input:    data   byte to write to LCD
          rs     1: write data    
                 0: write instruction
Returns:  none
*************************************************************************/
static void lcd_write(uint8_t data,uint8_t rs) 
{
    unsigned char dataBits ;


    if (rs) {   /* write data        (RS=1, RW=0) */
       lcd_rs_high();
    } else {    /* write instruction (RS=0, RW=0) */
       lcd_rs_low();
    }
    lcd_rw_low();

    if ( ( &LCD_DATA0_PORT == &LCD_DATA1_PORT) && ( &LCD_DATA1_PORT == &LCD_DATA2_PORT ) && ( &LCD_DATA2_PORT == &LCD_DATA3_PORT )
      && (LCD_DATA0_PIN == 0) && (LCD_DATA1_PIN == 1) && (LCD_DATA2_PIN == 2) && (LCD_DATA3_PIN == 3) )
    {
        /* configure data pins as output */
        LCD_DATA0_PORT.DIRSET = 0x0F;

        /* output high nibble first */
        dataBits = LCD_DATA0_PORT.IN & 0xF0;
        LCD_DATA0_PORT.OUT = dataBits |((data>>4)&0x0F);
        lcd_e_toggle();

        /* output low nibble */
        LCD_DATA0_PORT.OUT = dataBits | (data&0x0F);
        lcd_e_toggle();

        /* all data pins high (inactive) */
        LCD_DATA0_PORT.OUT = dataBits | 0x0F;
    }
    else
    {
        /* configure data pins as output */
        LCD_DATA0_PORT.DIR |= (1<<LCD_DATA0_PIN);
        LCD_DATA1_PORT.DIR |= (1<<LCD_DATA1_PIN);
        LCD_DATA2_PORT.DIR |= (1<<LCD_DATA2_PIN);
        LCD_DATA3_PORT.DIR |= (1<<LCD_DATA3_PIN);
        
        /* output high nibble first */
        LCD_DATA3_PORT.OUT &= ~(1<<LCD_DATA3_PIN);
        LCD_DATA2_PORT.OUT &= ~(1<<LCD_DATA2_PIN);
        LCD_DATA1_PORT.OUT &= ~(1<<LCD_DATA1_PIN);
        LCD_DATA0_PORT.OUT &= ~(1<<LCD_DATA0_PIN);
    	if(data & 0x80) LCD_DATA3_PORT.OUT |= (1<<LCD_DATA3_PIN);
    	if(data & 0x40) LCD_DATA2_PORT.OUT |= (1<<LCD_DATA2_PIN);
    	if(data & 0x20) LCD_DATA1_PORT.OUT |= (1<<LCD_DATA1_PIN);
    	if(data & 0x10) LCD_DATA0_PORT.OUT |= (1<<LCD_DATA0_PIN);   
        lcd_e_toggle();
        
        /* output low nibble */
        LCD_DATA3_PORT.OUT &= ~(1<<LCD_DATA3_PIN);
        LCD_DATA2_PORT.OUT &= ~(1<<LCD_DATA2_PIN);
        LCD_DATA1_PORT.OUT &= ~(1<<LCD_DATA1_PIN);
        LCD_DATA0_PORT.OUT &= ~(1<<LCD_DATA0_PIN);
    	if(data & 0x08) LCD_DATA3_PORT.OUT |= (1<<LCD_DATA3_PIN);
    	if(data & 0x04) LCD_DATA2_PORT.OUT |= (1<<LCD_DATA2_PIN);
    	if(data & 0x02) LCD_DATA1_PORT.OUT |= (1<<LCD_DATA1_PIN);
    	if(data & 0x01) LCD_DATA0_PORT.OUT |= (1<<LCD_DATA0_PIN);
        lcd_e_toggle();        
        
        /* all data pins high (inactive) */
        LCD_DATA0_PORT.OUT |= (1<<LCD_DATA0_PIN);
        LCD_DATA1_PORT.OUT |= (1<<LCD_DATA1_PIN);
        LCD_DATA2_PORT.OUT |= (1<<LCD_DATA2_PIN);
        LCD_DATA3_PORT.OUT |= (1<<LCD_DATA3_PIN);
    }
}

/*************************************************************************
Low-level function to read byte from LCD controller
Input:    rs     1: read data    
                 0: read busy flag / address counter
Returns:  byte read from LCD controller
*************************************************************************/
static uint8_t lcd_read(uint8_t rs) 
{
    uint8_t data;
    
    
    if (rs)
        lcd_rs_high();                       /* RS=1: read data      */
    else
        lcd_rs_low();                        /* RS=0: read busy flag */
    
    lcd_rw_high();                           /* RW=1  read mode      */
    
    if ( ( &LCD_DATA0_PORT == &LCD_DATA1_PORT) && ( &LCD_DATA1_PORT == &LCD_DATA2_PORT ) && ( &LCD_DATA2_PORT == &LCD_DATA3_PORT )
      && ( LCD_DATA0_PIN == 0 )&& (LCD_DATA1_PIN == 1) && (LCD_DATA2_PIN == 2) && (LCD_DATA3_PIN == 3) )
    {
        LCD_DATA0_PORT.DIRCLR = 0x0F;         /* configure data pins as input */
        
        lcd_e_high();
        lcd_e_delay();        
        data = LCD_DATA0_PORT.IN << 4;     /* read high nibble first */
        lcd_e_low();
        
        lcd_e_delay();                       /* Enable 500ns low       */
        
        lcd_e_high();
        lcd_e_delay();
        data |= LCD_DATA0_PORT.IN &0x0F;    /* read low nibble        */
        lcd_e_low();
    }
    else
    {
        /* configure data pins as input */
        LCD_DATA0_PORT.DIRCLR = (1<<LCD_DATA0_PIN);
        LCD_DATA1_PORT.DIRCLR = (1<<LCD_DATA1_PIN);
        LCD_DATA2_PORT.DIRCLR = (1<<LCD_DATA2_PIN);
        LCD_DATA3_PORT.DIRCLR = (1<<LCD_DATA3_PIN);
		        
        /* read high nibble first */
        lcd_e_high();
        lcd_e_delay();        
        data = 0;
        if ( LCD_DATA0_PORT.IN & (1<<LCD_DATA0_PIN) ) data |= 0x10;
        if ( LCD_DATA1_PORT.IN & (1<<LCD_DATA1_PIN) ) data |= 0x20;
        if ( LCD_DATA2_PORT.IN & (1<<LCD_DATA2_PIN) ) data |= 0x40;
        if ( LCD_DATA3_PORT.IN & (1<<LCD_DATA3_PIN) ) data |= 0x80;
        lcd_e_low();

        lcd_e_delay();                       /* Enable 500ns low       */
    
        /* read low nibble */    
        lcd_e_high();
        lcd_e_delay();
        if ( LCD_DATA0_PORT.IN & (1<<LCD_DATA0_PIN) ) data |= 0x01;
        if ( LCD_DATA1_PORT.IN & (1<<LCD_DATA1_PIN) ) data |= 0x02;
        if ( LCD_DATA2_PORT.IN & (1<<LCD_DATA2_PIN) ) data |= 0x04;
        if ( LCD_DATA3_PORT.IN & (1<<LCD_DATA3_PIN) ) data |= 0x08;        
        lcd_e_low();
    }
    return data;
}

/*************************************************************************
loops while lcd is busy, returns address counter
*************************************************************************/
static uint8_t lcd_waitbusy(void)

{
    register uint8_t c;
    
    /* wait until busy flag is cleared */
    while ( (c=lcd_read(0)) & (1<<LCD_BUSY)) {}
    
    /* the address counter is updated 4us after the busy flag is cleared */
    _delay_us(2);

    /* now read the address counter */
    return (lcd_read(0));  // return address counter
    
}/* lcd_waitbusy */

/*************************************************************************
Move cursor to the start of next line or to the first line if the cursor 
is already on the last line.
*************************************************************************/
static inline void lcd_newline(uint8_t pos)
{
    register uint8_t addressCounter;


#if LCD_LINES==1
    addressCounter = 0;
#endif
#if LCD_LINES==2
    if ( pos < (LCD_START_LINE2) )
        addressCounter = LCD_START_LINE2;
    else
        addressCounter = LCD_START_LINE1;
#endif
#if LCD_LINES==4
    if ( pos < LCD_START_LINE3 )
        addressCounter = LCD_START_LINE2;
    else if ( (pos >= LCD_START_LINE2) && (pos < LCD_START_LINE4) )
        addressCounter = LCD_START_LINE3;
    else if ( (pos >= LCD_START_LINE3) && (pos < LCD_START_LINE2) )
        addressCounter = LCD_START_LINE4;
    else 
        addressCounter = LCD_START_LINE1;
#endif
    lcd_command((1<<LCD_DDRAM)+addressCounter);

}/* lcd_newline */

/*
** PUBLIC FUNCTIONS 
*/

/*************************************************************************
Send LCD controller instruction command
Input:   instruction to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_command(uint8_t cmd)
{
    lcd_waitbusy();
    lcd_write(cmd,0);
}


/*************************************************************************
Send data byte to LCD controller 
Input:   data to send to LCD controller, see HD44780 data sheet
Returns: none
*************************************************************************/
void lcd_data(uint8_t data)
{
    lcd_waitbusy();
    lcd_write(data,1);
}



/*************************************************************************
Set cursor to specified position
Input:    x  horizontal position  (0: left most position)
          y  vertical position    (0: first line)
Returns:  none
*************************************************************************/
void lcd_gotoxy(uint8_t x, uint8_t y)
{
#if LCD_LINES==1
    lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
#endif
#if LCD_LINES==2
    if ( y==0 ) 
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
    else
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
#endif
#if LCD_LINES==4
    if ( y==0 )
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
    else if ( y==1)
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
    else if ( y==2)
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE3+x);
    else /* y==3 */
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE4+x);
#endif

}/* lcd_gotoxy */


/*************************************************************************
*************************************************************************/
int lcd_getxy(void)
{
    return lcd_waitbusy();
}


/*************************************************************************
Clear display and set cursor to home position
*************************************************************************/
void lcd_clrscr(void)
{
    lcd_command(1<<LCD_CLR);
}


/*************************************************************************
Set cursor to home position
*************************************************************************/
void lcd_home(void)
{
    lcd_command(1<<LCD_HOME);
}


/*************************************************************************
Display character at current cursor position 
Input:    character to be displayed                                       
Returns:  none
*************************************************************************/
void lcd_putc(char c)
{
    uint8_t pos;


    pos = lcd_waitbusy();   // read busy-flag and address counter
    if (c=='\n')
    {
        lcd_newline(pos);
    }
    else
    {
#if LCD_WRAP_LINES==1
#if LCD_LINES==1
        if ( pos == LCD_START_LINE1+LCD_DISP_LENGTH ) {
            lcd_write((1<<LCD_DDRAM)+LCD_START_LINE1,0);
        }
#elif LCD_LINES==2
        if ( pos == LCD_START_LINE1+LCD_DISP_LENGTH ) {
            lcd_write((1<<LCD_DDRAM)+LCD_START_LINE2,0);    
        }else if ( pos == LCD_START_LINE2+LCD_DISP_LENGTH ){
            lcd_write((1<<LCD_DDRAM)+LCD_START_LINE1,0);
        }
#elif LCD_LINES==4
        if ( pos == LCD_START_LINE1+LCD_DISP_LENGTH ) {
            lcd_write((1<<LCD_DDRAM)+LCD_START_LINE2,0);    
        }else if ( pos == LCD_START_LINE2+LCD_DISP_LENGTH ) {
            lcd_write((1<<LCD_DDRAM)+LCD_START_LINE3,0);
        }else if ( pos == LCD_START_LINE3+LCD_DISP_LENGTH ) {
            lcd_write((1<<LCD_DDRAM)+LCD_START_LINE4,0);
        }else if ( pos == LCD_START_LINE4+LCD_DISP_LENGTH ) {
            lcd_write((1<<LCD_DDRAM)+LCD_START_LINE1,0);
        }
#endif
        lcd_waitbusy();
#endif
        lcd_write(c, 1);
    }

}/* lcd_putc */


/*************************************************************************
Display string without auto linefeed 
Input:    string to be displayed
Returns:  none
*************************************************************************/
void lcd_puts(const char *s)
/* print string on lcd (no auto linefeed) */
{
    register char c;

    while ( (c = *s++) ) {
        lcd_putc(c);
    }

}/* lcd_puts */


/*************************************************************************
Display string from program memory without auto linefeed 
Input:     string from program memory be be displayed                                        
Returns:   none
*************************************************************************/
void lcd_puts_p(const char *progmem_s)
/* print string from program memory on lcd (no auto linefeed) */
{
    register char c;

    while ( (c = pgm_read_byte(progmem_s++)) ) {
        lcd_putc(c);
    }

}/* lcd_puts_p */

/*************************************************************************
Initialize display and select type of cursor 
Input:    dispAttr LCD_DISP_OFF            display off
                   LCD_DISP_ON             display on, cursor off
                   LCD_DISP_ON_CURSOR      display on, cursor on
                   LCD_DISP_CURSOR_BLINK   display on, cursor on flashing
Returns:  none
*************************************************************************/

#define FCPU  32000000
void lcd_init(uint8_t dispAttr)
{
    TCF0.CTRLA = TC_CLKSEL_DIV1_gc;
    TCF0.CTRLB = 0x00;
    TCF0.INTCTRLA = 0x02;
    TCF0.PER = (uint16_t) (FCPU/1000000);

	/*
     *  Initialize LCD to 4 bit I/O mode
     */
     
    if ( ( &LCD_DATA0_PORT == &LCD_DATA1_PORT) && ( &LCD_DATA1_PORT == &LCD_DATA2_PORT ) && ( &LCD_DATA2_PORT == &LCD_DATA3_PORT )
      && ( &LCD_RS_PORT == &LCD_DATA0_PORT) && ( &LCD_RW_PORT == &LCD_DATA0_PORT) && (&LCD_E_PORT == &LCD_DATA0_PORT)
      && (LCD_DATA0_PIN == 0 ) && (LCD_DATA1_PIN == 1) && (LCD_DATA2_PIN == 2) && (LCD_DATA3_PIN == 3) 
      && (LCD_RS_PIN == 4 ) && (LCD_RW_PIN == 5) && (LCD_E_PIN == 6 ) )
    {
        /* configure all port bits as output (all LCD lines on same port) */
        //DDR(LCD_DATA0_PORT) |= 0x7F;
		LCD_DATA0_PORT.DIRSET = 0x7F;
    }
    else if ( ( &LCD_DATA0_PORT == &LCD_DATA1_PORT) && ( &LCD_DATA1_PORT == &LCD_DATA2_PORT ) && ( &LCD_DATA2_PORT == &LCD_DATA3_PORT )
           && (LCD_DATA0_PIN == 0 ) && (LCD_DATA1_PIN == 1) && (LCD_DATA2_PIN == 2) && (LCD_DATA3_PIN == 3) )
    {
        /* configure all port bits as output (all LCD data lines on same port, but control lines on different ports) */
        /*DDR(LCD_DATA0_PORT) |= 0x0F;
        DDR(LCD_RS_PORT)    |= (1<<LCD_RS_PIN);
        DDR(LCD_RW_PORT)    |= (1<<LCD_RW_PIN);
        DDR(LCD_E_PORT)     |= (1<<LCD_E_PIN);*/
		LCD_DATA0_PORT.DIRSET = 0x0F;
		LCD_RS_PORT.DIRSET = (1<<LCD_RS_PIN);
		LCD_RW_PORT.DIRSET = (1<<LCD_RW_PIN);
		LCD_E_PORT.DIRSET = (1<<LCD_E_PIN);
    }
    else
    {
        /* configure all port bits as output (LCD data and control lines on different ports */
        /*DDR(LCD_RS_PORT)    |= (1<<LCD_RS_PIN);
        DDR(LCD_RW_PORT)    |= (1<<LCD_RW_PIN);
        DDR(LCD_E_PORT)     |= (1<<LCD_E_PIN);
        DDR(LCD_DATA0_PORT) |= (1<<LCD_DATA0_PIN);
        DDR(LCD_DATA1_PORT) |= (1<<LCD_DATA1_PIN);
        DDR(LCD_DATA2_PORT) |= (1<<LCD_DATA2_PIN);
        DDR(LCD_DATA3_PORT) |= (1<<LCD_DATA3_PIN);*/

		LCD_DATA0_PORT.DIRSET = (1<<LCD_DATA0_PIN);
		LCD_DATA1_PORT.DIRSET = (1<<LCD_DATA1_PIN);
		LCD_DATA2_PORT.DIRSET = (1<<LCD_DATA2_PIN);
		LCD_DATA3_PORT.DIRSET = (1<<LCD_DATA3_PIN);
		LCD_RS_PORT.DIRSET = (1<<LCD_RS_PIN);
		LCD_RW_PORT.DIRSET = (1<<LCD_RW_PIN);
		LCD_E_PORT.DIRSET = (1<<LCD_E_PIN);

    }
    _delay_ms(60);        /* wait a minimum of 60ms or more after power-on       */

    /* configure for 4bit mode */
    LCD_DATA1_PORT.OUTSET = (1<<LCD_DATA1_PIN);
    lcd_e_toggle();          /* send this command 2 times */
    lcd_e_toggle();
    LCD_DATA1_PORT.OUTCLR = (1<<LCD_DATA1_PIN);
    /* set number of lines */
#if LCD_LINES==2
    LCD_DATA3_PORT.OUTSET = (1<<LCD_DATA3_PIN);
    lcd_e_toggle();
    LCD_DATA3_PORT.OUTCLR = (1<<LCD_DATA3_PIN);
#endif
    lcd_e_toggle();
    _delay_us(60);
    
    /* from now the LCD only accepts 4 bit I/O, we can use lcd_command() */    

    lcd_command(LCD_DISP_OFF);              /* display off                  */
    _delay_us(60);
    lcd_clrscr();                           /* display clear                */
    _delay_us(2000);
    lcd_command(LCD_MODE_DEFAULT);          /* set entry mode               */
    lcd_command(dispAttr);                  /* display/cursor control       */
}/* lcd_init */

