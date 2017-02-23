#include "config.h"

#include <string>
#include <vector>

#include <string.h>

#include "ispell.h"
#include "enchant.h"
#include "enchant-provider.h"
#include "unused-parameter.h"


ENCHANT_PLUGIN_DECLARE("Ispell")

extern "C" {

ENCHANT_MODULE_EXPORT (EnchantProvider *)
             init_enchant_provider (void);

static int
_ispell_provider_dictionary_exists (EnchantBroker * broker, const char *const szFile)
{
        std::vector <std::string> names;
        ISpellChecker * checker = new ISpellChecker (broker);

        checker->buildHashNames (names, broker, szFile);
        for (size_t i = 0; i < names.size(); i++) {
                if (g_file_test (names[i].c_str(), G_FILE_TEST_EXISTS))
                        {
                                delete checker;
                                return 1;
                        }
        }

        delete checker;
        return 0;
}

static char **
ispell_dict_suggest (EnchantDict * me, const char *const word,
                     size_t len, size_t * out_n_suggs)
{
        ISpellChecker * checker;

        checker = static_cast<ISpellChecker *>(me->user_data);
        return checker->suggestWord (word, len, out_n_suggs);
}

static int
ispell_dict_check (EnchantDict * me, const char *const word, size_t len)
{
        ISpellChecker * checker;

        checker = static_cast<ISpellChecker *>(me->user_data);

        if (checker->checkWord(word, len))
                return 0;

        return 1;
}

static EnchantDict *
ispell_provider_request_dict (EnchantProvider * me, const char *const tag)
{
        EnchantDict *dict;
        ISpellChecker * checker;

        checker = new ISpellChecker (me->owner);

        if (!checker)
                {
                        return NULL;
                }

        if (!checker->requestDictionary(tag)) {
                delete checker;
                return NULL;
        }

        dict = g_new0 (EnchantDict, 1);
        dict->user_data = checker;
        dict->check = ispell_dict_check;
        dict->suggest = ispell_dict_suggest;
        // don't implement session or personal

        return dict;
}

static void
ispell_provider_dispose_dict (EnchantProvider * me _GL_UNUSED_PARAMETER, EnchantDict * dict)
{
        ISpellChecker * checker;

        checker = static_cast<ISpellChecker *>(dict->user_data);
        delete checker;

        g_free (dict);
}

static char **
ispell_provider_list_dictionaries (EnchantProvider * me _GL_UNUSED_PARAMETER,
                                   size_t * out_n_dicts)
{
        size_t i, nb;
        ISpellChecker * checker = new ISpellChecker (me->owner);
        char ** out_dicts = g_new0 (char *, checker->ispell_map_size() + 1);

        nb = 0;
        for (i = 0; i < checker->ispell_map_size(); i++)
                if (_ispell_provider_dictionary_exists (me->owner, checker->ispell_map()[i].dict))
                        out_dicts[nb++] = g_strdup (checker->ispell_map()[i].lang);

        *out_n_dicts = nb;
        if (nb == 0) {
                g_free (out_dicts);
                out_dicts = nullptr;
        }

        delete checker;
        return out_dicts;
}

static int
ispell_provider_dictionary_exists (struct str_enchant_provider * me,
                                   const char *const tag)
{
        std::string shortened_dict (tag);
        ISpellChecker * checker = new ISpellChecker (me->owner);
        size_t uscore_pos;
        if ((uscore_pos = shortened_dict.rfind ('_')) != ((size_t)-1))
                shortened_dict = shortened_dict.substr(0, uscore_pos);

        for (size_t i = 0; i < checker->ispell_map_size(); i++)
                {
                        const IspellMap * mapping = &(checker->ispell_map()[i]);
                        if (!strcmp (tag, mapping->lang) || !strcmp (shortened_dict.c_str(), mapping->lang))
                                {
                                        delete checker;
                                        return _ispell_provider_dictionary_exists(me->owner, mapping->dict);
                                }
                }

        delete checker;
        return 0;
}

static void
ispell_provider_free_string_list (EnchantProvider * me _GL_UNUSED_PARAMETER, char **str_list)
{
        g_strfreev (str_list);
}

static void
ispell_provider_dispose (EnchantProvider * me)
{
        g_free (me);
}

static const char *
ispell_provider_identify (EnchantProvider * me _GL_UNUSED_PARAMETER)
{
        return "ispell";
}

static const char *
ispell_provider_describe (EnchantProvider * me _GL_UNUSED_PARAMETER)
{
        return "Ispell Provider";
}

EnchantProvider *
init_enchant_provider (void)
{
        EnchantProvider *provider;

        provider = g_new0 (EnchantProvider, 1);
        provider->dispose = ispell_provider_dispose;
        provider->request_dict = ispell_provider_request_dict;
        provider->dispose_dict = ispell_provider_dispose_dict;
        provider->dictionary_exists = ispell_provider_dictionary_exists;
        provider->identify = ispell_provider_identify;
        provider->describe = ispell_provider_describe;
        provider->list_dicts = ispell_provider_list_dictionaries;
        provider->free_string_list = ispell_provider_free_string_list;

        return provider;
}

} // extern C linkage
