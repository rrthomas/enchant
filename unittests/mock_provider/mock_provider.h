#ifndef ___MOCK_PROVIDER_H
#define ___MOCK_PROVIDER_H

#include "enchant-provider.h"

#ifdef _MSC_VER
#pragma once
#endif

typedef void (*ConfigureHook) (EnchantProvider * me, const char * dir_name);

#ifdef __cplusplus
extern "C" {
#endif

ENCHANT_MODULE_EXPORT(void) set_configure(ConfigureHook hook);

#ifdef __cplusplus
}
#endif

#endif