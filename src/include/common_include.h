/*
 * common_inc.h
 *
 *  Created on: Dec 12, 2017
 *      Author: jerry
 */

#ifndef INC_COMMON_INC_H_
#define INC_COMMON_INC_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <termios.h>
#include <sys/select.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <semaphore.h>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include "../project_config.h"

#include "../utilities/common_funcs.h"

//*****************************************************************
//
//*****************************************************************
#define   MAKEHALFWORD(high, low)    ((uint16_t)((((uint16_t)(high)) << 8 ) | ((uint16_t)(low))))
#define   MAKEDOUBLEWORD(high, low)    ((uint32_t)((((uint32_t)(high)) << 16 ) | ((uint32_t)(low)))) //yad change
#define   MAKEWORD(t_one,t_two,t_three,t_four)    \
    ((uint32_t) ((((uint32_t)(t_one)) << 24) | (((uint32_t)(t_two)) << 16) | (((uint32_t)(t_three)) << 8) | ((uint32_t)(t_four))))

#define OffsetOfStruct(s,m) ((size_t) &(((s *)0)->m))


#endif /* INC_COMMON_INC_H_ */
