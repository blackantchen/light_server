/*
 * File: socket_manager.h
 * Created Date: May 11th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#ifndef __SOCKET_MANAGER_H__
#define __SOCKET_MANAGER_H__

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <map>
#include <pthread.h>
#include <mutex>
#include <queue>

#include "common_definitions.h"
#include "socket_object.h"
#include "byte_buffer.h"

using namespace std;

#define MAX_CONTINUE_READS 30

#define _EPOLL_ALL_ERROR_EVENTS		(EPOLLERR | EPOLLHUP)


struct TSockMCommand {
    uint32_t cmd;
    union cmd_data {
        int fd;
        uint32_t u32;
        uint64_t u64;
    } data;

    TSockMCommand(uint32_t _cmd, int _fd){
        cmd = _cmd;
        data.fd = _fd;
    }
};

enum EVT_COMMAND{
    ECMD_SEND,
    ECMD_DISCONNECT,
    ECMD_SHUTDOWN,
};

class IIOHandler
{
public:
	virtual void OnCommand(TSockMCommand* pCmd)						= 0;
	virtual void OnTimer(uint64_t llExpirations)						= 0;

	virtual bool OnBeforeProcessIo(PVOID pv, uint32_t events)			= 0;
	virtual void OnAfterProcessIo(PVOID pv, uint32_t events, bool rs)	= 0;
	virtual bool OnReadyRead(PVOID pv, uint32_t events)					= 0;
	virtual bool OnReadyWrite(PVOID pv, uint32_t events)				= 0;
	virtual bool OnHungUp(PVOID pv, uint32_t events)					= 0;
	virtual bool OnError(PVOID pv, uint32_t events)						= 0;
	virtual bool OnReadyPrivilege(PVOID pv, uint32_t events)			= 0;

	// virtual void OnDispatchThreadStart(THR_ID tid)					= 0;
	// virtual void OnDispatchThreadEnd(THR_ID tid)					= 0;

    virtual void OnConnect(SOCKET socket_id) = 0;
    virtual void OnDisconnect(SOCKET socket_id) = 0;

public:
    virtual ~IIOHandler() = default;
};

class CIOHandler : public IIOHandler
{
public:
	virtual void OnCommand(TSockMCommand* pCmd)						override {}
	virtual void OnTimer(uint64_t llExpirations)						override {}

	virtual bool OnBeforeProcessIo(PVOID pv, uint32_t events)			override {return TRUE;}
	virtual void OnAfterProcessIo(PVOID pv, uint32_t events, bool rs)	override {}
	virtual bool OnReadyWrite(PVOID pv, uint32_t events)				override {return TRUE;}
	virtual bool OnHungUp(PVOID pv, uint32_t events)					override {return TRUE;}
	virtual bool OnError(PVOID pv, uint32_t events)						override {return TRUE;}
	virtual bool OnReadyPrivilege(PVOID pv, uint32_t events)			override {return TRUE;}
	// virtual void OnDispatchThreadStart(THR_ID tid)					override {}
	// virtual void OnDispatchThreadEnd(THR_ID tid)					override {}

    virtual void OnConnect(SOCKET socket_id) override {}
    virtual void OnDisconnect(SOCKET socket_id) override {}  
};

class SocketManager {
   private:
    /* data */
    using SocketObjectMap = std::map<SOCKET, SocketObject*>;
    // std::map<SOCKET, SocketObject*> sock_object_map_;
    SocketObjectMap sock_object_map_;
    mutex sock_map_mutex_; 

    FD epoll_fd_;
    FD exit_evt_fd_;
    FD cmd_evt_fd_;
    FD timer_evt_fd_;
    int max_events_;

    pthread_t work_thread_id_;

    std::queue<TSockMCommand*> cmd_queue_;

    IIOHandler* io_handler_;

   private:
    void Reset();
    bool HasStart();

    bool CtrlFD(FD fd, int op, uint32_t mask, PVOID pv);

    FD CreateTimer(uint64_t interval, uint64_t start_time = 0);
    bool ReadTimer(uint64_t* p_expiration);
    bool AddTimer(uint64_t interval);
    bool DelTimer(FD timer_fd);

    int StartWorkThread();
    int StopWorkThread();
    bool HasWorkStart();

    static void* WorkThread(void* arg);
    void DoWork();

    bool ProcessCommand(uint32_t events);
    bool ProcessExit(uint32_t events);
    bool ProcessTimer(uint32_t events);
    bool ProcessIo(PVOID pv, uint32_t events);

    bool DoProcessIo(PVOID pv, uint32_t events);

    void SocketMapLock() { sock_map_mutex_.lock(); }
    void SocketMapUnlock() { sock_map_mutex_.unlock(); }

    void TriggerCmdEvent(int cmd, FD fd);

   protected:
    // bool CreateSocketObject(IPV4 remote_ip, uint16_t remote_port);
    bool AddSocketObject(SOCKET sock, SocketObject* p_socket_obj);
    bool RemoveSocketObject(SocketObject* p_socket_obj);
    bool ReleaseSocketObjects();
    SocketObject* FindSocketObject(SOCKET socket_id);

   public:
    SocketManager();
    virtual ~SocketManager();

    int Start(IIOHandler* p_handler, uint64_t timer_interval = 0);
    int Stop(void);
    bool AddFD(FD fd, uint32_t mask, PVOID pv) { return CtrlFD(fd, EPOLL_CTL_ADD, mask, pv); }
    bool ModifyFD(FD fd, uint32_t mask, PVOID pv) { return CtrlFD(fd, EPOLL_CTL_MOD, mask, pv); }
    bool DeleteFD(FD fd) { return CtrlFD(fd, EPOLL_CTL_DEL, 0, nullptr); }

    int CloseSocket(SOCKET socket_id);

    /* Server Operations */
    // int AddListener();
    // int AddUdpListener();
    int AddUdpSocket(sockaddr* p_sockaddr, bool broadcast_en = true);
    int AddTcpSocket(SOCKET socket_id, sockaddr* from_addr);

    int ConnectToRemote(sockaddr* p_sockaddr);
    int AsynchConnectTo(sockaddr* p_sockaddr);
    int Disconnect(SOCKET socket_id);
    bool DisconnectSilenceConnections(uint64_t period);
    bool CloseAllSocket();

    void SetReceivePacketsLimit(SOCKET socket_id, uint32_t num_of_packets);
    void SetReceiveBytesLimit(SOCKET socket_id, uint32_t nbytes);

    /* Common Operations */
    int Send(SOCKET socket_id, void* buf, size_t nbytes);
    int UdpSend(SOCKET socket_id, sockaddr* to_addr, void* buf, size_t nbytes);

    static int SetNonBlocking(int sock);
    static uint32_t GetTime();
    uint64_t GetTimeInMillisecond();
    uint64_t TimespecToMillisecond(timespec ts){ return (((uint64_t)(ts.tv_sec)) * 1000 + ts.tv_nsec / 1000000);}
    void MillisecondToTimespec(uint64_t ms, timespec& ts) {
        ts.tv_sec = (time_t)(ms / 1000);
        ts.tv_nsec = (ms % 1000) * 1000000;
    }
};

#endif  // __SOCKET_MANAGER_H__
