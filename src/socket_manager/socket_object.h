/*
 * File: socket_object.h
 * Created Date: May 8th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */

#ifndef __SOCKET_OBJECT_H__
#define __SOCKET_OBJECT_H__

#include <netinet/in.h>
#include <poll.h>
#include <stdint.h>
#include <string.h>

#include <queue>
#include <iostream>
#include "common_definitions.h"

using namespace std;

#define MAX_SIZE_OF_MSG_PACKET 1024 * 1024 * 16
struct MsgPacket {
    uint8_t *payload;
    uint8_t *begin;
    uint8_t *end;
    int32_t pkg_length;
    int32_t pos;
    const int32_t kMaxSizeOfPacket = MAX_SIZE_OF_MSG_PACKET;
    sockaddr saddr;

    MsgPacket() : payload(nullptr), pkg_length(0), pos(0) {}
    MsgPacket(int32_t nbytes, void *data) {
        if (nbytes > 0 && nbytes < kMaxSizeOfPacket) {
            pkg_length = nbytes;
            payload = new uint8_t[nbytes];
            memcpy(payload, data, pkg_length);
            begin = payload;
            end = payload + pkg_length;
        } else {
            pkg_length = 0;
            payload = nullptr;
        }
        pos = 0;
    }
    MsgPacket(int32_t nbytes, void *data, sockaddr* p_sockaddr) {
        if (nbytes > 0 && nbytes < kMaxSizeOfPacket) {
            pkg_length = nbytes;
            payload = new uint8_t[nbytes];
            memcpy(payload, data, pkg_length);
            begin = payload;
            end = payload + pkg_length;
            memcpy(&saddr, p_sockaddr, sizeof(sockaddr));
        } else {
            pkg_length = 0;
            payload = nullptr;
        }
        pos = 0;
    }
    ~MsgPacket() {
        Dispose();
    }

    uint8_t* Ptr() { return begin;}
    int32_t Size() { return (int32_t)(end - begin); }
    sockaddr *pSocketAddr() { return &saddr; }
    void Reduce(int32_t length) {
        ASSERT(length >= 0);

        int32_t reduce = length < this->Size() ? length : this->Size();
        begin += reduce;
    }
    
    void Dispose(){
        if (payload != nullptr) {
            delete[] payload;
            payload = nullptr;
        }
        pkg_length = 0;
        pos = 0;
        begin = nullptr;
        end = nullptr;
    }
};

class SocketObject {
   private:
    /* data */
    SOCKET socket_id_;
    sockaddr remote_addr_;

    bool connect_state_;
    bool f_valid_;

    // MsgPacket in_packet_;
    const size_t kMaxNumOfPackets = 1000;
    queue<MsgPacket*> out_packets_queue_;
    MsgPacket *out_packet_;
    std::mutex out_packets_mutex_;

    // Traffic Limit
    bool f_receive_limit_;
    uint32_t max_rx_packets_per_second_;
    uint32_t max_rx_bytes_per_second_;
    uint32_t rx_packets_count_;
    uint32_t rx_bytes_count_;
    uint64_t last_rx_time_ms_;

    bool f_send_limit_;
    uint32_t max_tx_packets_per_second_;
    uint32_t max_tx_bytes_per_second_;

   public:
    time_t connect_time_;
    time_t active_time_;

   private:
    // bool ConnectTo(sockaddr remote_addr);
    void Dispose(){
        while(!OutPacketEmpty()){
            MsgPacket* p_packet = out_packets_queue_.front();
            if(p_packet != nullptr){
                delete p_packet;
            }
            out_packets_queue_.pop();
        }
    }

   public:
    SocketObject(/* args */) {
        socket_id_ = INVALID_SOCKET;
        memset((void *)&remote_addr_, 0, sizeof(sockaddr));
        connect_state_ = false;
        active_time_ = 0;

        out_packet_ = nullptr;

        f_receive_limit_ = false;
        f_send_limit_ = false;
    }
    ~SocketObject() { Dispose(); }

    void SetRemoteAddr(sockaddr *p_sa){
        memcpy((void*)&remote_addr_, (void*)p_sa, sizeof(sockaddr));
    }

    int PushMsg(void *buf, size_t nbytes) {
        out_packets_mutex_.lock();
        if(out_packets_queue_.size() >= kMaxNumOfPackets){
            out_packets_mutex_.unlock();
            return -1;
        }
            
        MsgPacket *pack = new MsgPacket(nbytes, (uint8_t *)buf);
        out_packets_queue_.push(pack);

        out_packets_mutex_.unlock();
        return 0;
    }
    int PushUdpMsg(void *buf, size_t nbytes, sockaddr* p_sockaddr) {
        out_packets_mutex_.lock();
        if(out_packets_queue_.size() >= kMaxNumOfPackets){
            out_packets_mutex_.unlock();
            return -1;
        }
            
        MsgPacket *pack = new MsgPacket(nbytes, (uint8_t *)buf, p_sockaddr);
        out_packets_queue_.push(pack);

        out_packets_mutex_.unlock();
        return 0;
    }

    MsgPacket *FrontOutPacket() { return out_packets_queue_.front(); }
    void PopOutPacket(){
        out_packets_mutex_.lock();
        MsgPacket* p_packet = out_packets_queue_.front();
        if(p_packet != nullptr){
            delete p_packet;
        }
        out_packets_queue_.pop();
        out_packets_mutex_.unlock();
    }
    bool OutPacketEmpty() { return out_packets_queue_.empty(); }

    sockaddr *GetRemoteAddr() { return &remote_addr_; }
    void SetSocketId(SOCKET sock) { socket_id_ = sock; }
    SOCKET SocketId() { return socket_id_; }

    static bool IsValid(SocketObject *pSocketObj) {
        return (pSocketObj != nullptr && pSocketObj->f_valid_);
    }
    static bool IsInvalid(SocketObject *pSocketObj) {
        return (pSocketObj == nullptr || pSocketObj->f_valid_ == false);
    }
    void SetInvalid() { this->f_valid_ = FALSE; }
    void SetValid() { this->f_valid_ = TRUE; }
    void SetConnected(time_t time) {
        connect_state_ = true;
        connect_time_ = time;
    }
    bool IsConnected() { return connect_state_; }

    bool IsReceiveLimit() { return f_receive_limit_; }
    void SetReceivePacketLimit(uint32_t num_of_packets) {
        if (num_of_packets > 0) {
            f_receive_limit_ = true;
            max_rx_bytes_per_second_ = 0;
            max_rx_packets_per_second_ = num_of_packets;
        } else {
            f_receive_limit_ = false;
            max_rx_bytes_per_second_ = 0;
            max_rx_packets_per_second_ = 0;
        }
        last_rx_time_ms_ = 0;
    }
    void SetReceiveBytesLimit(uint32_t num_of_bytes) {
        if (num_of_bytes > 0) {
            f_receive_limit_ = true;
            max_rx_bytes_per_second_ = num_of_bytes;
            max_rx_packets_per_second_ = 0;
        } else {
            f_receive_limit_ = false;
            max_rx_bytes_per_second_ = 0;
            max_rx_packets_per_second_ = 0;
        }
        last_rx_time_ms_ = 0;
    }
    void CountReceivedPacket(uint32_t num_of_packets, uint64_t now_in_ms) {
        if (now_in_ms - last_rx_time_ms_ >= 1 * 1000) {
            rx_packets_count_ = 0;
            last_rx_time_ms_ = now_in_ms;
        }
        rx_packets_count_ += num_of_packets;
    }
    void CountReceivedBytes(uint32_t num_of_bytes, uint64_t now_in_ms){
        if (now_in_ms - last_rx_time_ms_ >= 1 * 1000) {
            rx_bytes_count_ = 0;
            last_rx_time_ms_ = now_in_ms;
        }
        rx_bytes_count_ += num_of_bytes;
    }
    // void ResetReceiveCounter() { rx_packets_count_ = 0; }
    bool IsReceiveOverflow() {
        if ((max_rx_packets_per_second_ > 0 && rx_packets_count_ > max_rx_packets_per_second_) ||
            (max_rx_bytes_per_second_ > 0 && rx_bytes_count_ > max_rx_bytes_per_second_)) {
            return true;
        } else {
            return false;
        }
        // return (rx_packets_count_ > max_rx_packets_per_second_);
    }

    void SetSendPacketLimit(uint32_t num_of_packets) {
        if (num_of_packets > 0) {
            f_send_limit_ = true;
            max_tx_bytes_per_second_ = 0;
            max_tx_packets_per_second_ = num_of_packets;
        } else {
            f_send_limit_ = false;
            max_tx_bytes_per_second_ = 0;
            max_tx_packets_per_second_ = 0;
        }
    }

};

// SocketObject::SocketObject(/* args */) {
// }

// SocketObject::~SocketObject() {}

#endif  // __SOCKET_OBJECT_H__
