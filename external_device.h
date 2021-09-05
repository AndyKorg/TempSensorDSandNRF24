/*
 external device property
 */

#ifndef MAIN_DRIVER_EXTERNAL_DEVICE_H_
#define MAIN_DRIVER_EXTERNAL_DEVICE_H_

#define DEVICE_TYPE_MH_Z19			1
#define DEVICE_TYPE_DS18B20			2
#define DEVICE_TYPE_INTER_TEMPR		3

#define DEVICE_ANSWER_LEN			3	//first byte reserved, second and third sleep number period. 
#define DEVICE_ANSWER_PERIOD_OFFSET	1	//period value offset, bytes
#define DEVICE_ANSWER_NUM_PERIOD	2	//the number of bytes for the sleep period. high byte ahead?


#endif /* MAIN_DRIVER_EXTERNAL_DEVICE_H_ */
