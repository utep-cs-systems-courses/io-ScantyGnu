	.arch msp430g2553
	.p2align 1,0

	.data 			;Initializes variables
	
redVal:
	.word 0
	.word 1
greenVal:
	.word 0
	.word 64
ledFlags:
	.word 0
red_on:
	.word 1
green_on:
	.word 1
led_changed:
	.word 1
	
	.text
	.global led_init, led_update, red_on, green_on, led_changed
	.extern P1OUT, P1DIR

led_init:
	bis #65, &P1DIR		;P1DIR |= LEDS
	mov #1, led_changed	;led_changed = 1
	jmp led_update		;goto led_update

led_update:
	cmp #0, led_changed 	;if (led_changed != 0)
	jnz update		;goto update

	pop r0

update:
	mov &redVal, r5			;r5 = redVal
	mov red_on(r5), ledFlags 	;ledFlags = redVal[red_on]
	mov &greenVal, r5	 	;r5 = greenVal
	bis green_on(r5), ledFlags	;ledFlags = redVal[red_on] | greenVal[green_on]

	mov #255, r4		;r4 = 0xff
	xor #65, r4		;r4 = 0xff ^ LEDS
	bis ledFlags, r4	;r4 = (0xff ^ LEDS) | ledFlags
	and r4, &P1OUT		;P1OUT &= r4
	
	bis ledFlags, &P1OUT	;P1OUT |= ledFlags

	mov #0, led_changed 	;led_changed = 0

	pop r0
	
