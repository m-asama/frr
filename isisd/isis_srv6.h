
#ifndef _QUAGGA_ISIS_SRV6_H
#define _QUAGGA_ISIS_SRV6_H

#include "linklist.h"
#include "prefix.h"
#include "zclient.h"
#include "isisd/isis_constants.h"

#define SRV6_AFFNAME_SIZE	256
#define SRV6_AFFARR_SIZE	8

#define SRV6_FLEXALGO_PRIO_DEFAULT	128

struct isis_area;

struct isis_srv6_locator {
	char name[SRV6_LOCNAME_SIZE];
	struct prefix_ipv6 prefix;
	uint8_t function_bits_length;
	int algonum;
	struct list *functions;
};

struct isis_srv6_function {
	struct prefix_ipv6 prefix;
	uint8_t owner_proto;
	uint16_t owner_instance;
	struct isis_srv6_locator *locator;
};

struct isis_srv6_affinity_map {
	char name[SRV6_AFFNAME_SIZE];
	int bit_position;
};

struct isis_srv6_flex_algo_definition {
	int algonum;
	uint32_t exclude[SRV6_AFFARR_SIZE];
	uint32_t include_any[SRV6_AFFARR_SIZE];
	uint32_t include_all[SRV6_AFFARR_SIZE];
	int priority;
	bool use_fapm;
};

struct isis_srv6_flex_algo_participate {
	int algonum;
	uint32_t exclude[SRV6_AFFARR_SIZE];
	uint32_t include_any[SRV6_AFFARR_SIZE];
	uint32_t include_all[SRV6_AFFARR_SIZE];
	int priority;
	bool use_fapm;
	bool participating;
	uint8_t sysid[ISIS_SYS_ID_LEN];
	struct isis_spftree *spftree[ISIS_LEVELS];
	bool check_flag;
	struct isis_area *area;
};

/*
 * locator
 */

extern struct isis_srv6_locator *isis_srv6_locator_alloc(const char *name);
extern void isis_srv6_locator_free(struct isis_srv6_locator *locator);

extern void isis_srv6_locator_add_zebra(struct isis_srv6_locator *locator);
extern void isis_srv6_locator_delete_zebra(struct isis_srv6_locator *locator);
extern struct isis_srv6_locator *isis_srv6_locator_lookup_zebra(const char *name);

extern void isis_srv6_locator_add(struct isis_srv6_locator *locator, struct isis_area *area);
extern void isis_srv6_locator_delete(struct isis_srv6_locator *locator, struct isis_area *area);
extern struct isis_srv6_locator *isis_srv6_locator_lookup(const char *name, struct isis_area *area);

/*
 * function
 */

extern struct isis_srv6_function *isis_srv6_function_alloc(struct prefix_ipv6 *prefix);
extern void isis_srv6_function_free(struct isis_srv6_function *function);

extern void isis_srv6_function_add(struct isis_srv6_function *function, struct isis_srv6_locator *locator);
extern void isis_srv6_function_delete(struct isis_srv6_function *function, struct isis_srv6_locator *locator);
extern struct isis_srv6_function *isis_srv6_function_lookup(const struct isis_srv6_locator *locator, const struct prefix_ipv6 *prefix);

/*
 * affinity_map
 */

extern struct isis_srv6_affinity_map *isis_srv6_affinity_map_alloc(const char *name, int pos);
extern void isis_srv6_affinity_map_free(struct isis_srv6_affinity_map *affinity_map);

extern void isis_srv6_affinity_map_add(struct isis_srv6_affinity_map *affinity_map, struct isis_area *area);
extern void isis_srv6_affinity_map_delete(struct isis_srv6_affinity_map *affinity_map, struct isis_area *area);
extern struct isis_srv6_affinity_map *isis_srv6_affinity_map_lookup(const char *name, struct isis_area *area);

/*
 * flex_algo_definition
 */

extern struct isis_srv6_flex_algo_definition *isis_srv6_flex_algo_definition_alloc(int algonum, uint32_t *exclude, uint32_t *include_any, uint32_t *include_all, bool use_fapm, int priority);
extern void isis_srv6_flex_algo_definition_free(struct isis_srv6_flex_algo_definition *flex_algo);

extern void isis_srv6_flex_algo_definition_add(struct isis_srv6_flex_algo_definition *flex_algo, struct isis_area *area);
extern void isis_srv6_flex_algo_definition_delete(struct isis_srv6_flex_algo_definition *flex_algo, struct isis_area *area);
extern struct isis_srv6_flex_algo_definition *isis_srv6_flex_algo_definition_lookup(int algonum, struct isis_area *area);

/*
 * flex_algo_participate
 */

extern struct isis_srv6_flex_algo_participate *isis_srv6_flex_algo_participate_alloc(int algonum, uint32_t *exclude, uint32_t *include_any, uint32_t *include_all, struct isis_area *area);
extern void isis_srv6_flex_algo_participate_free(struct isis_srv6_flex_algo_participate *participate);

extern void isis_srv6_flex_algo_participate_add(struct isis_srv6_flex_algo_participate *participate, struct isis_area *area);
extern void isis_srv6_flex_algo_participate_delete(struct isis_srv6_flex_algo_participate *participate, struct isis_area *area);
extern struct isis_srv6_flex_algo_participate *isis_srv6_flex_algo_participate_lookup(int algonum, struct isis_area *area);
extern int isis_srv6_flex_algo_participate_cmp(void *val1, void *val2);

/*
 * misc
 */

extern bool isis_srv6_affinity_maps_set(uint32_t *affinity_maps_arr, const char *affinity_maps_str, struct isis_area *area);
extern bool isis_srv6_affinity_maps_eq(uint32_t *affinity_maps_arr1, uint32_t *affinity_maps_arr2);
extern bool isis_srv6_affinity_maps_exc(uint32_t *affinity_maps_arr1, uint32_t *affinity_maps_arr2);
extern bool isis_srv6_affinity_maps_incany(uint32_t *affinity_maps_arr1, uint32_t *affinity_maps_arr2);
extern bool isis_srv6_affinity_maps_incall(uint32_t *affinity_maps_arr1, uint32_t *affinity_maps_arr2);
extern bool isis_srv6_affinity_maps_zero(uint32_t *affinity_maps_arr);

extern bool isis_srv6_affinity_map_name_valid(const char *affinity_map_name);
extern bool isis_srv6_affinity_map_name_used(const char *affinity_map_name, struct isis_area *area);

extern void isis_srv6_algo_fill(uint8_t *algo, struct isis_area *area);
extern bool isis_srv6_flex_algo_participate_update(struct isis_area *area);

void isis_srv6_init(void);

#endif /* _QUAGGA_ISIS_SRV6_H */
