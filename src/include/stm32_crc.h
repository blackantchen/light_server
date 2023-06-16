/*
 * stm32_crc.h
 *
 *  Created on: Sep 6, 2017
 *      Author: gangrong
 */

#ifndef SERVICE_STM32_CRC_H_
#define SERVICE_STM32_CRC_H_

// #include <stdint.h>

#ifdef __cpluscplus
extern "C" {
#endif

uint32_t stm32_crc(uint32_t crc, uint8_t *s, uint32_t len);

#ifdef __cpluscplus
}
#endif

#endif /* SERVICE_STM32_CRC_H_ */
