
#include <zebra.h>

#include "command.h"
#include "table.h"
#include "srcdest_table.h"
#include "prefix.h"

#include "isisd/isis_memory.h"
#include "isisd/isisd.h"
#include "isisd/isis_srv6.h"
#include "isisd/isis_cli.h"
#include "isisd/isis_tlvs.h"
#include "isisd/isis_misc.h"
#include "isisd/isis_spf.h"
#include "isis_spf_private.h"

DEFINE_MTYPE_STATIC(ISISD, SRV6, "ISIS SRv6");

/*
 * locator
 */

struct isis_srv6_locator *isis_srv6_locator_alloc(const char *name)
{
	struct isis_srv6_locator *locator = NULL;
	locator = XCALLOC(MTYPE_SRV6, sizeof(struct isis_srv6_locator));
	if (locator) {
		int namelen = sizeof(locator->name);
		if (namelen > SRV6_LOCNAME_SIZE - 1) {
			namelen = SRV6_LOCNAME_SIZE - 1;
		}
		strlcpy(locator->name, name, namelen);
		locator->functions = list_new();
	}
	return locator;
}

void isis_srv6_locator_free(struct isis_srv6_locator *locator)
{
	struct listnode *node, *nextnode;
	struct isis_srv6_function *function;
	if (locator->functions) {
		/* XXX: */
		for (ALL_LIST_ELEMENTS((struct list *)locator->functions, node, nextnode, function)) {
			isis_srv6_function_free(function);
		}
		list_delete(&locator->functions);
	}
	XFREE(MTYPE_SRV6, locator);
}

void isis_srv6_locator_add_zebra(struct isis_srv6_locator *locator)
{
	struct isis_area *area;
	struct listnode *node;
	struct isis_srv6_locator *tmp;
	listnode_add(isis->srv6_locators, locator);
	for (ALL_LIST_ELEMENTS_RO(isis->area_list, node, area)) {
		tmp = isis_srv6_locator_lookup(locator->name, area);
		if (tmp) {
			tmp->prefix = locator->prefix;
			lsp_regenerate_schedule(area, area->is_type, 0);
		}
	}
}

void isis_srv6_locator_delete_zebra(struct isis_srv6_locator *locator)
{
	struct isis_area *area;
	struct listnode *node;
	struct isis_srv6_locator *tmp;
	struct prefix_ipv6 prefix_zero = { .family = AF_INET6, };
	listnode_delete(isis->srv6_locators, locator);
	for (ALL_LIST_ELEMENTS_RO(isis->area_list, node, area)) {
		tmp = isis_srv6_locator_lookup(locator->name, area);
		if (tmp) {
			tmp->prefix = prefix_zero;
			lsp_regenerate_schedule(area, area->is_type, 0);
		}
	}
}

struct isis_srv6_locator *isis_srv6_locator_lookup_zebra(const char *name)
{
	struct isis_srv6_locator *locator;
	struct listnode *node;
	for (ALL_LIST_ELEMENTS_RO(isis->srv6_locators, node, locator)) {
		if (!strncmp(name, locator->name, SRV6_LOCNAME_SIZE)) {
			return locator;
		}
	}
	return NULL;
}

void isis_srv6_locator_add(struct isis_srv6_locator *locator, struct isis_area *area)
{
	struct isis_srv6_locator *tmp;
	listnode_add(area->srv6_locators, locator);
	tmp = isis_srv6_locator_lookup_zebra(locator->name);
	if (tmp) {
		locator->prefix = tmp->prefix;
		lsp_regenerate_schedule(area, area->is_type, 0);
	}
}

void isis_srv6_locator_delete(struct isis_srv6_locator *locator, struct isis_area *area)
{
	struct isis_srv6_locator *tmp;
	struct prefix_ipv6 prefix_zero = { .family = AF_INET6, };
	listnode_delete(area->srv6_locators, locator);
	tmp = isis_srv6_locator_lookup_zebra(locator->name);
	if (tmp) {
		locator->prefix = prefix_zero;
		lsp_regenerate_schedule(area, area->is_type, 0);
	}
}

struct isis_srv6_locator *isis_srv6_locator_lookup(const char *name, struct isis_area *area)
{
	struct isis_srv6_locator *locator;
	struct listnode *node;
	for (ALL_LIST_ELEMENTS_RO(area->srv6_locators, node, locator)) {
		if (!strncmp(name, locator->name, SRV6_LOCNAME_SIZE)) {
			return locator;
		}
	}
	return NULL;
}

/*
 * function
 */

struct isis_srv6_function *isis_srv6_function_alloc(struct prefix_ipv6 *prefix)
{
	struct isis_srv6_function *function = NULL;
	function = XCALLOC(MTYPE_SRV6, sizeof(struct isis_srv6_function));
	if (function) {
		function->prefix = *prefix;
	}
	return function;
}

void isis_srv6_function_free(struct isis_srv6_function *function)
{
	XFREE(MTYPE_SRV6, function);
}

void isis_srv6_function_add(struct isis_srv6_function *function, struct isis_srv6_locator *locator)
{
	listnode_add(locator->functions, function);
	function->locator = locator;
}

void isis_srv6_function_delete(struct isis_srv6_function *function, struct isis_srv6_locator *locator)
{
	function->locator = NULL;
	listnode_delete(locator->functions, function);
}

struct isis_srv6_function *isis_srv6_function_lookup(const struct isis_srv6_locator *locator, const struct prefix_ipv6 *prefix)
{
	struct isis_srv6_function *function;
	struct listnode *node;
	for (ALL_LIST_ELEMENTS_RO(locator->functions, node, function)) {
		if (!prefix_cmp(prefix, &function->prefix)) {
			return function;
		}
	}
	return NULL;
}

/*
 * affinity_map
 */

struct isis_srv6_affinity_map *isis_srv6_affinity_map_alloc(const char *name, int pos)
{
	struct isis_srv6_affinity_map *affinity_map = NULL;
	affinity_map = XCALLOC(MTYPE_SRV6, sizeof(struct isis_srv6_affinity_map));
	if (affinity_map) {
		int namelen = sizeof(affinity_map->name);
		if (namelen > SRV6_AFFNAME_SIZE - 1) {
			namelen = SRV6_AFFNAME_SIZE - 1;
		}
		strlcpy(affinity_map->name, name, namelen);
		affinity_map->bit_position = pos;
	}
	return affinity_map;
}

void isis_srv6_affinity_map_free(struct isis_srv6_affinity_map *affinity_map)
{
	XFREE(MTYPE_SRV6, affinity_map);
}

void isis_srv6_affinity_map_add(struct isis_srv6_affinity_map *affinity_map, struct isis_area *area)
{
	listnode_add(area->srv6_affinity_maps, affinity_map);
}

void isis_srv6_affinity_map_delete(struct isis_srv6_affinity_map *affinity_map, struct isis_area *area)
{
	listnode_delete(area->srv6_affinity_maps, affinity_map);
}

struct isis_srv6_affinity_map *isis_srv6_affinity_map_lookup(const char *name, struct isis_area *area)
{
	struct isis_srv6_affinity_map *affinity_map;
	struct listnode *node;
	for (ALL_LIST_ELEMENTS_RO(area->srv6_affinity_maps, node, affinity_map)) {
		if (!strncmp(name, affinity_map->name, SRV6_AFFNAME_SIZE)) {
			return affinity_map;
		}
	}
	return NULL;
}

/*
 * flex_algo_definition
 */

struct isis_srv6_flex_algo_definition *isis_srv6_flex_algo_definition_alloc(int algonum, uint32_t *exclude, uint32_t *include_any, uint32_t *include_all, bool use_fapm, int priority)
{
	struct isis_srv6_flex_algo_definition *flex_algo = NULL;
	flex_algo = XCALLOC(MTYPE_SRV6, sizeof(struct isis_srv6_flex_algo_definition));
	if (flex_algo) {
		flex_algo->algonum = algonum;
		for (int i = 0; i < SRV6_AFFARR_SIZE; i++) {
			flex_algo->exclude[i] = exclude[i];
			flex_algo->include_any[i] = include_any[i];
			flex_algo->include_all[i] = include_all[i];
		}
		flex_algo->priority = priority;
		flex_algo->use_fapm = use_fapm;
	}
	return flex_algo;
}

void isis_srv6_flex_algo_definition_free(struct isis_srv6_flex_algo_definition *flex_algo)
{
	XFREE(MTYPE_SRV6, flex_algo);
}

void isis_srv6_flex_algo_definition_add(struct isis_srv6_flex_algo_definition *flex_algo, struct isis_area *area)
{
	listnode_add(area->srv6_flex_algo_definitions, flex_algo);
	isis_srv6_flex_algo_participate_update(area);
	lsp_regenerate_schedule(area, area->is_type, 0);
}

void isis_srv6_flex_algo_definition_delete(struct isis_srv6_flex_algo_definition *flex_algo, struct isis_area *area)
{
	listnode_delete(area->srv6_flex_algo_definitions, flex_algo);
	isis_srv6_flex_algo_participate_update(area);
	lsp_regenerate_schedule(area, area->is_type, 0);
}

struct isis_srv6_flex_algo_definition *isis_srv6_flex_algo_definition_lookup(int algonum, struct isis_area *area)
{
	struct isis_srv6_flex_algo_definition *flex_algo;
	struct listnode *node;
	for (ALL_LIST_ELEMENTS_RO(area->srv6_flex_algo_definitions, node, flex_algo)) {
		if (algonum == flex_algo->algonum) {
			return flex_algo;
		}
	}
	return NULL;
}

/*
 * flex_algo_participate
 */

struct isis_srv6_flex_algo_participate *isis_srv6_flex_algo_participate_alloc(int algonum, uint32_t *exclude, uint32_t *include_any, uint32_t *include_all, struct isis_area *area)
{
	struct isis_srv6_flex_algo_participate *participate = NULL;
	participate = XCALLOC(MTYPE_SRV6, sizeof(struct isis_srv6_flex_algo_participate));
	if (participate) {
		participate->algonum = algonum;
		for (int i = 0; i < SRV6_AFFARR_SIZE; i++) {
			participate->exclude[i] = exclude[i];
			participate->include_any[i] = include_any[i];
			participate->include_all[i] = include_all[i];
		}
		participate->priority = SRV6_FLEXALGO_PRIO_DEFAULT;
		participate->use_fapm = false;
		for (int level = ISIS_LEVEL1; level <= ISIS_LEVEL2; level++) {
			if (!(area->is_type & level))
				continue;
			if (participate->spftree[level - 1])
				continue;
			participate->spftree[level - 1] = isis_spftree_new(area);
		}
		participate->participating = true;
		participate->area = area;
	}
	return participate;
}

void isis_srv6_flex_algo_participate_free(struct isis_srv6_flex_algo_participate *participate)
{
	if (participate) {
		for (int level = ISIS_LEVEL1; level <= ISIS_LEVEL2; level++) {
			if (!(participate->area->is_type & level))
				continue;
			if (!participate->spftree[level - 1])
				continue;
			isis_spftree_del(participate->spftree[level - 1]);
		}
	}
	XFREE(MTYPE_SRV6, participate);
}

void isis_srv6_flex_algo_participate_add(struct isis_srv6_flex_algo_participate *participate, struct isis_area *area)
{
	struct isis_srv6_flex_algo_participate *current;
	current = isis_srv6_flex_algo_participate_lookup(participate->algonum, area);
	if (current) {
		if ((current->priority > participate->priority)
		 || (current->priority == participate->priority
		  && memcmp(current->sysid, participate->sysid, ISIS_SYS_ID_LEN) > 0)) {
			for (int i = 0; i < SRV6_AFFARR_SIZE; i++) {
				participate->exclude[i] = current->exclude[i];
				participate->include_any[i] = current->include_any[i];
				participate->include_all[i] = current->include_all[i];
			}
			participate->priority = current->priority;
			participate->use_fapm = current->use_fapm;
			memcpy(current->sysid, participate->sysid, ISIS_SYS_ID_LEN);
			participate->participating = current->participating;
		}
		isis_srv6_flex_algo_participate_free(current);
	}
	listnode_add_sort(area->srv6_flex_algo_participates, participate);
}

void isis_srv6_flex_algo_participate_delete(struct isis_srv6_flex_algo_participate *participate, struct isis_area *area)
{
	listnode_delete(area->srv6_flex_algo_participates, participate);
}

struct isis_srv6_flex_algo_participate *isis_srv6_flex_algo_participate_lookup(int algonum, struct isis_area *area)
{
	struct isis_srv6_flex_algo_participate *participate;
	struct listnode *node;
	for (ALL_LIST_ELEMENTS_RO(area->srv6_flex_algo_participates, node, participate)) {
		if (algonum == participate->algonum) {
			return participate;
		}
	}
	return NULL;
}

int isis_srv6_flex_algo_participate_cmp(void *val1, void *val2)
{
	struct isis_srv6_flex_algo_participate *participate1 = (struct isis_srv6_flex_algo_participate *)val1;
	struct isis_srv6_flex_algo_participate *participate2 = (struct isis_srv6_flex_algo_participate *)val2;
	assert(participate1);
	assert(participate2);
	if (participate1->algonum < participate2->algonum)
		return -1;
	if (participate1->algonum > participate2->algonum)
		return 1;
	return 0;
}

/*
 * misc
 */

bool isis_srv6_affinity_maps_set(uint32_t *affinity_maps_arr, const char *affinity_maps_str, struct isis_area *area)
{
	char buf[SRV6_AFFNAME_SIZE];
	char *head, *tail;
	bool last;
	struct isis_srv6_affinity_map *affinity_map;
	int arridx;
	uint32_t bitmap;
	for (int i = 0; i < SRV6_AFFARR_SIZE; i++) {
		affinity_maps_arr[i] = 0;
	}
	strlcpy(buf, affinity_maps_str, SRV6_AFFNAME_SIZE);
	head = tail = buf;
	last = false;
	while (!last) {
		while (*tail != '\0' && *tail != ',') {
			++tail;
		}
		if (*tail == '\0') {
			last = true;
		} else {
			*tail = '\0';
		}
		affinity_map = isis_srv6_affinity_map_lookup(head, area);
		if (!affinity_map) {
			return false;
		}
		arridx = affinity_map->bit_position >> 5;
		bitmap = 1 << (affinity_map->bit_position & 0x1f);
		affinity_maps_arr[arridx] |= bitmap;
		++tail;
		head = tail;
	}
	return true;
}

bool isis_srv6_affinity_maps_eq(uint32_t *affinity_maps_arr1, uint32_t *affinity_maps_arr2)
{
	for (int i = 0; i < SRV6_AFFARR_SIZE; i++) {
		if (affinity_maps_arr1[i] != affinity_maps_arr2[i]) {
			return false;
		}
	}
	return true;
}

bool isis_srv6_affinity_maps_exc(uint32_t *affinity_maps_arr1, uint32_t *affinity_maps_arr2)
{
	for (int i = 0; i < SRV6_AFFARR_SIZE; i++) {
		if (affinity_maps_arr1[i] & affinity_maps_arr2[i]) {
			return false;
		}
	}
	return true;
}

bool isis_srv6_affinity_maps_incany(uint32_t *affinity_maps_arr1, uint32_t *affinity_maps_arr2)
{
	for (int i = 0; i < SRV6_AFFARR_SIZE; i++) {
		if (affinity_maps_arr1[i] & affinity_maps_arr2[i]) {
			return true;
		}
	}
	return false;
}

bool isis_srv6_affinity_maps_incall(uint32_t *affinity_maps_arr1, uint32_t *affinity_maps_arr2)
{
	for (int i = 0; i < SRV6_AFFARR_SIZE; i++) {
		if ((affinity_maps_arr1[i] & affinity_maps_arr2[i]) != affinity_maps_arr1[i]) {
			return false;
		}
	}
	return true;
}

bool isis_srv6_affinity_maps_zero(uint32_t *affinity_maps_arr)
{
	for (int i = 0; i < SRV6_AFFARR_SIZE; i++) {
		if (affinity_maps_arr[i])
			return false;
	}
	return true;
}

bool isis_srv6_affinity_map_name_valid(const char *affinity_map_name)
{
	const char *c;
	for (c = affinity_map_name; *c != '\0'; c++) {
		if ('a' < *c && *c < 'z')
			continue;
		if ('A' < *c && *c > 'Z')
			continue;
		if ('0' < *c && *c > '9')
			continue;
		if (*c == '_' || *c == '-')
			continue;
		return false;
	}
	return true;
}

bool isis_srv6_affinity_map_name_used(const char *affinity_map_name, struct isis_area *area)
{
	struct listnode *node;
	struct isis_srv6_flex_algo_definition *flex_algo;
	struct isis_circuit *circuit;
	uint32_t affmap[SRV6_AFFARR_SIZE];
	isis_srv6_affinity_maps_set(affmap, affinity_map_name, area);
	for (ALL_LIST_ELEMENTS_RO(area->srv6_flex_algo_definitions, node, flex_algo)) {
		if (isis_srv6_affinity_maps_incany(affmap, flex_algo->exclude))
			return true;
		if (isis_srv6_affinity_maps_incany(affmap, flex_algo->include_any))
			return true;
		if (isis_srv6_affinity_maps_incany(affmap, flex_algo->include_all))
			return true;
	}
	for (ALL_LIST_ELEMENTS_RO(area->circuit_list, node, circuit)) {
		if (isis_srv6_affinity_maps_incany(affmap, circuit->affinity_flex_algo))
			return true;
	}
	return false;
}

void isis_srv6_algo_fill(uint8_t *algo, struct isis_area *area)
{
	struct isis_srv6_flex_algo_participate *participate;
	struct listnode *node;
	int algoindex;
	algo[0] = SR_ALGORITHM_SPF;
	algoindex = 1;
	for (ALL_LIST_ELEMENTS_RO(area->srv6_flex_algo_participates, node, participate)) {
		if (algoindex == SR_ALGORITHM_COUNT)
			break;
		if (!participate->participating)
			continue;
		algo[algoindex] = participate->algonum;
		++algoindex;
	}
	for (; algoindex < SR_ALGORITHM_COUNT; ++algoindex) {
		algo[algoindex] = SR_ALGORITHM_UNSET;
	}
}

bool isis_srv6_flex_algo_participate_update(struct isis_area *area)
{
	bool modified = false;
	struct listnode *node, *nextnode;
	struct isis_srv6_flex_algo_definition *flex_algo;
	struct isis_srv6_flex_algo_participate *participate;
	struct isis_lsp *lsp;
	for (ALL_LIST_ELEMENTS_RO(area->srv6_flex_algo_participates, node, participate)) {
		participate->check_flag = false;
	}
	for (ALL_LIST_ELEMENTS_RO(area->srv6_flex_algo_definitions, node, flex_algo)) {
		participate = isis_srv6_flex_algo_participate_lookup(flex_algo->algonum, area);
		if (!participate) {
			participate = isis_srv6_flex_algo_participate_alloc(flex_algo->algonum,
				flex_algo->exclude, flex_algo->include_any, flex_algo->include_all, area);
			participate->priority = flex_algo->priority;
			participate->use_fapm = flex_algo->use_fapm;
			memcpy(participate->sysid, isis->sysid, ISIS_SYS_ID_LEN);
			isis_srv6_flex_algo_participate_add(participate, area);
			participate->check_flag = true;
			modified = true;
			continue;
		}
		participate->check_flag = true;
		if (flex_algo->priority > participate->priority
		 || (flex_algo->priority == participate->priority
		  && memcmp(isis->sysid, participate->sysid, ISIS_SYS_ID_LEN) > 0)) {
			for (int i = 0; i < SRV6_AFFARR_SIZE; ++i) {
				participate->exclude[i] = flex_algo->exclude[i];
				participate->include_any[i] = flex_algo->include_any[i];
				participate->include_all[i] = flex_algo->include_all[i];
			}
			participate->priority = flex_algo->priority;
			participate->use_fapm = flex_algo->use_fapm;
			memcpy(participate->sysid, isis->sysid, ISIS_SYS_ID_LEN);
			modified = true;
			continue;
		}
	}
	for (int level = ISIS_LEVEL1; level <= ISIS_LEVEL2; level++) {
		if (!lspdb_count(&area->lspdb[level - 1]))
			continue;
		frr_each (lspdb, &area->lspdb[level - 1], lsp) {
			if (lsp->own_lsp) {
				continue;
			}
			if (!lsp->tlvs) {
				continue;
			}
			if (!lsp->tlvs->router_cap) {
				continue;
			}
			if (!lsp->tlvs->router_cap->srv6_cap) {
				continue;
			}
			for (int i = 0; i < SR_ALGORITHM_COUNT; ++i) {
				struct isis_router_cap_fad *fad;
				fad = lsp->tlvs->router_cap->fads[i];
				if (!fad)
					break;
				if (fad->algonum < 128) {
					continue;
				}
				participate = isis_srv6_flex_algo_participate_lookup(fad->algonum, area);
				if (!participate) {
					/* XXX: */
					uint32_t exclude[SRV6_AFFARR_SIZE];
					uint32_t include_any[SRV6_AFFARR_SIZE];
					uint32_t include_all[SRV6_AFFARR_SIZE];
					int i;
					/* */
					for (i = 0; i < fad->exclude_arr_size; ++i) {
						if (i == SRV6_AFFARR_SIZE) break;
						exclude[i] = fad->exclude_arr[i];
					}
					for (; i < SRV6_AFFARR_SIZE; ++i) { exclude[i] = 0x000000; }
					/* */
					for (i = 0; i < fad->include_any_arr_size; ++i) {
						if (i == SRV6_AFFARR_SIZE) break;
						include_any[i] = fad->include_any_arr[i];
					}
					for (; i < SRV6_AFFARR_SIZE; ++i) { include_any[i] = 0x000000; }
					/* */
					for (i = 0; i < fad->include_all_arr_size; ++i) {
						if (i == SRV6_AFFARR_SIZE) break;
						include_all[i] = fad->include_all_arr[i];
					}
					for (; i < SRV6_AFFARR_SIZE; ++i) { include_all[i] = 0x000000; }
					/* */
					participate = isis_srv6_flex_algo_participate_alloc(fad->algonum,
						exclude, include_any, include_all, area);
					participate->priority = fad->priority;
					participate->use_fapm = fad->m_flag;
					memcpy(participate->sysid, lsp->hdr.lsp_id, ISIS_SYS_ID_LEN);
					if (fad->metric_type != 0 || fad->calc_type != 0) {
						participate->participating = false;
					}
					isis_srv6_flex_algo_participate_add(participate, area);
					participate->check_flag = true;
					modified = true;
					continue;
				}
				participate->check_flag = true;
				if (fad->priority > participate->priority
				 || (fad->priority == participate->priority
				  && memcmp(lsp->hdr.lsp_id, participate->sysid, ISIS_SYS_ID_LEN) > 0)) {
					int i;
					for (i = 0; i < SRV6_AFFARR_SIZE; ++i) {
						if (i == fad->exclude_arr_size)
							break;
						participate->exclude[i] = fad->exclude_arr[i];
					}
					for (; i < SRV6_AFFARR_SIZE; ++i) {
						participate->exclude[i] = 0x00000000;
					}
					for (i = 0; i < SRV6_AFFARR_SIZE; ++i) {
						if (i == fad->include_any_arr_size)
							break;
						participate->include_any[i] = fad->include_any_arr[i];
					}
					for (; i < SRV6_AFFARR_SIZE; ++i) {
						participate->include_any[i] = 0x00000000;
					}
					for (i = 0; i < SRV6_AFFARR_SIZE; ++i) {
						if (i == fad->include_all_arr_size)
							break;
						participate->include_all[i] = fad->include_all_arr[i];
					}
					for (; i < SRV6_AFFARR_SIZE; ++i) {
						participate->include_all[i] = 0x00000000;
					}
					participate->priority = fad->priority;
					participate->use_fapm = fad->m_flag;
					memcpy(participate->sysid, lsp->hdr.lsp_id, ISIS_SYS_ID_LEN);
					if (fad->metric_type != 0 || fad->calc_type != 0) {
						participate->participating = false;
					}
					modified = true;
					continue;
				}
			}
		}
	}
	for (ALL_LIST_ELEMENTS(area->srv6_flex_algo_participates, node, nextnode, participate)) {
		if (!participate->check_flag) {
			isis_srv6_flex_algo_participate_delete(participate, area);
			isis_srv6_flex_algo_participate_free(participate);
			modified = true;
		}
	}
	return modified;
}

void isis_srv6_init(void)
{
}
