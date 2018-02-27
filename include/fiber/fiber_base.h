#ifndef FIBER_BASE_INCLUDE_H
#define FIBER_BASE_INCLUDE_H

#include "fiber_define.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 设置是否需要 hook 系统中的 IO 相关的 API，内部缺省值为 1
 * @param onoff {int} 是否需要 hook
 */
FIBER_API void acl_fiber_hook_api(int onoff);

/**
 * 创建一个协程
 * @param fn {void (*)(ACL_FIBER*, void*)} 协程运行时的回调函数地址
 * @param arg {void*} 回调 fn 函数时的第二个参数
 * @param size {size_t} 所创建协程所占栈空间大小
 * @return {ACL_FIBER*}
 */
FIBER_API ACL_FIBER* acl_fiber_create(void (*fn)(ACL_FIBER*, void*),
	void* arg, size_t size);

/**
 * 返回当前线程中处于消亡状态的协程数
 * @retur {int}
 */
FIBER_API int acl_fiber_ndead(void);

FIBER_API void acl_fiber_check_timer(size_t max);

/**
 * 返回当前正在运行的协程对象
 * @retur {ACL_FIBER*} 返回 NULL 表示当前没有正在运行的协程
 */
FIBER_API ACL_FIBER* acl_fiber_running(void);

/**
 * 获得所给协程的协程 ID 号
 * @param fiber {const ACL_FIBER*} acl_fiber_create 创建的协程对象，必须非空
 * @return {unsigned int} 协程 ID 号
 */
FIBER_API unsigned int acl_fiber_id(const ACL_FIBER* fiber);

/**
 * 获得当前所运行的协程的 ID 号
 * @return {unsigned int} 当前运行协程的 ID 号
 */
FIBER_API unsigned int acl_fiber_self(void);

/**
 * 设置所给协程的错误号
 * @param fiber {ACL_FIBER*} 指定的协程对象，为 NULL 则使用当前运行的协程
 * @param errnum {int} 错误号
 */
FIBER_API void acl_fiber_set_errno(ACL_FIBER* fiber, int errnum);

/**
 * 获得指定协程的错误号
 * @param fiber {ACL_FIBER*} 指定的协程对象，若为 NULL 则使用当前协程对象
 * @return {int} 所给协程错误号
 */
FIBER_API int acl_fiber_errno(ACL_FIBER* fiber);

/**
 * 获得当前系统级的 errno 号
 * @return {int}
 */
FIBER_API int acl_fiber_sys_errno(void);

/**
 * 设置当前系统的 errno 号
 * @param errnum {int}
 */
FIBER_API void acl_fiber_sys_errno_set(int errnum);

/**
 * 是否保持所指定协程的错误号，当设置为“保持”后，则该协程仅保持当前状态下的
 * 错误号，之后该协程的错误号 errno 将不再改变，走到再次调用本函数取消保持
 * @param fiber {ACL_FIBER*} 指定的协程对象，为 NULL 则使用当前运行的协程
 * @param yesno {int} 是否保持
 */
FIBER_API void acl_fiber_keep_errno(ACL_FIBER* fiber, int yesno);

/**
 * 获得指定协程的当前状态
 * @param fiber {const ACL_FIBER*} 指定的协程对象，为 NULL 则使用当前协程
 * @return {int} 协程状态
 */
FIBER_API int acl_fiber_status(const ACL_FIBER* fiber);

/**
 * 通知处于休眠状态的协程退出 
 * @param fiber {const ACL_FIBER*} 指定的协程对象，必须非 NULL
 */
FIBER_API void acl_fiber_kill(ACL_FIBER* fiber);

/**
 * 检查本协程是否被其它协程通知退出
 * @param fiber {const ACL_FIBER*} 指定的协程对象，若为 NULL 则自动使用当前
 *  正在运行的协程
 * @return {int} 返回值为 0 表示没有被通知退出，非 0 表示被通知退出
 */
FIBER_API int acl_fiber_killed(ACL_FIBER* fiber);

/**
 * 唤醒因 IO 等原因处于休眠的协程
 * @param fiber {const ACL_FIBER*} 协程对象，必须非 NULL
 * @param signum {int} SIGINT, SIGKILL, SIGTERM ... 参考系统中 bits/signum.h
 */
FIBER_API void acl_fiber_signal(ACL_FIBER* fiber, int signum);

/**
 * 获得其它协程发送给指定协程的信号值
 * @param fiber {const ACL_FIBER*} 指定的协程对象，为 NULL 时则使用当前协程
 * @retur {int} 返回指定协程收到的信号值
 */
FIBER_API int acl_fiber_signum(ACL_FIBER* fiber);

/**
 * 将当前运行的协程挂起，由调度器选择下一个需要运行的协程
 * @return {int}
 */
FIBER_API int acl_fiber_yield(void);

/**
 * 将指定协程对象置入待运行队列中
 * @param fiber {ACL_FIBER*} 指定协程，必须非 NULL
 */
FIBER_API void acl_fiber_ready(ACL_FIBER* fiber);

/**
 * 将当前运行的协程挂起，同时执行等待队列下一个待运行的协程
 */
FIBER_API void acl_fiber_switch(void);

/**
 * 设置当前线程的协程调度器是否为自启动模式，自启动模式即当创建协程后会检查是
 * 否处于协程调度状态，如果非协程调度状态，则自动启动调度器调度协程过程（如不
 * 调用本函数，则内部缺省为非自动启动模式）
 */
FIBER_API void acl_fiber_schedule_init(int on);

/**
 * 调用本函数启动协程的调度过程
 */
FIBER_API void acl_fiber_schedule(void);

/**
 * 采用指定的事件引擎启动协程调度器，内部缺省的事件引擎为 FIBER_EVENT_KERNEL
 * @param event_mode {int} 事件引擎类型，参见：FIBER_EVENT_XXX
 */
#define FIBER_EVENT_KERNEL	0	/* epoll/kqueue	*/
#define FIBER_EVENT_POLL	1	/* poll		*/
#define FIBER_EVENT_SELECT	2	/* select	*/
#define FIBER_EVENT_WMSG	3	/* win message	*/
FIBER_API void acl_fiber_schedule_with(int event_mode);

/**
 * 设置协程调度时的事件引擎类型
 * @param event_mode {int} 事件引擎类型，参见：FIBER_EVENT_XXX
 */
FIBER_API void acl_fiber_schedule_set_event(int event_mode);

/**
 * 调用本函数检测当前线程是否处于协程调度状态
 * @return {int} 0 表示非协程状态，非 0 表示处于协程调度状态
 */
FIBER_API int acl_fiber_scheduled(void);

/**
 * 停止协程过程
 */
FIBER_API void acl_fiber_schedule_stop(void);

/**
 * 使当前运行的协程休眠指定毫秒数
 * @param milliseconds {unsigned int} 指定要休眠的毫秒数
 * @return {unsigned int} 本协程休眠后再次被唤醒后剩余的毫秒数
 */
FIBER_API unsigned int acl_fiber_delay(unsigned int milliseconds);

/**
 * 使当前运行的协程休眠指定秒数
 * @param seconds {unsigned int} 指定要休眠的秒数
 * @return {unsigned int} 本协程休眠后再次被唤醒后剩余的秒数
 */
FIBER_API unsigned int acl_fiber_sleep(unsigned int seconds);

/**
 * 创建一个协程用作定时器
 * @param milliseconds {unsigned int} 所创建定时器被唤醒的毫秒数
 * @param size {size_t} 所创建协程的栈空间大小
 * @param fn {void (*)(ACL_FIBER*, void*)} 定时器协程被唤醒时的回调函数
 * @param ctx {void*} 回调 fn 函数时的第二个参数
 * @return {ACL_FIBER*} 新创建的定时器协程
 */
FIBER_API ACL_FIBER* acl_fiber_create_timer(unsigned int milliseconds,
	size_t size, void (*fn)(ACL_FIBER*, void*), void* ctx);

/**
 * 在定时器协程未被唤醒前，可以通过本函数重置该协程被唤醒的时间
 * @param timer {ACL_FIBER*} 由 acl_fiber_create_timer 创建的定时器协程
 * @param milliseconds {unsigned int} 指定该定时器协程被唤醒的毫秒数
 */
FIBER_API void acl_fiber_reset_timer(ACL_FIBER* timer, unsigned int milliseconds);

/**
 * 本函数设置 DNS 服务器的地址
 * @param ip {const char*} DNS 服务器 IP 地址
 * @param port {int} DNS 服务器的端口
 */
FIBER_API void acl_fiber_set_dns(const char* ip, int port);

/* for fiber specific */

/**
 * 设定当前协程的局部变量
 * @param key {int*} 协程局部变量的索引键的地址，初始时该值应 <= 0，内部会自动
 *  分配一个 > 0 的索引键，并给该地址赋值，后面的协程可以复用该值设置各自的
 *  局部变量，该指针必须非 NULL
 * @param ctx {void *} 协程局部变量
 * @param free_fn {void (*)(void*)} 当协程退出时会调用此函数释放协程局部变量
 * @return {int} 返回所设置的协程局部变量的键值，返回 -1 表示当前协程不存在
 */
FIBER_API int acl_fiber_set_specific(int* key, void* ctx, void (*free_fn)(void*));

/**
 * 获得当前协程局部变量
 * @param key {int} 由 acl_fiber_set_specific 返回的键值
 * @retur {void*} 返回 NULL 表示不存在
 */
FIBER_API void* acl_fiber_get_specific(int key);

/****************************************************************************/

/**
 * 在将写日志至日志文件前回调用户自定义的函数，且将日志信息传递给该函数，
 * 只有当用户通过 msg_pre_write 进行设置后才生效
 * @param ctx {void*} 用户的自定义参数
 * @param fmt {const char*} 格式参数
 * @param ap {va_list} 格式参数列表
 */
typedef void (*FIBER_MSG_PRE_WRITE_FN)(void *ctx, const char *fmt, va_list ap);

/**
 * 应用通过此函数类型可以自定义日志记录函数，当应用在打开日志前调用
 * msg_register 注册了自定义记录函数，则当应用写日志时便用此自定义
 * 函数记录日志，否则用缺省的日志记录函数
 * @param ctx {void*} 应用传递进去的参数
 * @param fmt {const char*} 格式参数
 * @param ap {va_list} 参数列表
 */
typedef void (*FIBER_MSG_WRITE_FN) (void *ctx, const char *fmt, va_list ap);

/**
 * 在打开日志前调用此函数注册应用自己的日志记录函数
 * @param write_fn {MSG_WRITE_FN} 自定义日志记录函数
 * @param ctx {void*} 自定义参数
 */
FIBER_API void acl_fiber_msg_register(FIBER_MSG_WRITE_FN write_fn, void *ctx);

/**
 * 将 msg_register 注册自定义函数清除，采用缺省的日志函数集
 */
FIBER_API void acl_fiber_msg_unregister(void);

/**
 * 在打开日志前调用此函数注册应用的私有函数，在记录日志前会先记录信息通过
 * 此注册的函数传递给应用
 * @param pre_write {MSG_PRE_WRITE_FN} 日志记录前调用的函数
 * @param ctx {void*} 自定义参数
 */
FIBER_API void acl_fiber_msg_pre_write(FIBER_MSG_PRE_WRITE_FN pre_write, void *ctx);

/**
 * 当未调用 msg_open 方式打开日志时，调用了 msg_info/error/fatal/warn
 * 的操作，是否允许信息输出至标准输出屏幕上，通过此函数来设置该开关，该开关
 * 仅影响是否需要将信息输出至终端屏幕而不影响是否输出至文件中
 * @param onoff {int} 非 0 表示允许输出至屏幕
 */
FIBER_API void acl_fiber_msg_stdout_enable(int onoff);

/**
 * 获得上次系统调用出错时的错误号
 * @return {int} 错误号
 */
FIBER_API int acl_fiber_last_error(void);

/**
 * 获得上次系统调用出错时的错误信息
 * @return {const char*}
 */
FIBER_API const char *acl_fiber_last_serror(void);

/**
 * 手工设置错误号
 * @param errnum {int} 错误号
 */
void acl_fiber_set_error(int errnum);

/****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
