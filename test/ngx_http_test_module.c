/*
 * ngx_http_test_module.c
 *
 *  Created on: 2017��10��20��
 *      Author: ym
 */

#include "../cosocket/ngx_cosocket.hpp"
#include <hiredis/hiredis.h>
#include "redis_list_test.hpp"

static char *ngx_http_test_parse(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_test_block(ngx_http_request_t *r);

static ngx_chain_t *ngx_http_test_handler(ngx_http_request_t *r);

static ngx_command_t ngx_http_test_commands[] =
{
		{
				ngx_string("test"),
				NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
				ngx_http_test_parse,
				NGX_HTTP_LOC_CONF_OFFSET,
				0,
				NULL
		},
		ngx_null_command
};

static ngx_http_module_t ngx_http_test_ctx =
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

ngx_module_t ngx_http_test_module =
{
		NGX_MODULE_V1,
		&ngx_http_test_ctx,
		ngx_http_test_commands,
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

void redis_test()
{
	redisContext *ctx = redisConnect("127.0.0.1", 6379);
	redisReply *reply = (redisReply *)redisCommand(ctx, "set test 123");
	printf("str = %s\n", reply->str);
	freeReplyObject(reply);
//	reply = (redisReply *)redisCommand(ctx, "get test");
//	freeReplyObject(reply);
	redisFree(ctx);
//	sleep(1);
}

ngx_chain_t *ngx_http_test_handler(ngx_http_request_t *r)
{
	static ngx_str_t str = ngx_string("yangmeng12");

	ngx_chain_t *out;
	ngx_buf_t *buf;

	out = ngx_alloc_chain_link(r->pool);
	buf = ngx_create_temp_buf(r->pool, str.len);

	redis_list_test();

	if (out == NULL || buf == NULL)
	{
		ngx_log_debug0(NGX_LOG_DEBUG_ALLOC, r->pool->log, 0, "no memory.");
		return NGX_CONF_ERROR;
	}

	ngx_memcpy(buf->last, str.data, str.len);
	buf->last += str.len;
	buf->last_buf = 1;
	out->buf = buf;
	out->next = NULL;

	return out;
}

static void ngx_http_test_finish(ngx_http_request_t *r)
{
	ngx_chain_t *out;
	ngx_int_t rc;

	rc = ngx_cosocket_parse(r, (ngx_cosocket_handler_pt)ngx_http_test_handler, (void **)&out);

	if (rc != NGX_OK)
	{
		if (rc != NGX_DONE)
			ngx_http_finalize_request(r, rc);
		return;
	}

	ngx_str_set(&r->headers_out.content_type, "text/plain");
	r->headers_out.content_length_n = out->buf->last - out->buf->pos;
	r->headers_out.status = NGX_HTTP_OK;

	rc = ngx_http_send_header(r);

	if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
	{
		ngx_http_finalize_request(r, rc);
		return;
	}

	rc = ngx_http_output_filter(r, out);
	ngx_http_finalize_request(r, rc);
}

ngx_int_t ngx_http_test_block(ngx_http_request_t *r)
{
	r->write_event_handler = ngx_http_test_finish;
	ngx_http_read_client_request_body(r, ngx_http_test_finish);
	return NGX_DONE;
}

char *ngx_http_test_parse(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
	ngx_http_core_loc_conf_t *clcf;

	clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

	if (clcf->name.data[clcf->name.len - 1] == '/')
		clcf->auto_redirect = 1;

	clcf->handler = ngx_http_test_block;

	return NGX_CONF_OK;
}
