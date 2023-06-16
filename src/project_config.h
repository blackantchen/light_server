/*
 * ProjectConfig.h
 *
 *  Created on: Sep 4, 2018
 *      Author: jerry
 */

#ifndef PROJECT_CONFIG_H_
#define PROJECT_CONFIG_H_

#define SET_VERSION(v1, v2, v3, v4)                                                             \
    ((uint32_t)((((uint32_t)(v4)) << 24) | (((uint32_t)(v3)) << 16) | (((uint32_t)(v2)) << 8) | \
                ((uint32_t)(v1))))

#define PROJECT_VERSION SET_VERSION(1, 0, 0, 2)

// #define _DEBUG_ON_PC

#define LIB_VERSION PROJECT_VERSION

#endif /* PROJECT_CONFIG_H_ */
