/*
 * File: tcp_server_base.cpp
 * Created Date: June 16th 2022
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2022 (efancc@163.com)
 */

#include "tcp_server_base.h"

#include <unistd.h>

#include <iostream>

#ifdef _DEBUG_EN
#include "../memory_leak_detector/debug_new.h"
#endif

TcpServerBase::TcpServerBase() { p_tcp_server_ = new TcpServer(); }

TcpServerBase::~TcpServerBase() {
    if (p_tcp_server_ != nullptr) {
        delete p_tcp_server_;
        p_tcp_server_ = nullptr;
    }
}

bool TcpServerBase::Start() {
    return true;
}

bool TcpServerBase::Stop() { return p_tcp_server_->Stop(); }

int TcpServerBase::OnConnect(SOCKET socket_id) {
    cout << "TcpServerBase::OnConnect" << endl;
    return 0;
}

int TcpServerBase::OnDisconnect(SOCKET socket_id) {
    cout << "TcpServerBase::OnDisconnect" << endl;
    return 0;
}

int TcpServerBase::DoRead(SOCKET socket_id, void* buf, size_t nbytes) {
    cout << "TcpServerBase::DoRead, len: " << nbytes << ", " << (char*)buf << endl;
    return 0;
}

int TcpServerBase::SendMsg(SOCKET socket_id, void* buf, size_t nbytes) {
    return p_tcp_server_->Send(socket_id, buf, nbytes);
}