/*
 * File: socket_manager.cpp
 * Created Date: May 11th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */

#include "socket_manager.h"

#include <unistd.h>
#include <sys/timerfd.h>
#include <iostream>
#include <memory>
#include <fcntl.h>
#include <errno.h>

#ifdef _DEBUG_EN
#include "../memory_leak_detector/debug_new.h"
#endif

SocketManager::SocketManager() { Reset(); }
SocketManager::~SocketManager() {
}

int SocketManager::Start(IIOHandler* p_handler, uint64_t timer_interval) {
    if (HasStart()) {
        cout << "Error: Socket Manager has started" << endl;
        return -1;
    }

    ASSERT(p_handler!=nullptr);
    io_handler_ = p_handler;

    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
    if (IS_INVALID_FD(epoll_fd_)) return -1;

    do {
        cmd_evt_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (IS_INVALID_FD(cmd_evt_fd_)) break;
        if (AddFD(cmd_evt_fd_, EPOLLIN | EPOLLET, &cmd_evt_fd_) == false) break;

        exit_evt_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (IS_INVALID_FD(exit_evt_fd_)) break;
        if (AddFD(exit_evt_fd_, EPOLLIN | EPOLLET, &exit_evt_fd_) == false) break;

        if (timer_interval > 0) {
            if (AddTimer(timer_interval) == false) break;
        }

        if (StartWorkThread()) {
            cout << "Create Work Thread failure" << endl;
            break;
        }

        return 0;
    } while (0);

    Stop();
    return -1;
}
int SocketManager::Stop(void) {
    if(!cmd_queue_.empty()){
        // if cmd_queue not empty, then wait for 100ms
        usleep(1000*100);
    }
    
    if (HasWorkStart()) {
        StopWorkThread();
    }

    if (HasStart() == false) {
        return -1;
    }

    TSockMCommand* p_cmd;
    while (!cmd_queue_.empty()) {
        p_cmd = cmd_queue_.front();
        if (p_cmd != nullptr) {
            delete p_cmd;
        }
        cmd_queue_.pop();
    }

    ReleaseSocketObjects();

    if (IS_VALID_FD(cmd_evt_fd_)) {
        DeleteFD(cmd_evt_fd_);
        close(cmd_evt_fd_);
    }
    if (IS_VALID_FD(exit_evt_fd_)) {
        DeleteFD(exit_evt_fd_);
        close(exit_evt_fd_);
    }
    if (IS_VALID_FD(timer_evt_fd_)) {
        DelTimer(timer_evt_fd_);
    }

    if (IS_VALID_FD(epoll_fd_)) close(epoll_fd_);

    Reset();

    return 0;
}
bool SocketManager::HasStart() { return (epoll_fd_ != INVALID_FD); }

void SocketManager::Reset() {
    epoll_fd_ = INVALID_FD;
    exit_evt_fd_ = INVALID_FD;
    cmd_evt_fd_ = INVALID_FD;
    timer_evt_fd_ = INVALID_FD;
    work_thread_id_ = 0;
    max_events_ = 6;
}

int SocketManager::SetNonBlocking(int sock) {
    int opts;
    opts = fcntl(sock, F_GETFL);

    if (opts < 0) {
        perror("fcntl(sock, GETFL)");
        return -1;
    }

    opts = opts | O_NONBLOCK;

    if (fcntl(sock, F_SETFL, opts) < 0) {
        perror("fcntl(sock, SETFL, opts)");
        return -1;
    }
    return 0;
}

bool SocketManager::CtrlFD(FD fd, int op, uint32_t mask, PVOID pv) {
    epoll_event evt = {mask, pv};
    if (epoll_ctl(epoll_fd_, op, fd, &evt) == -1)
        return false;
    else
        return true;
}

FD SocketManager::CreateTimer(uint64_t interval, uint64_t start_time){
    struct itimerspec new_value;
    struct timespec now;

    if (start_time == 0) {
        if (clock_gettime(CLOCK_MONOTONIC, &now) == -1) {
            return -1;
        }
    }else{
        MillisecondToTimespec(start_time, now);
    }

    new_value.it_value.tv_sec = now.tv_sec + 1;
    new_value.it_value.tv_nsec = now.tv_nsec;

    MillisecondToTimespec(interval, new_value.it_interval);

    FD timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timer_fd == -1) {
        return -1;
    }

    if (timerfd_settime(timer_fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1) {
        return -1;
    }

    return timer_fd;
}
bool SocketManager::ReadTimer(uint64_t* p_expiration){
    ssize_t s = read(timer_evt_fd_, p_expiration, sizeof(uint64_t));
    if(s != sizeof(uint64_t))
        return false;

    return true; 
}
bool SocketManager::AddTimer(uint64_t interval){
    FD time_fd = CreateTimer(interval);

    if( AddFD(time_fd, EPOLLIN | EPOLLET, &timer_evt_fd_) == false){
        close(time_fd);
        timer_evt_fd_ = INVALID_FD;
        return false;
    }
    timer_evt_fd_ = time_fd;
    return true;
}
bool SocketManager::DelTimer(FD timer_fd){
    if(IS_VALID_FD(timer_fd)){
        DeleteFD(timer_fd);
        close(timer_fd);
    }
    return true;
}

void* SocketManager::WorkThread(void* arg) {
    SocketManager* p_sock_manager = (SocketManager*)arg;
    p_sock_manager->DoWork();
    pthread_exit((void*)66);
    return p_sock_manager;
}

void SocketManager::DoWork() {
    bool bRun = true;
    // shared_ptr<epoll_event> pEvents = make_shared<epoll_event>(max_events_);
    epoll_event* pEvents = new epoll_event[max_events_];
    cout << "SocketManager::DoWork()" << endl;

    while (bRun) {
        int rs = epoll_pwait(epoll_fd_, pEvents, max_events_, -1, nullptr);

        if (rs == -1) {
            bRun = false;
            break;
        }

        for (int i = 0; i < rs; i++) {
            uint32_t events = pEvents[i].events;
            PVOID ptr = pEvents[i].data.ptr;

            if (ptr == &cmd_evt_fd_) 
                ProcessCommand(events);
            else if (ptr == &timer_evt_fd_)
                ProcessTimer(events);
            else if (ptr == &exit_evt_fd_)
                bRun = ProcessExit(events);
            else
                ProcessIo(ptr, events);
        }
    }

    if (pEvents != nullptr) {
        delete[] pEvents;
        pEvents = nullptr;
    }
}

int SocketManager::StartWorkThread() {
    if (pthread_create(&work_thread_id_, NULL, WorkThread, this)) {
        work_thread_id_ = 0;
        return -1;
    }

    return 0;
}
int SocketManager::StopWorkThread() {
    // Send Exit signal to exit_evt_fd_
    eventfd_write(exit_evt_fd_, 6);

    return pthread_join(work_thread_id_, NULL);
}

bool SocketManager::HasWorkStart() {
    if (work_thread_id_ > 0)
        return true;
    else
        return false;
}

bool SocketManager::ProcessCommand(uint32_t events) {
    // if received this event, send msg to remote
    if (!(events & EPOLLIN)) return false;

    int count = 20;
    TSockMCommand* p_cmd;
    while (!cmd_queue_.empty() && count) {
        p_cmd = cmd_queue_.front();
        FD sock = p_cmd->data.fd;
        switch (p_cmd->cmd) {
            case ECMD_SEND: {
                SocketObject* p_sock_obj = FindSocketObject(sock);
                if (p_sock_obj != nullptr) {
                    ProcessIo((PVOID)p_sock_obj, EPOLLOUT);
                }
            } break;
            case ECMD_DISCONNECT: {
                RemoveSocketObject(FindSocketObject(sock));
            } break;

            default:
                break;
        }
        if (p_cmd != nullptr) {
            delete p_cmd;
        }
        cmd_queue_.pop();

        count--;
    }
    return true;
}

bool SocketManager::ProcessExit(uint32_t events) { return false; }
bool SocketManager::ProcessTimer(uint32_t events) {
    if (!(events & EPOLLIN)) return false;

    uint64_t exp;
    if (ReadTimer(&exp)) {
        io_handler_->OnTimer(exp);
        return true;
    } else {
        return false;
    }
}
bool SocketManager::ProcessIo(PVOID pv, uint32_t events) {
    if (io_handler_->OnBeforeProcessIo(pv, events) == FALSE) {
        return FALSE;
    }

    bool rs = DoProcessIo(pv, events);
    io_handler_->OnAfterProcessIo(pv, events, rs);
    return rs;
}

bool SocketManager::DoProcessIo(PVOID pv, uint32_t events) {
    // if(events & EPOLLERR)
    // 	return m_pHandler->OnError(pv, events);
    // if((events & EPOLLPRI) && !m_pHandler->OnReadyPrivilege(pv, events))
    // 	return FALSE;
    if ((events & EPOLLIN) && !io_handler_->OnReadyRead(pv, events))
        return FALSE;
    if ((events & EPOLLOUT) && !io_handler_->OnReadyWrite(pv, events))
        return FALSE;
    // if((events & (EPOLLHUP | EPOLLRDHUP)) && !m_pHandler->OnHungUp(pv, events))
    // 	return FALSE;
    return TRUE;
}

void SocketManager::TriggerCmdEvent(int cmd, FD fd){
    if (HasStart() == false) {
        return;
    }

    TSockMCommand* p_cmd = new TSockMCommand(cmd, fd);
    cmd_queue_.push(p_cmd);

    if (cmd_evt_fd_ != -1) {
        eventfd_write(cmd_evt_fd_, 6);
    }
}

int SocketManager::ConnectToRemote(sockaddr* p_sockaddr) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        cout << "Failue to create tcp socket" << endl;
        return -1;
    }

    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    if (connect(sock, p_sockaddr, sizeof(sockaddr)) < 0) {
        close(sock);
        return -1;
    } else {
        SetNonBlocking(sock);
        // Add new socket object to socketobject map and epoll wait list
        SocketObject* p_sock_obj = new SocketObject();
        p_sock_obj->SetRemoteAddr(p_sockaddr);
        uint32_t now = GetTime();
        p_sock_obj->SetConnected(now);
        AddSocketObject(sock, p_sock_obj);
        io_handler_->OnConnect(sock);
    }
    return sock;
}

int SocketManager::AsynchConnectTo(sockaddr* p_sockaddr) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        cout << "Failue to create tcp socket" << endl;
        return INVALID_SOCKET;
    }

    SetNonBlocking(sock);
    if (connect(sock, p_sockaddr, sizeof(sockaddr)) < 0) {
        if (errno == EINPROGRESS) {
            SocketObject* p_sock_obj = new SocketObject();
            p_sock_obj->SetRemoteAddr(p_sockaddr);
            AddSocketObject(sock, p_sock_obj);
        } else {
            cout << "Error in connecting " << errno << ", " << strerror(errno) << endl;
        }
    } else {
        // connect successfully
        // Add new socket object to socketobject map and epoll wait list
        SocketObject* p_sock_obj = new SocketObject();
        p_sock_obj->SetRemoteAddr(p_sockaddr);
        uint32_t now = GetTime();
        p_sock_obj->SetConnected(now);
        AddSocketObject(sock, p_sock_obj);
        io_handler_->OnConnect(sock);
    }
    return sock;
}

int SocketManager::AddUdpSocket(sockaddr* p_sockaddr, bool broadcast_en) {
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        cout << "Failue to create UDP socket" << endl;
        return -1;
    }

    do {
        int opts = 1;
        SetNonBlocking(sock);
        // if (setsockopt(sock, SOL_SOCKET, (broadcast_en ? SO_BROADCAST : 0),
        //                &opts, sizeof(opts)) < 0)
        if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opts, sizeof(opts)) < 0)
            break;

        if (bind(sock, (struct sockaddr*)p_sockaddr, sizeof(sockaddr)) >= 0) {
            SocketObject* p_sock_obj = new SocketObject();
            uint32_t now = GetTime();
            p_sock_obj->SetConnected(now);
            AddSocketObject(sock, p_sock_obj);
            io_handler_->OnConnect(sock);
            return sock;
        }
    } while (0);

    close(sock);
    return -1;
}
int SocketManager::AddTcpSocket(SOCKET socket_id, sockaddr* from_addr) {
    if(socket_id == INVALID_SOCKET){
        return -1;
    }
    
    SocketObject* p_sock_obj = new SocketObject();
    p_sock_obj->SetRemoteAddr(from_addr);
    uint32_t now = GetTime();
    p_sock_obj->SetConnected(now);
    if (AddSocketObject(socket_id, p_sock_obj) == FALSE)
        return -1;
    else
        return 0;
}

int SocketManager::Disconnect(SOCKET socket_id) {
    TriggerCmdEvent(ECMD_DISCONNECT, socket_id);
    return 0;
}

bool SocketManager::AddSocketObject(SOCKET sock, SocketObject* p_socket_obj) {
    if (sock_object_map_.count(sock) > 0) {
        return FALSE;
    }
    p_socket_obj->SetSocketId(sock);
    p_socket_obj->SetValid();
    sock_object_map_.insert(std::pair<SOCKET, SocketObject*>(sock, p_socket_obj));
    AddFD(sock, EPOLLIN | EPOLLOUT | EPOLLET, p_socket_obj);
    cout << "SocketManager::AddSocketObject, socket = " << dec << sock << endl;
    return TRUE;
}
bool SocketManager::RemoveSocketObject(SocketObject* p_socket_obj) {
    if (SocketObject::IsValid(p_socket_obj) == FALSE) return FALSE;

    p_socket_obj->SetInvalid();

    FD sock = p_socket_obj->SocketId();
    shutdown(sock, SHUT_RDWR);
    io_handler_->OnDisconnect(sock);

    SocketMapLock();
    DeleteFD(sock);
    close(sock);
    if (p_socket_obj != nullptr) {
        delete p_socket_obj;
    }
    sock_object_map_.erase(sock);
    SocketMapUnlock();
    return TRUE;
}
bool SocketManager::ReleaseSocketObjects() {
    for (SocketObjectMap::iterator it = sock_object_map_.begin(); it != sock_object_map_.end();
         ++it) {
        DeleteFD(it->first);
        if (it->second != nullptr) {
            delete it->second;
        }
    }
    sock_object_map_.clear();
    return true;
}
SocketObject* SocketManager::FindSocketObject(SOCKET socket_id){
    if (sock_object_map_.count(socket_id) == 0) return nullptr;

    return sock_object_map_.at(socket_id);
}
int SocketManager::CloseSocket(SOCKET socket_id){
    TriggerCmdEvent(ECMD_DISCONNECT, socket_id);
    return 0;
}

uint32_t SocketManager::GetTime() {
    timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != -1)
        return ts.tv_sec;
    else
        return 0;
}
uint64_t SocketManager::GetTimeInMillisecond() {
    timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != -1)
        return TimespecToMillisecond(ts);
    else
        return 0;
}

bool SocketManager::DisconnectSilenceConnections(uint64_t period) {
    uint32_t now = GetTime();

    SocketObject* p_sockobj;
    for (SocketObjectMap::iterator it = sock_object_map_.begin(); it != sock_object_map_.end();
         ++it) {
        p_sockobj = it->second;
        if (SocketObject::IsValid(p_sockobj) && (int)(now - p_sockobj->active_time_) >= (int)period)
            Disconnect(p_sockobj->SocketId());
    }
    return TRUE;
}
bool SocketManager::CloseAllSocket() {
    SocketObject* p_sockobj;
    for (SocketObjectMap::iterator it = sock_object_map_.begin();
         it != sock_object_map_.end(); ++it) {
        p_sockobj = it->second;
        if (SocketObject::IsValid(p_sockobj)) 
            Disconnect(p_sockobj->SocketId());
    }
    return TRUE;
}

void SocketManager::SetReceivePacketsLimit(SOCKET socket_id, uint32_t num_of_packets) {
    SocketObject* p_sock_obj = FindSocketObject(socket_id);
    if (SocketObject::IsInvalid(p_sock_obj)) {
        return;
    } else {
        p_sock_obj->SetReceivePacketLimit(num_of_packets);
    }
}
void SocketManager::SetReceiveBytesLimit(SOCKET socket_id, uint32_t nbytes) {
    SocketObject* p_sock_obj = FindSocketObject(socket_id);
    if (SocketObject::IsInvalid(p_sock_obj)) {
        return;
    } else {
        p_sock_obj->SetReceiveBytesLimit(nbytes);
    }
}
// BOOL CTcpServer::HandleAccept(UINT events)
// {
// 	if(events & _EPOLL_ALL_ERROR_EVENTS)
// 	{
// 		VERIFY(!HasStarted());
// 		return FALSE;
// 	}

// 	while(TRUE)
// 	{
// 		HP_SOCKADDR addr;

// 		socklen_t addrLen	= (socklen_t)addr.AddrSize();
// 		SOCKET soClient		= ::accept(m_soListen, addr.Addr(), &addrLen);

// 		if(soClient == INVALID_SOCKET)
// 		{
// 			int code = ::WSAGetLastError();

// 			if(code == ERROR_WOULDBLOCK)
// 				return TRUE;
// 			else if(code == ERROR_CONNABORTED)
// 				continue;
// 			else if(code == ERROR_HANDLES_CLOSED)
// 				return FALSE;
			
// 			ERROR_EXIT2(EXIT_CODE_SOFTWARE, code);
// 		}

// 		VERIFY(::fcntl_SETFL(soClient, O_NOATIME | O_NONBLOCK | O_CLOEXEC));

// 		CONNID dwConnID = 0;

// 		if(!m_bfActiveSockets.AcquireLock(dwConnID))
// 		{
// 			::ManualCloseSocket(soClient, SHUT_RDWR);
// 			continue;
// 		}

// 		TSocketObj* pSocketObj = GetFreeSocketObj(dwConnID, soClient);

// 		AddClientSocketObj(dwConnID, pSocketObj, addr);

// 		if(TRIGGER(FireAccept(pSocketObj)) == HR_ERROR)
// 		{
// 			AddFreeSocketObj(pSocketObj, SCF_NONE);
// 			continue;
// 		}

// 		UINT evts = (pSocketObj->IsPending() ? EPOLLOUT : 0) | (pSocketObj->IsPaused() ? 0 : EPOLLIN);
// 		VERIFY(m_ioDispatcher.AddFD(pSocketObj->socket, evts | EPOLLRDHUP | EPOLLONESHOT, pSocketObj));
// 	}

// 	return TRUE;
// }

int SocketManager::Send(SOCKET socket_id, void* buf, size_t nbytes) {
    if (sock_object_map_.count(socket_id) == 0) return SOCKET_ERROR;

    SocketObject* pSockObj = sock_object_map_.at(socket_id);
    if (SocketObject::IsInvalid(pSockObj)) return SOCKET_ERROR;

    if (pSockObj->PushMsg(buf, nbytes) == SOCKET_ERROR) return SOCKET_ERROR;

    TriggerCmdEvent(ECMD_SEND, socket_id);
    return 0;
}
int SocketManager::UdpSend(SOCKET socket_id, sockaddr* to_addr, void* buf, size_t nbytes){
    if (sock_object_map_.count(socket_id) == 0) return SOCKET_ERROR;

    SocketObject* pSockObj = sock_object_map_.at(socket_id);
    if (SocketObject::IsInvalid(pSockObj)) return SOCKET_ERROR;

    if (pSockObj->PushUdpMsg(buf, nbytes, to_addr) == SOCKET_ERROR)
        return SOCKET_ERROR;

    TriggerCmdEvent(ECMD_SEND, socket_id);
    return 0;
}
