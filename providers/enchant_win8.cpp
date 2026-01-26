/* Note: This file links against LGPLv3+ gnulib modules (bcp47).
 * The combined work is thus distributed under LGPLv3+.
 *
 * MIT License
 *
 * Copyright (c) 2026 Moritz Mechelk
 *
 * HexChat
 * Copyright (c) 2015 Patrick Griffis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "config.h"

#include <bcp47.h>
#include <glib.h>
#include <spellcheck.h>

#include "enchant-provider.h"

/* --------- Utils ----------*/

// convert bcp47 tags (e.g. "en-US") to xpg tags (e.g. "en_US")
static char*
tag_bcp47_to_xpg(const char* const bcp47)
{
	if (!bcp47) {
		return nullptr;
	}

	char* xpg = g_new0(char, BCP47_MAX);
	if (!xpg) {
		return nullptr;
	}

	bcp47_to_xpg(xpg, bcp47, nullptr);
	return xpg;
}

// convert xpg tags (e.g. "en_US") to bcp47 tags (e.g. "en-US")
static char*
tag_xpg_to_bcp47(const char* const xpg)
{
	if (!xpg) {
		return nullptr;
	}

	char* bcp47 = g_new0(char, BCP47_MAX);
	if (!bcp47) {
		return nullptr;
	}

	xpg_to_bcp47(bcp47, xpg);
	return bcp47;
}

static char**
enumstring_to_chararray(IEnumString* strings, size_t* out_len, gboolean tags_from_bcp47)
{
	GArray* array = g_array_new(TRUE, FALSE, sizeof(char*));

	LPOLESTR w_str;
	while (SUCCEEDED(strings->Next(1, &w_str, nullptr)) && w_str) {
		char* str = g_utf16_to_utf8((gunichar2*)w_str, -1, nullptr, nullptr, nullptr);

		if (str) {
			if (tags_from_bcp47) {
				char* xpg_tag = tag_bcp47_to_xpg(str);
				g_free(str);

				if (xpg_tag) {
					g_array_append_val(array, xpg_tag);
				}
			} else {
				g_array_append_val(array, str);
			}
		}

		CoTaskMemFree(w_str);
	}

	strings->Release();

	*out_len = array->len;
	return (char**)g_array_free(array, FALSE);
}

/* ---------- Dict ------------ */

static void
win8_dict_add_to_session(EnchantProviderDict* dict, const char* const word, size_t len)
{
	auto checker = static_cast<ISpellChecker*>(dict->user_data);
	LPWSTR w_word = (LPWSTR)g_utf8_to_utf16(word, (glong)len, nullptr, nullptr, nullptr);

	if (w_word) {
		checker->Add(w_word);
		g_free(w_word);
	}
}

static void
win8_dict_remove_from_session(EnchantProviderDict* dict, const char* const word, size_t len)
{
	auto checker = static_cast<ISpellChecker*>(dict->user_data);

	// try to use ISpellChecker2::Remove if available (Windows 10+)
	ISpellChecker2* checker2;
	if (SUCCEEDED(checker->QueryInterface(__uuidof(ISpellChecker2), (void**)&checker2))) {
		LPWSTR w_word = (LPWSTR)g_utf8_to_utf16(word, (glong)len, nullptr, nullptr, nullptr);

		if (w_word) {
			checker2->Remove(w_word);
			g_free(w_word);
		}

		checker2->Release();
	}
}

static int
win8_dict_check(EnchantProviderDict* dict, const char* const word, size_t len)
{
	auto checker = static_cast<ISpellChecker*>(dict->user_data);
	LPWSTR w_word = (LPWSTR)g_utf8_to_utf16(word, (glong)len, nullptr, nullptr, nullptr);

	if (!w_word) {
		return -1; // conversion error
	}

	IEnumSpellingError* errors;
	HRESULT hr = checker->Check(w_word, &errors);
	g_free(w_word);

	if (FAILED(hr)) {
		return -1; // error
	}

	ISpellingError* error;
	if (errors->Next(&error) == S_OK) {
		error->Release();
		errors->Release();
		return 1; // spelling issue
	}

	errors->Release();
	return 0; // correct
}

static char**
win8_dict_suggest(EnchantProviderDict* dict, const char* const word, size_t len, size_t* out_n_suggs)
{
	auto checker = static_cast<ISpellChecker*>(dict->user_data);
	LPWSTR w_word = (LPWSTR)g_utf8_to_utf16(word, (glong)len, nullptr, nullptr, nullptr);

	if (!w_word) {
		*out_n_suggs = 0;
		return nullptr;
	}

	IEnumString* suggestions;
	HRESULT hr = checker->Suggest(w_word, &suggestions);
	g_free(w_word);

	if (FAILED(hr)) {
		*out_n_suggs = 0;
		return nullptr;
	}

	return enumstring_to_chararray(suggestions, out_n_suggs, FALSE);
}

/* ---------- Provider ------------ */

static EnchantProviderDict*
win8_provider_request_dict(EnchantProvider* provider, const char* const xpg_tag)
{
	auto factory = static_cast<ISpellCheckerFactory*>(provider->user_data);

	char* bcp47_tag = tag_xpg_to_bcp47(xpg_tag);
	if (!bcp47_tag) {
		return nullptr;
	}

	LPWSTR w_bcp47_tag = (LPWSTR)g_utf8_to_utf16(bcp47_tag, -1, nullptr, nullptr, nullptr);
	g_free(bcp47_tag);
	if (!w_bcp47_tag) {
		return nullptr;
	}

	ISpellChecker* checker;
	HRESULT hr = factory->CreateSpellChecker(w_bcp47_tag, &checker);
	g_free(w_bcp47_tag);

	if (FAILED(hr)) {
		return nullptr;
	}

	EnchantProviderDict* dict = enchant_provider_dict_new(provider, xpg_tag);
	dict->user_data = checker;

	dict->suggest = win8_dict_suggest;
	dict->check = win8_dict_check;
	dict->add_to_session = win8_dict_add_to_session;
	dict->remove_from_session = win8_dict_remove_from_session;

	return dict;
}

static void
win8_provider_dispose_dict(_GL_UNUSED EnchantProvider* provider, EnchantProviderDict* dict)
{
	if (dict) {
		auto checker = static_cast<ISpellChecker*>(dict->user_data);
		checker->Release();
	}
}

static int
win8_provider_dictionary_exists(EnchantProvider* provider, const char* const xpg_tag)
{
	auto factory = static_cast<ISpellCheckerFactory*>(provider->user_data);

	char* bcp47_tag = tag_xpg_to_bcp47(xpg_tag);
	if (!bcp47_tag) {
		return 0;
	}

	LPWSTR w_bcp47_tag = (LPWSTR)g_utf8_to_utf16(bcp47_tag, -1, nullptr, nullptr, nullptr);
	g_free(bcp47_tag);
	if (!w_bcp47_tag) {
		return 0;
	}

	BOOL is_supported = FALSE;
	factory->IsSupported(w_bcp47_tag, &is_supported);
	g_free(w_bcp47_tag);

	return is_supported;
}

static char**
win8_provider_list_dicts(EnchantProvider* provider, size_t* out_n_dicts)
{
	auto factory = static_cast<ISpellCheckerFactory*>(provider->user_data);

	IEnumString* dicts;
	if (FAILED(factory->get_SupportedLanguages(&dicts))) {
		*out_n_dicts = 0;
		return nullptr;
	}

	return enumstring_to_chararray(dicts, out_n_dicts, TRUE);
}

static void
win8_provider_dispose(EnchantProvider* provider)
{
	if (provider) {
		auto factory = static_cast<ISpellCheckerFactory*>(provider->user_data);
		factory->Release();
	}

	CoUninitialize();
}

static const char*
win8_provider_identify(_GL_UNUSED EnchantProvider* provider)
{
    return "win8";
}

static const char*
win8_provider_describe(_GL_UNUSED EnchantProvider* provider)
{
    return "Windows 8 SpellCheck Provider";
}

extern "C" EnchantProvider*
init_enchant_provider(void);

EnchantProvider*
init_enchant_provider(void)
{
	// Initialize COM (required before using SpellCheckerFactory)
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
		// RPC_E_CHANGED_MODE means COM was already initialized with a different
		// threading model, which is fine - we can still use it
		return nullptr;
	}

	ISpellCheckerFactory* factory;
	if (FAILED(CoCreateInstance(__uuidof(SpellCheckerFactory), nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory)))) {
		CoUninitialize();
		return nullptr;
	}

	EnchantProvider* provider = enchant_provider_new();
	provider->user_data = factory;

	provider->dispose = win8_provider_dispose;
	provider->request_dict = win8_provider_request_dict;
	provider->dispose_dict = win8_provider_dispose_dict;
	provider->dictionary_exists = win8_provider_dictionary_exists;
	provider->identify = win8_provider_identify;
	provider->describe = win8_provider_describe;
	provider->list_dicts = win8_provider_list_dicts;

	return provider;
}
