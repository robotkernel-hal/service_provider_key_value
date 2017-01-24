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

#include "interface_key_value.h"
#include "robotkernel/kernel.h"
#include "robotkernel/exceptions.h"
#include <interface_key_value/module_intf.h>

#include <string_util/string_util.h>

INTERFACE_DEF(key_value, interface_key_value::key_value)

using namespace std;
using namespace robotkernel;
using namespace interface_key_value;

#ifndef __linux__
static char *strndup(const char *s, size_t n) {
    const char* cp = s;
    size_t i = 0;
    while(*cp) {
        i++;
        if(i >= n)
            break; // enough chars
        cp++;
    }
    i ++;
    char* result = new char[i];
    memcpy(result, s, i);
    result[i - 1] = 0;
    return result;
}
#endif

//! default construction
/*!
 * \param node configuration node
 */
key_value::key_value(const YAML::Node& node) 
    : interface_base("key_value", node) {
    kernel& k = *kernel::get_instance();
    
    stringstream base;
    base << mod_name << "." << dev_name << ".key_value.";

    k.add_service(mod_name, base.str() + "read", service_definition_read,
            boost::bind(&key_value::service_read, this, _1));
    k.add_service(mod_name, base.str() + "write", service_definition_write,
            boost::bind(&key_value::service_write, this, _1));
    k.add_service(mod_name, base.str() + "list", service_definition_list,
            boost::bind(&key_value::service_list, this, _1));
}

//! service callback key-value read
/*!
 * \param message service message
 * \return success
 */
int key_value::service_read(YAML::Node& message) {
    // vectors for keys and values
    std::vector<uint32_t> keys = 
        get_as<std::vector<uint32_t> >(message["request"], "keys");    
    std::vector<string> values(keys.size());
    
    // default response values
    message["response"]["values"] = values;
    message["response"]["error_message"] = "";

    key_value_transfer_t t;
    memset(&t, 0, sizeof(t));
    t.slave_id      = slave_id;
    t.command       = kvc_read;
    t.keys          = &keys[0];
    t.keys_len      = keys.size();
    t.values        = new char*[keys.size()];
    t.values_len    = keys.size();

    int state = kernel::request_cb(mod_name.c_str(), 
            MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);
    
    if (state != 0) {
        // error!
        if (t.error_msg)
            message["response"]["error_message"] = t.error_msg;
        else {
            stringstream ss;
            ss << "kernel request failed with " << state;
            message["response"]["error_message"] = ss.str(); 
        }

        return 0;        
    } 

    for (unsigned i = 0; i < t.values_len; ++i) {
        values[i] = string(t.values[i]);
            
        if (t.values[i])
            free(t.values[i]);
    }

    delete[] t.values;

    message["response"]["values"] = values;

    return 0;
}

const std::string key_value::service_definition_read = 
    "request:\n"
    "   uint32_t*: keys\n"
    "response:\n"
    "   string*: values\n"
    "   string: error_message\n";

//! service callback key-value write
/*!
 * \param message service message
 * \return success
 */
int key_value::service_write(YAML::Node& message) {
    // vectors for keys and values
    std::vector<uint32_t> keys = 
        get_as<std::vector<uint32_t> >(message["request"], "keys");    
    std::vector<string> values = 
        get_as<std::vector<string> >(message["request"], "values");
    
    // default response values
    message["response"]["error_message"] = "";

    if (keys.size() != values.size()) {
        stringstream ss;
        ss << "error keys_len is " << keys.size() 
            << ", values_len is " << + values.size();
        message["response"]["error_message"] = ss.str();
        return 0;
    }

    key_value_transfer_t t;
    memset(&t, 0, sizeof(t));
    t.slave_id      = slave_id;
    t.command       = kvc_write;
    t.keys          = &keys[0];
    t.keys_len      = keys.size();
    t.values        = new char*[keys.size()];
    t.values_len    = keys.size();

    for (unsigned i = 0; i < t.values_len; ++i)
        t.values[i] = strdup(values[i].c_str());

    int state = kernel::request_cb(mod_name.c_str(), 
            MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);
    
    if (state != 0) {
        // error!
        if (t.error_msg)
            message["response"]["error_message"] = t.error_msg;
        else {
            stringstream ss;
            ss << "kernel request failed with " << state;
            message["response"]["error_message"] = ss.str();
        }

        return 0;        
    } 

    for (unsigned i = 0; i < t.values_len; ++i) {
        values[i] = string(t.values[i]);
            
        if (t.values[i])
            free(t.values[i]);
    }

    delete[] t.values;

    return 0;
}

const std::string key_value::service_definition_write = 
    "request:\n"
    "   uint32_t*: keys\n"
    "   string*: values\n"
    "response:\n"
    "   string: error_message\n";

//! service callback key-value list
/*!
 * \param message service message
 * \return success
 */
int key_value::service_list(YAML::Node& message) {
    std::vector<uint32_t> keys;
    std::vector<string> names;

    // default response values
    message["response"]["keys"] = keys;
    message["response"]["names"] = names;
    message["response"]["error_message"] = "";

    key_value_transfer_t t;
    memset(&t, 0, sizeof(t));
    t.slave_id = slave_id;
    t.command = kvc_list;
    
    int state = kernel::request_cb(mod_name.c_str(), 
            MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);

    if (state != 0) {
        // error!
        if (t.error_msg)
            message["response"]["error_message"] = t.error_msg;
        else {
            stringstream ss;
            ss << "kernel request failed with " << state;
            message["response"]["error_message"] = ss.str(); 
        }
    } else {
        keys.assign(t.keys, t.keys + t.keys_len);
        names.resize(t.values_len);

        for (unsigned i = 0; i < t.values_len; ++i) {
            names[i] = string(t.values[i]); 
        }
    
        message["response"]["keys"] = keys;
        message["response"]["names"] = names;
    }
    
    if (t.keys)
        free(t.keys);

    for (unsigned i = 0; i < t.values_len; ++i)
        if (t.values[i])
            free(t.values[i]);

    return 0;
}

const std::string key_value::service_definition_list =
    "response:\n"
    "   uint32_t*: keys\n"
    "   string*: names\n"
    "   string: error_message\n";

