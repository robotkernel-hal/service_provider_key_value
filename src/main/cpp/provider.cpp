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

//! handler construction
key_value::handler::handler(const robotkernel::sp_service_requester_t& req)
    : log_base("key_value", req->owner + "." + req->service_prefix + ".key_value") {
    robotkernel::kernel& k = *robotkernel::kernel::get_instance();

    _instance = std::dynamic_pointer_cast<service_provider::key_value::base>(req);
    if (!_instance)
        throw str_exception("wrong base class");

    stringstream base;
    base << _instance->owner << "." << _instance->service_prefix << ".key_value.";

    k.add_service(_instance->owner, base.str() + "read", service_definition_read,
            std::bind(&key_value::handler::service_read, this, _1, _2));
    k.add_service(_instance->owner, base.str() + "write", service_definition_write,
            std::bind(&key_value::handler::service_write, this, _1, _2));
    k.add_service(_instance->owner, base.str() + "list", service_definition_list,
            std::bind(&key_value::handler::service_list, this, _1, _2));
}

//! handler destruction
key_value::handler::~handler() {
    kernel& k = *kernel::get_instance();

    stringstream base;
    base << _instance->owner << "." << _instance->service_prefix << ".key_value.";
    k.remove_service(base.str() + "read");
    k.remove_service(base.str() + "write");
    k.remove_service(base.str() + "list");
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

const std::string key_value::handler::service_definition_read = 
"request:\n"
"   vector/uint32_t: keys\n"
"response:\n"
"   vector/string: values\n"
"   string: error_message\n";

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

const std::string key_value::handler::service_definition_write = 
"request:\n"
"   uint32_t*: keys\n"
"   string*: values\n"
"response:\n"
"   string: error_message\n";

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
        _instance->key_value_write(t);
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

const std::string key_value::handler::service_definition_list =
"response:\n"
"   uint32_t*: keys\n"
"   string*: names\n"
"   string: error_message\n";

