/*
 * ngx_cosocket.hpp
 *
 *  Created on: 2017��10��19��
 *      Author: ym
 */

#ifndef SRC_COSOCKET_NGX_COSOCKET_HPP_
#define SRC_COSOCKET_NGX_COSOCKET_HPP_


#ifdef __cplusplus
extern "C" {
#endif

#include <ngx_core.h>
#include <ngx_config.h>
#include <ngx_http.h>
#include <ucontext.h>

#define MAX_STACK_LEN	(1<<16)

#define swap_slave_ctx(ctx) swapcontext(&ctx.main, &ctx.base)
#define swap_main_ctx(ctx) swapcontext(&ctx.base, &ctx.main)

typedef void *(*ngx_cosocket_handler_pt)(ngx_http_request_t *r);


typedef struct
{
	ucontext_t main;
	ucontext_t base;
	u_char stack[MAX_STACK_LEN];
} ngx_cosocket_ctx_t;

typedef struct
{
	ngx_http_request_t *cur_req;
	ngx_cosocket_handler_pt handler;
	void *out;
	ngx_cosocket_ctx_t ctx;
	ngx_flag_t finish;
	ngx_event_t timer;//timer����
} ngx_cosocket_task_t;


ngx_int_t ngx_cosocket_parse(ngx_http_request_t *r, ngx_cosocket_handler_pt handler,
		void **out);
void ngx_cosocket_read_event_handler(ngx_event_t *ev);
void ngx_cosocket_write_event_handler(ngx_event_t *ev);
void ngx_cosocket_timer_event_handler(ngx_event_t *ev);
void set_close_task(ngx_cosocket_task_t *task);

ngx_cosocket_task_t *get_cur_task();
void set_cur_task(ngx_cosocket_task_t *task);

#ifdef __cplusplus
}
#endif


#endif /* SRC_COSOCKET_NGX_COSOCKET_HPP_ */
