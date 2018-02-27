/**
 * Copyright (C) 2015-2018
 * All rights reserved.
 *
 * AUTHOR(S)
 *   Zheng Shuxin
 *   E-mail: shuxin.zheng@qq.com
 * 
 * VERSION
 *   Tue 27 Feb 2018 09:14:19 PM CST
 */

#ifndef FIBER_CHANNEL_INCLUDE_H
#define FIBER_CHANNEL_INCLUDE_H

#include "fiber_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/* channel communication */

/**
 * 协程间通信的管道
 */
typedef struct ACL_CHANNEL ACL_CHANNEL;

/**
 * 创建协程通信管道
 * @param elemsize {int} 在 ACL_CHANNEL 进行传输的对象的固定尺寸大小（字节）
 * @param bufsize {int} ACL_CHANNEL 内部缓冲区大小，即可以缓存 elemsize 尺寸大小
 *  对象的个数
 * @return {CHANNNEL*}
 */
FIBER_API ACL_CHANNEL* acl_channel_create(int elemsize, int bufsize);

/**
 * 释放由 acl_channel_create 创建的协程通信管道对象
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 */
FIBER_API void acl_channel_free(ACL_CHANNEL* c);

/**
 * 阻塞式向指定 ACL_CHANNEL 中发送指定的对象地址
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 * @param v {void*} 被发送的对象地址
 * @return {int} 返回值 >= 0
 */
FIBER_API int acl_channel_send(ACL_CHANNEL* c, void* v);

/**
 * 非阻塞式向指定 ACL_CHANNEL 中发送指定的对象，内部会根据 acl_channel_create 中指定
 * 的 elemsize 对象大小进行数据拷贝
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 * @param v {void*} 被发送的对象地址
 */
FIBER_API int acl_channel_send_nb(ACL_CHANNEL* c, void* v);

/**
 * 从指定的 ACL_CHANNEL 中阻塞式读取对象，
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 * @param v {void*} 存放结果内容
 * @return {int} 返回值 >= 0 表示成功读到数据
 */
FIBER_API int acl_channel_recv(ACL_CHANNEL* c, void* v);

/**
 * 从指定的 ACL_CHANNEL 中非阻塞式读取对象，无论是否读到数据都会立即返回
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 * @param v {void*} 存放结果内容
 * @return {int} 返回值 >= 0 表示成功读到数据，否则表示未读到数据
 */
FIBER_API int acl_channel_recv_nb(ACL_CHANNEL* c, void* v);

/**
 * 向指定的 ACL_CHANNEL 中阻塞式发送指定对象的地址
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 * @param v {void*} 被发送对象的地址
 * @return {int} 返回值 >= 0
 */
FIBER_API int acl_channel_sendp(ACL_CHANNEL* c, void* v);

/**
 * 从指定的 CHANNLE 中阻塞式接收由 acl_channel_sendp 发送的对象的地址
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 * @return {void*} 返回非 NULL，指定接收到的对象的地址
 */
FIBER_API void *acl_channel_recvp(ACL_CHANNEL* c);

/**
 * 向指定的 ACL_CHANNEL 中非阻塞式发送指定对象的地址
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 * @param v {void*} 被发送对象的地址
 * @return {int} 返回值 >= 0
 */
FIBER_API int acl_channel_sendp_nb(ACL_CHANNEL* c, void* v);

/**
 * 从指定的 CHANNLE 中阻塞式接收由 acl_channel_sendp 发送的对象的地址
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 * @return {void*} 返回非 NULL，指定接收到的对象的地址，如果返回 NULL 表示
 *  没有读到任何对象
 */
FIBER_API void *acl_channel_recvp_nb(ACL_CHANNEL* c);

/**
 * 向指定的 ACL_CHANNEL 中发送无符号长整形数值
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 * @param val {unsigned long} 要发送的数值
 * @return {int} 返回值 >= 0
 */
FIBER_API int acl_channel_sendul(ACL_CHANNEL* c, unsigned long val);

/**
 * 从指定的 ACL_CHANNEL 中接收无符号长整形数值
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 * @return {unsigned long}
 */
FIBER_API unsigned long acl_channel_recvul(ACL_CHANNEL* c);

/**
 * 向指定的 ACL_CHANNEL 中以非阻塞方式发送无符号长整形数值
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 * @param val {unsigned long} 要发送的数值
 * @return {int} 返回值 >= 0
 */
FIBER_API int acl_channel_sendul_nb(ACL_CHANNEL* c, unsigned long val);

/**
 * 从指定的 ACL_CHANNEL 中以非阻塞方式接收无符号长整形数值
 * @param c {ACL_CHANNEL*} 由 acl_channel_create 创建的管道对象
 * @return {unsigned long}
 */
FIBER_API unsigned long acl_channel_recvul_nb(ACL_CHANNEL* c);

#ifdef __cplusplus
}
#endif

#endif
