/*
 * File: common_definitions.h
 * Created Date: May 9th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#ifndef __COMMON_TYPEDEF_H__
#define __COMMON_TYPEDEF_H__
#include <assert.h>

typedef int SOCKET;
typedef int FD;
typedef unsigned int IPV4;
typedef void* PVOID;

#if !defined(THR_ID)
#define THR_ID pthread_t
#endif

#if !defined(FALSE)
#define FALSE false
#endif // FALSE

#if !defined(TRUE)
#define TRUE true
#endif // TRUE

#define INVALID_FD  -1
#define INVALID_SOCKET -1

#define SOCKET_ERROR -1

/* functions */
#define IS_VALID_FD(fd)					((fd) != INVALID_FD)
#define IS_INVALID_FD(fd)				(!IS_VALID_FD(fd))

#define CHECK_ERROR_FD(fd)				{if(IS_INVALID_FD(fd)) return FALSE;}

#define ASSERT assert

#endif // __COMMON_TYPEDEF_H__
