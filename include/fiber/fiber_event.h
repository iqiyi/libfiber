/**
 * Copyright (C) 2015-2018
 * All rights reserved.
 *
 * AUTHOR(S)
 *   Zheng Shuxin
 *   E-mail: shuxin.zheng@qq.com
 * 
 * VERSION
 *   Tue 27 Feb 2018 09:09:24 PM CST
 */

#ifndef FIBER_EVENT_INCLUDE_H
#define FIBER_EVENT_INCLUDE_H

#include "fiber_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/* fiber_event.c */

/* 线程安全的协程锁，可以用在不同线程的协程之间及不同线程之间的互斥 */
typedef struct ACL_FIBER_EVENT ACL_FIBER_EVENT;

/**
 * 创建基于事件的协程/线程混合锁
 * @return {ACL_FIBER_EVENT *}
 */
FIBER_API ACL_FIBER_EVENT *acl_fiber_event_create(void);

/**
 * 释放事件锁
 * @param {ACL_FIBER_EVENT *}
 */
FIBER_API void acl_fiber_event_free(ACL_FIBER_EVENT *event);

/**
 * 等待事件锁可用
 * @param {ACL_FIBER_EVENT *}
 * @return {int} 返回 0 表示成功，-1 表示出错
 */
FIBER_API int acl_fiber_event_wait(ACL_FIBER_EVENT *event);

/**
 * 尝试等待事件锁可用
 * @param {ACL_FIBER_EVENT *}
 * @return {int} 返回 0 表示成功，-1 表示锁被占用
 */
FIBER_API int acl_fiber_event_trywait(ACL_FIBER_EVENT *event);

/**
 * 事件锁拥有者通知等待者事件锁可用，则等待者收到通知后则可获得事件锁
 * @param {ACL_FIBER_EVENT *}
 * @return {int} 返回 0 表示成功，-1 表示出错
 */
FIBER_API int acl_fiber_event_notify(ACL_FIBER_EVENT *event);

#ifdef __cplusplus
}
#endif

#endif
