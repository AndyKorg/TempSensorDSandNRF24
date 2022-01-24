/*
 external device property
 */

#ifndef MAIN_DRIVER_EXTERNAL_DEVICE_H_
#define MAIN_DRIVER_EXTERNAL_DEVICE_H_
//sender type 
#define DEVICE_TYPE_MH_Z19			1	//CO2 sensor, 
//temperature sensors - send 2 byte result. Resolution 12 bit - first 8 bit integer part and last 4 bit fraction part
#define DEVICE_TYPE_DS18B20			2	//temperature sensor ds18b20, Accuracy 0,125 celsiy 
#define DEVICE_TYPE_INTER_TEMPR		3	//internal temperature sensor in MK chip, accuracy 1 celsiy
//answer reciver
#define DEVICE_ANSWER_LEN			3	//first byte reserved, second and third sleep number period. 
#define DEVICE_ANSWER_PERIOD_OFFSET	1	//period value offset, bytes
#define DEVICE_ANSWER_NUM_PERIOD	2	//the number of bytes for the sleep period. high byte ahead?


#endif /* MAIN_DRIVER_EXTERNAL_DEVICE_H_ */
