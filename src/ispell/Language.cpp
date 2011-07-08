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
#include "Language.h"
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <unistd.h>

using namespace std;
using namespace RFC_3066;

Language::Language(string rfc_3066) throw(domain_error) {
   const char *c = rfc_3066.c_str();
   a.push_back("");

   do {
      if (isdigit(*c)) {
	 if (a.size() == 1)
	    throw domain_error("RFC 3066 allows digits only in subtags.");
	 a.back().push_back(*c);
      } else if (isalpha(*c)) {
	 a.back().push_back(tolower(*c));
      } else if (*c == '-') {
	 a.push_back("");
      } else
	 throw *new domain_error("RFC 3066 tags must contain only letters, spaces "
	    + string() + "and dashes.");
   } while (*++c);

}

bool Language::operator==(const Language &o) const throw() {
   list<string>::const_iterator me = this->a.begin(), you = o.a.begin();

   while (true) {
      if (me == this->a.end() || you == o.a.end())
	 return true;
      else if (*me != *you)
	 return false;
      me++;
      you++;
   }
}

bool Language::operator<(const Language &o) const throw() {
   list<string>::const_iterator me = this->a.begin(), you = o.a.begin();

   while (true) {
      if (me == this->a.end() || you == o.a.end())
	 return false;
      else if (*me < *you)
	 return true;
      else if (*me > *you)
	 return false;
      me++;
      you++;
   }
}

Language::operator string() const throw() {
   return concat(a.size());
}

std::string Language::concat(int depth, const string& sep) const throw() {
   string accum;
   for (list<string>::const_iterator i = a.begin(); i != a.end(); i++) {
      accum += ((i == a.begin()) ? "" : sep) + *i;
      if (--depth <= 0) break;
   }

   return accum;
}

#include <iostream>
#include <fstream>
using namespace std;
std::string Language::find_suitable_file(const string &dir) const 
   throw(domain_error) 
{
   struct stat buf;
   std::string path;

   for (int i = a.size(); i > 0; i--) {
      path = dir + concat(i);      
      if (stat(path.c_str(), &buf) != -1)
	 return path;
   }  

   cout<<path;
   throw domain_error("libhyphenate: No suitable hyphenation file for language "
      + concat(a.size()) + " found in " + dir);
}
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
#include "Language.h"
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <unistd.h>

using namespace std;
using namespace RFC_3066;

Language::Language(string rfc_3066) throw(domain_error) {
   const char *c = rfc_3066.c_str();
   a.push_back("");

   do {
      if (isdigit(*c)) {
	 if (a.size() == 1)
	    throw domain_error("RFC 3066 allows digits only in subtags.");
	 a.back().push_back(*c);
      } else if (isalpha(*c)) {
	 a.back().push_back(tolower(*c));
      } else if (*c == '-') {
	 a.push_back("");
      } else
	 throw *new domain_error("RFC 3066 tags must contain only letters, spaces "
	    + string() + "and dashes.");
   } while (*++c);

}

bool Language::operator==(const Language &o) const throw() {
   list<string>::const_iterator me = this->a.begin(), you = o.a.begin();

   while (true) {
      if (me == this->a.end() || you == o.a.end())
	 return true;
      else if (*me != *you)
	 return false;
      me++;
      you++;
   }
}

bool Language::operator<(const Language &o) const throw() {
   list<string>::const_iterator me = this->a.begin(), you = o.a.begin();

   while (true) {
      if (me == this->a.end() || you == o.a.end())
	 return false;
      else if (*me < *you)
	 return true;
      else if (*me > *you)
	 return false;
      me++;
      you++;
   }
}

Language::operator string() const throw() {
   return concat(a.size());
}

std::string Language::concat(int depth, const string& sep) const throw() {
   string accum;
   for (list<string>::const_iterator i = a.begin(); i != a.end(); i++) {
      accum += ((i == a.begin()) ? "" : sep) + *i;
      if (--depth <= 0) break;
   }

   return accum;
}

#include <iostream>

std::string Language::find_suitable_file(const string &dir) const 
   throw(domain_error) 
{
   struct stat buf;
   std::string path;

   for (int i = a.size(); i > 0; i--) {
      path = dir + concat(i);
      if (stat(path.c_str(), &buf) != -1)
	 return path;
   }
   
   cout<<path;
   throw domain_error("libhyphenate: No suitable hyphenation file for language "
      + concat(a.size()) + " found in " + dir);
}
