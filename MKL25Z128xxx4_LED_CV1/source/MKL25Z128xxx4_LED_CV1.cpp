/*
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    MKL25Z128xxx4_LED_CV1.cpp
 * @brief   Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_pit.h"

volatile bool pitIsrFlag = false;

//void delay_pit(uint32_t t) {
//	volatile uint32_t i = 0;
//	for (i = 0; i < 150000 * t; ++i) { //200000 -> 200ms // 20Mhz clozk
//		__asm("NOP");
//		/* delay */
//	}
//}

char *MORSE_TABLE[] = { "._", //A
		"_...", //B
		"_._.", //C
		"_..", //D
		".", //E
		".._.", //F
		"__.", //G
		"....", //H
		"..", //I
		".___", //J
		"_._", //K
		"._..", //L
		"__", //M
		"_.", //N
		"___", //O
		".__.", //P
		"__._", //Q
		"._.", //R
		"...", //S
		"_", //T
		".._", //U
		"..._", //V
		".__", //W
		"_.._", //X
		"_.__", //Y
		"__..", //Z
		};

#define LED_OFF() GPIO_SetPinsOutput(BOARD_INITPINS_LED_BLUE_GPIO, BOARD_INITPINS_LED_BLUE_GPIO_PIN_MASK);
#define LED_ON() GPIO_ClearPinsOutput(BOARD_INITPINS_LED_BLUE_GPIO, BOARD_INITPINS_LED_BLUE_GPIO_PIN_MASK);

extern "C" void PIT_IRQHandler() {
	/* Clear interrupt flag.*/
	PIT_ClearStatusFlags(PIT, kPIT_Chnl_0, kPIT_TimerFlag);
	pitIsrFlag = true;

}

void delay_pit(int i) {
	/* Start channel 0 */
	PIT_StartTimer(PIT, kPIT_Chnl_0);
	for (int j = 0; j < i; j++) {
		while (!pitIsrFlag) {
			__asm("NOP");
		}
		pitIsrFlag = false;
	}
	PIT_StopTimer(PIT, kPIT_Chnl_0);
}

/*
 * @brief   Application entry point.
 */
int main(void) {
	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
	/* Init FSL debug console. */
	BOARD_InitDebugConsole();
#endif
	/* Structure of initialize PIT */
	pit_config_t pitConfig;

	PIT_GetDefaultConfig(&pitConfig);

	/* Init pit module */
	PIT_Init(PIT, &pitConfig);
	/* Set timer period for channel 0 */
	PIT_SetTimerPeriod(PIT, kPIT_Chnl_0,
			USEC_TO_COUNT(150000U, CLOCK_GetBusClkFreq()));

	/* Enable timer interrupts for channel 0 */
	PIT_EnableInterrupts(PIT, kPIT_Chnl_0, kPIT_TimerInterruptEnable);

	/* Enable at the NVIC */
	EnableIRQ (PIT_IRQn);


	PRINTF("Hello World\n");
	char *text = "SOS AHOJ SVET";

	/* Force the counter to be placed into memory. */
	volatile static int i = 0;
	/* Enter an infinite loop, just incrementing a counter. */
	while (1) {
		char *pt = text;
		while (*pt != 0) {
			if (*pt != ' ') {
				char *c = MORSE_TABLE[*pt - 'A'];
				while (*c != 0) {
					LED_ON();
					if (*c == '.') {
						delay_pit(1); //.
					} else {
						delay_pit(3); // _
					}
					LED_OFF();
					delay_pit(1);
					c++;
				}
				delay_pit(2); // 1 + 2 = 3
			} else {
				// Medzera
				delay_pit(7);
			}
			pt++;
		}
		delay_pit(4); //  3 + 4 = 7

	}
	return 0;
}
