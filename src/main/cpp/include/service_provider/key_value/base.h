//! robotkernel service provider key value
/*!
 * author: Robert Burger
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

#ifndef __SERVICE_PROVIDER__KEY_VALUE__BASE__H__
#define __SERVICE_PROVIDER__KEY_VALUE__BASE__H__

#include <list>
#include <vector>

#include "robotkernel/service_interface.h"

namespace service_provider {
#ifdef EMACS
}
#endif

namespace key_value {
#ifdef EMACS
}
#endif

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
        virtual void key_value_read(key_value_transfer_t& transfer);

        //! write key value pairs
        /*!
         * \param[in] transfer      Key value request. Key and values vector has to 
         *                          be filled with keys and values you want to write.
         */
        virtual void key_value_write(const key_value_transfer_t& transfer);

        //! list keys with names
        /*!
         * \param[out] transfer     Key value request. Key vector will be filled with
         *                          all available key, values vector will be filled 
         *                          with their names.
         */
        virtual void key_value_list(key_value_transfer_t& transfer);
	
        //! list descriptions
        /*!
         * \param[out] transfer     Key value request. The data vector will returned the
         *                          descriptions of all available keys.
         */
        virtual void key_value_list_descriptions(std::vector<key_value_description_t>& data);
};

#ifdef EMACS
{
#endif
}; // namespace key_value

#ifdef EMACS
{
#endif
}; // namespace service_provider

#endif // __SERVICE_PROVIDER__KEY_VALUE__BASE__H__

