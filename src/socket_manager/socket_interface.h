/*
 * File: socket_interface.h
 * Created Date: September 16th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */

#ifndef __SOCKET_INTERFACE_H__
#define __SOCKET_INTERFACE_H__
#include <stdint.h>
#include <sys/socket.h>
#include "common_definitions.h"

using namespace std;

class ComplexListener {
   public:
    virtual int OnConnect(SOCKET socket_id) = 0;
    virtual int OnDisconnect(SOCKET socket_id) = 0;
    virtual int DoRead(SOCKET socket_id, void* buf, size_t nbytes) = 0;

   public:
    virtual ~ComplexListener() {}
};

class TcpClientListener : public ComplexListener {
   public:
    virtual int OnConnect(SOCKET socket_id) override { return 0; };
    virtual int OnDisconnect(SOCKET socket_id) override { return 0; };
    virtual int DoRead(SOCKET socket_id, void* buf, size_t nbytes) override { return 0; };
};

class UdpServerListener : public ComplexListener {
   public:
    virtual int OnConnect(SOCKET socket_id) override { return 0; };
    virtual int OnDisconnect(SOCKET socket_id) override { return 0; };
    virtual int DoRead(SOCKET socket_id, void* buf, size_t nbytes) override { return 0; };
    virtual int DoRead(SOCKET socket_id, void* buf, size_t nbytes, sockaddr *from_addr) = 0;
};

class TcpServerListener : public ComplexListener {
   public:
    virtual int OnConnect(SOCKET socket_id) override { return 0; };
    virtual int OnDisconnect(SOCKET socket_id) override { return 0; };
    virtual int DoRead(SOCKET socket_id, void* buf, size_t nbytes) override { return 0; };
};

#endif // __SOCKET_INTERFACE_H__
