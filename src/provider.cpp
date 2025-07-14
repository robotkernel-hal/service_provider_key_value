//! robotkernel interface key value requests
/*!
 * author: Robert Burger
 *
 * $Id$
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

#include "provider.h"
#include "service_definitions.h"

#include "robotkernel/exceptions.h"

#include "service_provider_key_value/base.h"

SERVICE_PROVIDER_DEF(key_value, service_provider_key_value::provider);

using namespace std;
using namespace robotkernel;
using namespace service_provider_key_value;

//! handler construction
handler::handler(const robotkernel::sp_service_interface_t& req)
    : log_base(req->owner, "key_value", req->device_name) 
{
    _instance = std::dynamic_pointer_cast<service_provider_key_value::base>(req);
    if (!_instance) {
        throw runtime_error(string("wrong base class"));
    }

    add_svc_read(_instance->owner, _instance->device_name + ".read");
    add_svc_write(_instance->owner, _instance->device_name + ".write");
    add_svc_list(_instance->owner, _instance->device_name + ".list");
    add_svc_list_descriptions(_instance->owner, _instance->device_name + ".list_descriptions");
}

//! handler destruction
handler::~handler() {
};

//! svc_read
/*!
 * \param[in]   req     Service request data.
 * \param[out]  resp    Service response data.
 */
void handler::svc_read(const struct svc_req_read& req, struct svc_resp_read& resp) {
    key_value_transfer_t t;
    t.keys.assign(req.keys.begin(), req.keys.end());
    
    try { 
        _instance->key_value_read(t);
        resp.values.assign(t.values.begin(), t.values.end());
    } catch (std::exception& e) {
        resp.error_message = e.what();
    }
}

//! svc_write
/*!
 * \param[in]   req     Service request data.
 * \param[out]  resp    Service response data.
 */
void handler::svc_write(const struct svc_req_write& req, struct svc_resp_write& resp) {
    key_value_transfer_t t;
    t.keys.assign(req.keys.begin(), req.keys.end());
    for (auto val : req.values) {
        t.values.push_back((string)val);
    }
    
    try { 
        _instance->key_value_write(t);
    } catch (std::exception& e) {
        resp.error_message = e.what();
    }
}

//! svc_list
/*!
 * \param[in]   req     Service request data.
 * \param[out]  resp    Service response data.
 */
void handler::svc_list(const struct svc_req_list& req, struct svc_resp_list& resp) {
    key_value_transfer_t t;
    
    try { 
        _instance->key_value_list(t);
        resp.keys.assign(t.keys.begin(), t.keys.end());
        resp.names.assign(t.values.begin(), t.values.end());
    } catch (std::exception& e) {
        resp.error_message = e.what();
    }
}

//! svc_list_descriptions
/*!
 * \param[in]   req     Service request data.
 * \param[out]  resp    Service response data.
 */
void handler::svc_list_descriptions(const struct svc_req_list_descriptions& req, struct svc_resp_list_descriptions& resp) {
    std::vector<key_value_description_t> t;
    
    try { 
        _instance->key_value_list_descriptions(t);

        resp.description.resize(t.size());
        resp.unit.resize(t.size());
        resp.default_value.resize(t.size());
        resp.format.resize(t.size());
        resp.read_only.resize(t.size());

        for (unsigned i = 0; i < t.size(); ++i) {
            resp.description[i]   = t[i].description;
            resp.unit[i]          = t[i].unit;
            resp.default_value[i] = t[i].default_value;
            resp.format[i]        = t[i].format;
            resp.read_only[i]     = t[i].read_only;
        }
    } catch (std::exception& e) {
        resp.error_message = e.what();
    }
}

