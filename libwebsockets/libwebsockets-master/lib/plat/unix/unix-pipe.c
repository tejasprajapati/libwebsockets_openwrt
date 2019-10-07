/*
 * libwebsockets - small server side websockets and web server implementation
 *
 * Copyright (C) 2010 - 2019 Andy Green <andy@warmcat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#define _GNU_SOURCE
#include "private-lib-core.h"


int
lws_plat_pipe_create(struct lws *wsi)
{
	struct lws_context_per_thread *pt = &wsi->context->pt[(int)wsi->tsi];

#if defined(LWS_HAVE_PIPE2)
	return pipe2(pt->dummy_pipe_fds, O_NONBLOCK);
#else
	return pipe(pt->dummy_pipe_fds);
#endif
}

int
lws_plat_pipe_signal(struct lws *wsi)
{
	struct lws_context_per_thread *pt = &wsi->context->pt[(int)wsi->tsi];
	char buf = 0;
	int n;

	n = write(pt->dummy_pipe_fds[1], &buf, 1);

	return n != 1;
}

void
lws_plat_pipe_close(struct lws *wsi)
{
	struct lws_context_per_thread *pt = &wsi->context->pt[(int)wsi->tsi];

	if (pt->dummy_pipe_fds[0] && pt->dummy_pipe_fds[0] != -1)
		close(pt->dummy_pipe_fds[0]);
	if (pt->dummy_pipe_fds[1] && pt->dummy_pipe_fds[1] != -1)
		close(pt->dummy_pipe_fds[1]);

	pt->dummy_pipe_fds[0] = pt->dummy_pipe_fds[1] = -1;
}
