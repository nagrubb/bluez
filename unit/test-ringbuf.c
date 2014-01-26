/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <glib.h>

#include "src/shared/ringbuf.h"

static unsigned int nlpo2(unsigned int x)
{
	x--;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return x + 1;
}

static unsigned int align_power2(unsigned int u)
{
	return 1 << ((sizeof(u) * 8) - __builtin_clz(u - 1));
}

static void test_power2(void)
{
	size_t i;

	for (i = 1; i < 1000000; i++) {
		size_t size1, size2, size3 = 1;

		size1 = nlpo2(i);
		size2 = align_power2(i);

		/* Find the next power of two */
		while (size3 < i && size3 < SIZE_MAX)
			size3 <<= 1;

		if (g_test_verbose())
			g_print("%zu -> size1=%zu size2=%zu size3=%zu\n",
						i, size1, size2, size3);

		g_assert(size1 == size2);
		g_assert(size2 == size3);
		g_assert(size3 == size1);
	}
}

static void test_alloc(void)
{
	int i;

	for (i = 2; i < 10000; i++) {
		struct ringbuf *rb;

		if (g_test_verbose())
			g_print("Iteration %i\n", i);

		rb = ringbuf_new(i);
		g_assert(rb != NULL);

		g_assert(ringbuf_capacity(rb) == ringbuf_avail(rb));

		ringbuf_free(rb);
	}
}

static void test_printf(void)
{
	static size_t rb_size = 500;
	static size_t rb_capa = 512;
	struct ringbuf *rb;
	int i;

	rb = ringbuf_new(rb_size);
	g_assert(rb != NULL);
	g_assert(ringbuf_capacity(rb) == rb_capa);

	for (i = 0; i < 10000; i++) {
		size_t len, count = i % rb_capa;
		char *str, *ptr;

		if (!count)
			continue;

		if (g_test_verbose())
			g_print("Iteration %i\n", i);

		len = asprintf(&str, "%*c", (int) count, 'x');
		g_assert(len == count);

		len = ringbuf_printf(rb, "%s", str);
		g_assert(len == count);
		g_assert(ringbuf_len(rb) == count);
		g_assert(ringbuf_avail(rb) == rb_capa - len);

		ptr = ringbuf_peek(rb, 0, &len);
		g_assert(ptr != NULL);
		g_assert(len == count);
		g_assert(strncmp(str, ptr, len) == 0);

		len = ringbuf_drain(rb, count);
		g_assert(len == count);
		g_assert(ringbuf_len(rb) == 0);
		g_assert(ringbuf_avail(rb) == rb_capa);

		free(str);
	}

	ringbuf_free(rb);
}

int main(int argc, char *argv[])
{
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/ringbuf/power2", test_power2);
	g_test_add_func("/ringbuf/alloc", test_alloc);
	g_test_add_func("/ringbuf/printf", test_printf);

	return g_test_run();
}
