/*
 * File: tcp_client.cpp
 * Created Date: September 16th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#include <unistd.h>
#include <iostream>

#include "socket_manager.h"
#include "tcp_client.h"

TcpClient::TcpClient()
    : p_rx_buffer_(nullptr),
      heartbeat_interval_in_seconds_(0),
      keep_alive_time_in_seconds_(0),
      timer_event_count_(0) {
    p_rx_buffer_ = new TByteBuffer(kSocketRxBufferSize);
}

void TcpClient::Dispose() {
    if (p_rx_buffer_ != nullptr) {
        delete p_rx_buffer_;
        p_rx_buffer_ = nullptr;
    }
}

bool TcpClient::Start(TcpClientListener* p_listener){
    ASSERT(p_listener != nullptr);

    p_listener_ = p_listener;
    socket_manager_.Start(this, heartbeat_interval_in_seconds_*1000);

    // p_rx_buffer_ = new TByteBuffer(kSocketRxBufferSize);
    return TRUE;
}
bool TcpClient::Stop() { return (0 == socket_manager_.Stop()); }

void TcpClient::OnConnect(SOCKET socket_id) {
    cout << "OnConnect" << endl;
    p_listener_->OnConnect(socket_id);
}
void TcpClient::OnDisconnect(SOCKET socket_id) {
    cout << "OnDisconnect" << endl;
    p_listener_->OnDisconnect(socket_id);
}
void TcpClient::OnTimer(uint64_t llExpirations) {
    timer_event_count_ += llExpirations;
    // Clear silence connections per one keep_alive_time
    if (heartbeat_interval_in_seconds_ > 0 &&
        keep_alive_time_in_seconds_ > heartbeat_interval_in_seconds_) {
        if (timer_event_count_ >= (keep_alive_time_in_seconds_ / heartbeat_interval_in_seconds_)) {
            timer_event_count_ = 0;
            DisconnectSilenceConnections(keep_alive_time_in_seconds_);
        }
    }
}

bool TcpClient::OnBeforeProcessIo(PVOID pv, uint32_t events) {
    if (SocketObject::IsInvalid((SocketObject*)pv)) {
        return FALSE;
    }

    SocketObject* p_sock_obj = (SocketObject*)pv;
    if (p_sock_obj->IsConnected()) return TRUE;

    // if (events && _EPOLL_ALL_ERROR_EVENTS) {
    //     // p_sock_obj->CloseSocket(sock_id);
    //     cout << "Error Event captured" << endl;
    //     return FALSE;
    // }
    cout << "EVENTS: " << hex << events << endl;

    SOCKET sock_id = p_sock_obj->SocketId();
    int optval;
    socklen_t len = sizeof(optval);
    if (getsockopt(sock_id, SOL_SOCKET, SO_ERROR, &optval, &len) < 0) {
        socket_manager_.CloseSocket(sock_id);
        cout << "Error in getsockopt() " << errno << ", " << strerror(errno) << endl;
        return FALSE;
    }

    if (optval == 0) {
        // means connect sucessfully
        uint32_t now = SocketManager::GetTime();
        p_sock_obj->SetConnected(now);
        OnConnect(sock_id);
        cout << "Connect() Successfully" << endl;
    } else {
        socket_manager_.CloseSocket(sock_id);
        cout << "Error in delayed connection() " << errno << ", " << strerror(errno) << endl;
        return FALSE;
    }
    return TRUE;
}
bool TcpClient::OnReadyRead(PVOID ptr, uint32_t events){
    // cout << "TcpClient::OnReadyRead" << endl;
    return HandleReceive((SocketObject*)ptr, events);
}
bool TcpClient::OnReadyWrite(PVOID ptr, uint32_t events){
    // cout << "TcpClient::OnReadyWrite" << endl;
    return HandleSend((SocketObject*)ptr, events);
}

bool TcpClient::HandleReceive(SocketObject* pSocketObj, int flag) {
    // ASSERT(SocketObject::IsValid(pSocketObj));
    if (SocketObject::IsInvalid(pSocketObj)) return FALSE;

    SOCKET sock_id = pSocketObj->SocketId();

    TByteBuffer* buffer = this->RxBuffer();
    pSocketObj->active_time_ = socket_manager_.GetTime();

    int reads = flag ? -1 : MAX_CONTINUE_READS;

    for (int i = 0; i < reads || reads < 0; i++) {
        int rc = (int)read(sock_id, buffer->Ptr(), buffer->Size());

        if (rc > 0) {
            if (pSocketObj->IsReceiveLimit()) {
                uint64_t time_ms = socket_manager_.GetTimeInMillisecond();
                pSocketObj->CountReceivedBytes(rc, time_ms);
                if (pSocketObj->IsReceiveOverflow()) {
                    cout << ">>>>>> TCP receive OVER FLOW >>>>>>" << endl;
                    return FALSE;
                }
            }
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
bool TcpClient::HandleSend(SocketObject* pSocketObj, int flag) {
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

int TcpClient::ConnectToRemote(sockaddr* p_sockaddr) {
    return socket_manager_.ConnectToRemote(p_sockaddr);
}
int TcpClient::AsynchConnectTo(sockaddr* p_sockaddr) {
    return socket_manager_.AsynchConnectTo(p_sockaddr);
}

int TcpClient::Disconnect(SOCKET socket_id) { return socket_manager_.Disconnect(socket_id); }
int TcpClient::Send(SOCKET socket_id, void* buf, size_t nbytes) {
    return socket_manager_.Send(socket_id, buf, nbytes);
}

bool TcpClient::DisconnectSilenceConnections(uint64_t period) {
    return socket_manager_.DisconnectSilenceConnections(period);
}

void TcpClient::SetReceiveBytesLimit(SOCKET socket_id, uint32_t nbytes) {
    socket_manager_.SetReceiveBytesLimit(socket_id, nbytes);
}
