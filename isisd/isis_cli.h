/*
 * Copyright (C) 2018       Volta Networks
 *                          Emanuele Di Pascale
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; see the file COPYING; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef ISISD_ISIS_CLI_H_
#define ISISD_ISIS_CLI_H_

/* add cli_show declarations here as externs */
void cli_show_router_isis(struct vty *vty, struct lyd_node *dnode,
			  bool show_defaults);
void cli_show_ip_isis_ipv4(struct vty *vty, struct lyd_node *dnode,
			   bool show_defaults);
void cli_show_ip_isis_ipv6(struct vty *vty, struct lyd_node *dnode,
			   bool show_defaults);
void cli_show_ip_isis_bfd_monitoring(struct vty *vty, struct lyd_node *dnode,
				     bool show_defaults);
void cli_show_isis_area_address(struct vty *vty, struct lyd_node *dnode,
				bool show_defaults);
void cli_show_isis_is_type(struct vty *vty, struct lyd_node *dnode,
			   bool show_defaults);
void cli_show_isis_dynamic_hostname(struct vty *vty, struct lyd_node *dnode,
				    bool show_defaults);
void cli_show_isis_attached(struct vty *vty, struct lyd_node *dnode,
			    bool show_defaults);
void cli_show_isis_overload(struct vty *vty, struct lyd_node *dnode,
			    bool show_defaults);
void cli_show_isis_metric_style(struct vty *vty, struct lyd_node *dnode,
				bool show_defaults);
void cli_show_isis_area_pwd(struct vty *vty, struct lyd_node *dnode,
			    bool show_defaults);
void cli_show_isis_domain_pwd(struct vty *vty, struct lyd_node *dnode,
			      bool show_defaults);
void cli_show_isis_lsp_gen_interval(struct vty *vty, struct lyd_node *dnode,
				    bool show_defaults);
void cli_show_isis_lsp_ref_interval(struct vty *vty, struct lyd_node *dnode,
				    bool show_defaults);
void cli_show_isis_lsp_max_lifetime(struct vty *vty, struct lyd_node *dnode,
				    bool show_defaults);
void cli_show_isis_lsp_mtu(struct vty *vty, struct lyd_node *dnode,
			   bool show_defaults);
void cli_show_isis_spf_min_interval(struct vty *vty, struct lyd_node *dnode,
				    bool show_defaults);
void cli_show_isis_spf_ietf_backoff(struct vty *vty, struct lyd_node *dnode,
				    bool show_defaults);
void cli_show_isis_purge_origin(struct vty *vty, struct lyd_node *dnode,
				bool show_defaults);
void cli_show_isis_mpls_te(struct vty *vty, struct lyd_node *dnode,
			   bool show_defaults);
void cli_show_isis_mpls_te_router_addr(struct vty *vty, struct lyd_node *dnode,
				       bool show_defaults);
void cli_show_isis_def_origin_ipv4(struct vty *vty, struct lyd_node *dnode,
				   bool show_defaults);
void cli_show_isis_def_origin_ipv6(struct vty *vty, struct lyd_node *dnode,
				   bool show_defaults);
void cli_show_isis_redistribute_ipv4(struct vty *vty, struct lyd_node *dnode,
				     bool show_defaults);
void cli_show_isis_redistribute_ipv6(struct vty *vty, struct lyd_node *dnode,
				     bool show_defaults);
void cli_show_isis_mt_ipv4_multicast(struct vty *vty, struct lyd_node *dnode,
				     bool show_defaults);
void cli_show_isis_mt_ipv4_mgmt(struct vty *vty, struct lyd_node *dnode,
				bool show_defaults);
void cli_show_isis_mt_ipv6_unicast(struct vty *vty, struct lyd_node *dnode,
				   bool show_defaults);
void cli_show_isis_mt_ipv6_multicast(struct vty *vty, struct lyd_node *dnode,
				     bool show_defaults);
void cli_show_isis_mt_ipv6_mgmt(struct vty *vty, struct lyd_node *dnode,
				bool show_defaults);
void cli_show_isis_mt_ipv6_dstsrc(struct vty *vty, struct lyd_node *dnode,
				  bool show_defaults);
void cli_show_ip_isis_passive(struct vty *vty, struct lyd_node *dnode,
			      bool show_defaults);
void cli_show_ip_isis_password(struct vty *vty, struct lyd_node *dnode,
			       bool show_defaults);
void cli_show_ip_isis_metric(struct vty *vty, struct lyd_node *dnode,
			     bool show_defaults);
void cli_show_ip_isis_hello_interval(struct vty *vty, struct lyd_node *dnode,
				     bool show_defaults);
void cli_show_ip_isis_hello_multi(struct vty *vty, struct lyd_node *dnode,
				  bool show_defaults);
void cli_show_ip_isis_threeway_shake(struct vty *vty, struct lyd_node *dnode,
				     bool show_defaults);
void cli_show_ip_isis_hello_padding(struct vty *vty, struct lyd_node *dnode,
				    bool show_defaults);
void cli_show_ip_isis_csnp_interval(struct vty *vty, struct lyd_node *dnode,
				    bool show_defaults);
void cli_show_ip_isis_psnp_interval(struct vty *vty, struct lyd_node *dnode,
				    bool show_defaults);
void cli_show_ip_isis_mt_ipv4_unicast(struct vty *vty, struct lyd_node *dnode,
				      bool show_defaults);
void cli_show_ip_isis_mt_ipv4_multicast(struct vty *vty, struct lyd_node *dnode,
					bool show_defaults);
void cli_show_ip_isis_mt_ipv4_mgmt(struct vty *vty, struct lyd_node *dnode,
				   bool show_defaults);
void cli_show_ip_isis_mt_ipv6_unicast(struct vty *vty, struct lyd_node *dnode,
				      bool show_defaults);
void cli_show_ip_isis_mt_ipv6_multicast(struct vty *vty, struct lyd_node *dnode,
					bool show_defaults);
void cli_show_ip_isis_mt_ipv6_mgmt(struct vty *vty, struct lyd_node *dnode,
				   bool show_defaults);
void cli_show_ip_isis_mt_ipv6_dstsrc(struct vty *vty, struct lyd_node *dnode,
				     bool show_defaults);
void cli_show_ip_isis_circ_type(struct vty *vty, struct lyd_node *dnode,
				bool show_defaults);
void cli_show_ip_isis_network_type(struct vty *vty, struct lyd_node *dnode,
				   bool show_defaults);
void cli_show_ip_isis_priority(struct vty *vty, struct lyd_node *dnode,
			       bool show_defaults);
void cli_show_isis_log_adjacency(struct vty *vty, struct lyd_node *dnode,
				 bool show_defaults);

void cli_show_isis_srv6(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_flex_algo_export_fapm_from_l1_to_l2(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_flex_algo_export_fapm_from_l2_to_l1(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_affinity_map(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_affinity_map_affinity_map_name(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_affinity_map_bit_position(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_flex_algo(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_flex_algo_algo_num(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_flex_algo_exclude_affinity(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_flex_algo_include_any_affinity(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_flex_algo_include_all_affinity(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_flex_algo_use_fapm(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_flex_algo_priority(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_srv6_locator(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_srv6_locator_algorithm(struct vty *vty, struct lyd_node *dnode, bool show_defaults);
void cli_show_isis_srv6_if_affinity_flex_algo(struct vty *vty, struct lyd_node *dnode, bool show_defaults);

#endif /* ISISD_ISIS_CLI_H_ */
