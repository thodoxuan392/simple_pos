///*
// * lcd.c
// *
// *  Created on: May 14, 2023
// *      Author: xuanthodo
// */
//
//
#include "main.h"
#include "stdarg.h"
#include "Device/lcd.h"
#include "Hal/gpio.h"

#define LCD_BUFFER_SIZE			256

//basic commands
#define LCD_FUNCTIONSET 		0x30u
#define LCD_DISPLAYCONTROL 		0x08u
#define LCD_DISPLAYCLEAR 		0x01u
#define LCD_RETURNHOME			0x02u
#define LCD_ENTRYMODE	 		0x04u
#define LCD_SETDRAMADDRESS 		0x80u // or-ed with address

//extended commands
#define LCD_FUNCTIONSETEXTENDED 			0x24u
#define LCD_FUNCTIONSCROLLORADDRESSSELECT 	0x02u
#define LCD_DISPLAYCONTROLEXTENDED 			0x0Cu
#define LCD_SETGRAPHICADDRESS 				0x80u // or-ed with address

//FunctionSet features
#define LCD_INTERFACE_8BITS 0x10u
#define LCD_INTERFACE_4BITS 0x00u
#define LCD_GRAPHIC_ON      0x02u // only for extended
#define LCD_GRAPHIC_OFF     0x00u // only for extended

//DisplayControl features
#define LCD_DISPLAY_ON 		0x04u
#define LCD_DISPLAY_OFF		0x00u
#define LCD_CURSOR_ON		0x02u
#define LCD_CURSOR_OFF		0x00u
#define LCD_BLINK_ON		0x01u
#define LCD_BLINK_OFF		0x00u

//EntryMode features
#define LCD_CURSOR_MOVE_RIGHT 0x02u
#define LCD_CURSOR_MOVE_LEFT  0x00u
#define LCD_DISPLAY_SHIFT	  0x01u
#define LCD_DISPLAY_NO_SHIFT  0x00u

enum {
	LCD_RS,
	LCD_RW,
	LCD_ENABLE,
	LCD_D0,
	LCD_D1,
	LCD_D2,
	LCD_D3,
	LCD_D4,
	LCD_D5,
	LCD_D6,
	LCD_D7
};
//
static GPIO_info_t gpio_table[] = {
		[LCD_RS] = 		{GPIOB, { GPIO_PIN_13, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH}},
		[LCD_RW] = 		{GPIOB,{ GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[LCD_ENABLE] = 	{GPIOB,{ GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[LCD_D0] = 		{GPIOD,{ GPIO_PIN_8, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[LCD_D1] = 		{GPIOD,{ GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[LCD_D2] = 		{GPIOD,{ GPIO_PIN_10, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[LCD_D3] = 		{GPIOD,{ GPIO_PIN_11, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[LCD_D4] = 		{GPIOD,{ GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[LCD_D5] = 		{GPIOD,{ GPIO_PIN_13, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[LCD_D6] = 		{GPIOD,{ GPIO_PIN_14, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
		[LCD_D7] = 		{GPIOD,{ GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH }},
};

static const uint8_t offset_line[] = { 0x80u, 0x90u, 0x88u, 0x98u };
static uint8_t display_control;
static uint8_t display_function;
static uint8_t lcd_str_buf[LCD_BUFFER_SIZE];
static uint8_t lcd_bitmap_buf[32][32];

/**
 * Section: Private Functions
 */

static void LCD_begin();
static void LCD_text_mode();
static void LCD_graphic_mode();
static void LCD_home();
static void LCD_clear();
static void LCD_no_display();
static void LCD_display();
static void LCD_draw_pixel(int8_t x, int8_t y);
static void LCD_render_bitmap();
static void LCD_no_blink();
static void LCD_blink();
static void LCD_no_cursor();
static void LCD_cursor();
static void LCD_set_cursor( uint8_t col, uint8_t row );
static void LCD_output_pins( uint8_t rs, uint8_t rw, uint8_t data );
static void LCD_strobe();
static void  LCD_send_command( uint8_t cmd );
static void LCD_send_data( char d );
static bool LCD_delay_microseconds(size_t us);
static bool LCD_delay(size_t ms);


bool LCD_init(){
	// Init GPIO
	int nb_io = sizeof(gpio_table)/sizeof(GPIO_info_t);
	for (uint8_t var = 0; var < nb_io; ++var) {
		HAL_GPIO_Init(gpio_table[var].port, &gpio_table[var].init_info);
	}
	LCD_begin();
	LCD_graphic_mode();
	return true;
}

void LCD_clear_screen(){
	LCD_clear();
}
void LCD_display_str(char * fmt, ...){
	// Build String
	va_list args;
	va_start(args, fmt);
	va_end(args);
	size_t len = vsnprintf(lcd_str_buf, LCD_BUFFER_SIZE , fmt, args);
	// Display
	for (int var = 0; var < len; ++var) {
		LCD_send_data(lcd_str_buf[var]);
	}
}

// Fixed size 32x32
void LCD_draw_bitmap(uint8_t * bitmap_p){
	LCD_clear_bitmap();
	uint16_t x,y;
	for (y = 0; y < 64; y++) {
		for (x = 0; x < 128; x++) {
			if (bitmap_p[(y / 8) * 128 + x] & (1 << (y % 8))) {
				LCD_draw_pixel(x, y);
			}
		}
	}
	LCD_render_bitmap();
}

void LCD_clear_bitmap(){
	memset(lcd_bitmap_buf, 0, sizeof(lcd_bitmap_buf));
}

bool LCD_test(){
//	LCD_clear();
//	LCD_display_str("Hello");
}

// Internal function


static void LCD_begin(){

	display_control = LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF;
	display_function = LCD_INTERFACE_8BITS;

	LCD_send_command(
	LCD_FUNCTIONSETEXTENDED | display_function | LCD_GRAPHIC_OFF);
	LCD_delay(1);
	LCD_send_command( LCD_FUNCTIONSET | display_function);
	LCD_delay(1);

	LCD_send_command( LCD_DISPLAYCLEAR);
	LCD_delay(1);

	LCD_send_command(
	LCD_ENTRYMODE | LCD_CURSOR_MOVE_RIGHT | LCD_DISPLAY_NO_SHIFT);
	LCD_delay(1);

	LCD_send_command( LCD_DISPLAYCONTROL | display_control);

	LCD_delay_microseconds(160);

	LCD_send_command(
	LCD_FUNCTIONSETEXTENDED | display_function | LCD_GRAPHIC_ON);
	LCD_delay(1);
	LCD_send_command( LCD_FUNCTIONSCROLLORADDRESSSELECT);
	LCD_delay(1);
	for (uint8_t i = 0; i < 32; i++) {
		LCD_send_command( LCD_SETGRAPHICADDRESS | i); // y
		LCD_delay(1);
		LCD_send_command( LCD_SETGRAPHICADDRESS | 0); // x
		LCD_delay(1);

		for (uint8_t j = 0; j < 32; j++) {
			LCD_delay(1);
			LCD_send_data(0u);
		}
	}
}



static void LCD_text_mode( ){
  //Function Set
  LCD_send_command( LCD_FUNCTIONSET | LCD_INTERFACE_8BITS );
  //Wait time > 100uS
  LCD_delay_microseconds(150);

  //Function Set
  LCD_send_command( LCD_FUNCTIONSET | LCD_INTERFACE_8BITS );
  //Wait time > 37uS
  LCD_delay_microseconds(60);

  //Display ON/OFF  Control
  LCD_send_command( LCD_DISPLAYCONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF );
  //Wait time > 100uS
  LCD_delay_microseconds(150);

  LCD_send_command( LCD_RETURNHOME );
  //Wait time > 10mS
  LCD_delay(20);

  LCD_send_command( LCD_ENTRYMODE | LCD_CURSOR_MOVE_RIGHT | LCD_DISPLAY_NO_SHIFT );

  LCD_delay(50);
}

static void LCD_graphic_mode() {
	LCD_send_command(
	LCD_FUNCTIONSETEXTENDED | LCD_INTERFACE_8BITS | LCD_GRAPHIC_OFF); // functionSetExt with G = 0
	LCD_delay_microseconds(80);
	LCD_send_command(
	LCD_FUNCTIONSETEXTENDED | LCD_INTERFACE_8BITS | LCD_GRAPHIC_ON); // functionSetExt with G = 1
}


static void LCD_clear(){
	LCD_send_command( LCD_DISPLAYCLEAR );
}

static void LCD_home(){
	LCD_send_command( LCD_RETURNHOME );
}



static void LCD_no_display(){
	display_control &= ~(LCD_DISPLAY_ON);
	LCD_send_command( LCD_DISPLAYCONTROL | display_control );
}

static void LCD_display(){
	display_control |= (LCD_DISPLAY_ON);
	LCD_send_command( LCD_DISPLAYCONTROL | display_control );
}

static void LCD_draw_pixel(int8_t x, int8_t y) {
	if (y > 63 || y < 0) {
		return;
	}
	if (x / 8 + 16 > 32 || x < 0) {
		return;
	}
	if (y < 32)
		lcd_bitmap_buf[x / 8][y] |= 1u << (8 - x % 8 - 1);
	else
		lcd_bitmap_buf[x / 8 + 16][y - 32] |= 1u << (8 - x % 8 - 1);

}

static void LCD_render_bitmap(){
	uint8_t startX = 0;
	uint8_t cantX = 32;
	for (uint8_t i = 0; i < 32; i++) {  // iterate for 'y' axis
		LCD_send_command( LCD_SETGRAPHICADDRESS | i); // y
		LCD_delay_microseconds(80);
		LCD_send_command( LCD_SETGRAPHICADDRESS | startX); // x
		for (uint8_t j = 2 * startX; j < cantX; j++) {
			LCD_delay_microseconds(80);
			LCD_send_data(lcd_bitmap_buf[j][i]);
		}
		LCD_delay_microseconds(40);
	}
}

static void LCD_no_blink(){
	display_control &= ~(LCD_BLINK_ON);
	LCD_send_command( LCD_DISPLAYCONTROL | display_control );
}

static void LCD_blink(){
	display_control |= (LCD_BLINK_ON);
	LCD_send_command( LCD_DISPLAYCONTROL | display_control );
}

static void LCD_no_cursor(){
	display_control &= ~(LCD_CURSOR_ON);
	LCD_send_command( LCD_DISPLAYCONTROL | display_control );
}

static void LCD_cursor(){
	display_control |= (LCD_CURSOR_ON);
	LCD_send_command( LCD_DISPLAYCONTROL | display_control );
}

static void LCD_set_cursor( uint8_t col, uint8_t row ){
	if( row >= 4u)
		row = 3u;

	if( col >= 8u )
		col = 7u;
	LCD_send_command( LCD_SETDRAMADDRESS | (offset_line[row] + col ) );
}

static void LCD_output_pins( uint8_t rs, uint8_t rw, uint8_t data ){
	HAL_GPIO_WritePin(gpio_table[LCD_RS].port, gpio_table[LCD_RS].init_info.Pin, rs);
	HAL_GPIO_WritePin(gpio_table[LCD_RW].port, gpio_table[LCD_RW].init_info.Pin, rw);
	HAL_GPIO_WritePin(gpio_table[LCD_D7].port, gpio_table[LCD_D7].init_info.Pin, (data >> 7) & 0x01);
	HAL_GPIO_WritePin(gpio_table[LCD_D6].port, gpio_table[LCD_D6].init_info.Pin, (data >> 6) & 0x01);
	HAL_GPIO_WritePin(gpio_table[LCD_D5].port, gpio_table[LCD_D5].init_info.Pin, (data >> 5) & 0x01);
	HAL_GPIO_WritePin(gpio_table[LCD_D4].port, gpio_table[LCD_D4].init_info.Pin, (data >> 4) & 0x01);
	HAL_GPIO_WritePin(gpio_table[LCD_D3].port, gpio_table[LCD_D3].init_info.Pin, (data >> 3) & 0x01);
	HAL_GPIO_WritePin(gpio_table[LCD_D2].port, gpio_table[LCD_D2].init_info.Pin, (data >> 2) & 0x01);
	HAL_GPIO_WritePin(gpio_table[LCD_D1].port, gpio_table[LCD_D1].init_info.Pin, (data >> 1) & 0x01);
	HAL_GPIO_WritePin(gpio_table[LCD_D0].port, gpio_table[LCD_D0].init_info.Pin, (data) & 0x01);
}

static void LCD_strobe(){
	HAL_GPIO_WritePin(gpio_table[LCD_ENABLE].port, gpio_table[LCD_ENABLE].init_info.Pin, SET);
	LCD_delay_microseconds(1);
	HAL_GPIO_WritePin(gpio_table[LCD_ENABLE].port, gpio_table[LCD_ENABLE].init_info.Pin, RESET);
}

static void  LCD_send_command( uint8_t cmd ){
	LCD_output_pins( 0u, 0u, cmd );
	LCD_strobe();
}

static void LCD_send_data( char d ){
	LCD_output_pins( 1u, 0u, d );
	LCD_strobe();
}


static bool LCD_delay_microseconds(size_t us){
	size_t _us = us * 10;
	while(_us--);
}

static bool LCD_delay(size_t ms){
	HAL_Delay(ms);
}
