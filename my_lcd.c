/*
 * Program: my_lcd.c
 * Author:  D. O. Corlett
 *
 * Initialisation, and display functions
 * for dot matrix LCD.
 */

#include <stdio.h>
#include <util/delay.h>
#include <avr/io.h>
#include <util/delay_basic.h>
#include "my_lcd.h"

//define LCD control signal macros
#define LCD_RS 0x20
#define LCD_RW 0x10
#define LCD_EN 0x08


static unsigned char ctrl_port_mode_b = 0;		// ctrl_port_mode_b = 0: use port C for LCD control bits, ctrl_port_mode_b > 0: use port B for LCD control bits.
static unsigned char bitmode = 4;				// bitmode = 4: data is sent to LCD using 4 bits, bitmode = 8: data is sent to LCD using 8 bits.

static unsigned int g_delay = 100;				// Delay time for processing general instructions in us.
static unsigned int c_delay = 2000;				// Delay time for processing the clear instruction in us.

/*
 *	Sets the delay time for the LCD to process instructions to enable comparability between various LCD modules.
 *	general_delay sets the delay time for processing general instructions in us.
 *	clear_delay delay time for processing the clear instruction in us.
 */
void my_lcd_process_delayTime(unsigned int general_delay, unsigned int clear_delay)
{
	g_delay = general_delay;
	c_delay = clear_delay;
}

/*
 * 	Sets the given control port value to 1
 * 	for either port b or port c depending on the
 * 	setting in the initalisation function.
 */
static void ctrl_set(unsigned char ctrl)
{
	if ( ctrl_port_mode_b )
	{
		PORTB |= ctrl;
	}
	else
	{
		PORTC |= ctrl;
	}
}

/*
 * 	Sets the given control port value to 0
 * 	for either port b or port c depending on the
 * 	setting in the initalisation function.
 */
static void ctrl_clear(unsigned char ctrl)
{
	if ( ctrl_port_mode_b )
	{
		PORTB &= ~ctrl;
	}
	else
	{
		PORTC &= ~ctrl;
	}
}


/*
 * 	Delays from 0 us to 65535 us
 */
static inline void delay_us(unsigned int microsecs)
{
	_delay_loop_2(4*microsecs);
}

/* Pulses the enable line for 1 us to send data to and from the LCD. */
static inline void pulse_EN(void)
{
	ctrl_set(LCD_EN);		// Enable instruction read
	delay_us(1);
	ctrl_clear(LCD_EN);		// Disable instruction read.
}

/*
 *	Wait for LCD to process instruction given a duration time for this delay.
 */
static void LCD_process(unsigned int delayTime)
{
	delay_us(delayTime);		// Wait for LCD to process instruction.
}


/*
 * 	Sends instruction to LCD in either 8 bit mode or 4 bit mode.
 */
static void LCD_writeInstruction(int instruction)
{
	/* Set up control lines to write to the instruction register: */
	ctrl_clear(LCD_RS);		// RS=0 instruction register.
	ctrl_clear(LCD_RW);		// RW=0 write.

	// 8 bit mode:
	if (bitmode == 8)
	{
		// Send instruction to LCD:
		PORTD = instruction;	// Load the instruction to portD.
		pulse_EN();				// Pulse the enable pin to send the instruction to the LCD.
		LCD_process(g_delay);	// Wait for LCD to process instruction.
	}

	// 4 bit mode:
	if (bitmode == 4)
	{
		// Send instruction to LCD:
		// Send the 4 MSB of the instruction first:
		PORTD = 0xF0 & instruction;			// Load 4 MSBs of the instruction to portD.
		pulse_EN();							// Pulse the enable pin to send the instruction to the LCD.
		delay_us(1);						// Wait for LCD to process instruction.

		// Send the 4 LSB of the instruction second:
		PORTD = 0xF0 & (instruction<<4);	// Load 4 LSBs of the instruction to portD.
		pulse_EN();							// Pulse the enable pin to send the instruction to the LCD.
		LCD_process(g_delay);	// Wait for LCD to process instruction.
	}
}

/*
 * 	Write a character to DDRAM in either 8 bit mode or 4 bit mode.
 */
static void LCD_writeToDDRAM(int data, int address)
{
	// Set DD RAM address
	LCD_writeInstruction(0x80 | address);

	/* Set up control lines to write to the data register: */
	ctrl_set(LCD_RS);			// RS=1 data register.
	ctrl_clear(LCD_RW);			// RW=0 write.


	// 8 bit mode:
	if (bitmode == 8)
	{
		// Send data to LCD:
		PORTD = data;				// Load the data to portD.
		pulse_EN();					// Pulse the enable pin to send the instruction to the LCD.
		LCD_process(g_delay);	// Wait for LCD to process instruction.
	}
	// 4 bit mode:
	if (bitmode == 4)
	{
		// Send data to LCD:
		// Send the 4 MSB of the data first:
		PORTD = 0xF0 & data;		// Load 4 MSBs of the data to portD.
		pulse_EN();					// Pulse the enable pin to send the instruction to the LCD.
		delay_us(1);				// Wait for LCD to process instruction.

		// Send the 4 LSB of the data second:
		PORTD = 0xF0 & (data<<4);	// Load 4 LSBs of the data to portD.
		pulse_EN();					// Pulse the enable pin to send the instruction to the LCD.
		LCD_process(g_delay);	// Wait for LCD to process instruction.
	}
}

/*
 *	Clears the LCD.
 */
void my_lcd_clear()
{
	int instruction = 0x01;					// Define the clear instruction

	/* Set up control lines to write to the instruction register: */
	ctrl_clear(LCD_RS);						// RS=0 instruction register.
	ctrl_clear(LCD_RW);						// RW=0 write.

	// 8 bit mode:
	if (bitmode == 8)
	{
		// Send instruction to LCD:
		PORTD = instruction;				// Load the instruction to portD.
		pulse_EN();							// Pulse the enable pin to send the instruction to the LCD.
		LCD_process(c_delay);				// Wait for LCD to process instruction.
	}
	// 4 bit mode:
	if (bitmode == 4)
	{
		// Send instruction to LCD:
		// Send the 4 MSB of the instruction first:
		PORTD = 0xF0 & instruction;			// Load 4 MSBs of the instruction to portD.
		pulse_EN();							// Pulse the enable pin to send the instruction to the LCD.
		delay_us(1);						// Wait for LCD to process instruction.

		// Send the 4 LSB of the instruction second:
		PORTD = 0xF0 & (instruction<<4);	// Load 4 LSBs of the instruction to portD.
		pulse_EN();							// Pulse the enable pin to send the instruction to the LCD.
		LCD_process(c_delay);				// Wait for LCD to process instruction.
	}
}

/*
 * 	Initialise the LCD for Control on portC or portB and Data on portD.
 *
 * 	Set argument  bit to 8-bit mode or 4 for 4-bit mode.
 * 	Set argument ctrl_port_mode_b to 0 for control lines on port c, or >= 1 for port b.
 */
void my_lcd_init(unsigned char bit, unsigned char ctrl_port_b)
{
	ctrl_port_mode_b = ctrl_port_b;

	/* selects control ports on port B or C depending on the argument control port setting. */
	if (ctrl_port_mode_b)
	{
		DDRB = 0x38;					// Set pins 3, 4, 5 on port b to outputs for  control pins: EN, RW, and RS respectively.
	}
	else
	{
		DDRC = 0x38;					// Set pins 3, 4, 5 on port c to outputs for  control pins: EN, RW, and RS respectively.
	}


	/*
	 * 	Set the four upper pits of port D to output
	 *	for the 4 bit LCD parallel data lines
	 *	(LCD startup only uses the 4 bit data lines).
	 */
	DDRD |= 0xF0;

	_delay_ms(45);					// Wait for 45 ms or more after VDD reaches 4.5 V.
	LCD_writeInstruction(0x30);		// Function Set
	delay_us(4100);					// Wait 4.1 ms or more.

	LCD_writeInstruction(0x30);		// Function Set
	delay_us(100);					// Wait 100 us or more.

	LCD_writeInstruction(0x30);		// Function Set
	delay_us(100);					// Wait 100 us or more.

		// if initialise in 8 bit mode:
	if(bit == 8)
	{
		bitmode = 8;					// Set to 8 bit mode

		DDRD |= 0xFF;					// Set all of port D to output for the 8 bit LCD parallel data lines.

		LCD_writeInstruction(0x38);		// Function Set Duty: 1/16, Font: 5x7. ExT=40us
		LCD_writeInstruction(0x08);		// Display OFF	 ExT=40us
		my_lcd_clear();					// Display Clear	ExT=1.64ms
		LCD_writeInstruction(0x06);		// Entry Mode Set I/D: move right, S: no shift.	ExT=40us
		LCD_writeInstruction(0x0F);		// Display ON	 ExT=40us
	}
		// if initialise in 4 bit mode:
	if(bit == 4)
	{
		bitmode = 4;					// Set to 4 bit mode

		ctrl_clear(LCD_EN);
		PORTD = 0x20;					//Function Set change to 4bit interface. ExT=40us
		ctrl_set(LCD_EN);
		delay_us(600);

		LCD_writeInstruction(0x28);		// Function Set Duty: 1/16, Font: 5x7. ExT=40us
		LCD_writeInstruction(0x08);		// Display OFF	 ExT=40us
		my_lcd_clear();					// Display Clear	ExT=1.64ms
		LCD_writeInstruction(0x06);		// Entry Mode Set I/D: move right, S: no shift.	ExT=40us
		LCD_writeInstruction(0x0C);		// Display ON	 ExT=40us
	}
}

/*
 * 	Takes a string to find the first word in it,
 * 	and an array to store the first word. Returns
 * 	the first word found in that string and store it in
 * 	the array.
 */
static int GetWordFromString(char string[], int stringSize, int wordNumber, char word[])
{
	int wordLength = 0;			// Stores the length of the word
	int spaceNumber = 0;		// Stores the number of spaces up to the word of wordNumber
	int wordStartIndex = 0;		// The array index of the start of the word
	// find the array index for the start of the word
	for(int h=0; spaceNumber!=wordNumber && (int)string[h]!=0 && h<stringSize; h++)
	{
		if((int)string[h]==' ')
		{
    		spaceNumber++;
       		wordStartIndex = h+1;
    	}
    }

    // loop through the string adding each character to the
    // word until a space occurs or the end of the string,
    // hence until end of word occurs.
    for(int i = wordStartIndex; string[i]!=' ' && (int)string[i]!=0 && i<stringSize; i++)
    {
       	word[i-wordStartIndex] = string[i];
       	wordLength = i-wordStartIndex+1;
    }

    word[wordLength] = 0;	// end string null character
    return wordLength;
}

/*
 * 	Finds the number of words in a string
 */
static int NumberOfWords(char string[], int stringSize)
{
	int numberOfWords = 0;

	// loop through the whole string.
	for(int i = 0; i<stringSize; i++)
	{
		// count the word before the space character ' '.
		if((int)string[i] == ' ')
		{
			numberOfWords++;
		}
		// increment count last word in string (the word before the end string character 0)
		if((int)string[i] == 0)
		{
			numberOfWords++;
			break;
		}
	}
	return numberOfWords;
}

/*
 * 	Displays the argument string of characters on the LCD.
 */
void my_lcd_display(char string[])
{
	my_lcd_clear();					// Clear the LCD before writing to it.
	int address = 0x00;				// Start DD RAM address at 0x00 (beginning of line 1)

		/* Writes each character of string[] to the LCD.
		 * Moves to second line if new line character
		 * is found or if the end of the first line is
		 * full. */
	for(int i = 0; (i < 32) && (string[i] != 0); i++, address++)
	{
		// If new line character on first line
		if((string[i] == '\n') && (i < 16))
		{
			address = 0x40;						// Line 2 position 0 address
			i++; 								// increment string index to next character because '/n' dosn't need to be displayed.
		}
		// If end of line
		else if((i == 16) && (address < 0x40))
		{
			address = 0x40;						// Line 2 position 0 address
		}
		// If a new line character appears on the second line then finish writing to display.
		else if((string[i] == '\n') && (i >= 16))
		{
			break;
		}
		LCD_writeToDDRAM(string[i], address);	// Write the character to the LCD.
	}
}

/*
 * 	Displays a string of words on the LCD without
 *	splitting words between a line on the display.
 */
void my_lcd_display_AutoWrap(int stringSize, char string[])
{
	int lineLength = 16;
	int lineNum = 0;
	int linePos = 0;
	char word[64];
	char displayString[lineLength*2];
	int wordLength = 0;
	int displayIndex = 0;
	int wordNum = 0;
	int numWords = 0;

	// Finds the number of words in the string.
	numWords = NumberOfWords(string, stringSize);
	// Increments wordNum to add each word to the displayString
	for(wordNum = 0; wordNum < numWords; wordNum++)
	{
		// finds the length of wordNum and also loads the word into the word array
		wordLength = GetWordFromString(string, stringSize, wordNum, word);

		// Checks if word will fit in current line if not it prints the new line character
		if((linePos + wordLength) > lineLength)
		{
			displayString[displayIndex] = '\n';		// load new line character 10 character to displayString
			displayIndex++;							// increment displayIndex
			lineNum++;								// increment line number because new line
			linePos = 0;							// reset line position to 0 because new line
		}

		// loads word wordNum into the display String.
		for(int i = 0; i < wordLength; i++)
		{
			displayString[displayIndex] = word[i];	// load word into displayString
			displayIndex++;							// increment displayIndex
			linePos++;								// increment line position
		}
		// Puts a space after the word that has just been loaded into displayString, unless it is the last word.
		if(wordNum < (numWords - 1))
		{
			displayString[displayIndex] = ' ';		// load space ' ' character to displayString
			displayIndex++;							// increment displayIndex
			linePos++;								// increment line position
		}
	}

	// Displays the string on the LCD
	displayString[displayIndex] = 0;					// end of string character (not working, will only display once) only an issue when more than 32 characters are in string
	snprintf(displayString, 32, "%s", displayString);	// format displayString as string (include end of string character at end) fixes problem above
	my_lcd_display(displayString);						// display displayString on LCD
}
