/*
 * File: dm_packet.cpp
 * Created Date: September 18th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include "../utilities/common_funcs.h"
#include "dm_packet.h"

#define MIN(x, y) (x > y ? y : x)
#define PKG_HEADER_SIZE 4
#define PKG_LENGTH_POS  PKG_HEADER_SIZE
#define PKG_LENGTH_SIZE 4
#define PKG_DATA_POS (PKG_LENGTH_SIZE + PKG_HEADER_SIZE)
static const char kMsgHeader[] = {
    '[',
    'D',
    'M',
    '-',
};
static const char kMsgTail[] = {
    '-',
    'N',
    'R',
    ']',
};

const char* DmPacketHeader() { return kMsgHeader; }
const char* DmPacketTail() { return kMsgTail; }

bool IsDmPacketValid(const char* p_msg_buf, uint32_t msg_len) {
    if (memcmp(p_msg_buf, kMsgHeader, sizeof(kMsgHeader))) return false;

    uint32_t actual_len = 0;
    memcpy((char*)&actual_len, &p_msg_buf[4], sizeof(int));
    if (actual_len != msg_len) return false;

    if (memcmp(p_msg_buf + msg_len - sizeof(kMsgTail), kMsgTail, sizeof(kMsgTail))) return false;

    uint32_t msg_crc = 0;
    uint32_t actual_crc = 0;
    memcpy((char*)&msg_crc, p_msg_buf + msg_len - sizeof(uint32_t) - sizeof(kMsgTail),
           sizeof(uint32_t));
    actual_crc = CommonFuncs::CalculateStm32Crc((uint8_t*)p_msg_buf, (msg_len - 8));
    if (msg_crc != actual_crc) return false;

    return true;
}

bool DmPacketRead(DmPacket* p_packet, void* buf, size_t nbytes, size_t* b_r_len) {
    if (p_packet == nullptr) {
        return false;
    }

    size_t len = 0;
    int8_t* rx_buf = (int8_t*)buf;
    size_t rx_len = nbytes;
    *b_r_len = 0;

    while (rx_len > 0) {
        if (p_packet->pos_ < PKG_HEADER_SIZE) {
            len = MIN((PKG_HEADER_SIZE - p_packet->pos_), rx_len);
            memcpy(&p_packet->header_buf_[p_packet->pos_], rx_buf, len);
            p_packet->pos_ += len;
            if (p_packet->pos_ >= PKG_HEADER_SIZE) {
                if (memcmp(p_packet->header_buf_, kMsgHeader, sizeof(kMsgHeader))) {
                    p_packet->Reset();
                }
            }
        } else if (p_packet->pos_ < PKG_DATA_POS) {
            len = MIN((PKG_DATA_POS - p_packet->pos_), rx_len);
            memcpy(&p_packet->header_buf_[p_packet->pos_], rx_buf, len);
            p_packet->pos_ += len;
            if (p_packet->pos_ == PKG_DATA_POS) {
                uint32_t msg_len = 0;
                memcpy((char*)&msg_len, &p_packet->header_buf_[PKG_LENGTH_POS], PKG_LENGTH_SIZE);
                if ((msg_len >= 16) && (msg_len < p_packet->kMaxOfMsgSize)) {
                    // msg header and msg length received success, malloc memory for receiving msg
                    // data
                    p_packet->p_buf_ = new char[msg_len];
                    memcpy(p_packet->p_buf_, p_packet->header_buf_, PKG_DATA_POS);
                    p_packet->len_ = msg_len;
                    p_packet->remain_len_ = msg_len - PKG_DATA_POS;
                    // cout << "msg len = " << (int)msg_len << endl;
                } else {
                    p_packet->Reset();
                }
            }
        } else if (p_packet->remain_len_) {
            len = MIN(p_packet->remain_len_, rx_len);
            memcpy(&p_packet->p_buf_[p_packet->pos_], rx_buf, len);
            p_packet->pos_ += len;
            p_packet->remain_len_ -= len;
            if (p_packet->remain_len_ == 0) {
                if (IsDmPacketValid(p_packet->p_buf_, p_packet->len_) == true) {
                    *b_r_len += len;
                    return true;
                } else {
                    cout << "----> error CRC" << endl;
                }
                p_packet->Reset();
            }
        } else {
            p_packet->Reset();
            cout << "----> error pos" << endl;
        }
        rx_buf += len;
        rx_len -= len;
        *b_r_len += len;
    }

    return false;
}