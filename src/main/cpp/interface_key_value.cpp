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
using namespace ::robotkernel;
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
    if (!k.clnt)
	    throw ::str_exception("[interface_key_value|%s] "
                "no ln_connection!\n", mod_name.c_str());
    
    stringstream base;
    base << k.clnt->name << "." << mod_name << "." << dev_name << ".";

    string group_name_str = "";
    if(node["ln_service_group"])
	    group_name_str = node["ln_service_group"].as<string>();
    const char* group_name = NULL;
    if(group_name_str.size())
	    group_name = group_name_str.c_str();
    
    register_read(k.clnt, base.str() + "key_value.read", group_name);
    register_write(k.clnt, base.str() + "key_value.write", group_name);
    register_list(k.clnt, base.str() + "key_value.list", group_name);
    register_list_descriptions(k.clnt, base.str() + "key_value.list_descriptions", group_name);
}

//! service reading key value pairs
int key_value::on_read(ln::service_request& req, robotkernel_key_value_read_t& svc) {
    memset(&svc.resp, 0, sizeof(svc.resp));
    
    key_value_transfer_t t;
    memset(&t, 0, sizeof(t));
    t.slave_id = slave_id;
    t.command = kvc_read;
    t.keys = svc.req.keys;
    t.keys_len = svc.req.keys_len;
    t.values = new char*[svc.req.keys_len];
    t.values_len = t.keys_len;
    
    int state = kernel::request_cb(mod_name.c_str(), MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);
    
    if (state != 0) {
        // error!
        if(t.error_msg)
            svc.resp.error_message = t.error_msg;
        else
            svc.resp.error_message = strdup(format_string("kernel request failed with %d", state).c_str());
        svc.resp.error_message_len = strlen(svc.resp.error_message);
        log(error, "%d: %s\n", state, svc.resp.error_message);
    } else {
        svc.resp.values_len = t.values_len;
        svc.resp.values = new robotkernel_key_value_string_t[svc.resp.values_len];
        for (unsigned i = 0; i < t.values_len; ++i) {
            svc.resp.values[i].value = t.values[i]; 
            svc.resp.values[i].value_len = strlen(t.values[i]);
        }
    }
    
    req.respond();

    if(svc.resp.error_message)
        free(svc.resp.error_message);
    
    if(t.values) {
        for(unsigned i = 0; i < t.keys_len; ++i)
            if (t.values[i])
                free(t.values[i]);
        delete[] t.values;
    }
    
    if(svc.resp.values)
        delete[] svc.resp.values;
    
    return 0;
}


int key_value::on_write(ln::service_request& req, robotkernel_key_value_write_t& svc) {
    memset(&svc.resp, 0, sizeof(svc.resp));
    
    if(svc.req.keys_len != svc.req.values_len) {
        ln::string_buffer error_msg(
            &svc.resp.error_message,
            format_string("error keys_len is %d, values_len is %d!",
                          svc.req.keys_len,
                          svc.req.values_len));
        req.respond();
        return 0;
    }
    
    key_value_transfer_t t;
    memset(&t, 0, sizeof(t));
    t.slave_id = slave_id;
    t.command = kvc_write;
    t.keys = svc.req.keys;
    t.keys_len = svc.req.keys_len;
    t.values = new char*[svc.req.values_len];
    t.values_len = svc.req.values_len;
    
    for (unsigned i = 0; i < t.values_len; ++i)
        t.values[i] = strndup(svc.req.values[i].value, svc.req.values[i].value_len);

    int state = kernel::request_cb(mod_name.c_str(), MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);

    if (state != 0) {
        // error!
        if(t.error_msg)
            svc.resp.error_message = t.error_msg;
        else
            svc.resp.error_message = strdup(format_string("kernel request failed with %d", state).c_str());
        svc.resp.error_message_len = strlen(svc.resp.error_message);
    }
    
    req.respond();

    for (unsigned i = 0; i < t.values_len; ++i)
        if (t.values[i])
            free(t.values[i]);
    delete[] t.values;
    
    if (svc.resp.error_message)
        free(svc.resp.error_message);

    return 0;
}
        
int key_value::on_list(ln::service_request& req, robotkernel_key_value_list_t& svc) {
    memset(&svc.resp, 0, sizeof(svc.resp));
    
    key_value_transfer_t t;
    memset(&t, 0, sizeof(t));
    t.slave_id = slave_id;
    t.command = kvc_list;
    
    int state = kernel::request_cb(mod_name.c_str(), MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);

    if (state != 0) {
        // error!
        if(t.error_msg)
            svc.resp.error_message = t.error_msg;
        else
            svc.resp.error_message = strdup(format_string("kernel request failed with %d", state).c_str());
        svc.resp.error_message_len = strlen(svc.resp.error_message);
        log(error, "%d: %s\n", state, svc.resp.error_message);
    } else {
        svc.resp.keys = t.keys;
        svc.resp.keys_len = t.keys_len;

        svc.resp.names_len = t.values_len;
        svc.resp.names = new robotkernel_key_value_string_t[svc.resp.names_len];
        for (unsigned i = 0; i < svc.resp.names_len; ++i) {
            svc.resp.names[i].value = t.values[i]; 
            svc.resp.names[i].value_len = strlen(t.values[i]);
        }
    }
    
    req.respond();

    if (svc.resp.error_message)
        free(svc.resp.error_message);

    if (t.keys)
        free(t.keys);

    for (unsigned i = 0; i < t.values_len; ++i)
        if (t.values[i])
            free(t.values[i]);
    
    if(svc.resp.names)
        delete[] svc.resp.names;
    
    return 0;
}

int key_value::on_list_descriptions(ln::service_request& req, robotkernel_key_value_list_descriptions_t& data) {
    memset(&data.resp, 0, sizeof(data.resp));
    
    key_value_transfer_t t;
    memset(&t, 0, sizeof(t));
    t.slave_id = slave_id;
    t.command = kvc_list_descriptions;
    
    try {
        int ret = kernel::request_cb(mod_name.c_str(), MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);
        
        if(ret) {
            // error!
            if(t.error_msg) {
                throw ::str_exception_tb("module returned error %d for list_descriptions key-value-transfer:\n%s", ret, t.error_msg);
            } else {
                throw ::str_exception_tb("module returned error %d for list_descriptions key-value-transfer (without error message)!", ret);
            }
        }
        
        vector<robotkernel_key_value_description_t> descriptions(t.keys_len);
        for(unsigned int i = 0; i < t.keys_len; i++) {
            
#define pass_string(target, source)                                     \
            target.value_len = source ? strlen(source) : 0;             \
            target.value     = target.value_len ? (char*)source : NULL;   \

#define pass_string_field(field)                                        \
            pass_string(descriptions[i].field, t.descriptions[i].field);

#define pass_field(field)                                                \
            descriptions[i].field = t.descriptions[i].field;

            pass_string_field(description);
            pass_string_field(unit);
            pass_string_field(default_value);
            pass_string_field(format);
            pass_field       (read_only);
            
        }
        data.resp.descriptions = &descriptions[0];
        data.resp.descriptions_len = descriptions.size();

        req.respond();        
    }
    catch(const exception& e) {
        log(error, "%s: caught exception:\n%s\n", __func__, e.what());
        ln::string_buffer err(&data.resp.error_message, e.what());
        req.respond();
    }

    if(t.keys_len && t.descriptions)
        free(t.descriptions);
    
    if(t.error_msg)
        free(t.error_msg);
    
    return 0;

}
