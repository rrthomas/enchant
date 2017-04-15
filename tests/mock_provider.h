#ifndef ___MOCK_PROVIDER_H
#define ___MOCK_PROVIDER_H

#include "enchant-provider.h"

typedef void (*ConfigureHook) (EnchantProvider * me, const char * dir_name);

#ifdef __cplusplus
extern "C" {
#endif

ENCHANT_MODULE_EXPORT(void) set_configure(ConfigureHook hook);
ENCHANT_MODULE_EXPORT(EnchantProvider *) init_enchant_provider(void);
ENCHANT_MODULE_EXPORT(void) configure_enchant_provider(EnchantProvider * me, const char *dir_name);

#ifdef __cplusplus
}
#endif

#endif