/*
* 1-wire bus
*/

#include <stddef.h>
#include "one_wire.h"
#include "HAL.h"

#define ONE_WIRE_15_us				DELAY_US(15)
#define ONE_WIRE_RESET_480_us		DELAY_US(480)
#define ONE_WIRE_TIME_SLOT			DELAY_US(60)	//time - slot
#define ONE_WIRE_2_us				2		//Промежуток между таймслотами, обычно 1 мкс, довел до 2 мкс на всякий случай
#define ONE_WIRE_240_us				240								//Максимальная длительность ожидания ответа на Reset
#define ONE_WIRE_14_us				14								//Конец периода анализа ответного бита от устройства
#define ONE_WIRE_5_us				5								//Максимальная длительность периода после таймслота чтения

#define ONE_WIRE_NULL()				do {ONE_WIRE_PORT.DIRSET = ONE_WIRE_PIN;} while (0)	//pin set down
#define ONE_WIRE_ONE_REL()			do {ONE_WIRE_PORT.DIRCLR = ONE_WIRE_PIN;} while (0)	//pin release
#define ONE_WIRE_IS_NULL()			((ONE_WIRE_PORT.IN & ONE_WIRE_PIN)?0:1)	//input low level
#define ONE_WIRE_IS_ONE()			((ONE_WIRE_PORT.IN & ONE_WIRE_PIN)?1:0)	//input hight level
#define one_wire_int_en()			do{ONE_WIRE_PORT.ONE_WIRE_PIN_CTRL = PORT_ISC_BOTHEDGES_gc;} while(0)
#define one_wire_int_dis()			do{ONE_WIRE_PORT.ONE_WIRE_PIN_CTRL = PORT_ISC_INTDISABLE_gc;} while(0)

#define ONE_WIRE_TIMEOUT			0xff

one_wire_cb	func_cb;
typedef void(*int_func)(uint8_t value);

static int_func internal_func;

ISR(ONE_WIRE_INT){
	if (internal_func){
		internal_func(ONE_WIRE_TIMEOUT);
	}
	one_wire_int_reset();
}

ISR(ONE_WIRE_PORT_INT){
	if (ONE_WIRE_PORT.INTFLAGS & ONE_WIRE_PIN){
		PORTB.OUTCLR = PIN3_bm;
		PORTB.OUTSET = PIN3_bm;
		ONE_WIRE_PORT.INTFLAGS = ONE_WIRE_PIN;
		if (internal_func){
			internal_func(ONE_WIRE_PORT.IN & ONE_WIRE_PIN);
		}
	}
}

/*
* Reset bus and presence
*/
static void one_wire_presence_answer_end(uint8_t value){
	one_wire_timer_stop();
	static bool down = false;
	if (value == 0)	{				//start device response
		down = true;
		return;						//waiting for the device to respond
	}
	uint8_t device_present = ((value == ONE_WIRE_PIN) && down)?1:0; //the end of the device's response was also the beginning of the response.
	if (func_cb){
		func_cb(&device_present);
	}
	one_wire_int_dis();
}

static void one_wire_presence_end(uint8_t value){
	ONE_WIRE_ONE_REL();
	one_wire_timer_stop();
	internal_func = one_wire_presence_answer_end;
	one_wire_int_en();
	one_wire_timer_start(ONE_WIRE_TIME_SLOT);	//start waiting for a response
}

void OneWareReset(one_wire_cb answer_cb){
	func_cb = answer_cb;
	internal_func = one_wire_presence_end;
	ONE_WIRE_NULL();
	one_wire_timer_start(ONE_WIRE_RESET_480_us); //start reset command
}

/*
* send bit true
*/
static void one_wire_send(uint8_t *value);

static void one_wire_free_bus(uint8_t value){
	one_wire_send(NULL);
}

static void one_wire_end_send(uint8_t value){
	ONE_WIRE_ONE_REL();
	internal_func = one_wire_free_bus;
	one_wire_timer_start(ONE_WIRE_TIME_SLOT);	//waiting end slot
}

static void one_wire_true_bit_send(uint8_t value){
	internal_func = one_wire_end_send;
	ONE_WIRE_NULL();
	one_wire_timer_start(ONE_WIRE_15_us);		//start slot
}

static void one_wire_send(uint8_t *value){
	static uint8_t send_byte = 0, bit_cnt = 0;
	if (value){
		send_byte = *value;
		bit_cnt = 0;
	}
	if (bit_cnt < 8){
		if (send_byte & (1<<bit_cnt)){
			one_wire_true_bit_send(1);
		}
		else{
			one_wire_true_bit_send(0);
		}
		return;
	}
	if (func_cb){
		func_cb(0);
	}
}

void OneWareSendByte(uint8_t SendByte, one_wire_cb send_end_cb){
	func_cb = send_end_cb;
	one_wire_send(&SendByte);
}

uint8_t OneWareReciveByte(void){
	return 0;
}

void OneWareIni(void){
	ONE_WIRE_PORT.OUTCLR = ONE_WIRE_PIN;
	ONE_WIRE_PORT.ONE_WIRE_PIN_CTRL = PORT_ISC_INTDISABLE_gc;
	one_wire_timer_init();
}
