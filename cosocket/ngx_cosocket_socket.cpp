/*
 * ngx_cosocket_socket.cpp
 *
 *  Created on: 2017锟斤拷10锟斤拷19锟斤拷
 *      Author: ym
 */

#include "ngx_cosocket.hpp"
#include <iostream>
#include <map>
#include <dlfcn.h>
#include <errno.h>

using namespace std;

typedef int (*g_close_t) (int __fd);
typedef ssize_t (*g_write_t) (int __fd, const void *__buf, size_t __n);
typedef ssize_t (*g_read_t) (int __fd, void *__buf, size_t __nbytes);
typedef int (*g_connect_t) (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
typedef unsigned int (*g_sleep_t) (unsigned int __seconds);
typedef int (*g_usleep_t) (__useconds_t __useconds);

static g_connect_t g_connect = (g_connect_t)dlsym(RTLD_NEXT, "connect");
static g_close_t g_close = (g_close_t)dlsym(RTLD_NEXT, "close");
static g_write_t g_write = (g_write_t)dlsym(RTLD_NEXT, "write");
static g_read_t g_read = (g_read_t)dlsym(RTLD_NEXT, "read");
static g_sleep_t g_sleep = (g_sleep_t)dlsym(RTLD_NEXT, "sleep");
static g_usleep_t g_usleep = (g_usleep_t)dlsym(RTLD_NEXT, "usleep");

static map<ngx_socket_t, ngx_connection_t * > conns;

static unsigned int my_sleep(unsigned int msec)
{
	ngx_cosocket_task_t *task;
	ngx_event_t *ev;

	task = get_cur_task();
	ev = &task->timer;
	ev->data = task;
	ev->handler = ngx_cosocket_timer_event_handler;
	ev->log = ngx_cycle->log;

	ev->timedout = 0;
	ngx_add_timer(ev, msec);

	set_cur_task(NULL);
	swap_main_ctx(task->ctx);

	return NGX_OK;
}


int usleep (__useconds_t __useconds)
{
	if (get_cur_task())
		return my_sleep(__useconds / 1000);
	else
		return g_usleep(__useconds);
}

unsigned int sleep (unsigned int __seconds)
{
	if (get_cur_task())
		return my_sleep(__seconds * 1000);
	else
		return g_sleep(__seconds);
}

int connect (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len)
{
	ngx_cosocket_task_t *task;
	ngx_connection_t *conn;

	task = get_cur_task();

	if (task == NULL)
		return g_connect(__fd, __addr, __len);

	ngx_nonblocking(__fd);
	g_connect(__fd, __addr, __len);
	conn = ngx_get_connection(__fd, ngx_cycle->log);

	if (conn == NULL)
	{
		g_close(__fd);
		return NGX_INVALID_FILE;
	}

	conn->data = (void *)task;
	conn->write->handler = ngx_cosocket_write_event_handler;
	conn->read->handler = ngx_cosocket_read_event_handler;
	conn->read->log = conn->write->log = ngx_cycle->log;
	ngx_add_conn(conn);

	conns[__fd] = conn;
	set_cur_task(NULL);
	swap_main_ctx(task->ctx);

	return NGX_OK;
}

ssize_t write (int __fd, const void *__buf, size_t __n)
{
	ngx_int_t tmp;
	ngx_int_t len;
	ngx_cosocket_task_t *task;

	task = get_cur_task();

	if (task)
	{
		len = 0;
		ngx_nonblocking(__fd);

		while (__n - len)
		{
			tmp = g_write(__fd, (const char *)__buf + len, __n - len);

			if (tmp <= 0)
			{
				if (ngx_errno == EAGAIN || ngx_errno == EINTR)
				{
					set_cur_task(NULL);
					swap_main_ctx(task->ctx);
					continue;
				}

				return tmp;
			}

			len += tmp;
		}

		return __n;
	}
	else
	{
		return g_write(__fd, __buf, __n);
	}
}


ssize_t read (int __fd, void *__buf, size_t __nbytes)
{
	ngx_int_t tmp;
	ngx_cosocket_task_t *task;
	ngx_event_t *ev;

	task = get_cur_task();

	if (task)
	{
		ev = conns[__fd]->read;
		ngx_nonblocking(__fd);
lable:
		tmp = g_read(__fd, (char *)__buf, __nbytes);

		if (tmp <= 0)
		{
			if (ngx_errno == EAGAIN || ngx_errno == EINTR)
			{
				ngx_add_timer(ev, 10000);
				set_cur_task(NULL);
				swap_main_ctx(task->ctx);
				if (ev->timedout)
				{
					ev->timedout = 0;
				}
				else
				{
					ngx_del_timer(ev);
				}

				goto lable;
			}

			printf("tmp = %d\n", (int)tmp);
			return tmp;
		}

		return tmp;
	}
	else
	{
		return g_read(__fd, __buf, __nbytes);
	}
}

int close (int __fd)
{
	ngx_cosocket_task_t *task;

	task = get_cur_task();
	if (task)
	{
		set_cur_task(NULL);
		ngx_close_connection(conns[__fd]);
		set_cur_task(task);
		conns.erase(__fd);
	}
	else
	{
		g_close(__fd);
	}

	return NGX_OK;
}

void set_close_task(ngx_cosocket_task_t *task)
{
	map<ngx_socket_t, ngx_connection_t * >::iterator mit;

	for (mit = conns.begin(); mit != conns.end(); ++mit)
	{
		if (mit->second->data == task)
			mit->second->data = NGX_CONF_ERROR;
	}
}
