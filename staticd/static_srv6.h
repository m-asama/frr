
#ifndef _QUAGGA_STATIC_SRV6_H
#define _QUAGGA_STATIC_SRV6_H

#include "linklist.h"
#include "prefix.h"
#include "zclient.h"

struct static_srv6_locator {
	char name[SRV6_LOCNAME_SIZE];
	struct prefix_ipv6 prefix;
	uint8_t function_bits_length;
	int algonum;
	struct list *functions;
};

struct static_srv6_function {
	struct prefix_ipv6 prefix;
	uint8_t owner_proto;
	uint16_t owner_instance;
	uint32_t request_key;
	struct static_srv6_locator *locator;
};

/*
 * locator
 */

extern struct static_srv6_locator *static_srv6_locator_alloc(const char *name);
extern void static_srv6_locator_free(struct static_srv6_locator *locator);

extern void static_srv6_locator_add(struct static_srv6_locator *locator);
extern void static_srv6_locator_delete(struct static_srv6_locator *locator);
extern struct static_srv6_locator *static_srv6_locator_lookup(const char *name);

/*
 * function
 */

extern struct static_srv6_function *static_srv6_function_alloc(struct prefix_ipv6 *prefix);
extern void static_srv6_function_free(struct static_srv6_function *function);

extern void static_srv6_function_add(struct static_srv6_function *function, struct static_srv6_locator *locator);
extern void static_srv6_function_delete(struct static_srv6_function *function, struct static_srv6_locator *locator);
extern struct static_srv6_function *static_srv6_function_lookup(const struct static_srv6_locator *locator, const struct prefix_ipv6 *prefix);
extern struct static_srv6_function *static_srv6_function_find_by_request_key(const struct static_srv6_locator *locator, const uint32_t request_key);

/*
 * misc
 */

extern struct list *static_srv6_locators(void);
extern int static_srv6_config(struct vty *vty);
extern void static_srv6_init(void);

#endif /* _QUAGGA_STATIC_SRV6_H */
