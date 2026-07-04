#ifndef ___MOCK_PROVIDER_H
#define ___MOCK_PROVIDER_H

#include "enchant-provider.h"

typedef void (*ConfigureHook) (EnchantProvider * me);

#ifdef __cplusplus
extern "C" {
#endif

void set_configure(ConfigureHook hook, ConfigureHook hook2);
EnchantProvider * init_enchant_provider(void);

#ifdef __cplusplus
}
#endif

#endif