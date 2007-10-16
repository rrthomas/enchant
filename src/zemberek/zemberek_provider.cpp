/** Copyright (C) 2006 Barış Metin <baris@pardus.org.tr>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

/* Get Zemberek and Zemberek-server from
   https://zemberek.dev.java.net */

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <glib.h>

#include "enchant.h"
#include "enchant-provider.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#ifndef ZEMBEREK_HOST
#define ZEMBEREK_HOST "localhost"
#endif
#ifndef ZEMBEREK_PORT
#define ZEMBEREK_PORT 10444
#endif

ENCHANT_PLUGIN_DECLARE("Zemberek")

using namespace std;

/******************************/
/* Zemberek class decleration */
/*****************************/

class Zemberek
{
public:
    Zemberek();
    ~Zemberek();
    
    bool checkWord(const string& str, size_t len) const;
    vector<string> suggestWord(const string& str, size_t len);

    // chek if connection is O.K.
    bool connOK(void) { return !_connError; }
    
private:
    int _conn;
    bool _connError;
    string recvResult() const;
};

/*****************************/
/* Zemberek class definition */
/*****************************/

Zemberek::Zemberek()
    : _connError(false)
{
    struct hostent *he;
    struct sockaddr_in saddr;

    if ( ( he = (struct hostent *)gethostbyname(ZEMBEREK_HOST) ) == NULL ) {
	perror( "gethostbyname()" );
	_connError = true;
	return;
    }

    if ( ( _conn = socket(AF_INET, SOCK_STREAM, 0) ) == -1 ) {
	perror( "socket()" );
	_connError = true;
	return;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons( (uint16_t)ZEMBEREK_PORT );
    saddr.sin_addr = *( (struct in_addr *)he->h_addr );
    memset( &(saddr.sin_zero), '\0', 8 );

    if ( connect(_conn, (struct sockaddr *)&saddr, sizeof(struct sockaddr)) == -1) {
        perror("connect()");
	_connError = true;
	return;
    }
}

Zemberek::~Zemberek()
{
    if ( _conn ) {
        shutdown( _conn, SHUT_RDWR );
        close( _conn );
    }
}

bool Zemberek::checkWord( const string& str, size_t len ) const
{
    if (_connError)
	return false;

    stringstream strstream;
    strstream << str.length()+2 << " * " << str;
    string checkStr = strstream.str();
    if ( send(_conn, checkStr.c_str(), checkStr.length(), 0) == -1) {
        perror("send()");
	return false;
    }

    switch ( recvResult()[0] ) {
    case '*':
        return true;
        break;
    case '#':
        return false;
        break;
    default:
        return false;
        break;
    }
}

vector<string> Zemberek::suggestWord(const string& str, size_t size)
{
    vector<string> suggestions;

    if (_connError)
	return suggestions;
    
    stringstream strstream;
    strstream << str.length()+2 << " & " << str;
    string checkStr = strstream.str();
    if ( send( _conn, checkStr.c_str(), checkStr.length(), 0 ) == -1 ) {
        perror( "send()" );
	return suggestions;
    }

    string result = recvResult();
    if ( result[0] != '&' ) {
        return suggestions;
    }

    string::iterator it = result.begin();
    string::iterator end = result.end();
    bool start = false;
    string tmp;
    for ( ; it != end; ++it ) {
        if ( *it == '(' ) {
            start = true;
            continue;

        }

        if ( !start ) continue;


        if ( *it == ',' ) {
            suggestions.push_back( tmp );
            tmp.erase();
            continue;
        } else if ( *it == ')' ) {
            suggestions.push_back( tmp );
            break;
        }

        tmp += *it;
    }

    return suggestions;
}

string Zemberek::recvResult() const
{
    int numbytes = 0;
    string buf("");

    int size = 0;
    while (true) {
        char s;
        numbytes = recv (_conn, &s, 1, 0);

	// how many chars do we have to read?
        if (s == ' ') {
            char *endptr;
            size = strtol (buf.c_str() , &endptr, 0);
            buf.erase();
            break;
        }

        buf += s;
    }
    char *ret = new char[size+1];
    numbytes = recv (_conn, ret, size, 0);
    ret[numbytes]='\0';

    string result = ret;
    delete ret;

    return result;
}

/******************************/
/* Enchant provider functions */
/******************************/

static int
zemberek_dict_check (EnchantDict * dict, const char *const word, size_t len)
{
    Zemberek *checker;
	
    checker = (Zemberek *) dict->user_data;
	
    if (checker->checkWord(word, len))
	return 0;
    return 1;
}

static char**
zemberek_dict_suggest (EnchantDict * dict, const char *const word,
		     size_t len, size_t * out_n_suggs)
{
    Zemberek *checker;
	
    checker = (Zemberek *) dict->user_data;
    vector<string> suggestions = checker->suggestWord (word, len);
    *out_n_suggs = suggestions.size();

    vector<string>::const_iterator it = suggestions.begin();
    int suglen = suggestions.size();

    if (suglen > 0)
    {
	char **sug;
	sug = g_new0(char *, suglen+1);

	for (int i=0 ; i < suglen ; ++i, ++it )
	    sug[i] = g_strdup(it->c_str());

	return sug;
    }
    return 0;
}

static void
zemberek_provider_dispose(EnchantProvider *me)
{
    g_free(me);
}

static EnchantDict*
zemberek_provider_request_dict(EnchantProvider *me, const char *tag)
{
    Zemberek* checker = new Zemberek();

    if (!checker || !checker->connOK())
	return NULL;

    EnchantDict* dict = g_new0(EnchantDict, 1);
    dict->user_data = (void *) checker;
    dict->check = zemberek_dict_check;
    dict->suggest = zemberek_dict_suggest;
	
    return dict;
}

static void
zemberek_provider_dispose_dict (EnchantProvider * me, EnchantDict * dict)
{
    Zemberek *checker;
    checker = (Zemberek *) dict->user_data;
    delete checker;
    g_free (dict);
}

static char *
zemberek_provider_identify (EnchantProvider * me)
{
	return "zemberek";
}

static char *
zemberek_provider_describe (EnchantProvider * me)
{
	return "Zemberek Provider";
}

static void
zemberek_provider_free_string_list (EnchantProvider * me, char **str_list)
{
	g_strfreev (str_list);
}

static gboolean
zemberek_provider_server_is_running (void)
{
  Zemberek checker;

  return checker.connOK();
}

static char ** 
zemberek_provider_list_dicts (EnchantProvider * me, 
			      size_t * out_n_dicts)
{
	char ** out_list = NULL;

	if (zemberek_provider_server_is_running ()) {
	    *out_n_dicts = 1;
	    out_list = g_new0 (char *, 2);
	    out_list[0] = g_strdup ("tr");
	}
	else
	    *out_n_dicts = 0;
		
	return out_list;
}

static int
zemberek_provider_dictionary_exists (EnchantProvider * me,
				     const char *const tag)
{
	EnchantDict * dict;
	Zemberek * checker;

	if (!strcmp ("tr", tag)) {
	  return zemberek_provider_server_is_running ();
	}
	else {
	    return 0;
	}
}

extern "C" {

ENCHANT_MODULE_EXPORT(EnchantProvider *) 
init_enchant_provider(void)
{
    EnchantProvider *provider;
	
    provider = g_new0(EnchantProvider, 1);
    provider->dispose = zemberek_provider_dispose;
    provider->request_dict = zemberek_provider_request_dict;
    provider->dispose_dict = zemberek_provider_dispose_dict;
    provider->identify = zemberek_provider_identify;
    provider->describe = zemberek_provider_describe;
    provider->list_dicts = zemberek_provider_list_dicts;
    provider->dictionary_exists = zemberek_provider_dictionary_exists;
    provider->free_string_list = zemberek_provider_free_string_list;

    return provider;
}

} 
