#include "config.h"

#include <string>
#include <vector>

#include <string.h>

#include "ispell.h"
#include "enchant.h"
#include "enchant-provider.h"
#include "unused-parameter.h"


#ifndef ENCHANT_ISPELL_HOME_DIR
#define ENCHANT_ISPELL_HOME_DIR "ispell"
#endif

ENCHANT_PLUGIN_DECLARE("Ispell")

extern "C" {

ENCHANT_MODULE_EXPORT (EnchantProvider *)
             init_enchant_provider (void);

static void
ispell_checker_free_helper (gpointer p, gpointer user _GL_UNUSED_PARAMETER)
{
        g_free (p);
}

static GSList *
ispell_checker_get_dictionary_dirs ()
{
        GSList *dirs = NULL;

        {
                GSList *config_dirs, *iter;

                config_dirs = enchant_get_user_config_dirs ();

                for (iter = config_dirs; iter; iter = iter->next)
                        {
                                dirs = g_slist_append (dirs, g_build_filename (static_cast<const gchar *>(iter->data),
                                                                               ENCHANT_ISPELL_HOME_DIR, nullptr));
                        }

                g_slist_foreach (config_dirs, ispell_checker_free_helper, nullptr);
                g_slist_free (config_dirs);
        }

        /* Dynamically locate library and search for modules relative to it. */
        char * enchant_prefix = enchant_get_prefix_dir();
        if(enchant_prefix)
                {
                        char * ispell_prefix = g_build_filename(enchant_prefix, "share", "enchant", "ispell", nullptr);
                        g_free(enchant_prefix);
                        dirs = g_slist_append (dirs, ispell_prefix);
                }

#ifdef ENCHANT_ISPELL_DICT_DIR
        dirs = g_slist_append (dirs, g_strdup (ENCHANT_ISPELL_DICT_DIR));
#endif

        return dirs;
}

static void
buildHashNames (std::vector<std::string> & names, const char * dict)
{
        names.clear ();

        GSList *dirs, *iter;

        dirs = ispell_checker_get_dictionary_dirs();
        for (iter = dirs; iter; iter = iter->next)
                {
                        char *tmp;

                        tmp = g_build_filename (static_cast<const gchar *>(iter->data), dict, nullptr);
                        names.push_back (tmp);
                        g_free (tmp);
                }

        g_slist_foreach (dirs, ispell_checker_free_helper, nullptr);
        g_slist_free (dirs);
}

/*!
 * Load ispell dictionary hash file for given language.
 *
 * \param szLang -  The language tag ("en-US") we want to use
 * \return The name of the dictionary file
 */
static bool
loadDictionaryForLanguage (ISpellChecker * checker, const char * szLang)
{
        const char * encoding = NULL;
        const char * szFile = NULL;

        for (size_t i = 0; i < checker->ispell_map_size(); i++)
                {
                        const IspellMap * mapping = &(checker->ispell_map()[i]);
                        if (!strcmp (szLang, mapping->lang))
                                {
                                        szFile = mapping->dict;
                                        encoding = mapping->enc;
                                        break;
                                }
                }

        if (!szFile || !strlen(szFile))
                return false;

        std::vector<std::string> dict_names;
        buildHashNames (dict_names, szFile);

        return checker->loadDictionaryWithEncoding (szFile, encoding, dict_names);
}

static bool
requestDictionary(ISpellChecker * checker, const char *szLang)
{
        if (!loadDictionaryForLanguage (checker, szLang))
                {
                        // handle a shortened version of the language tag: en_US => en
                        std::string shortened_dict (szLang);
                        size_t uscore_pos;

                        if ((uscore_pos = shortened_dict.rfind ('_')) != (static_cast<size_t>(-1))) {
                                shortened_dict = shortened_dict.substr(0, uscore_pos);
                                if (!loadDictionaryForLanguage (checker, shortened_dict.c_str()))
                                        return false;
                        } else
                                return false;
                }

        checker->finishInit();

        return true;
}

static int
_ispell_provider_dictionary_exists (const char *const szFile)
{
        std::vector <std::string> names;

        buildHashNames (names, szFile);
        for (size_t i = 0; i < names.size(); i++) {
                if (g_file_test (names[i].c_str(), G_FILE_TEST_EXISTS))
                        return 1;
        }

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
ispell_provider_request_dict (EnchantProvider * me _GL_UNUSED_PARAMETER, const char *const tag)
{
        EnchantDict *dict;
        ISpellChecker * checker;

        checker = new ISpellChecker ();

        if (!checker)
                {
                        return NULL;
                }

        if (!requestDictionary(checker, tag)) {
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
        ISpellChecker * checker = new ISpellChecker ();
        char ** out_dicts = g_new0 (char *, checker->ispell_map_size() + 1);

        nb = 0;
        for (i = 0; i < checker->ispell_map_size(); i++)
                if (_ispell_provider_dictionary_exists (checker->ispell_map()[i].dict))
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
ispell_provider_dictionary_exists (struct str_enchant_provider * me _GL_UNUSED_PARAMETER,
                                   const char *const tag)
{
        std::string shortened_dict (tag);
        ISpellChecker * checker = new ISpellChecker ();
        size_t uscore_pos;
        if ((uscore_pos = shortened_dict.rfind ('_')) != ((size_t)-1))
                shortened_dict = shortened_dict.substr(0, uscore_pos);

        for (size_t i = 0; i < checker->ispell_map_size(); i++)
                {
                        const IspellMap * mapping = &(checker->ispell_map()[i]);
                        if (!strcmp (tag, mapping->lang) || !strcmp (shortened_dict.c_str(), mapping->lang))
                                {
                                        delete checker;
                                        return _ispell_provider_dictionary_exists(mapping->dict);
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
