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
 * the non-LGPL Spelling Provider libraries (eg: a MSFT Office
 * spell checker backend) and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
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
	
	class Broker
		{
		friend class Dict;
			
		public:
			
			static Broker * instance () {
				return &m_instance;
			}
			
			class Dict
				{
				friend class enchant::Broker;
					
				public:
					
					~Dict () {
						m_broker->_release_dict (this);
					}
					
					bool check (const std::string & utf8word) {
						if (enchant_dict_check (m_dict, utf8word.c_str(), 
									utf8word.size()) == 0)
							return true;
						return false;
					}
					
					void add_to_personal (const std::string & utf8word) {
						enchant_dict_add_to_personal (m_dict, utf8word.c_str(), 
									      utf8word.size());
					}
					
					void add_to_session (const std::string & utf8word) {
						enchant_dict_add_to_session (m_dict, utf8word.c_str(), 
									     utf8word.size());
					}
					
					void store_replacement (const std::string & utf8bad, 
								const std::string & utf8good) {
						enchant_dict_store_replacement (m_dict, 
										utf8bad.c_str(), utf8bad.size(),
										utf8good.c_str(), utf8good.size());
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
							
							enchant_dict_free_suggestions (m_dict, suggs);
						}
					}
					
					std::vector<std::string> suggest (const std::string & utf8word) {
						std::vector<std::string> result;
						suggest (utf8word, result);
						return result;
					}
					
				private:
					
					Dict (EnchantDict * dict, enchant::Broker * broker)
						: m_dict (dict), m_broker (broker)
						{
						}
					
					// private, unimplemented
					Dict ();
					Dict (const Dict & rhs);
					Dict& operator=(const Dict & rhs);
					
					EnchantDict * m_dict;
					enchant::Broker * m_broker;
				}; // class enchant::Broker::Dict
			
			Broker::Dict * request_dict (const std::string & lang) {
				EnchantDict * dict = enchant_broker_request_dict (m_broker, lang.c_str());
				
				if (!dict) {
					throw std::exception ();
					return 0; // not actually reached
				}
				
				return new Broker::Dict (dict, this);
			}
			
			EnchantDictStatus dict_status (const std::string & lang) {
				return enchant_broker_dictionary_status (m_broker, lang.c_str());
			}
			
			void set_ordering (const std::string tag, const std::string & ordering) {
				enchant_broker_set_ordering (m_broker, tag.c_str(), ordering.c_str());
			}
			
			void describe (EnchantBrokerDescribeFn fn, void * user_data) {
				enchant_broker_describe (m_broker, fn, user_data);
			}
			
		private:
			
			Broker ()
				: m_broker (0)
				{
					m_broker = enchant_broker_init ();
				}
			
			~Broker () {
				if (m_broker)
					enchant_broker_term (m_broker);
			}
			
			// only called by Dict's d'tor
			void _release_dict (Broker::Dict * dict) {
				enchant_broker_release_dict (m_broker, dict->m_dict);
			}
			
			// not implemented
			Broker (const Broker & rhs);
			Broker& operator=(const Broker & rhs);
			
			static Broker m_instance;
			
			EnchantBroker * m_broker;
		}; // class enchant::Broker
	
	// define the instance
	Broker Broker::m_instance;
	
} // enchant namespace

#endif /* ENCHANT_PLUS_PLUS_H */
