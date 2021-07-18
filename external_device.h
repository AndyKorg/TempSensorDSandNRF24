/*
 external device property
 */

#ifndef MAIN_DRIVER_EXTERNAL_DEVICE_H_
#define MAIN_DRIVER_EXTERNAL_DEVICE_H_

#define DEVICE_TYPE_MH_Z19			1
#define DEVICE_TYPE_DS18B20			2

#define DEVICE_ANSWER_LEN			3	//first byte this length, second and third sleep number period. 
#define DEVICE_ANSWER_NUM_PERIOD	2	//the number of bytes for the sleep period. the byte value must be between 0x1 and 0xF.


#endif /* MAIN_DRIVER_EXTERNAL_DEVICE_H_ */
