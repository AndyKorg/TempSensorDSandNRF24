/*
 * soft reset for debug
 *
 */ 


#ifndef SOFTRESET_H_
#define SOFTRESET_H_

#ifdef DEBUG
void soft_reset_init(void);
void soft_reset_check(void);
#endif

#endif /* SOFTRESET_H_ */