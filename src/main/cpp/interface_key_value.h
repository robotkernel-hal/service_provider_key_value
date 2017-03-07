//! robotkernel interface key value requests
/*!
 * author: Robert Burger
 *
 * $Id$
 */

// vim: tabstop=4 softtabstop=4 shiftwidth=4 expandtab:

/*
 * This file is part of robotkernel.
 *
 * robotkernel is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * robotkernel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with robotkernel.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __INTERFACE_KEY_VALUE_H__
#define __INTERFACE_KEY_VALUE_H__

#include "robotkernel/service_provider_base.h"
#include "robotkernel/service_provider_intf.h"
#include "robotkernel/service.h"
#include "robotkernel/kernel.h"
#include "robotkernel/log_base.h"

namespace interface_key_value {
	extern const char* key_value_sp_magic;

	// forward declaration
	class key_value_handler;

	class key_value : 
		public robotkernel::service_provider_base<key_value_handler> {
		public:
			//! default construction
			/*!
			 * \param node configuration node
			 */
			key_value()
				: service_provider_base("key_value") {};

			~key_value() {};

			//! service provider magic 
			/*!
			 * \return return service provider magic string
			 */
			const char* get_sp_magic() 
			{ return key_value_sp_magic; };
	};
	
	class key_value_handler : public robotkernel::log_base {
		public:
			std::string mod_name;	//!< slave owner module
			std::string dev_name;	//!< service device name
			int slave_id;			//!< slave identifier

			//! handler construction
			key_value_handler(std::string mod_name, std::string dev_name, 
					int slave_id);
			
			//! handler destruction
			~key_value_handler();

			//! service callback key-value read
			/*!
			 * \param request service request data
			 * \parma response service response data
			 * \return success
			 */
			int service_read(const robotkernel::service_arglist_t& request, 
					robotkernel::service_arglist_t& response);
			static const std::string service_definition_read;

			//! service callback key-value write
			/*!
			 * \param request service request data
			 * \parma response service response data
			 * \return success
			 */
			int service_write(const robotkernel::service_arglist_t& request, 
					robotkernel::service_arglist_t& response);
			static const std::string service_definition_write;

			//! service callback key-value list
			/*!
			 * \param request service request data
			 * \parma response service response data
			 * \return success
			 */
			int service_list(const robotkernel::service_arglist_t& request, 
					robotkernel::service_arglist_t& response);
			static const std::string service_definition_list;
	};

} // namespace interface

#endif // __INTERFACE_KEY_VALUE_H__

