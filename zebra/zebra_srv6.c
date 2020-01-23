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

#include <zebra.h>

#include "network.h"
#include "prefix.h"
#include "stream.h"
#include "zebra/zserv.h"
#include "zebra/zebra_router.h"
#include "zebra/zebra_srv6.h"
#include "zebra/zebra_memory.h"
#include "zebra/zebra_errors.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/seg6_iptunnel.h>
#include <linux/lwtunnel.h>
#include <linux/seg6_local.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>

DEFINE_MTYPE_STATIC(ZEBRA, SRV6, "zebra SRv6");

/*
 * locator
 */

struct zebra_srv6_locator *zebra_srv6_locator_alloc(char *name)
{
	struct zebra_srv6_locator *locator = NULL;
	locator = XCALLOC(MTYPE_SRV6, sizeof(struct zebra_srv6_locator));
	if (locator) {
		int namelen = sizeof(locator->name);
		if (namelen > SRV6_LOCNAME_SIZE - 1) {
			namelen = SRV6_LOCNAME_SIZE - 1;
		}
		strlcpy(locator->name, name, namelen);
		locator->current = 0;
		locator->functions = list_new();
	}
	return locator;
}

void zebra_srv6_locator_free(struct zebra_srv6_locator *locator)
{
	struct listnode *node, *nextnode;
	struct zebra_srv6_function *function;
	if (locator->functions) {
		/* XXX: */
		for (ALL_LIST_ELEMENTS((struct list *)locator->functions, node, nextnode, function)) {
			zebra_srv6_function_free(function);
		}
	}
	XFREE(MTYPE_SRV6, locator);
}

void zebra_srv6_locator_add(struct zebra_srv6_locator *locator)
{
	struct zebra_srv6 *srv6 = zebra_srv6_get_default();
	struct zebra_srv6_locator *tmp;
	tmp = zebra_srv6_locator_lookup(locator->name);
	if (!tmp) {
		listnode_add(srv6->locators, locator);
	}
	zsend_bcast_zebra_srv6_locator_add(locator);
}

void zebra_srv6_locator_delete(struct zebra_srv6_locator *locator)
{
	struct zebra_srv6 *srv6 = zebra_srv6_get_default();
	struct listnode *node;
	struct zebra_srv6_function *function;
	for (ALL_LIST_ELEMENTS_RO((struct list *)locator->functions, node, function)) {
		zsend_bcast_zebra_srv6_function_delete(function);
	}
	zsend_bcast_zebra_srv6_locator_delete(locator);
	listnode_delete(srv6->locators, locator);
}

struct zebra_srv6_locator *zebra_srv6_locator_lookup(const char *name)
{
	struct zebra_srv6 *srv6 = zebra_srv6_get_default();
	struct zebra_srv6_locator *locator;
	struct listnode *node;
	for (ALL_LIST_ELEMENTS_RO(srv6->locators, node, locator)) {
		if (!strncmp(name, locator->name, SRV6_LOCNAME_SIZE)) {
			return locator;
		}
	}
	return NULL;
}

void zsend_bcast_zebra_srv6_locator_add(struct zebra_srv6_locator *locator)
{
	struct listnode *node;
	struct zserv *client;
	for (ALL_LIST_ELEMENTS_RO(zrouter.client_list, node, client)) {
		zsend_zebra_srv6_locator_add(client, locator);
	}
}

void zsend_bcast_zebra_srv6_locator_delete(struct zebra_srv6_locator *locator)
{
	struct listnode *node;
	struct zserv *client;
	for (ALL_LIST_ELEMENTS_RO(zrouter.client_list, node, client)) {
		zsend_zebra_srv6_locator_delete(client, locator);
	}
}

void zsend_zebra_srv6_locator_add(struct zserv *client, struct zebra_srv6_locator *locator)
{
	struct stream *s = stream_new(ZEBRA_MAX_PACKET_SIZ);
	unsigned char namelen = strlen(locator->name);

	zclient_create_header(s, ZEBRA_SRV6_LOCATOR_ADD, VRF_DEFAULT);

	stream_putc(s, namelen);
	stream_put(s, locator->name, namelen);
	stream_put(s, &locator->prefix, sizeof(locator->prefix));
	stream_putc(s, locator->function_bits_length);

	stream_putw_at(s, 0, stream_get_endp(s));

	zserv_send_message(client, s);
}

void zsend_zebra_srv6_locator_delete(struct zserv *client, struct zebra_srv6_locator *locator)
{
	struct stream *s = stream_new(ZEBRA_MAX_PACKET_SIZ);
	unsigned char namelen = strlen(locator->name);

	zclient_create_header(s, ZEBRA_SRV6_LOCATOR_DELETE, VRF_DEFAULT);

	stream_putc(s, namelen);
	stream_put(s, locator->name, namelen);
	stream_put(s, &locator->prefix, sizeof(locator->prefix));
	stream_putc(s, locator->function_bits_length);

	stream_putw_at(s, 0, stream_get_endp(s));

	zserv_send_message(client, s);
}

void zrecv_zebra_srv6_locator_add(ZAPI_HANDLER_ARGS)
{
}

void zrecv_zebra_srv6_locator_delete(ZAPI_HANDLER_ARGS)
{
}

/*
 * function
 */

struct zebra_srv6_function *zebra_srv6_function_alloc(struct prefix_ipv6 *prefix)
{
	struct zebra_srv6_function *function = NULL;
	function = XCALLOC(MTYPE_SRV6, sizeof(struct zebra_srv6_function));
	if (function) {
		function->prefix = *prefix;
	}
	return function;
}

void zebra_srv6_function_free(struct zebra_srv6_function *function)
{
	XFREE(MTYPE_SRV6, function);
}

void zebra_srv6_function_add(struct zebra_srv6_function *function, struct zebra_srv6_locator *locator)
{
	listnode_add(locator->functions, function);
	function->locator = locator;
	zsend_bcast_zebra_srv6_function_add(function);
}

void zebra_srv6_function_delete(struct zebra_srv6_function *function, struct zebra_srv6_locator *locator)
{
	zsend_bcast_zebra_srv6_function_delete(function);
	function->locator = NULL;
	listnode_delete(locator->functions, function);
}

struct zebra_srv6_function *zebra_srv6_function_lookup(const struct zebra_srv6_locator *locator, const struct prefix_ipv6 *prefix)
{
	struct zebra_srv6_function *function;
	struct listnode *node;
	for (ALL_LIST_ELEMENTS_RO(locator->functions, node, function)) {
		if (!prefix_cmp(prefix, &function->prefix)) {
			return function;
		}
	}
	return NULL;
}

void zsend_bcast_zebra_srv6_function_add(struct zebra_srv6_function *function)
{
	struct listnode *node;
	struct zserv *client;
	for (ALL_LIST_ELEMENTS_RO(zrouter.client_list, node, client)) {
		zsend_zebra_srv6_function_add(client, function);
	}
}

void zsend_bcast_zebra_srv6_function_delete(struct zebra_srv6_function *function)
{
	struct listnode *node;
	struct zserv *client;
	for (ALL_LIST_ELEMENTS_RO(zrouter.client_list, node, client)) {
		zsend_zebra_srv6_function_delete(client, function);
	}
}

void zsend_zebra_srv6_function_add(struct zserv *client, struct zebra_srv6_function *function)
{
	struct stream *s = stream_new(ZEBRA_MAX_PACKET_SIZ);
	unsigned char namelen = strlen(function->locator->name);

	zclient_create_header(s, ZEBRA_SRV6_FUNCTION_ADD, VRF_DEFAULT);

	stream_putc(s, namelen);
	stream_put(s, function->locator->name, namelen);
	stream_put(s, &function->prefix, sizeof(function->prefix));
	stream_putc(s, function->owner_proto);
	stream_putw(s, function->owner_instance);
	stream_putl(s, function->request_key);

	stream_putw_at(s, 0, stream_get_endp(s));

	zserv_send_message(client, s);
}

void zsend_zebra_srv6_function_delete(struct zserv *client, struct zebra_srv6_function *function)
{
	struct stream *s = stream_new(ZEBRA_MAX_PACKET_SIZ);
	unsigned char namelen = strlen(function->locator->name);

	zclient_create_header(s, ZEBRA_SRV6_FUNCTION_DELETE, VRF_DEFAULT);

	stream_putc(s, namelen);
	stream_put(s, function->locator->name, namelen);
	stream_put(s, &function->prefix, sizeof(function->prefix));
	stream_putc(s, function->owner_proto);
	stream_putw(s, function->owner_instance);
	stream_putl(s, function->request_key);

	stream_putw_at(s, 0, stream_get_endp(s));

	zserv_send_message(client, s);
}

static void get_function_addr(struct in6_addr *locator, uint64_t current, int lshift)
{
	uint32_t *p = (uint32_t *)locator;
	if (lshift > 0x5f)
		return;
	uint32_t tmp[4] = {};
	tmp[lshift >> 5] = current << (lshift & 0x1f);
	tmp[(lshift >> 5) + 1] = current >> (32 - (lshift & 0x1f));
	p[0] |= ntohl(tmp[3]);
	p[1] |= ntohl(tmp[2]);
	p[2] |= ntohl(tmp[1]);
	p[3] |= ntohl(tmp[0]);
}

void zrecv_zebra_srv6_function_add(ZAPI_HANDLER_ARGS)
{
	struct zapi_srv6_function api;
	struct zebra_srv6_locator *locator;
	struct zebra_srv6_function *function;
	struct prefix_ipv6 addr_zero = {.family = AF_INET6};
	int function_prefixlen;
	zapi_srv6_function_decode(msg, &api);
	locator = zebra_srv6_locator_lookup(api.locator_name);
	if (!locator) {
		zlog_err("zrecv_zebra_srv6_function_add: locator not found %s", api.locator_name);
		return;
	}
	function_prefixlen = locator->prefix.prefixlen + locator->function_bits_length;
	if (api.prefix.prefixlen != 0 && api.prefix.prefixlen != function_prefixlen) {
		zlog_err("zrecv_zebra_srv6_function_add: function prefixlen mis-match");
		return;
	}
	if (!prefix_same(&api.prefix, &addr_zero)
	 && !prefix_match((const struct prefix *)&locator->prefix, (const struct prefix *)&api.prefix)) {
		zlog_err("zrecv_zebra_srv6_function_add: function prefix mis-match");
		return;
	}
	if (prefix_same(&api.prefix, &addr_zero)) {
		struct prefix_ipv6 prefix = {
			.family = AF_INET6,
			.prefix = locator->prefix.prefix,
			.prefixlen = locator->prefix.prefixlen + locator->function_bits_length,
		};
		uint64_t mask = ~((1ull << locator->function_bits_length) - 1);
		int retry_count = 0;
		do {
			locator->current += 1;
			if (locator->current & mask) {
				locator->current = 0;
			}
			get_function_addr(&prefix.prefix, locator->current, 128 - prefix.prefixlen);
			if (retry_count++ > 100) {
				zlog_err("zrecv_zebra_srv6_function_add: cannot get new function prefix");
				return;
			}
		} while(zebra_srv6_function_lookup(locator, &prefix));
		api.prefix.prefix = prefix.prefix;
		api.prefix.prefixlen = prefix.prefixlen;
	}
	function = zebra_srv6_function_alloc(&api.prefix);
	if (!function) {
		zlog_err("zrecv_zebra_srv6_function_add: alloc failed");
		return;
	}
	function->owner_proto = client->proto;
	function->owner_instance = client->instance;
	function->request_key = api.request_key;
	zebra_srv6_function_add(function, locator);
	return;
}

void zrecv_zebra_srv6_function_delete(ZAPI_HANDLER_ARGS)
{
	struct zapi_srv6_function api;
	struct zebra_srv6_locator *locator;
	struct zebra_srv6_function *function;
	zapi_srv6_function_decode(msg, &api);
	locator = zebra_srv6_locator_lookup(api.locator_name);
	if (!locator) {
		zlog_err("zrecv_zebra_srv6_function_delete: locator not found");
		return;
	}
	function = zebra_srv6_function_lookup(locator, &api.prefix);
	if (!function) {
		zlog_err("zrecv_zebra_srv6_function_delete: function not found");
		return;
	}
	function->request_key = api.request_key;
	zebra_srv6_function_delete(function, locator);
	zebra_srv6_function_free(function);
	return;
}

struct zebra_srv6 *zebra_srv6_get_default(void)
{
	static struct zebra_srv6 srv6;
	static bool first_execution = true;
	if (first_execution) {
		srv6.is_enable = false;
		first_execution = false;
		srv6.locators = list_new();
	}
	return &srv6;
}

void zebra_srv6_init()
{
}
