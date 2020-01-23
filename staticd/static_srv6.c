
#include <zebra.h>

#include "command.h"
#include "linklist.h"

#include "staticd/static_memory.h"
#include "staticd/static_srv6.h"

extern void zsend_static_srv6_function_add(struct static_srv6_function *function);
extern void zsend_static_srv6_function_delete(struct static_srv6_function *function);

DEFINE_MTYPE_STATIC(STATIC, SRV6, "Static SRv6");

static struct list *srv6_locators;

/*
 * locator
 */

struct static_srv6_locator *static_srv6_locator_alloc(const char *name)
{
	struct static_srv6_locator *locator = NULL;
	locator = XCALLOC(MTYPE_SRV6, sizeof(struct static_srv6_locator));
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

void static_srv6_locator_free(struct static_srv6_locator *locator)
{
	struct listnode *node, *nextnode;
	struct static_srv6_function *function;
	if (locator->functions) {
		/* XXX: */
		for (ALL_LIST_ELEMENTS((struct list *)locator->functions, node, nextnode, function)) {
			static_srv6_function_free(function);
		}
		list_delete(&locator->functions);
	}
	XFREE(MTYPE_SRV6, locator);
}

void static_srv6_locator_add(struct static_srv6_locator *locator)
{
	listnode_add(srv6_locators, locator);
}

void static_srv6_locator_delete(struct static_srv6_locator *locator)
{
	listnode_delete(srv6_locators, locator);
}

struct static_srv6_locator *static_srv6_locator_lookup(const char *name)
{
	struct static_srv6_locator *locator;
	struct listnode *node;
	for (ALL_LIST_ELEMENTS_RO(srv6_locators, node, locator)) {
		if (!strncmp(name, locator->name, SRV6_LOCNAME_SIZE)) {
			return locator;
		}
	}
	return NULL;
}

/*
 * function
 */

struct static_srv6_function *static_srv6_function_alloc(struct prefix_ipv6 *prefix)
{
	struct static_srv6_function *function = NULL;
	function = XCALLOC(MTYPE_SRV6, sizeof(struct static_srv6_function));
	if (function) {
		function->prefix = *prefix;
	}
	return function;
}

void static_srv6_function_free(struct static_srv6_function *function)
{
	XFREE(MTYPE_SRV6, function);
}

void static_srv6_function_add(struct static_srv6_function *function, struct static_srv6_locator *locator)
{
	listnode_add(locator->functions, function);
	function->locator = locator;
	zsend_static_srv6_function_add(function);
}

void static_srv6_function_delete(struct static_srv6_function *function, struct static_srv6_locator *locator)
{
	zsend_static_srv6_function_delete(function);
	// function->locator = NULL;
	// listnode_delete(locator->functions, function);
}

struct static_srv6_function *static_srv6_function_lookup(const struct static_srv6_locator *locator, const struct prefix_ipv6 *prefix)
{
	struct static_srv6_function *function;
	struct listnode *node;
	for (ALL_LIST_ELEMENTS_RO(locator->functions, node, function)) {
		if (!prefix_cmp(prefix, &function->prefix)) {
			return function;
		}
	}
	return NULL;
}

struct static_srv6_function *static_srv6_function_find_by_request_key(const struct static_srv6_locator *locator, const uint32_t request_key)
{
	struct static_srv6_function *function;
	struct listnode *node;
	for (ALL_LIST_ELEMENTS_RO(locator->functions, node, function)) {
		if (request_key == function->request_key) {
			return function;
		}
	}
	return NULL;
}

/*
 * misc
 */

struct list *static_srv6_locators(void)
{
	return srv6_locators;
}

int static_srv6_config(struct vty *vty)
{
	struct listnode *lnode, *fnode;
	struct static_srv6_locator *locator;
	struct static_srv6_function *function;
	char str[256];
	for (ALL_LIST_ELEMENTS_RO((struct list *)srv6_locators, lnode, locator)) {
		for (ALL_LIST_ELEMENTS_RO((struct list *)locator->functions, fnode, function)) {
			inet_ntop(AF_INET6, &function->prefix.prefix, str, sizeof(str));
			vty_out(vty, " %s %s\n", locator->name, str);
		}
	}
	return 0;
}

void static_srv6_init(void)
{
	srv6_locators = list_new();
}
