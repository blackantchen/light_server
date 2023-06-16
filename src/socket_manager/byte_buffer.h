/*
 * File: byte_buffer.h
 * Created Date: September 8th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#ifndef __BYTE_BUFFER_H__
#define __BYTE_BUFFER_H__

#include <stdint.h>

using namespace std;

class TByteBuffer {
   private:
    /* data */
    uint32_t size_in_bytes_;
    int8_t* p_buf_;

   private:
    void operator=(const TByteBuffer& obj);
    void Dispose() {
        if (p_buf_ != nullptr) delete[] p_buf_;
        size_in_bytes_ = 0;
    }

   public:
    TByteBuffer(uint32_t num_of_bytes) {
        if (num_of_bytes > 0) {
            p_buf_ = new int8_t[num_of_bytes];
        }
        size_in_bytes_ = num_of_bytes;
    }
    ~TByteBuffer() {
        Dispose();
    }

    uint32_t Size() { return size_in_bytes_; }
    int8_t* Ptr() { return p_buf_; }
};

#endif  // __BYTE_BUFFER_H__
