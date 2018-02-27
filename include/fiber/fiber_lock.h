/**
 * Copyright (C) 2015-2018
 * All rights reserved.
 *
 * AUTHOR(S)
 *   Zheng Shuxin
 *   E-mail: shuxin.zheng@qq.com
 * 
 * VERSION
 *   Tue 27 Feb 2018 09:05:09 PM CST
 */

#ifndef FIBER_LOCK_INCLUDE_H
#define FIBER_LOCK_INCLUDE_H

#include "fiber_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/* fiber locking */

/**
 * 协程互斥锁，线程非安全，只能用在同一线程内
 */
typedef struct ACL_FIBER_MUTEX ACL_FIBER_MUTEX;

/**
 * 协程读写锁，线程非安全，只能用在同一线程内
 */
typedef struct ACL_FIBER_RWLOCK ACL_FIBER_RWLOCK;

/**
 * 创建协程互斥锁，线程非安全，只能用在同一线程内
 * @return {ACL_FIBER_MUTEX*}
 */
FIBER_API ACL_FIBER_MUTEX* acl_fiber_mutex_create(void);

/**
 * 释放协程互斥锁
 * @param l {ACL_FIBER_MUTEX*} 由 acl_fiber_mutex_create 创建的协程互斥锁
 */
FIBER_API void acl_fiber_mutex_free(ACL_FIBER_MUTEX* l);

/**
 * 对协程互斥锁进行阻塞式加锁，如果加锁成功则返回，否则则阻塞
 * @param l {ACL_FIBER_MUTEX*} 由 acl_fiber_mutex_create 创建的协程互斥锁
 */
FIBER_API void acl_fiber_mutex_lock(ACL_FIBER_MUTEX* l);

/**
 * 对协程互斥锁尝试性进行加锁，无论是否成功加锁都会立即返回
 * @param l {ACL_FIBER_MUTEX*} 由 acl_fiber_mutex_create 创建的协程互斥锁
 * @return {int} 如果加锁成功则返回 0 值，否则返回 -1
 */
FIBER_API int acl_fiber_mutex_trylock(ACL_FIBER_MUTEX* l);

/**
 * 加锁成功的协程调用本函数进行解锁，调用本函数的协程必须是该锁的属主，否则
 * 内部会产生断言
 * @param l {ACL_FIBER_MUTEX*} 由 acl_fiber_mutex_create 创建的协程互斥锁
 */
FIBER_API void acl_fiber_mutex_unlock(ACL_FIBER_MUTEX* l);

/**
 * 创建协程读写锁，线程非安全，只能用在同一线程内
 * @return {ACL_FIBER_RWLOCK*}
 */
FIBER_API ACL_FIBER_RWLOCK* acl_fiber_rwlock_create(void);

/**
 * 释放协程读写锁
 * @param l {ACL_FIBER_RWLOCK*} 由 acl_fiber_rwlock_create 创建的读写锁
 */
FIBER_API void acl_fiber_rwlock_free(ACL_FIBER_RWLOCK* l);

/**
 * 对协程读写锁加读锁，如果该锁当前正被其它协程加了读锁，则本协程依然可以
 * 正常加读锁，如果该锁当前正被其它协程加了写锁，则本协程进入阻塞状态，直至
 * 写锁释放
 * @param l {ACL_FIBER_RWLOCK*} 由 acl_fiber_rwlock_create 创建的读写锁
 */
FIBER_API void acl_fiber_rwlock_rlock(ACL_FIBER_RWLOCK* l);

/**
 * 对协程读写锁尝试性加读锁，加锁无论是否成功都会立即返回
 * @param l {ACL_FIBER_RWLOCK*} 由 acl_fiber_rwlock_create 创建的读写锁
 * @retur {int} 返回 1 表示加锁成功，返回 0 表示加锁失败
 */
FIBER_API int acl_fiber_rwlock_tryrlock(ACL_FIBER_RWLOCK* l);

/**
 * 对协程读写锁加写锁，只有当该锁未被任何协程加读/写锁时才会返回，否则阻塞，
 * 直至该锁可加写锁
 * @param l {ACL_FIBER_RWLOCK*} 由 acl_fiber_rwlock_create 创建的读写锁
 */
FIBER_API void acl_fiber_rwlock_wlock(ACL_FIBER_RWLOCK* l);

/**
 * 对协程读写锁尝试性加写锁，无论是否加锁成功都会立即返回
 * @param l {ACL_FIBER_RWLOCK*} 由 acl_fiber_rwlock_create 创建的读写锁
 * @return {int} 返回 1 表示加写锁成功，返回 0 表示加锁失败
 */
FIBER_API int acl_fiber_rwlock_trywlock(ACL_FIBER_RWLOCK* l);

/**
 * 对协程读写锁成功加读锁的协程调用本函数解读锁，调用者必须是之前已成功加读
 * 锁成功的协程
 * @param l {ACL_FIBER_RWLOCK*} 由 acl_fiber_rwlock_create 创建的读写锁
 */
FIBER_API void acl_fiber_rwlock_runlock(ACL_FIBER_RWLOCK* l);
/**
 * 对协程读写锁成功加写锁的协程调用本函数解写锁，调用者必须是之前已成功加写
 * 锁成功的协程
 * @param l {ACL_FIBER_RWLOCK*} 由 acl_fiber_rwlock_create 创建的读写锁
 */
FIBER_API void acl_fiber_rwlock_wunlock(ACL_FIBER_RWLOCK* l);

#ifdef __cplusplus
}
#endif

#endif
