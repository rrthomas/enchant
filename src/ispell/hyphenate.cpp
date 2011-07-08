/** This file provides the C interface to the libhyphenate. */

/* libhyphenate: A TeX-like hyphenation algorithm.
 * Copyright (C) 2007 Steve Wolter
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * If you have any questions, feel free to contact me:
 * http://swolter.sdf1.org
 **/

#include "hyphenate.h"
#include "Hyphenator.h"

using namespace std;

/** Build an hyphenator for the language given in a RFC-3066 compliant string.
 *  The rules file will be searched in the directory given by the environment
 *  variable LIBHYPHENATE_PATH or, if failing on that, in the compiled-in
 *  pattern directory which defaults to
 *  /usr/local/share/libhyphenate/patterns
 *
 *  */
Hyphenator *hyphenate_create_hyphenator(const char *language) {
   return (Hyphenator*)new Hyphenate::Hyphenator(RFC_3066::Language(language));
}

/** Build an hyphenator using the rules from the FILE provided. */
Hyphenator *hyphenate_create_hyphenator_from_file(const char *file) {
   return (Hyphenator*)new Hyphenate::Hyphenator(file);
}

/** Free the resources for that hyphenator. */
void hyphenate_destroy_hyphenator(Hyphenator *h) {
   delete ((Hyphenate::Hyphenator*)h);
}

/** The actual workhorse. You'll want to call this function once
* for each word (NEW: or complete string, not only word. The library
* will do the word-splitting for you) you want hyphenated.  
* The result must be free()ed.
*  
*  Usage example: 
*  	Hyphenator* h = hyphenate_create_hyphenator("de-DE");
*  	char *res = hyphenate_hyphenate(h, "Schifffahrt");
*  	printf("%s", res);
*  	free(res);
*
*  	yields "Schiff-fahrt", while 
*
*  	Hyphenator* h = hyphenate_create_hyphenator("en");
*  	char *res = hyphenate_hyphenate(h, "example");
*  	printf("%s", res);
*  	free(res);
*
*  	yields "ex&shy;am&shy;ple".
*
*  \param word A single UTF-8 encoded word to be hyphenated.
*  \param hyphen The string to put at each possible
*                hyphenation point. The default is an ASCII dash.
*
*/
char *hyphenate_hyphenate(Hyphenator *h, const char *word, 
                          const char *hyphen) 
{
   string res = ((Hyphenate::Hyphenator*)h)
                        ->hyphenate(string(word), string(hyphen));
   char *r = (char*)malloc((res.size()+1) * sizeof(char));
   strcpy(r, res.c_str());
   return r;
}

/** Find a single hyphenation point in the string so that the first
   *  part (including a hyphen) will be shorter or equal in length
   *  to the parameter len. If this is not possible, choose the shortest
   *  possible string.
   *
   *  The returned element is the result, the "rest" pointer will point
   *  to the rest of the string afterwards. Both must be free()ed.
   **/
char *hyphenate_hyphenate_at(Hyphenator *h, 
   char *word, const char *hyphen, size_t len) 
{
   int max = strlen(word)+1;
   pair<string,string> res = ((Hyphenate::Hyphenator*)h)
                        ->hyphenate_at(string(word), string(hyphen), len);

   char *r = (char*)malloc((res.first.size()+1) * sizeof(char));
   strcpy(r, res.first.c_str());

   strncpy(word, res.second.c_str(), max);
   return r;
}
/** This file provides the C interface to the libhyphenate. */

/* libhyphenate: A TeX-like hyphenation algorithm.
 * Copyright (C) 2007 Steve Wolter
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * If you have any questions, feel free to contact me:
 * http://swolter.sdf1.org
 **/

#include "hyphenate.h"
#include "Hyphenator.h"

using namespace std;

/** Build an hyphenator for the language given in a RFC-3066 compliant string.
 *  The rules file will be searched in the directory given by the environment
 *  variable LIBHYPHENATE_PATH or, if failing on that, in the compiled-in
 *  pattern directory which defaults to
 *  /usr/local/share/libhyphenate/patterns
 *
 *  */
Hyphenator *hyphenate_create_hyphenator(const char *language) {
   return (Hyphenator*)new Hyphenate::Hyphenator(RFC_3066::Language(language));
}

/** Build an hyphenator using the rules from the FILE provided. */
Hyphenator *hyphenate_create_hyphenator_from_file(const char *file) {
   return (Hyphenator*)new Hyphenate::Hyphenator(file);
}

/** Free the resources for that hyphenator. */
void hyphenate_destroy_hyphenator(Hyphenator *h) {
   delete ((Hyphenate::Hyphenator*)h);
}

/** The actual workhorse. You'll want to call this function once
* for each word (NEW: or complete string, not only word. The library
* will do the word-splitting for you) you want hyphenated.  
* The result must be free()ed.
*  
*  Usage example: 
*  	Hyphenator* h = hyphenate_create_hyphenator("de-DE");
*  	char *res = hyphenate_hyphenate(h, "Schifffahrt");
*  	printf("%s", res);
*  	free(res);
*
*  	yields "Schiff-fahrt", while 
*
*  	Hyphenator* h = hyphenate_create_hyphenator("en");
*  	char *res = hyphenate_hyphenate(h, "example");
*  	printf("%s", res);
*  	free(res);
*
*  	yields "ex&shy;am&shy;ple".
*
*  \param word A single UTF-8 encoded word to be hyphenated.
*  \param hyphen The string to put at each possible
*                hyphenation point. The default is an ASCII dash.
*
*/
char *hyphenate_hyphenate(Hyphenator *h, const char *word, 
                          const char *hyphen) 
{
   string res = ((Hyphenate::Hyphenator*)h)
                        ->hyphenate(string(word), string(hyphen));
   char *r = (char*)malloc((res.size()+1) * sizeof(char));
   strcpy(r, res.c_str());
   return r;
}

/** Find a single hyphenation point in the string so that the first
   *  part (including a hyphen) will be shorter or equal in length
   *  to the parameter len. If this is not possible, choose the shortest
   *  possible string.
   *
   *  The returned element is the result, the "rest" pointer will point
   *  to the rest of the string afterwards. Both must be free()ed.
   **/
char *hyphenate_hyphenate_at(Hyphenator *h, 
   char *word, const char *hyphen, size_t len) 
{
   int max = strlen(word)+1;
   pair<string,string> res = ((Hyphenate::Hyphenator*)h)
                        ->hyphenate_at(string(word), string(hyphen), len);

   char *r = (char*)malloc((res.first.size()+1) * sizeof(char));
   strcpy(r, res.first.c_str());

   strncpy(word, res.second.c_str(), max);
   return r;
}
