/*
 * File: t_auto_locker.h
 * Created Date: May 28th 2020
 * Author: Jerry
 * -----
 * Last Modified:
 * Modified By:
 * -----
 * Copyright (c) 2020 (efancc@163.com)
 */
#ifndef __T_AUTO_LOCKER_H__
#define __T_AUTO_LOCKER_H__

#include <mutex>

// using namespace std;

class TAutoLocker {
   private:
    std::mutex *p_mutex_obj_;

   public:
    TAutoLocker(std::mutex &mutex_obj) {
        p_mutex_obj_ = &mutex_obj;
        p_mutex_obj_->lock();
    }
    ~TAutoLocker() { p_mutex_obj_->unlock(); }
};

#endif  // __T_AUTO_LOCKER_H__
