/*
\brief internal temperature sensor driver MK
*/
#include <avr/io.h>

uint16_t GetTemperature(uint8_t attempt){

	VREF_CTRLA = VREF_ADC0REFSEL_1V1_gc; /* Voltage reference at 1.1V */

	ADC0.CTRLC = ADC_PRESC_DIV4_gc      /* CLK_PER divided by 4 */
	| ADC_REFSEL_INTREF_gc /* Internal reference */
	| 0 << ADC_SAMPCAP_bp; /* Sample Capacitance Selection: disabled */

	ADC0.MUXPOS = ADC_MUXPOS_TEMPSENSE_gc; /* Temp sensor/DAC1 */

	ADC0.CTRLA = 1 << ADC_ENABLE_bp     /* ADC Enable: enabled */
	| 0 << ADC_FREERUN_bp  /* ADC Freerun mode: disabled */
	| ADC_RESSEL_10BIT_gc  /* 10-bit mode */
	| 0 << ADC_RUNSTBY_bp; /* Run standby mode: disabled */

	ADC0.COMMAND = ADC_STCONV_bm;

	while(!(ADC0.INTFLAGS & ADC_RESRDY_bm));

	uint16_t adc_reading = ADC0.RES; // ADC conversion result with 1.1 V internal reference

	ADC0.INTFLAGS |= ADC_RESRDY_bm;

	int8_t sigrow_offset = SIGROW.TEMPSENSE1; // Read signed value from signature row
	uint8_t sigrow_gain = SIGROW.TEMPSENSE0; // Read unsigned value from signature row
	uint32_t temp = adc_reading - sigrow_offset;
	temp *= sigrow_gain; // Result might overflow 16 bit variable (10bit+8bit)
	temp += 0x80; // Add 1/2 to get correct rounding on division below
	temp >>= 8; // Divide result to get Kelvin
	return (temp-273)<<4; //convert to format ds18b20
}
