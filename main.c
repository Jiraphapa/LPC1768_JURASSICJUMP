#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include "lpc17xx_gpio.h"
#include "lpc17xx_systick.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_adc.h"
#include "glcd_touch.h"
#include "button.h"
#include "led.h"
#include "lpc17xx_pwm.h"
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define BT_SENTIVITY 	50
#define BT_LONG_SENTIVITY 2000
#define BT_CLICK_TIME	250

unsigned long timeval;

#define BUTTON_LEFT		1
#define BUTTON_RIGHT	2
#define BUTTON_CENTER   3
#define JL_BT		    29
#define JR_BT		    28
#define JC_BT           25

#define LED_PORT 	2
#define LED_1		1
#define LED_2		2
#define LED_3		4
#define LED_4		8
#define LED_5		16
#define LED_6		32
#define LED_7		64
#define LED_8		128

#define P_ADC_CH		ADC_CHANNEL_5

LED_OBJ Led[8];
////////////////////////////////////////////////
////////////////////////////////////////////////    STRUCT

typedef enum {
	FALLING, JUMPING, RUNNING
} Statuss;

typedef enum {
	UART, NORMAL, UNDEFINE
} Gamemode;

typedef enum {
	LEFT, RIGHT
} boxStatus;

typedef struct {
	int score;
	int xPos;
	int yPos;
	int height;
	Statuss sta;
} PLAYER;
typedef struct {
	int xPos;
	int yPos;
} OBSTACLE;

typedef struct {
	int xPos;
	int yPos;
	boxStatus sta;
} BOX;

typedef struct {
	uint16_t IsAdcEnd;
	uint16_t Value;
} AdcState_t;

/*
 typedef struct {
 int xSize = 20;
 int ySize = 5;
 int xPosBegin;
 int yPosBegin;
 int yOnTop;
 } OBSTACLE;
 */
struct {
	uint8_t Tick10ms;
	uint8_t Tick100ms;
	uint8_t Tick1sec;
} TickTime;

struct {
	uint8_t ms1Tick;
	uint8_t ms10Tick;
	uint8_t ms100Tick;
	uint8_t secTick;
} ObjectTick;

typedef struct {
	uint8_t *pBuff;
	uint8_t Head;
	uint8_t Tail;
	uint8_t Size;
	uint8_t Reserve;
} SFifo_t;
////////////////////////////////////////////////
///////////////////////////////////////////////     DECLARATION
const char dino3[] = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xF79E, 0x4A49, 0x4228, 0x4228, 0x4228, 0x4228,
		0x4228, 0x4228, 0x4228, 0x4228, 0x4228, 0x4228, 0x4228, 0x4228, 0x4228,
		0x4228, 0x94B2, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x632C, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF7DE, 0x73AE, 0x632C, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x3166, 0x6B6D, 0x94B2, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xEF9D, 0x0000, 0x0000, 0x0000, 0x2104, 0xA534, 0x94B2, 0x0821,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x2925, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF79E, 0x0000,
		0x0000, 0x0000, 0x31A6, 0xFFFF, 0xFFFF, 0x0020, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x4228, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF79E, 0x0000, 0x0000, 0x0000, 0x18C3,
		0x6B6D, 0x632C, 0x0821, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4228, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xF79E, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x4228, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF79E, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x4228, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF79E, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4228, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xF79E, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x4208, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF79E, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x39A7, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF79E, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x3186, 0xC638, 0xC638, 0xC638,
		0xC638, 0xC638, 0xC618, 0xBE17, 0xBE17, 0xBE17, 0xCE99, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xF79E, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x4A49, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF7DE, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0821,
		0x3166, 0x3166, 0x3166, 0x3166, 0x3166, 0x7BEF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFDF, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x4A49, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0x630C, 0x2945, 0xA534, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0x39E7, 0x2945, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x2965, 0xD6BA, 0xD6BA, 0xD6BA, 0xD6BA, 0xD6BA, 0xD6BA, 0xD6BA, 0xE73C,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x3166, 0x0000, 0x8C51,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x39C7, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0x4208, 0x0000, 0x9CD3, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0x4208, 0x2104, 0x20E4, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x39C7, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0x4208, 0x0000, 0x9492, 0xFFDF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFDF, 0xFFDF, 0x1082, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x31A6, 0xFFDF, 0xFFDF, 0xFFDF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x4228, 0x0000, 0x0000,
		0x18A3, 0xB596, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0x4A49, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x5ACB, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0x4228, 0x0000, 0x0000, 0x0000, 0x9492, 0xF7BE,
		0xFFDF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF7BE, 0xF7BE, 0x2104, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x39E7, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0x4228, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xBDF7, 0xFFFF, 0xFFFF,
		0xFFFF, 0x528A, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x3186, 0xEF9D, 0xB5B6, 0x0000, 0x39C7, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x4228, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x9CD3, 0xEF7D, 0xEF7D, 0xEF7D, 0x3166, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x39C7, 0xFFFF, 0xD6BA,
		0x0000, 0x52AA, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0x4228, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x31A6, 0xFFFF, 0xFFFF, 0xF7DE, 0xFFDF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0x4228, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x31A6, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x3186, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x31A6, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0x630C, 0x2124, 0x1062, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x2104, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0x5AEB, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0821, 0x2945,
		0x52AA, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x8C51,
		0x3166, 0x1082, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x2945, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x528A, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x1082, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x8430, 0x39C7, 0x1082, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0821, 0x39C7, 0x5ACB, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0x4228, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0x8430, 0x4228, 0x10A2, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0821, 0x4208, 0x5ACB,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x39C7,
		0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x18A3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x8410, 0x4A69, 0x10A2, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000, 0x4A49, 0x4A69, 0x0821, 0x0000, 0x0000,
		0x10A2, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFDF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x4A29, 0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFDF, 0xFFFF,
		0xFFFF, 0xFFDF, 0xFFDF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0x4208, 0x0000, 0x0000, 0x0000, 0x4A49, 0x5ACB, 0xF7BE, 0xFFFF,
		0x632C, 0x52AA, 0x0000, 0x0000, 0xFFDF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x4208, 0x0000,
		0x0000, 0x0000, 0xEF5D, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000,
		0x0000, 0xFFDF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x41E8, 0x0000, 0x4A69, 0x630C, 0xE73C,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF79E, 0x0000, 0x0020, 0xFFDF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0x39C7, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xF79E, 0x0000, 0x0020, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x39E7, 0x0000,
		0x73AE, 0x9492, 0xEF7D, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xF79E, 0x0000,
		0x0000, 0x8C71, 0x9CF3, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x2104, 0x0000, 0x0000, 0x0000, 0xC638,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xEF9D, 0x0000, 0x0000, 0x0000, 0x0000,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0x73AE, 0x4228, 0x4208, 0x4208, 0xDF1B, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xF7BE, 0x4A69, 0x4228, 0x4208, 0x5ACB, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
		0xFFFF, 0xFFFF

};

PLAYER player;
PLAYER* playerPointer;
BUTTON_OBJ ButtonLeft;
BUTTON_OBJ ButtonRight;
BUTTON_OBJ ButtonCenter;
AdcState_t pAdc;

int StartObstaclePosX = 25 + 220;
int StartObstaclePosY = 50;
OBSTACLE ob1;
OBSTACLE ob2;
BOX box2;
BOX box1;
BOX heightLine;
OBSTACLE obstacleArr[2];
uint8_t rxBuff[16];
SFifo_t rxFifo;
Gamemode MODE;

int GameOver;
int NewGame;
int Intro;
int HighestScore = 0;
int setMode;
int UARTMode;
///////////////////////////////////////////////
///////////////////////////////////////////////     FUNCTION

void SysTick_Handler(void);
void delay_ms(unsigned long ms);
void ButtonInit(BUTTON_OBJ *obj);
void ButtonCheckStatus(BUTTON_OBJ *obj);
void ButtonCallback(void* obj, BT_EVT_t sta);
void jump();
int life = 8;

void SendData(LPC_UART_TypeDef* port, uint8_t *data, uint8_t len) {
	uint8_t current;
	for (current = 0; current < len; current++) {
		while (!(port->LSR & UART_LSR_THRE))
			;
		port->THR = data[current];
	}
}

void SendString(LPC_UART_TypeDef * port, uint8_t *string) {
	uint8_t currentPos;
	currentPos = 0;
	while (string[currentPos]) {
		while (!(port->LSR & UART_LSR_THRE))
			;
		port->THR = string[currentPos];
		currentPos++;
	}
}

uint8_t SimpleFifoWrite(SFifo_t* fifo, uint8_t data) {
	//uint8_t result;
	uint8_t mask;
	uint8_t head;
	uint8_t tail;
	mask = fifo->Size - 1;
	head = fifo->Head;
	tail = fifo->Tail;
	//check fifo is avaliable
	if (((head + 1) & mask) == (tail & mask)) {
		//fifo is full
		return -1;
	}
	fifo->pBuff[head] = data;
	head = (head + 1) & mask;
	fifo->Head = head;
	return 0;
}

uint8_t SimpleFifoRead(SFifo_t* fifo, uint8_t *out) {
	uint8_t mask;
	uint8_t head;
	uint8_t tail;
	//uint8_t data;

	mask = fifo->Size - 1;
	head = fifo->Head;
	tail = fifo->Tail;
	//check fifo is avaliable
	if (tail == head) {
		//fifo is full
		return -1;
	}

	*out = fifo->pBuff[tail];
	fifo->Tail = (tail + 1) & mask;
	return 0;
}

uint8_t SimpleFifoCount(SFifo_t* fifo) {
	uint8_t count;
	uint8_t size;
	uint8_t head;
	uint8_t tail;

	head = fifo->Head;
	tail = fifo->Tail;
	size = fifo->Size;

	count = head + ((head < tail) ? size : 0) - tail;

	return count;
}

void UART0_IRQHandler(void) {
	uint8_t iirVal;

	iirVal = LPC_UART0->IIR;
	switch (iirVal & 0x0E) {
	case UART_IIR_INTID_RDA:
		SimpleFifoWrite(&rxFifo, LPC_UART0->RBR);
		break;
	}
}

void UART2_IRQHandler(void) {
	uint8_t iirVal;

	iirVal = LPC_UART2->IIR;
	switch (iirVal & 0x0E) {
	case UART_IIR_INTID_RDA:
		SimpleFifoWrite(&rxFifo, LPC_UART2->RBR);
		break;
	}
}

void UartConfig(void) {
	UART_CFG_Type uartCfg;
	LPC_UART_TypeDef *pUart;
	PINSEL_CFG_Type pinCfg;

	pUart = LPC_UART2;

	pinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	pinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
	pinCfg.Portnum = 0;
	pinCfg.Funcnum = 1;
	pinCfg.Pinnum = 10;

	PINSEL_ConfigPin(&pinCfg);
	pinCfg.Pinnum = 11;
	PINSEL_ConfigPin(&pinCfg);

	CLKPWR_SetPCLKDiv(CLKPWR_PCLKSEL_UART2, CLKPWR_PCLKSEL_CCLK_DIV_2);

	uartCfg.Baud_rate = 38400;
	uartCfg.Databits = UART_DATABIT_8;
	uartCfg.Parity = UART_PARITY_NONE;
	uartCfg.Stopbits = UART_STOPBIT_1;

	UART_Init(pUart, &uartCfg);
	UART_TxCmd(pUart, ENABLE);

	UART_IntConfig(pUart, UART_INTCFG_RBR, ENABLE);
	NVIC_EnableIRQ(UART2_IRQn);
}

void SerailComProcess(void) {
	uint8_t data;
	SimpleFifoRead(&rxFifo, &data);
	UART_SendByte(LPC_UART2, data);
}

void LedInitBSP(void) {
	Led[0].Port = 2;
	Led[0].Pin = LED_1;
	Led[1].Port = 2;
	Led[1].Pin = LED_2;
	Led[2].Port = 2;
	Led[2].Pin = LED_3;
	Led[3].Port = 2;
	Led[3].Pin = LED_4;
	Led[4].Port = 2;
	Led[4].Pin = LED_5;
	Led[5].Port = 2;
	Led[5].Pin = LED_6;
	Led[6].Port = 2;
	Led[6].Pin = LED_7;
	Led[7].Port = 2;
	Led[7].Pin = LED_8;

	LEDInit(&Led[0]);
	LEDInit(&Led[1]);
	LEDInit(&Led[2]);
	LEDInit(&Led[3]);
	LEDInit(&Led[4]);
	LEDInit(&Led[5]);
	LEDInit(&Led[6]);
	LEDInit(&Led[7]);
	int k = 0;
	for (k = 0; k < life; k++) {
		LEDOn(&Led[k]);
	}

}
void LifeDecreaser(void) {
	int k;
	for (k = 0; k < 8; k++) {
		if (k + 1 > life) {
			LEDOff(&Led[k]);
		}
	}
}
void delay_ms(unsigned long ms) {
	timeval = 0;
	while (timeval != ms) {
		;
	}
}

void ButtonCheckStatus(BUTTON_OBJ *obj) {

	BUTTON_OBJ *bt;
	LPC_GPIO_TypeDef* gpio;
	uint32_t pinState;
	BT_EVT_t btEvt;

	bt = obj;
	// Point to GPIO Port
	gpio = (LPC_GPIO_TypeDef*) bt->Port;
	// Read Value form input port
	pinState = gpio->FIOPIN;
	// Getting positions of input pin;
	pinState = (pinState >> obj->Pin) & 1;
	switch (bt->CurrentState) {
	case BT_STATE_IDLE:
	default:
		// Check if button is press
		if (pinState == RESET) {
			if (MODE == UNDEFINE) {
				if (Intro) {
					Intro = 0;
				} else if (setMode) {
					if (bt->Instance == BUTTON_CENTER) {
						if (box2.sta == LEFT)
							MODE = NORMAL;
						else if (box2.sta == RIGHT)
							MODE = UART;
						setMode = 0;
					} else if (bt->Instance == BUTTON_RIGHT) {
						box2.sta = RIGHT;
					} else if (bt->Instance == BUTTON_LEFT) {
						box2.sta = LEFT;
					}
				}

			} else if (MODE == NORMAL) {
				if (bt->Instance == BUTTON_CENTER) {
					if (NewGame) {
						NewGame = 0;
					} else if (playerPointer->sta == RUNNING)
						bt->Callback(bt, BT_EVT_CLICK);
				} else if (bt->Instance == BUTTON_LEFT) {
					box1.sta = LEFT;
				} else if (bt->Instance == BUTTON_RIGHT) {
					box1.sta = RIGHT;
				}
			} else if (MODE == UART) {
				if (bt->Instance == BUTTON_CENTER) {
					SendString(LPC_UART2, "1");
				}
				if (bt->Instance == BUTTON_RIGHT) {
					UARTMode = 0;
				}
			}
			if (++bt->DecisionTime >= BT_SENTIVITY) {
				// State changed
//				bt->CurrentState = BT_STATE_PUSH;
//				bt->DecisionTime = 0;
//
//				if (bt->Callback != 0) {
//					bt->Callback(obj, BT_EVT_CLICK);
//				}
			}
		} else {
			bt->DecisionTime = 0;
		}
		break;
	case BT_STATE_PUSH:
		// Check if button is release
		if (pinState == SET) {
			if (++bt->DecisionTime >= BT_SENTIVITY) {
				// State Changed
				bt->CurrentState = BT_STATE_RELEASE;
				bt->DecisionTime = 0;
				bt->LongCountTime = 0;
			}
		} else {
			if (++bt->LongCountTime >= BT_LONG_SENTIVITY) {
				bt->LongCountTime = 0;
				bt->CurrentState = BT_STATE_LONG_PRESS;

				if (bt->Callback != 0) {
					bt->Callback(obj, BT_EVT_LONG_CLICK);
				}
			}
			bt->DecisionTime = 0;
		}
		break;

	case BT_STATE_RELEASE:
		//sama
		Circle(playerPointer->xPos, playerPointer->yPos, 20, 2, WHITE, 0);
		bt->clicks++;
		bt->CurrentState = BT_STATE_IDLE;
		break;
	case BT_STATE_LONG_PRESS:
		// Check if button is release
		if (pinState == SET) {
			if (++bt->DecisionTime >= BT_SENTIVITY) {
				// State Changed
				bt->CurrentState = BT_STATE_LONG_PRESS_RELESE;
				bt->DecisionTime = 0;
			}
		} else {
			bt->DecisionTime = 0;
		}
		break;
	case BT_STATE_LONG_PRESS_RELESE:
		Circle(playerPointer->xPos, playerPointer->yPos, 20, 2, BLUE, 0);
		if (bt->Callback != 0) {
			bt->Callback(obj, BT_EVT_LONG_RELEASE);
		}
		bt->CurrentState = BT_STATE_IDLE;
	}

	if (bt->clicks) {
		if (++bt->clickCount >= BT_CLICK_TIME) {
			if (bt->clicks == 1) {
				// Callback event BT_EVT_CLICK
				btEvt = BT_EVT_CLICK;
			} else {
				// Callback event BT_EVT_DB_CLICK
				btEvt = BT_EVT_DB_CLICK;
			}
			if (bt->Callback != 0) {
				bt->Callback(bt, btEvt);
			}
			bt->clicks = 0;
			bt->clickCount = 0;
		}
	}
}

void ButtonCallback(void* obj, BT_EVT_t sta) {
	BUTTON_OBJ* bt;
	// Point to Button object
	bt = (BUTTON_OBJ*) obj;
	switch (sta) {
	case BT_EVT_CLICK:
		// if (bt->Instance == BUTTON_LEFT) {
		// 	Circle(playerPointer->xPos, playerPointer->yPos, 20, 2, BLACK, 0);
		// 	playerPointer->xPos -= 30;
		// 	Circle(playerPointer->xPos, playerPointer->yPos, 20, 2, YELLOW, 0);
		// 	//GLCD_clear(GREEN);
		// 	//lcd_printStr_hor("Screen Change to Green",50,180,WHITE,GREEN);
		// } else if (bt->Instance == BUTTON_RIGHT) {
		// 	Circle(playerPointer->xPos, playerPointer->yPos, 20, 2, BLACK, 0);
		// 	playerPointer->xPos += 30;
		// 	Circle(playerPointer->xPos, playerPointer->yPos, 20, 2, YELLOW, 0);
		// 	//GLCD_clear(WHITE);
		// 	//lcd_printStr_hor("Welcome to my world",50,180,BRIGHT_BLUE,WHITE);
		// 	//lcd_printStr_hor("What you want to built",50,160,BRIGHT_BLUE,WHITE);
		// 	//lcd_printStr_hor("Try joy stick for demo",50,140,BRIGHT_BLUE,WHITE);
		// }

		if (bt->Instance == BUTTON_CENTER) {
			playerPointer->sta = JUMPING;
		}
		break;
	default:
		break;
	}
}

void ButtonInit(BUTTON_OBJ *obj) {
	LPC_GPIO_TypeDef* gpio;
	uint32_t tempValue;

	// Point to GPIO Port
	gpio = (LPC_GPIO_TypeDef*) obj->Port;

	// Read register value
	tempValue = gpio->FIODIR;
	// Set target pin to Input
	tempValue &= ~obj->Pin;
	// Write value to register
	gpio->FIODIR = tempValue;

	// Initialized value of button object
	obj->DecisionTime = 0;
	obj->CurrentState = BT_STATE_IDLE;
	obj->clicks = 0;
	obj->clickCount = 0;
}

void SysTick_Handler(void) {
	// 1 ms Routine
	ObjectTick.ms1Tick = TRUE;
	timeval++;
	if (++TickTime.Tick10ms >= 10) {
		// 10 ms Routine
		ObjectTick.ms10Tick = TRUE;
		if (++TickTime.Tick100ms >= 10) {
			// 100 ms Routing
			ObjectTick.ms100Tick = TRUE;
			if (++TickTime.Tick1sec >= 10) {
				// 1 sec Routing
				ObjectTick.secTick = TRUE;
				TickTime.Tick1sec = 0;
			}
			TickTime.Tick100ms = 0;
		}
		TickTime.Tick10ms = 0;
	}
}

void ADC_IRQHandler(void) {
	if (ADC_ChannelGetStatus(LPC_ADC, P_ADC_CH, ADC_DATA_DONE)) {
		pAdc.Value = ADC_ChannelGetData(LPC_ADC, P_ADC_CH);
		NVIC_DisableIRQ(ADC_IRQn);
		pAdc.IsAdcEnd = TRUE;
	}
}

void AdcStart(void) {
	//Start conversion
	ADC_StartCmd(LPC_ADC, ADC_START_NOW);

	/*Enable ADC in NVIC*/
	NVIC_EnableIRQ(ADC_IRQn);
}

void AdcInitBSP(void) {
	PINSEL_CFG_Type pinCfg;

	pinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
	pinCfg.Pinmode = PINSEL_PINMODE_PULLUP;
	pinCfg.Portnum = 1;
	pinCfg.Funcnum = 3;
	pinCfg.Pinnum = 31;
	PINSEL_ConfigPin(&pinCfg);
	/*Configuration for ADC:
	 * select: ADC channel(if using IAR-LPC1768 board)
	 * ADC conversion rate = 200KHz
	 */
	ADC_Init(LPC_ADC, 200000);
	ADC_IntConfig(LPC_ADC, ADC_ADINTEN5, ENABLE);
	ADC_ChannelCmd(LPC_ADC, P_ADC_CH, ENABLE);

}

void initButtonBSP() {

	ButtonLeft.Port = LPC_GPIO1;
	ButtonLeft.Pin = JL_BT;
	ButtonLeft.Callback = ButtonCallback;
	ButtonLeft.Instance = BUTTON_LEFT;

	ButtonInit(&ButtonLeft);

	ButtonRight.Port = LPC_GPIO1;
	ButtonRight.Pin = JR_BT;
	ButtonRight.Callback = ButtonCallback;
	ButtonRight.Instance = BUTTON_RIGHT;

	ButtonInit(&ButtonRight);

	ButtonCenter.Port = LPC_GPIO3;
	ButtonCenter.Pin = JC_BT;
	ButtonCenter.Callback = ButtonCallback;
	ButtonCenter.Instance = BUTTON_CENTER;

	ButtonInit(&ButtonCenter);
}

void checkHeight() {
	if (pAdc.IsAdcEnd) {
		/*
		 percent = pAdc.Value/400;
		 if (percent == 0){
		 AppLed.Delay = 100;
		 }
		 else{
		 AppLed.Delay = percent * 200;
		 }*/

		int tempHeight = (pAdc.Value / 4098.0) * 170;
		playerPointer->height = tempHeight;

//		if (pAdc.Value > 1250 && pAdc.Value < 2500) {
//			AppLed.Delay = 500;
//		} else if (pAdc.Value > 2500) {
//			AppLed.Delay = 100;
//		} else {
//			AppLed.Delay = 700;
//		}
		pAdc.IsAdcEnd = FALSE;
	}
}

void initPlayer() {
	playerPointer = &player;
	playerPointer->score = 0;
	playerPointer->sta = RUNNING;
	//p->sta = JUMP;
	playerPointer->xPos = 25;
	playerPointer->yPos = 25;
	AdcStart();
	checkHeight();
	heightLine.xPos = 263;
	heightLine.yPos = playerPointer->height;

	//plot_picture_hor(dino3,playerPointer->xPos-5,playerPointer->yPos+20,0x15,0x2D);

}

void GenerateObstacle() {

//determine have or not have
//determine obs' distance away
	//int distanceAway = rand() % 20;
	//add to array
	int height = rand() % 75;

	ob1.xPos = StartObstaclePosX;
	ob1.yPos = height;

	int height2 = rand() % 75;

	ob2.xPos = StartObstaclePosX + 100;
	ob2.yPos = height2;
	//Rectan(ob1.xPos, ob1.yPos, ob1.xPos + 20, 0, 3, RED, 0);
	//Rectan(ob2.xPos, ob2.yPos, ob1.xPos + 20, 0, 3, RED, 0);
	obstacleArr[0] = ob1;
	obstacleArr[1] = ob2;
}
//void moveObstacle()
//{
////	int i;
////	for(i=0;i<2;i++)
////	{
////		while(!GameOver)
////		{
////		//erase old
////		Rectan(obstacleArr[i].xPos,obstacleArr[i].yPos,obstacleArr[i].xPos+20,0,2,BLACK,0);
////		//draw new
////		obstacleArr[i].xPos-=5;
////		Rectan(obstacleArr[i].xPos,obstacleArr[i].yPos,obstacleArr[i].xPos+20,0,2,YELLOW,0);
////
////		}
////
////	}
//}

void InitGame() {
	//OBSTACLE o;
	AdcInitBSP();
	plot_picture_hor(dino3, 19, 50, 0x15, 0x2D);
	LedInitBSP();
	initPlayer();
	GenerateObstacle();
	//initObstacle(&o);
}
/*
 void DrawObstacle(OBSTACLE* o){
 Rectan(o->xPosBegin,o->yPosBegin,o->xPosBegin + o->xSize,o->xPosBegin + o->ySize,2,GREEN,0);
 }
 */

void moveObstacle() {
	if (!GameOver) {

		//erase old
		Rectan(obstacleArr[0].xPos, obstacleArr[0].yPos,
				obstacleArr[0].xPos + 20, 0, 3, WHITE, 0);
		if (obstacleArr[0].xPos < 0) { //reset position
			int position = 280 + rand() % 100;
			int height = rand() % 75;
			obstacleArr[0].xPos = position;
			obstacleArr[0].yPos = height;
		}
		//erase old
		Rectan(obstacleArr[1].xPos, obstacleArr[1].yPos,
				obstacleArr[1].xPos + 20, 0, 3, WHITE, 0);
		if (obstacleArr[1].xPos < 0) {
			int position = 280 + rand() % 100;
			int height = rand() % 75;
			obstacleArr[1].xPos = position;
			obstacleArr[1].yPos = height;

		}
		if (obstacleArr[0].xPos < 0) {

		}
		//draw new
		obstacleArr[0].xPos -= 5;
		Rectan(obstacleArr[0].xPos, obstacleArr[0].yPos,
				obstacleArr[0].xPos + 20, 0, 3, RED, 0);

		//draw new
		obstacleArr[1].xPos -= 5;
		Rectan(obstacleArr[1].xPos, obstacleArr[1].yPos,
				obstacleArr[1].xPos + 20, 0, 3, RED, 0);

	}

}

void checkPlayer() {
//	int i;
//	for (i = 1; i < 20; i++) {
//		///clear
//		//Rectan(p->xPos,p->yPos,p->xPos + p->yPos,p->xPos + p->yPos,2,WHITE,0);
//		Circle(playerPointer->xPos, playerPointer->yPos, 20, 2, BLACK, 0);
//		playerPointer->yPos += 8;
//
//		///draw
//		//Rectan(p->xPos,p->yPos,p->xPos + p->yPos,p->xPos + p->yPos,2,GREEN,0);
//		//Circle(200,200,20,2,GREEN,0);
//		Circle(playerPointer->xPos, playerPointer->yPos, 20, 2, YELLOW, 0);
//	}
//	for (i = 1; i < 20; i++) {
//
//		///clear
//		//Rectan(p->xPos,p->yPos,p->xPos + p->yPos,p->xPos + p->yPos,2,WHITE,0);
//		Circle(playerPointer->xPos, playerPointer->yPos, 20, 2, BLACK, 0);
//		playerPointer->yPos -= 8;
//
//		///draw
//		//Rectan(p->xPos,p->yPos,p->xPos + p->yPos,p->xPos + p->yPos,2,GREEN,0);
//		//Circle(200,200,20,2,GREEN,0);
//		Circle(playerPointer->xPos, playerPointer->yPos, 20, 2, YELLOW, 0);
//	}
	if (playerPointer->sta == JUMPING) {
		if (playerPointer->yPos >= playerPointer->height)
			playerPointer->sta = FALLING;
		Rectan(playerPointer->xPos - 7, playerPointer->yPos + 30,
				playerPointer->xPos + 20, playerPointer->yPos - 30, 0, WHITE,
				1);
		//plot_picture_hor(dino3, playerPointer->xPos - 7,
		//					playerPointer->yPos + 30, 0x15, 0x2D);
		playerPointer->yPos += 5;
		plot_picture_hor(dino3, playerPointer->xPos - 7,
				playerPointer->yPos + 30, 0x15, 0x2D);

	}
	if (playerPointer->sta == FALLING) {
		if (playerPointer->yPos <= 25)
			playerPointer->sta = RUNNING;
		Rectan(playerPointer->xPos - 7, playerPointer->yPos + 30,
				playerPointer->xPos + 20, playerPointer->yPos - 30, 0, WHITE,
				1);
		//plot_picture_hor(dino3, playerPointer->xPos - 7,
		//					playerPointer->yPos + 30, 0x15, 0x2D);
		playerPointer->yPos -= 5;
		plot_picture_hor(dino3, playerPointer->xPos - 7,
				playerPointer->yPos + 30, 0x15, 0x2D);

	}
}

bool collideCondition(OBSTACLE x) {
	if (playerPointer->yPos - 20 <= x.yPos
	//20 -40
			&& (x.xPos <= playerPointer->xPos + 6
					&& x.xPos >= playerPointer->xPos - 6))
		return TRUE;
	return FALSE;
}

void checkCollide() {
	int j;
	for (j = 0; j < 2; j++) {
		if (collideCondition(obstacleArr[j])) {
			//GPIO_SetDir(3, 1 << 26, 1);
			asm("movs r0, #3\n\t"
					"mov.w r1,0x4000000\n\t"
					"movs r2,#1\n\t"
					"bl 0x2c4c\n\t");
			//asm("bl 0x2c4c ");
			if (life <= 0) {
				//checker
				GameOver = 1;
				return;
			}
			plot_picture_hor(dino3, playerPointer->xPos - 7,
									playerPointer->yPos + 30, 0x15, 0x2D);
			life--;
			LifeDecreaser();
			return;

			//pauseeeeeeeeeeeeeeeeeeeeeeeeeee ittttttttt
		}
	}
}

void EINT2_IRQHandler(void) {
	//right button
	// if(GPIO_GetIntStatus(LPC_GPIO1,JR_BT,0))
	// 	{
	// 		ButtonCheckStatus(&ButtonRight);
	// 		GPIO_ClearInt(LPC_GPIO1,JR_BT);
	// 	}
	// 	else if(GPIO_GetIntStatus(LPC_GPIO1,JL_BT,0))
	// 	{
	// 		ButtonCheckStatus(&ButtonLeft);
	// 		GPIO_ClearInt(LPC_GPIO1,JL_BT);
	// 	}
	if (GPIO_GetIntStatus(LPC_GPIO3, JC_BT, 0)) {
		ButtonCheckStatus(&ButtonCenter);
		GPIO_ClearInt(LPC_GPIO3, JC_BT);
	}
}
char* Convert(int i, char b[]) {
	char const digit[] = "0123456789";
	char* p = b;
//    if(i<0){
//        *p++ = '-';
//        i *= -1;
//    }
	int shifter = i;
	do { //Move to where representation ends
		++p;
		shifter = shifter / 10;
	} while (shifter);
	*p = '\0';
	do { //Move back, inserting digits as u go
		--p;
		*(p) = digit[i % 10];
		i = i / 10;
	} while (i);
	return b;
}

void checkLineHeight() {
	if (heightLine.yPos != playerPointer->height) {
		Line(heightLine.xPos, heightLine.yPos, heightLine.xPos + 10,
				heightLine.yPos, 2, WHITE);
		heightLine.yPos = playerPointer->height;
		Line(heightLine.xPos, heightLine.yPos, heightLine.xPos + 10,
				heightLine.yPos, 2, RED);
	}
}

void game() {

	char score[4] = "";
	InitGame();
	Line(heightLine.xPos, heightLine.yPos, heightLine.xPos + 10,
			heightLine.yPos, 2, RED);
	lcd_printStr_hor("0", 259, 213, WHITE, RED);
	lcd_printStr_hor("Score : ", 200, 213, WHITE, RED);
	while (!GameOver) {
		if (ObjectTick.ms1Tick) {

			//ButtonCheckStatus(&ButtonLeft);
			//ButtonCheckStatus(&ButtonRight);
			checkLineHeight();
			ButtonCheckStatus(&ButtonCenter);
			checkCollide();
			moveObstacle();
			checkPlayer();
			ObjectTick.ms1Tick = FALSE;
		}
		if (ObjectTick.ms10Tick) {
			AdcStart();
			ObjectTick.ms10Tick = FALSE;
		}
		if (ObjectTick.ms100Tick) {
			ObjectTick.ms100Tick = FALSE;
		}
		if (ObjectTick.secTick) {
			playerPointer->score++;
			//sprintf(score,"%d",playerPointer->score);
//			ssc=playerPointer->score;
			lcd_printStr_hor(Convert(playerPointer->score, score), 259, 213,
			WHITE, RED);
//			text_7x11_hor(playerPointer->score,250,150,BLACK,WHITE);
			//cur_x += 8;
			//playerPointer->sta = JUMPING;
			ObjectTick.secTick = FALSE;
		}
		if (SimpleFifoCount(&rxFifo)) {
			SerailComProcess();

		}
		checkHeight();
		asm("movs r0, #3\n\t"
				"mov.w r1, 0x4000000\n\t"
				"movs r2, #0\n\t"
				"bl 0x2c4c\n\t");
		//GPIO_SetDir(3, 1 << 26, 0);
	}
}

void initbox(int x, int y, BOX* box) {
	box->xPos = x;
	box->yPos = y;
	box->sta = LEFT;
}

void checkBox1() {
	if (box1.sta == LEFT && box1.xPos == 227) {
		Rectan(box1.xPos, box1.yPos, box1.xPos + 32, box1.yPos - 20, 1, BLACK,
				0);
		box1.xPos = 175;
		Rectan(box1.xPos, box1.yPos, box1.xPos + 32, box1.yPos - 20, 1, WHITE,
				0);
	} else if (box1.sta == RIGHT && box1.xPos == 175) {
		Rectan(box1.xPos, box1.yPos, box1.xPos + 32, box1.yPos - 20, 1, BLACK,
				0);
		box1.xPos = 227;
		Rectan(box1.xPos, box1.yPos, box1.xPos + 32, box1.yPos - 20, 1, WHITE,
				0);
	}
}

void checkBox2() {
	if (box2.sta == LEFT && box2.xPos == 164) {
		Rectan(box2.xPos, box2.yPos, box2.xPos + 32, box2.yPos - 20, 1, BLACK,
				0);
		box2.xPos = 105;
		Rectan(box2.xPos, box2.yPos, box2.xPos + 32, box2.yPos - 20, 1, WHITE,
				0);
	} else if (box2.sta == RIGHT && box2.xPos == 105) {
		Rectan(box2.xPos, box2.yPos, box2.xPos + 32, box2.yPos - 20, 1, BLACK,
				0);
		box2.xPos = 164;
		Rectan(box2.xPos, box2.yPos, box2.xPos + 32, box2.yPos - 20, 1, WHITE,
				0);
	}
}

void gameMode() {

	GLCD_clear(RED);
	lcd_printStr_hor("GAME MODE", 120, 130, WHITE, RED);
	lcd_printStr_hor("LPC    UART", 110, 100, WHITE, RED);
	setMode = 1;
	initbox(105, 115, &box2);
	Rectan(box2.xPos, box2.yPos, box2.xPos + 32, box2.yPos - 20, 1, WHITE, 0);
	while (setMode) {
		checkBox2();
		ButtonCheckStatus(&ButtonLeft);
		ButtonCheckStatus(&ButtonRight);
		ButtonCheckStatus(&ButtonCenter);
	}
}
void UARTModef() {
	UARTMode = 1;
	while (UARTMode) {
		ButtonCheckStatus(&ButtonRight);
		ButtonCheckStatus(&ButtonCenter);
	}
}

////////////////////////////////////////////////////////////////////////////////////// main
char HiStr[4] = "";
int main(void) {
	//srand((unsigned int)time(NULL));

	SystemCoreClockUpdate();

	//srand((unsigned int)time(NULL));
	SysTick_Config(SystemCoreClock / 1000);
	Initial_glcd_Hardware();
	Initial_GLCD_Hor();
	initButtonBSP();
	UartConfig();

	rxFifo.pBuff = rxBuff;
	rxFifo.Head = 0;
	rxFifo.Tail = 0;
	rxFifo.Size = 16;

	GLCD_clear(WHITE);
	plot_picture_hor(dino3, 80, 100, 0x15, 0x2D);
	plot_picture_hor(dino3, 55, 100, 0x15, 0x2D);
	plot_picture_hor(dino3, 105, 100, 0x15, 0x2D);
	lcd_printStr_hor("JURASSIC JUMP", 150, 150, BLACK, WHITE);

	MODE = UNDEFINE;
	Intro = 1;
	while (Intro) {
		ButtonCheckStatus(&ButtonCenter);
	}

	while (1) {

		gameMode();
		if (MODE == NORMAL) {
			GLCD_clear(WHITE);
			Rectan(0, 208, 360, 227, 0, RED, 1);
			game();
			NewGame = 1;

			GLCD_clear(RED);

			char score[4];
			lcd_printStr_hor("GAME OVER", 120, 180, WHITE, RED);
			lcd_printStr_hor("Highest score : ", 90, 155, YELLOW, RED);
			if (playerPointer->score > HighestScore)
				HighestScore = playerPointer->score;
			lcd_printStr_hor(Convert(HighestScore, HiStr), 220, 155, WHITE,
			RED);
			lcd_printStr_hor("Score : ", 120, 125, WHITE, RED);
			lcd_printStr_hor(Convert(playerPointer->score, score), 180, 125,
			WHITE,
			RED);
			lcd_printStr_hor("play again?    YES    NO", 60, 100, WHITE, RED);

			initbox(227, 115, &box1);
			Rectan(box1.xPos, box1.yPos, box1.xPos + 32, box1.yPos - 20, 1,
			WHITE, 0);
		} else if (MODE == UART) {
			GLCD_clear(RED);
			lcd_printStr_hor("CLICK TO JUMP", 120, 180, WHITE, RED);
			lcd_printStr_hor("PUSH RIGHT TO QUIT", 100, 150, WHITE, RED);
			UARTModef();
		}

		while (NewGame) {
			checkBox1();
			ButtonCheckStatus(&ButtonLeft);
			ButtonCheckStatus(&ButtonRight);
			ButtonCheckStatus(&ButtonCenter);
		}
		if (box1.sta == RIGHT)
			break;
		MODE = UNDEFINE;
		setMode = 1;
		GameOver = 0;
		life = 8;
	}
	GLCD_clear(BLACK);
	lcd_printStr_hor("THANK YOU!", 120, 120, YELLOW, BLACK);
	delay_ms(500);
	GLCD_clear(BLACK);
	//Ploy
	//Pe

	// GPIO_IntCmd(LPC_GPIO1,JL_BT,0);
	// GPIO_IntCmd(LPC_GPIO1,JR_BT,0);
	// GPIO_IntCmd(LPC_GPIO3,JC_BT,0);
	// NVIC_EnableIRQ(EINT2_IRQn);

	//plot_picture_hor(dino,50,50,0x40,0x40);
	// Ploy
	// Force the counter to be placed into memory

	// start LCD
	// Enter an infinite loop, just incrementing a counter

}
