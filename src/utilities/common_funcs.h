/*
 * common_funcs.h
 *
 *  Created on: Apr 18, 2018
 *      Author: jerry
 */

#ifndef UTILITIES_COMMON_FUNCS_H_
#define UTILITIES_COMMON_FUNCS_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <iostream>

using namespace std;

extern uint32_t get_stm32_crc(uint32_t crc, uint8_t *s, uint32_t len);

class CommonFuncs {
   public:
    static uint32_t CalculateStm32Crc(uint8_t *pbuf, uint32_t len) {
        return get_stm32_crc(0xffffffff, pbuf, len);
    }

    static uint16_t MakeShort(uint8_t high, uint8_t low) {
        return (uint16_t)(((uint16_t)(high) << 8) | ((uint16_t)(low)));
    }

    static uint32_t MakeWord(uint8_t byte3, uint8_t byte2, uint8_t byte1, uint8_t byte0) {
        return ((uint32_t)((((uint32_t)(byte3)) << 24) | (((uint32_t)(byte2)) << 16) |
                           (((uint32_t)(byte1)) << 8) | ((uint32_t)(byte0))));
    }

    static uint32_t SwapInt(uint32_t in) {
        uint8_t data[4];

        memcpy(data, (void *)&in, sizeof(in));
        return ((uint32_t)((((uint32_t)(data[0])) << 24) | (((uint32_t)(data[1])) << 16) |
                           (((uint32_t)(data[2])) << 8) | ((uint32_t)(data[3]))));
    }

    static void IntArraySet(uint32_t *p_array, uint32_t _c, uint32_t len) {
        for (uint32_t i = 0; i < len; i++) {
            *p_array++ = _c;
        }
    }
    static void FloatArraySet(float *p_array, float _c, uint32_t len) {
        for (uint32_t i = 0; i < len; i++) {
            *p_array++ = _c;
        }
    }

    static string Uint32ToString(uint32_t _uint) {
        unsigned char *pChar = (unsigned char *)&_uint;
        char buff[16];
        snprintf(buff, sizeof(buff), "%d.%d.%d.%d", *pChar, *(pChar + 1), *(pChar + 2),
                 *(pChar + 3));

        string ret_str = buff;
        return ret_str;
    }

    static int SnprintIntBuffer(char *p_wr_buf, uint32_t max_size, char *p_src_buf,
                                uint32_t len_in_words, uint32_t base_line_no) {
        uint32_t *p_word = (uint32_t *)p_src_buf;
        uint32_t len = len_in_words;
        char *p_out = p_wr_buf;
        int wr_len = 0;
        int n = max_size;

        for (uint32_t i = 0; i < len; i++) {
            if ((i % 0x008) == 0) {
                wr_len = snprintf(p_out, n, "%08x:", i + base_line_no);
                if (wr_len < 0 || wr_len >= n) {
                    return -1;
                }
                p_out += wr_len;
                n -= wr_len;
            }
            wr_len = snprintf(p_out, n, "%08x ", *p_word);
            if (wr_len < 0 || wr_len >= n) {
                return -1;
            }
            p_out += wr_len;
            n -= wr_len;

            p_word++;
            if ((i & 0x007) == 0x007) {
                wr_len = snprintf(p_out, n, "\r\n");
                if (wr_len < 0 || wr_len >= n) {
                    return -1;
                }
                p_out += wr_len;
                n -= wr_len;
            }
        }
        wr_len = snprintf(p_out, n, "\r\n");
        p_out += wr_len;

        return (int)(p_out - p_wr_buf);
    }

    static void PrintSockaddr(sockaddr *s_addr) {
        sockaddr_in *sock = (sockaddr_in *)s_addr;
        cout << "family: " << sock->sin_family << endl;
        cout << "ip addr : " << inet_ntoa(sock->sin_addr) << endl;
        cout << "port : " << dec << ntohs(sock->sin_port) << endl;
    }

    static uint32_t ip_of_sockaddr(sockaddr *s_addr) {
        sockaddr_in *sock = (sockaddr_in *)s_addr;
        return sock->sin_addr.s_addr;
    }
    static uint16_t port_of_sockaddr(sockaddr *s_addr) {
        sockaddr_in *sock = (sockaddr_in *)s_addr;
        return sock->sin_port;
    }
};

#endif /* UTILITIES_COMMON_FUNCS_H_ */
