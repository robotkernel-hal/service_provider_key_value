//! robotkernel service provider key value
/*!
 * author: Robert Burger
 */

// vim: tabstop=4 softtabstop=4 shiftwidth=4 expandtab:

/*
 * This file is part of service_provider_key_value.
 *
 * service_provider_key_value is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * service_provider_key_value is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with service_provider_key_value; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SERVICE_PROVIDER_KEY_VALUE__BASE_H
#define SERVICE_PROVIDER_KEY_VALUE__BASE_H

#include <list>
#include <vector>

#include "robotkernel/service_interface.h"

namespace service_provider_key_value {

typedef struct key_value_transfer {
    std::vector<uint32_t> keys;    
    std::vector<std::string> values;
} key_value_transfer_t;

typedef struct key_value_description {
    std::string description;
    std::string unit;
    std::string default_value;
    std::string format;
    uint8_t read_only;
} key_value_description_t;

class base : 
    public robotkernel::service_interface
{
    public:
        //! construction
        base(std::string owner, std::string service_prefix)
            : robotkernel::service_interface(owner, service_prefix + ".key_value") {};

        //! destruction
        virtual ~base() = 0;

        //! read key value pairs
        /*!
         * \param[in,out] transfer  Key value request. Key vector has to be filled
         *                          with keys you want to read, the values will 
         *                          be returned in the values vector.
         */
        virtual void key_value_read(key_value_transfer_t& transfer) = 0;

        //! write key value pairs
        /*!
         * \param[in] transfer      Key value request. Key and values vector has to 
         *                          be filled with keys and values you want to write.
         */
        virtual void key_value_write(const key_value_transfer_t& transfer) = 0;

        //! list keys with names
        /*!
         * \param[out] transfer     Key value request. Key vector will be filled with
         *                          all available key, values vector will be filled 
         *                          with their names.
         */
        virtual void key_value_list(key_value_transfer_t& transfer) = 0;
	
        //! list descriptions
        /*!
         * \param[out] transfer     Key value request. The data vector will returned the
         *                          descriptions of all available keys.
         */
        virtual void key_value_list_descriptions(std::vector<key_value_description_t>& data) = 0;
};
        
inline base::~base() { }

}; // namespace service_provider_key_value

#endif // SERVICE_PROVIDER_KEY_VALUE__BASE_H

