/* Zebra SRv6 VTY functions
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

#include "memory.h"
#include "if.h"
#include "prefix.h"
#include "command.h"
#include "table.h"
#include "rib.h"
#include "nexthop.h"
#include "vrf.h"
#include "lib/json.h"

#include "zebra/zserv.h"
#include "zebra/zebra_vrf.h"
#include "zebra/zebra_srv6.h"
#include "zebra/zebra_rnh.h"
#include "zebra/redistribute.h"
#include "zebra/zebra_routemap.h"

DEFUN (show_segment_routing_ipv6_locator,
       show_segment_routing_ipv6_locator_cmd,
       "show segment-routing-ipv6 locator",
       SHOW_STR
       "Segment-Routing IPv6\n"
			 "Locator Information\n")
{
	struct zebra_srv6 *srv6 = zebra_srv6_get_default();
	struct zebra_srv6_locator *locator;
	struct listnode *node;
	char str[256];
	int i;
	vty_out(vty, "Locator:\n");
	vty_out(vty, "Name                 ID      Prefix                   Status\n");
	vty_out(vty, "-------------------- ------- ------------------------ -------\n");
	i = 1;
	for (ALL_LIST_ELEMENTS_RO(srv6->locators, node, locator)) {
		prefix2str(&locator->prefix, str, sizeof(str));
		vty_out(vty, "%-20s %7d %-24s Up\n", locator->name, i, str);
		++i;
	}
	vty_out(vty, "\n");
	return CMD_SUCCESS;
}

DEFUN (show_segment_routing_ipv6_function,
       show_segment_routing_ipv6_function_cmd,
       "show segment-routing-ipv6 function",
       SHOW_STR
       "Segment-Routing IPv6\n"
       "Function Information\n")
{
	struct zebra_srv6 *srv6 = zebra_srv6_get_default();
	struct zebra_srv6_locator *locator;
	struct zebra_srv6_function *function;
	struct listnode *nodel, *nodef;
	char strl[256], strf[256];
	vty_out(vty, "Function:\n");
	vty_out(vty, "Locator Name         Prefix                   Ownter\n");
	vty_out(vty, "-------------------- ------------------------ ---------------\n");
	for (ALL_LIST_ELEMENTS_RO(srv6->locators, nodel, locator)) {
		prefix2str(&locator->prefix, strl, sizeof(strl));
		for (ALL_LIST_ELEMENTS_RO(locator->functions, nodef, function)) {
			prefix2str(&function->prefix, strf, sizeof(strf));
			vty_out(vty, "%-20s %-24s %-15s\n", locator->name, strf,
					zebra_route_string(function->owner_proto));
		}
	}
	vty_out(vty, "\n");
	return CMD_SUCCESS;
}

/* "segment-routing-ipv6" commands. */
DEFUN_NOSH (segment_routing_ipv6,
            segment_routing_ipv6_cmd,
            "segment-routing-ipv6",
            "Segment Routing IPv6\n")
{
	struct zebra_srv6 *srv6 = zebra_srv6_get_default();
	srv6->is_enable = true;
	vty->node = SRV6_NODE;
	return CMD_SUCCESS;
}

DEFUN (no_segment_routing_ipv6,
       no_segment_routing_ipv6_cmd,
       "no segment-routing-ipv6",
       NO_STR
       "negate Segment Routing IPv6\n")
{
	struct zebra_srv6 *srv6 = zebra_srv6_get_default();
	srv6->is_enable = false;
	if (vty->node == SRV6_NODE)
		vty->node = CONFIG_NODE;
	return CMD_SUCCESS;
}

DEFUN (locator_prefix,
       locator_prefix_cmd,
       "locator WORD prefix X:X::X:X/M [func-bits (8-64)]",
       "Configure SRv6 locator\n"
       "Configure SRv6 locator name\n"
       "Configure SRv6 locator prefix\n"
       "Specify SRv6 locator prefix\n"
       "Configure SRv6 locator function length in bits\n"
       "Specify SRv6 locator function length in bits\n")
{
	struct zebra_srv6_locator *locator;
	struct prefix_ipv6 prefix;
	uint8_t function_bits_length = 16;
	int ret;

	if (zebra_srv6_locator_lookup(argv[1]->arg)) {
		vty_out(vty, "%% Already exists \n");
		return CMD_WARNING_CONFIG_FAILED;
	}

	ret = str2prefix_ipv6(argv[3]->arg, &prefix);
	if (ret <= 0) {
		vty_out(vty, "%% Malformed address \n");
		return CMD_WARNING_CONFIG_FAILED;
	}
	apply_mask_ipv6(&prefix);

	if (argc >= 6) {
		function_bits_length = strtoul(argv[5]->arg, NULL, 10);
	}

	locator = zebra_srv6_locator_alloc(argv[1]->arg);
	if (!locator) {
		vty_out(vty, "%% Alloc failed \n");
		return CMD_WARNING_CONFIG_FAILED;
	}
	locator->prefix = prefix;
	locator->function_bits_length = function_bits_length;

	zebra_srv6_locator_add(locator);

	return CMD_SUCCESS;
}

DEFUN (no_locator_prefix,
       no_locator_prefix_cmd,
       "no locator WORD",
       NO_STR
       "Configure SRv6 locator\n"
       "Configure SRv6 locator name\n")
{
	struct zebra_srv6_locator *locator;

	locator = zebra_srv6_locator_lookup(argv[2]->arg);
	if (!locator) {
		vty_out(vty, "%% Not found \n");
		return CMD_WARNING_CONFIG_FAILED;
	}

	if (listcount(locator->functions) > 0) {
		vty_out(vty, "%% Already used \n");
		return CMD_WARNING_CONFIG_FAILED;
	}

	zebra_srv6_locator_delete(locator);

	zebra_srv6_locator_free(locator);

	return CMD_SUCCESS;
}

/* SRv6 SID configuration write function. */
static int zebra_srv6_config(struct vty *vty)
{
	struct zebra_srv6 *srv6 = zebra_srv6_get_default();
	vty_out(vty, "!\n");
	if (srv6->is_enable) {
		struct listnode *node;
		struct zebra_srv6_locator *locator;
		char str[256];
		vty_out(vty, "segment-routing-ipv6\n");
		for (ALL_LIST_ELEMENTS_RO((struct list *)srv6->locators, node, locator)) {
			inet_ntop(AF_INET6, &locator->prefix.prefix, str, sizeof(str));
			vty_out(vty, " locator %s prefix %s/%u\n", locator->name, str, locator->prefix.prefixlen);
		}
		vty_out(vty, "!\n");
	}

	return 0;
}

/* SRv6 node structure. */
static struct cmd_node srv6_node = {SRV6_NODE, "%s(config-srv6)# ", 1};

/* SRv6 VTY.  */
void zebra_srv6_vty_init(void)
{
	install_element(VIEW_NODE, &show_segment_routing_ipv6_locator_cmd);
	install_element(VIEW_NODE, &show_segment_routing_ipv6_function_cmd);

	install_node(&srv6_node, zebra_srv6_config);
	install_default(SRV6_NODE);

	install_element(CONFIG_NODE, &segment_routing_ipv6_cmd);
	install_element(CONFIG_NODE, &no_segment_routing_ipv6_cmd);
	install_element(SRV6_NODE, &locator_prefix_cmd);
	install_element(SRV6_NODE, &no_locator_prefix_cmd);
}

