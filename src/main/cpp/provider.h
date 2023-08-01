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

#ifndef __SERVICE_PROVIDER__KEY_VALUE__PROVIDER_H__
#define __SERVICE_PROVIDER__KEY_VALUE__PROVIDER_H__

// Robotkernel includes
#include "robotkernel/service_provider_base.h"
#include "robotkernel/log_base.h"

// Service provider includes.
#include "service_provider/key_value/base.h"
#include "service_definitions.h"

namespace service_provider {
namespace key_value {

// forward declaration
class handler;

class provider : public robotkernel::service_provider_base<handler, base> {
    public:
        //! default construction
        /*!
         * \param node configuration node
         */
        provider(const std::string& name)
            : service_provider_base(name, "key_value") {};
};

class handler : 
    public robotkernel::log_base,
    public svc_base_read,
    public svc_base_write,
    public svc_base_list,
    public svc_base_list_descriptions
{
    public:
        typedef std::shared_ptr<service_provider::key_value::base> sp_kv_base_t;
        sp_kv_base_t _instance;

        //! handler construction
        handler(const robotkernel::sp_service_interface_t& req);

        //! handler destruction
        ~handler();

        //! svc_read
        /*!
         * \param[in]   req     Service request data.
         * \param[out]  resp    Service response data.
         */
        virtual void svc_read(const struct svc_req_read& req, struct svc_resp_read& resp);
        
        //! svc_write
        /*!
         * \param[in]   req     Service request data.
         * \param[out]  resp    Service response data.
         */
        virtual void svc_write(const struct svc_req_write& req, struct svc_resp_write& resp);

        //! svc_list
        /*!
         * \param[in]   req     Service request data.
         * \param[out]  resp    Service response data.
         */
        virtual void svc_list(const struct svc_req_list& req, struct svc_resp_list& resp);
        
        //! svc_list_descriptions
        /*!
         * \param[in]   req     Service request data.
         * \param[out]  resp    Service response data.
         */
        virtual void svc_list_descriptions(const struct svc_req_list_descriptions& req, struct svc_resp_list_descriptions& resp);
};

}; // namespace key_value
}; // namespace service_provider

#endif // __SERVICE_PROVIDER__KEY_VALUE__PROVIDER_H__

