/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#ifndef _ROUND_UTIL_TIME_H_
#define _ROUND_UTIL_TIME_H_

#include <round/typedef.h>

#ifdef  __cplusplus
extern "C" {
#endif

/****************************************
* Function
****************************************/

float round_random(void);

void round_wait(clock_t mtime);
void round_waitrandom(clock_t mtime);

#define round_sleep(val) round_wait(val)
#define round_sleeprandom(val) round_waitrandom(val)

clock_t round_getcurrentsystemtime(void);

#ifdef  __cplusplus
}
#endif

#endif
