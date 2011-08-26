/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* enchant
* Copyright (C) 2003 Dom Lachowicz
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02110-1301, USA.
*
* In addition, as a special exception, Dom Lachowicz
* gives permission to link the code of this program with
* non-LGPL Spelling Provider libraries (eg: a MSFT Office
* spell checker backend) and distribute linked combinations including
* the two.  You must obey the GNU Lesser General Public License in all
* respects for all of the code used other than said providers.  If you modify
* this file, you may extend this exception to your version of the
* file, but you are not obligated to do so.  If you do not wish to
* do so, delete this exception statement from your version.
*/


#include <enchant.h>
#include <string>
#include <vector>
#include <exception>
#include "enchant++.h"

namespace enchant 
{

	bool Dict::check (const std::string & utf8word) {
		int val;

		val = enchant_dict_check (m_dict, utf8word.c_str(), 
			utf8word.size());
		if (val == 0)
			return true;
		else if (val > 0)
			return false;
		else {
			throw enchant::Exception (enchant_dict_get_error (m_dict));
		}

		return false; // never reached
	}

	void Dict::suggest (const std::string & utf8word, 
		std::vector<std::string> & out_suggestions) {
			size_t n_suggs;
			char ** suggs;

			out_suggestions.clear ();

			suggs = enchant_dict_suggest (m_dict, utf8word.c_str(), 
				utf8word.size(), &n_suggs);

			if (suggs && n_suggs) {
				for (size_t i = 0; i < n_suggs; i++) {
					out_suggestions.push_back (suggs[i]);
				}

				enchant_dict_free_string_list (m_dict, suggs);
			}
	}

	std::vector<std::string> Dict::suggest (const std::string & utf8word) {
		std::vector<std::string> result;
		suggest (utf8word, result);
		return result;
	}

	//void hyphenate (const std::string & utf8word, 
	//	std::string & out_suggestions) {
	//	    // we only need to return one result
	//		// so delete it
	//}

	std::string Dict::hyphenate (const std::string & utf8word) {
		std::string result;
		result=enchant_dict_hyphenate (m_dict,utf8word.c_str(), -1);
		return result;
	}



	Dict * Broker::request_dict (const std::string & lang) {
		EnchantDict * dict = enchant_broker_request_dict (m_broker, lang.c_str());

		if (!dict) {
			throw enchant::Exception (enchant_broker_get_error (m_broker));
			return 0; // never reached
		}

		return new Dict (dict, m_broker);
	}

	Dict * Broker::request_pwl_dict (const std::string & pwl) {
		EnchantDict * dict = enchant_broker_request_pwl_dict (m_broker, pwl.c_str());

		if (!dict) {
			throw enchant::Exception (enchant_broker_get_error (m_broker));
			return 0; // never reached
		}

		return new Dict (dict, m_broker);
	}	
} // enchant namespace

