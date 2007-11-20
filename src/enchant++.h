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
 * Boston, MA 02111-1307, USA.
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

#ifndef ENCHANT_PLUS_PLUS_H
#define ENCHANT_PLUS_PLUS_H

#include <enchant.h>
#include <string>
#include <vector>
#include <exception>

namespace enchant 
{
	class Broker;

	class Exception : public std::exception
		{
		public:
			explicit Exception (const char * ex) 
				: std::exception (), m_ex ("") {
				if (ex)
					m_ex = ex;
			}

			virtual ~Exception () throw() {
			}
			
			virtual const char * what () throw() {
				return m_ex.c_str();
			}

		private:
			Exception ();

			std::string m_ex;
		};

	class Dict
		{
			friend class enchant::Broker;
			
		public:
			
			~Dict () {
				enchant_broker_free_dict (m_broker, m_dict);
			}
					
			bool check (const std::string & utf8word) {
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

			void suggest (const std::string & utf8word, 
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
						
			std::vector<std::string> suggest (const std::string & utf8word) {
				std::vector<std::string> result;
				suggest (utf8word, result);
				return result;
			}
			
			void add (const std::string & utf8word) {
				enchant_dict_add (m_dict, utf8word.c_str(), 
							 utf8word.size());
			}
			
			void add_to_session (const std::string & utf8word) {
				enchant_dict_add_to_session (m_dict, utf8word.c_str(), 
							     utf8word.size());
			}
			
			void is_added (const std::string & utf8word) {
				enchant_dict_is_added (m_dict, utf8word.c_str(), 
							     utf8word.size());
			}
			
			void remove (const std::string & utf8word) {
				enchant_dict_remove (m_dict, utf8word.c_str(), 
							 utf8word.size());
			}
			
			void remove_from_session (const std::string & utf8word) {
				enchant_dict_remove_from_session (m_dict, utf8word.c_str(), 
							     utf8word.size());
			}

			void is_removed (const std::string & utf8word) {
				enchant_dict_is_removed (m_dict, utf8word.c_str(), 
							     utf8word.size());
			}

			void store_replacement (const std::string & utf8bad, 
						const std::string & utf8good) {
				enchant_dict_store_replacement (m_dict, 
								utf8bad.c_str(), utf8bad.size(),
								utf8good.c_str(), utf8good.size());
			}
			
			const std::string & get_lang () const {
				return m_lang;
			}

			const std::string & get_provider_name () const {
				return m_provider_name;
			}

			const std::string & get_provider_desc () const {
				return m_provider_desc;
			}

			const std::string & get_provider_file () const {
				return m_provider_file;
			}

			/* deprecated */
			void add_to_personal (const std::string & utf8word) {
				return add (utf8word);
			}

			/* deprecated */
			void add_to_pwl (const std::string & utf8word) {
				return add (utf8word);
			}
		private:

			// space reserved for API/ABI expansion
			void * _private[5];		       

			static void s_describe_fn (const char * const lang,
						   const char * const provider_name,
						   const char * const provider_desc,
						   const char * const provider_file,
						   void * user_data) {
				enchant::Dict * dict = static_cast<enchant::Dict *> (user_data);
				
				dict->m_lang = lang;
				dict->m_provider_name = provider_name;
				dict->m_provider_desc = provider_desc;
				dict->m_provider_file = provider_file;
			}

			Dict (EnchantDict * dict, EnchantBroker * broker)
				: m_dict (dict), m_broker (broker) {
				enchant_dict_describe (m_dict, s_describe_fn, this);
			}

			// private, unimplemented
			Dict ();
			Dict (const Dict & rhs);
			Dict& operator=(const Dict & rhs);
			
			EnchantDict * m_dict;
			EnchantBroker * m_broker;

			std::string m_lang;
			std::string m_provider_name;
			std::string m_provider_desc;
			std::string m_provider_file;
		}; // class enchant::Dict
	
	class Broker
		{
			
		public:
			
			static Broker * instance () {
				return &m_instance;
			}
						
			Dict * request_dict (const std::string & lang) {
				EnchantDict * dict = enchant_broker_request_dict (m_broker, lang.c_str());
				
				if (!dict) {
					throw enchant::Exception (enchant_broker_get_error (m_broker));
					return 0; // never reached
				}
				
				return new Dict (dict, m_broker);
			}

			Dict * request_pwl_dict (const std::string & pwl) {
				EnchantDict * dict = enchant_broker_request_pwl_dict (m_broker, pwl.c_str());
				
				if (!dict) {
					throw enchant::Exception (enchant_broker_get_error (m_broker));
					return 0; // never reached
				}
				
				return new Dict (dict, m_broker);
			}
			
			bool dict_exists (const std::string & lang) {
				if (enchant_broker_dict_exists (m_broker, lang.c_str()))
					return true;
				return false;
			}
			
			void set_ordering (const std::string & tag, const std::string & ordering) {
				enchant_broker_set_ordering (m_broker, tag.c_str(), ordering.c_str());
			}
			
			void describe (EnchantBrokerDescribeFn fn, void * user_data = NULL) {
				enchant_broker_describe (m_broker, fn, user_data);
			}
			
			void list_dicts (EnchantDictDescribeFn fn, void * user_data = NULL) {
				enchant_broker_list_dicts (m_broker, fn, user_data);
			}

		private:

			// space reserved for API/ABI expansion
			void * _private[5];
			
			Broker ()
				: m_broker (enchant_broker_init ())
				{
				}
			
			~Broker () {
				enchant_broker_free (m_broker);
			}
			
			// not implemented
			Broker (const Broker & rhs);
			Broker& operator=(const Broker & rhs);
			
			static Broker m_instance;
			
			EnchantBroker * m_broker;
		}; // class enchant::Broker
	
	// define the broker instance
	Broker Broker::m_instance;
	
} // enchant namespace

#endif /* ENCHANT_PLUS_PLUS_H */
