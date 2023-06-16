/*
 * File: dm_packet.h
 * Created Date: September 18th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#ifndef __DM_PACKET_H__
#define __DM_PACKET_H__

struct DmPacket {
    static const int kMsgHeaderBufferSize = 8;
    char header_buf_[kMsgHeaderBufferSize];

    const uint32_t kMaxOfMsgSize = (8 * 1024 * 1024);
    char* p_buf_;

    uint32_t pos_;
    uint32_t len_;
    uint32_t remain_len_;

    DmPacket() : p_buf_(nullptr), pos_(0), len_(0), remain_len_(0) {}
    ~DmPacket() { Reset(); }

    void Reset() {
        pos_ = 0;
        len_ = 0;
        remain_len_ = 0;
        if (p_buf_ != nullptr) {
            delete[] p_buf_;
            p_buf_ = nullptr;
        }
    }
};

const char* DmPacketHeader();
const char* DmPacketTail();
bool DmPacketRead(DmPacket* p_packet, void* buf, size_t nbytes, size_t* b_r_len);
bool IsDmPacketValid(const char* p_msg_buf, uint32_t msg_len);

#endif  // __DM_PACKET_H__
