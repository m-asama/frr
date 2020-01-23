/* BGP message definition header.
 * Copyright (C) 2019 Hiroki Shirokura
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; see the file COPYING; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _QUAGGA_ZEBRA_SRV6_H
#define _QUAGGA_ZEBRA_SRV6_H

#include "qobj.h"
#include "prefix.h"
#include <pthread.h>

/* SRv6 instance structure.  */
struct zebra_srv6 {
	bool is_enable;
	struct in6_addr encap_src;
	struct list *locators;

	QOBJ_FIELDS
};
DECLARE_QOBJ_TYPE(srv6)

struct zebra_srv6_locator {
	char name[SRV6_LOCNAME_SIZE];
	struct prefix_ipv6 prefix;
	uint8_t function_bits_length;
	uint64_t current;
	struct list *functions;
};

struct zebra_srv6_function {
	struct prefix_ipv6 prefix;
	uint8_t owner_proto;
	uint16_t owner_instance;
	uint32_t request_key;
	struct zebra_srv6_locator *locator;
};

/*
 * locator
 */

extern struct zebra_srv6_locator *zebra_srv6_locator_alloc(char *name);
extern void zebra_srv6_locator_free(struct zebra_srv6_locator *locator);

extern void zebra_srv6_locator_add(struct zebra_srv6_locator *locator);
extern void zebra_srv6_locator_delete(struct zebra_srv6_locator *locator);
extern struct zebra_srv6_locator *zebra_srv6_locator_lookup(const char *name);

extern void zsend_bcast_zebra_srv6_locator_add(struct zebra_srv6_locator *locator);
extern void zsend_bcast_zebra_srv6_locator_delete(struct zebra_srv6_locator *locator);

extern void zsend_zebra_srv6_locator_add(struct zserv *client, struct zebra_srv6_locator *locator);
extern void zsend_zebra_srv6_locator_delete(struct zserv *client, struct zebra_srv6_locator *locator);

extern void zrecv_zebra_srv6_locator_add(ZAPI_HANDLER_ARGS);
extern void zrecv_zebra_srv6_locator_delete(ZAPI_HANDLER_ARGS);

/*
 * function
 */

extern struct zebra_srv6_function *zebra_srv6_function_alloc(struct prefix_ipv6 *prefix);
extern void zebra_srv6_function_free(struct zebra_srv6_function *function);

extern void zebra_srv6_function_add(struct zebra_srv6_function *function, struct zebra_srv6_locator *locator);
extern void zebra_srv6_function_delete(struct zebra_srv6_function *function, struct zebra_srv6_locator *locator);
extern struct zebra_srv6_function *zebra_srv6_function_lookup(const struct zebra_srv6_locator *locator, const struct prefix_ipv6 *prefix);

extern void zsend_bcast_zebra_srv6_function_add(struct zebra_srv6_function *function);
extern void zsend_bcast_zebra_srv6_function_delete(struct zebra_srv6_function *function);

extern void zsend_zebra_srv6_function_add(struct zserv *client, struct zebra_srv6_function *function);
extern void zsend_zebra_srv6_function_delete(struct zserv *client, struct zebra_srv6_function *function);

extern void zrecv_zebra_srv6_function_add(ZAPI_HANDLER_ARGS);
extern void zrecv_zebra_srv6_function_delete(ZAPI_HANDLER_ARGS);

/*
 *
 */

extern struct zebra_srv6 *zebra_srv6_get_default(void);
extern void zebra_srv6_init(void);

/*
 *
 */

extern void zebra_srv6_vty_init(void);

#endif /* _QUAGGA_ZEBRA_SEG6_H */
