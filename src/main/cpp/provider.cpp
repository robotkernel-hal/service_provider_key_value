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

#include "provider.h"
#include "service_definitions.h"

#include "robotkernel/kernel.h"
#include "robotkernel/exceptions.h"

#include <string_util/string_util.h>

#include "service_provider/key_value/base.h"

SERVICE_PROVIDER_DEF(key_value, service_provider::key_value::provider);

using namespace std;
using namespace std::placeholders;
using namespace robotkernel;
using namespace service_provider;
using namespace string_util;
using namespace key_value;

const std::string service_provider::key_value::handler::service_definition_read = robotkernel_service_provider_key_value_read_service_definition;
const std::string service_provider::key_value::handler::service_definition_write = robotkernel_service_provider_key_value_write_service_definition;
const std::string service_provider::key_value::handler::service_definition_list = robotkernel_service_provider_key_value_list_service_definition;
const std::string service_provider::key_value::handler::service_definition_list_descriptions = robotkernel_service_provider_key_value_list_descriptions_service_definition;

//! handler construction
key_value::handler::handler(const robotkernel::sp_service_interface_t& req)
    : log_base(req->owner, "key_value", req->device_name) {
    robotkernel::kernel& k = *robotkernel::kernel::get_instance();

    _instance = std::dynamic_pointer_cast<service_provider::key_value::base>(req);
    if (!_instance)
        throw str_exception("wrong base class");

    k.add_service(_instance->owner, _instance->device_name + ".read", service_definition_read,
            std::bind(&key_value::handler::service_read, this, _1, _2));
    k.add_service(_instance->owner, _instance->device_name + ".write", service_definition_write,
            std::bind(&key_value::handler::service_write, this, _1, _2));
    k.add_service(_instance->owner, _instance->device_name + ".list", service_definition_list,
            std::bind(&key_value::handler::service_list, this, _1, _2));
    k.add_service(_instance->owner, _instance->device_name + ".list_descriptions", service_definition_list_descriptions,
            std::bind(&key_value::handler::service_list_descriptions, this, _1, _2));
}

//! handler destruction
key_value::handler::~handler() {
    kernel& k = *kernel::get_instance();
    k.remove_service(_instance->owner, _instance->device_name + ".read");
    k.remove_service(_instance->owner, _instance->device_name + ".write");
    k.remove_service(_instance->owner, _instance->device_name + ".list");
    k.remove_service(_instance->owner, _instance->device_name + ".list_descriptions");
};

//! service callback key-value read
/*!
 * \param request service request data
 * \parma response service response data
 * \return success
 */
int key_value::handler::service_read(const robotkernel::service_arglist_t& request, 
        robotkernel::service_arglist_t& response) {
    // vectors for keys and values
#define READ_REQ_KEYS   0
    std::vector<rk_type> keys = request[READ_REQ_KEYS];
    std::vector<rk_type> values(keys.size());
    string error_message = "";

    key_value_transfer_t t;
    t.keys.assign(keys.begin(), keys.end());
    
    try { 
        _instance->key_value_read(t);
        values.assign(t.values.begin(), t.values.end());
    } catch (std::exception& e) {
        error_message = e.what();
    }

#define READ_RESP_VALUES            0
#define READ_RESP_ERROR_MESSAGE     1
    response.resize(2);
    response[READ_RESP_VALUES]          = values;
    response[READ_RESP_ERROR_MESSAGE]   = error_message;

    return 0;
}

//! service callback key-value write
/*!
 * \param request service request data
 * \parma response service response data
 * \return success
 */
int key_value::handler::service_write(const robotkernel::service_arglist_t& request, 
        robotkernel::service_arglist_t& response) {
    // vectors for keys and values
#define WRITE_REQ_KEYS  0
#define WRITE_REQ_VALUES    1
    std::vector<rk_type> keys = request[WRITE_REQ_KEYS];
    std::vector<rk_type> values = request[WRITE_REQ_VALUES]; 
    string error_message = "";

    key_value_transfer_t t;
    t.keys.assign(keys.begin(), keys.end());
    for (std::vector<rk_type>::iterator it = values.begin(); it != values.end(); ++it) {
        std::string tmp = *it;
        t.values.push_back(tmp);
    }
    
    try { 
        _instance->key_value_write(t);
    } catch (std::exception& e) {
        error_message = e.what();
    }

#define WRITE_RESP_ERROR_MESSAGE        0
    response.resize(1);
    response[WRITE_RESP_ERROR_MESSAGE]  = error_message;

    return 0;
}

//! service callback key-value list
/*!
 * \param request service request data
 * \parma response service response data
 * \return success
 */
int key_value::handler::service_list(const robotkernel::service_arglist_t& request, 
        robotkernel::service_arglist_t& response) {
    std::vector<rk_type> keys;
    std::vector<rk_type> names;
    string error_message = "";

    key_value_transfer_t t;
    
    try { 
        _instance->key_value_list(t);
        keys.assign(t.keys.begin(), t.keys.end());
        names.assign(t.values.begin(), t.values.end());
    } catch (std::exception& e) {
        error_message = e.what();
    }

#define LIST_RESP_KEYS          0
#define LIST_RESP_NAMES         1
#define LIST_RESP_ERROR_MESSAGE 2
    response.resize(3);
    response[LIST_RESP_KEYS]            = keys;
    response[LIST_RESP_NAMES]           = names;
    response[LIST_RESP_ERROR_MESSAGE]   = error_message;

    return 0;
}

//! service callback descriptions
/*!
 * \param request service request data
 * \parma response service response data
 * \return success
 */
int key_value::handler::service_list_descriptions(const robotkernel::service_arglist_t& request, 
        robotkernel::service_arglist_t& response) {
    std::vector<rk_type> description;
    std::vector<rk_type> unit;
    std::vector<rk_type> default_value;
    std::vector<rk_type> format;
    std::vector<rk_type> read_only;
    string error_message = "";

    std::vector<key_value_description_t> t;
    
    try { 
        _instance->key_value_list_descriptions(t);

        description.resize(t.size());
        unit.resize(t.size());
        default_value.resize(t.size());
        format.resize(t.size());
        read_only.resize(t.size());

        for (unsigned i = 0; i < t.size(); ++i) {
            description[i]   = t[i].description;
            unit[i]          = t[i].unit;
            default_value[i] = t[i].default_value;
            format[i]        = t[i].format;
            read_only[i]     = t[i].read_only;
        }
    } catch (std::exception& e) {
        error_message = e.what();
    }

#define LIST_DESCRIPTIONS_RESP_DESCRIPTION          0
#define LIST_DESCRIPTIONS_RESP_UNIT                 1
#define LIST_DESCRIPTIONS_RESP_DEFAULT_VALUE        2
#define LIST_DESCRIPTIONS_RESP_FORMAT               3
#define LIST_DESCRIPTIONS_RESP_READ_ONLY            4
#define LIST_DESCRIPTIONS_RESP_ERROR_MESSAGE        5
    response.resize(6);
    response[LIST_DESCRIPTIONS_RESP_DESCRIPTION]     = description;
    response[LIST_DESCRIPTIONS_RESP_UNIT]            = unit;
    response[LIST_DESCRIPTIONS_RESP_DEFAULT_VALUE]   = default_value;
    response[LIST_DESCRIPTIONS_RESP_FORMAT]          = format;
    response[LIST_DESCRIPTIONS_RESP_READ_ONLY]       = read_only;
    response[LIST_DESCRIPTIONS_RESP_ERROR_MESSAGE]   = error_message;

    return 0;
}

