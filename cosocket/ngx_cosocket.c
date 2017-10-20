/*
 * ngx_cosocket.c
 *
 *  Created on: 2017��10��19��
 *      Author: ym
 */

#include "ngx_cosocket.hpp"

static ngx_cosocket_task_t *cur_task;

static ngx_http_module_t ngx_http_cosocket_ctx =
{
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
};

ngx_module_t ngx_http_cosocket_module =
{
		NGX_MODULE_V1,
		&ngx_http_cosocket_ctx,
		NULL,
		NGX_HTTP_MODULE,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NGX_MODULE_V1_PADDING
};

inline ngx_cosocket_task_t *get_cur_task()
{
	return cur_task;
}
inline void set_cur_task(ngx_cosocket_task_t *task)
{
	cur_task = task;
}

void ngx_cosocket_read_event_handler(ngx_event_t *ev)
{
	ngx_connection_t *conn = ev->data;
	ngx_cosocket_task_t *task = conn->data;

	if (task == NGX_CONF_ERROR || task->timer.timer_set)
		return ;

	set_cur_task(task);
	swap_slave_ctx(task->ctx);

	if (task->finish != NGX_AGAIN)
	{
		task->cur_req->write_event_handler(task->cur_req);
	}
}

void ngx_cosocket_write_event_handler(ngx_event_t *ev)
{
	ngx_connection_t *conn = ev->data;
	ngx_cosocket_task_t *task = conn->data;

	if (task == NGX_CONF_ERROR || task->timer.timer_set)
		return ;

	set_cur_task(task);
	swap_slave_ctx(task->ctx);

	if (task->finish != NGX_AGAIN)
	{
		task->cur_req->write_event_handler(task->cur_req);
	}
}

void ngx_cosocket_timer_event_handler(ngx_event_t *ev)
{
	ngx_cosocket_task_t *task;

	task = ev->data;

	set_cur_task(task);
	swap_slave_ctx(task->ctx);

	if (task->finish != NGX_AGAIN)
	{
		task->cur_req->write_event_handler(task->cur_req);
	}
}

static void ngx_http_cosocket_handler(ngx_http_request_t *r)
{
	ngx_cosocket_task_t *task;

	task = ngx_http_get_module_ctx(r, ngx_http_cosocket_module);
	task->out = task->handler(r);

	if (task->out == NGX_CONF_ERROR)
	{
		task->finish = NGX_ERROR;
	}
	else
	{
		task->finish = NGX_OK;
	}

	set_close_task(task);
	set_cur_task(NULL);
	swap_main_ctx(task->ctx);
}


static void ngx_http_init_cosocket()
{
	ngx_cosocket_ctx_t *ctx = &cur_task->ctx;
	getcontext(&ctx->base);
	getcontext(&ctx->main);
	ctx->base.uc_stack.ss_size = MAX_STACK_LEN;
	ctx->base.uc_stack.ss_sp = ctx->stack;
	makecontext(&ctx->base, (void (*)())ngx_http_cosocket_handler, 1, cur_task->cur_req);
}

ngx_int_t ngx_cosocket_parse(ngx_http_request_t *r, ngx_cosocket_handler_pt handler, void **out)
{
	ngx_cosocket_task_t *task;

	task = ngx_http_get_module_ctx(r, ngx_http_cosocket_module);

	if (task == NULL)
	{
		r->main->count++;
		task = ngx_pcalloc(r->pool, sizeof(ngx_cosocket_task_t));

		if (task == NULL)
		{
			ngx_log_debug0(NGX_LOG_DEBUG_ALLOC, r->connection->log, 0, "no memory.");
			return NGX_ERROR;
		}

		task->handler = handler;
		task->cur_req = r;

		task->finish = NGX_AGAIN;
		set_cur_task(task);
		ngx_http_init_cosocket();
		ngx_http_set_ctx(r, task, ngx_http_cosocket_module);
		swap_slave_ctx(task->ctx);
	}

	if (task->finish == NGX_AGAIN)
		return NGX_DONE;

	ngx_http_set_ctx(r, NULL, ngx_http_cosocket_module);
	ngx_http_finalize_request(r, NGX_DONE);

	if (task->finish != NGX_OK)
		return task->finish;

	*out = task->out;
	return NGX_OK;
}
