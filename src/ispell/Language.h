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
#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <list>
#include <string>
#include <stdexcept>

namespace RFC_3066 {
   /** This class implements a parser for RFC 3066-compliant language codes. */
   class Language {
      private:
	 /* This is a list of the components, all in lowercase; for example,
	  * for de-AT the list would have the two elements "de" and "at". */
	 std::list<std::string> a;

      public:
	 /** Construct from an RFC-3066-compliant string. */
	 Language(std::string rfc_3066) throw(std::domain_error);

	 /** Compare languages. The <-operator works lexicographically. */
	 bool operator==(const Language &o) const throw();
	 bool operator<(const Language &o) const throw();

	 /** Re-string to a RFC-3066-compliant string. */
	 operator std::string() const throw();
	 /** Concat only the first 'elements' elements of the language 
	  *  identifier and seperate them with the separator. */
	 std::string concat(int elements, 
	    const std::string& separator = "-") const throw();

	 /** Find the longest prefix match in the given directory for the given
	  *  language. For example, for de-AT-Vienna, de-AT-Vienna is checked
	  *  first, then de-AT, then de. The directory should be /-postfixed. */
	 std::string find_suitable_file(const std::string &dir) const
	    throw(std::domain_error);
   };
}

#endif
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
#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <list>
#include <string>
#include <stdexcept>

namespace RFC_3066 {
   /** This class implements a parser for RFC 3066-compliant language codes. */
   class Language {
      private:
	 /* This is a list of the components, all in lowercase; for example,
	  * for de-AT the list would have the two elements "de" and "at". */
	 std::list<std::string> a;

      public:
	 /** Construct from an RFC-3066-compliant string. */
	 Language(std::string rfc_3066) throw(std::domain_error);

	 /** Compare languages. The <-operator works lexicographically. */
	 bool operator==(const Language &o) const throw();
	 bool operator<(const Language &o) const throw();

	 /** Re-string to a RFC-3066-compliant string. */
	 operator std::string() const throw();
	 /** Concat only the first 'elements' elements of the language 
	  *  identifier and seperate them with the separator. */
	 std::string concat(int elements, 
	    const std::string& separator = "-") const throw();

	 /** Find the longest prefix match in the given directory for the given
	  *  language. For example, for de-AT-Vienna, de-AT-Vienna is checked
	  *  first, then de-AT, then de. The directory should be /-postfixed. */
	 std::string find_suitable_file(const std::string &dir) const
	    throw(std::domain_error);
   };
}

#endif
