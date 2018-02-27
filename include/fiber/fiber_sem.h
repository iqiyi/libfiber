/**
 * Copyright (C) 2015-2018
 * All rights reserved.
 *
 * AUTHOR(S)
 *   Zheng Shuxin
 *   E-mail: shuxin.zheng@qq.com
 * 
 * VERSION
 *   Tue 27 Feb 2018 09:11:38 PM CST
 */

#ifndef FIBER_SEM_INCLUDE_H
#define FIBER_SEM_INCLUDE_H

#include "fiber_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/* fiber semaphore */

typedef struct ACL_FIBER_SEM ACL_FIBER_SEM;

/**
 * 创建协程信号量，同时内部会将当前线程与该信号量绑定
 * @param num {int} 信号量初始值（必须 >= 0）
 * @return {ACL_FIBER_SEM *}
 */
FIBER_API ACL_FIBER_SEM* acl_fiber_sem_create(int num);

/**
 * 释放协程信号量
 * @param {ACL_FIBER_SEM *}
 */
FIBER_API void acl_fiber_sem_free(ACL_FIBER_SEM* sem);

/**
 * 获得当前协程信号量所绑定的线程 ID
 * @param sem {ACL_FIBER_SEM*} 协程信号量对象
 * @return {unsigned long}
 */
#if !defined(_WIN32) && !defined(_WIN64)
FIBER_API unsigned long acl_fiber_sem_get_tid(ACL_FIBER_SEM* sem);
#endif

/**
 * 设置指定协程信号量的的线程 ID，当改变本协程信号量所属的线程时如果等待的协程
 * 数据非 0 则内部自动 fatal，即当协程信号量上等待协程非空时禁止调用本方法
 * @param sem {ACL_FIBER_SEM*} 协程信号量对象
 * @param {unsigned long} 线程 ID
 */
FIBER_API void acl_fiber_sem_set_tid(ACL_FIBER_SEM* sem, unsigned long tid);

/**
 * 当协程信号量 > 0 时使信号量减 1，否则等待信号量 > 0
 * @param sem {ACL_FIBER_SEM *}
 * @retur {int} 返回信号量当前值，如果返回 -1 表明当前线程与协程信号量所属线程
 *  不是同一线程，此时该方法不等待立即返回
 */
FIBER_API int acl_fiber_sem_wait(ACL_FIBER_SEM* sem);

/**
 * 尝试使协程信号量减 1
 * @param sem {ACL_FIBER_SEM *}
 * @retur {int} 成功减 1 时返回值 >= 0，返回 -1 表示当前信号量不可用，或当前
 *  调用者线程与协程信号量所属线程不是同一线程
 */
FIBER_API int acl_fiber_sem_trywait(ACL_FIBER_SEM* sem);

/**
 * 使协程信号量加 1
 * @param sem {ACL_FIBER_SEM *}
 * @retur {int} 返回信号量当前值，返回 -1 表示当前调用者线程与协程信号量所属
 *  线程不是同一线程
 */
FIBER_API int acl_fiber_sem_post(ACL_FIBER_SEM* sem);

/**
 * 获得指定协程信号量的当前值，该值反映了目前等待该信号量的数量
 * @param sem {ACL_FIBER_SEM*}
 * @retur {int}
 */
FIBER_API int acl_fiber_sem_num(ACL_FIBER_SEM* sem);

#ifdef __cplusplus
}
#endif

#endif
