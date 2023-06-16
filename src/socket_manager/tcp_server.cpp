/*
 * File: tcp_server.cpp
 * Created Date: September 21st 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#include <unistd.h>
#include <iostream>

#include "tcp_server.h"
#ifdef _DEBUG_EN
#include "../memory_leak_detector/debug_new.h"
#endif

TcpServer::TcpServer()
    : ServerBase(),
      p_listener_(nullptr),
      listen_socket_(INVALID_SOCKET),
      max_connections_of_listen_socket_(MAX_PENDING_CONNECTTIONS_OF_TCP_SERVER) {
    MakeRxBuffer(kSocketRxBufferSize);
}
TcpServer::~TcpServer() { 
    ServerBase::Dispose(); 
}

bool TcpServer::Start(TcpServerListener* p_listener, in_addr_t ipv4, in_port_t port){
    ASSERT(p_listener != nullptr);

    p_listener_ = p_listener;
    if (CreateListenSocket(ipv4, port) == FALSE) {
        return FALSE;
    }
    if (socket_manager_.Start(this, heartbeat_interval_in_seconds_ * 1000) < 0) {
        CloseListenSocket();
        return FALSE;
    }
    StartAccept();

    // MakeRxBuffer(kSocketRxBufferSize);
    return TRUE;
}
bool TcpServer::Stop() {
    CloseListenSocket();
    socket_manager_.CloseAllSocket();
    return (0 == socket_manager_.Stop());
}

bool TcpServer::CreateListenSocket(in_addr_t ipv4, in_port_t port) {
    do {
        listen_socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if(listen_socket_ == INVALID_SOCKET){
            break;
        }

        int opts = 1;
        if(setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR, &opts, sizeof(opts)) < 0){
            break;
        }
        if(setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEPORT, &opts, sizeof(opts)) < 0){
            break;
        }
        if(SocketManager::SetNonBlocking(listen_socket_) < 0){
            break;
        }

        sockaddr_in my_addr;
        memset((void*)&my_addr, 0, sizeof(my_addr));
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(port);;
        my_addr.sin_addr.s_addr = htonl(ipv4);
        if(bind(listen_socket_, (sockaddr*)&my_addr, sizeof(my_addr))<0){
            break;
        }
        if(listen(listen_socket_, max_connections_of_listen_socket_) < 0){
            break;
        }

        return TRUE;
    } while (0);

    close(listen_socket_);
    return FALSE;
}
void TcpServer::CloseListenSocket() { close(listen_socket_); }
bool TcpServer::StartAccept() {
    if(listen_socket_ == INVALID_SOCKET){
        return FALSE;
    }

    return socket_manager_.AddFD(listen_socket_, EPOLLIN | EPOLLET, &listen_socket_);
}
void TcpServer::StopAccept() {}
bool TcpServer::HandleAccept(uint32_t events) {
    // cout << "TcpServer::HandleAccept" << endl;
    if (events & (EPOLLERR | EPOLLHUP)) {
        return FALSE;
    }

    SOCKET new_sock;
    sockaddr from_addr;
    socklen_t addr_len = sizeof(from_addr);
    memset((void*)&from_addr, 0, addr_len);

    new_sock = accept(listen_socket_, &from_addr, &addr_len);
    if(new_sock == INVALID_SOCKET){
        return FALSE;
    }

    if (SocketManager::SetNonBlocking(new_sock) < 0) {
        return FALSE;
    }
    if(socket_manager_.AddTcpSocket(new_sock, &from_addr) < 0){
        close(new_sock);
        return FALSE;
    }

    OnConnect(new_sock);
    return TRUE;
}

void TcpServer::OnConnect(SOCKET socket_id) {
    cout << "OnConnect" << endl;
    p_listener_->OnConnect(socket_id);
}
void TcpServer::OnDisconnect(SOCKET socket_id) {
    cout << "OnDisconnect" << endl;
    p_listener_->OnDisconnect(socket_id);
}
void TcpServer::OnTimer(uint64_t llExpirations) {}
bool TcpServer::OnBeforeProcessIo(PVOID pv, uint32_t events){
    if(pv == &listen_socket_){
        HandleAccept(events);
        return FALSE;
    }
    return TRUE;
}
void TcpServer::OnAfterProcessIo(PVOID pv, uint32_t events, bool rs){}
bool TcpServer::OnReadyRead(PVOID pv, uint32_t events) {
    // cout << "TcpServer::OnReadyRead" << endl;
    return HandleReceive((SocketObject*)pv, events);
}
bool TcpServer::OnReadyWrite(PVOID pv, uint32_t events) {
    // cout << "TcpServer::OnReadyWrite" << endl;
    return HandleSend((SocketObject*)pv, events);
}

int TcpServer::Disconnect(SOCKET socket_id) { return socket_manager_.Disconnect(socket_id); }
int TcpServer::Send(SOCKET socket_id, void* buf, size_t nbytes) {
    return socket_manager_.Send(socket_id, buf, nbytes);
}

bool TcpServer::HandleReceive(SocketObject* pSocketObj, int flag) {
    // ASSERT(SocketObject::IsValid(pSocketObj));
    if (SocketObject::IsInvalid(pSocketObj)) return FALSE;

    SOCKET sock_id = pSocketObj->SocketId();

    TByteBuffer* buffer = this->RxBuffer();
    pSocketObj->active_time_ = socket_manager_.GetTime();

    int reads = flag ? -1 : MAX_CONTINUE_READS;

    for (int i = 0; i < reads || reads < 0; i++) {
        int rc = (int)read(sock_id, buffer->Ptr(), buffer->Size());

        if (rc > 0) {
            p_listener_->DoRead(sock_id, buffer->Ptr(), rc);
        } else if (rc == 0) {
            // AddFreeSocketObj(pSocketObj, SCF_CLOSE, SO_RECEIVE, SE_OK);
            cout << "HandleReceive: rc = 0" << endl;
            socket_manager_.CloseSocket(sock_id);
            return FALSE;
        } else {
            ASSERT(rc == SOCKET_ERROR);

            // int code = ::WSAGetLastError();

            if (errno == EWOULDBLOCK) break;

            // AddFreeSocketObj(pSocketObj, SCF_ERROR, SO_RECEIVE, code);
            cout << "rc = " << rc << endl;
            cout << "Socket read error: " << strerror(errno) << endl;
            socket_manager_.CloseSocket(sock_id);
            return FALSE;
        }
    }

    return TRUE;
}
bool TcpServer::HandleSend(SocketObject* pSocketObj, int flag) {
    // ASSERT(SocketObject::IsValid(pSocketObj));
    if(SocketObject::IsInvalid(pSocketObj)) return -1;

    while (!pSocketObj->OutPacketEmpty()) {
        MsgPacket* p_packet = pSocketObj->FrontOutPacket();
        if (p_packet != nullptr) {
            int wr_len = 0;
            while (p_packet->Size()) {
                wr_len = write(pSocketObj->SocketId(), p_packet->Ptr(), p_packet->Size());
                if (wr_len > 0) {
                    p_packet->Reduce(wr_len);
                } else {
                    if (errno == EWOULDBLOCK || errno == EAGAIN)
                        return 0;
                    else
                        return -1;
                }
            }
        }
        pSocketObj->PopOutPacket();
    }

    return 0;
}
