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

#define LN_UNREGISTER_SERVICE_IN_BASE_DETOR  
#include "ln_messages.h"
#undef LN_UNREGISTER_SERVICE_IN_BASE_DETOR

#define INTFNAME "[interface_key_value] "

namespace interface {
    
class key_value : 
    public ln_service_read_base,
    public ln_service_write_base,
    public ln_service_list_base 
{
    const std::string _mod_name;
    const std::string _dev_name;
    const int _slave_id;

    public:
        //! default construction
        /*!
         * \param mod_name module name to register for
         */
        key_value(const std::string& mod_name, 
                const std::string& dev_name, const int& slave_id);

        //! service reading key value pairs
        int on_read(ln::service_request& req, 
                ln_service_robotkernel_key_value_read& svc);

        //! service writing key value pairs
        int on_write(ln::service_request& req, 
                ln_service_robotkernel_key_value_write& svc);

        //! service listing key value pairs
        int on_list(ln::service_request& req, 
                ln_service_robotkernel_key_value_list& svc);
};

} // namespace interface

#endif // __INTERFACE_KEY_VALUE_H__

