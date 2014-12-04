//! robotkernel module class
/*!
 * author: Robert Burger
 *
 * $Id$
 */

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
#include "robotkernel/exceptions.h"
#undef BUILD_DATE
#undef PACKAGE
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION
#include "config.h"

#include <string_util/string_util.h>

using namespace std;
using namespace robotkernel;
using namespace interface;

#define intf_error(format, ...) klog(error, INTFNAME "%s: %s():%d " format, _mod_name.c_str(), __func__, __LINE__, __VA_ARGS__);

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
 * \param mod_name module name to register for
 */
key_value::key_value(const std::string& mod_name, const std::string& dev_name, const int& slave_id) 
    : _mod_name(mod_name), _dev_name(dev_name), _slave_id(slave_id) {
    kernel& k = *kernel::get_instance();
    if (!k.clnt)
        throw robotkernel::str_exception("[interface_key_value|%s] no ln_connection!\n", mod_name.c_str());
    
    stringstream base;
    base << k.clnt->name << "." << _mod_name << "." << _dev_name << ".";

    register_read(k.clnt, base.str() + "key_value.read");
    register_write(k.clnt, base.str() + "key_value.write");
    register_list(k.clnt, base.str() + "key_value.list");
}

int key_value::on_read(ln::service_request& req, ln_service_robotkernel_key_value_read& svc) {
    memset(&svc.resp, 0, sizeof(svc.resp));
    
    key_value_transfer_t t;
    memset(&t, 0, sizeof(t));
    t.slave_id = _slave_id;
    t.command = kvc_read;
    t.keys = svc.req.keys;
    t.keys_len = svc.req.keys_len;
    t.values = new char*[svc.req.keys_len];
    t.values_len = t.keys_len;
    
    int state = kernel::request_cb(_mod_name.c_str(), MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);
    
    if (state != 0) {
        // error!
        if(t.error_msg)
            svc.resp.error_message = t.error_msg;
        else
            svc.resp.error_message = strdup(format_string("kernel request failed with %d", state).c_str());
        svc.resp.error_message_len = strlen(svc.resp.error_message);
        intf_error("error %d: %s\n", state, svc.resp.error_message);
    } else {
        svc.resp.values_len = t.values_len;
        svc.resp.values = new _robotkernel_key_value_read_string[svc.resp.values_len];
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

int key_value::on_write(ln::service_request& req, ln_service_robotkernel_key_value_write& svc) {
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
    t.slave_id = _slave_id;
    t.command = kvc_write;
    t.keys = svc.req.keys;
    t.keys_len = svc.req.keys_len;
    t.values = new char*[svc.req.values_len];
    t.values_len = svc.req.values_len;
    
    for (unsigned i = 0; i < t.values_len; ++i)
        t.values[i] = strndup(svc.req.values[i].value, svc.req.values[i].value_len);

    int state = kernel::request_cb(_mod_name.c_str(), MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);

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
        
int key_value::on_list(ln::service_request& req, ln_service_robotkernel_key_value_list& svc) {
    memset(&svc.resp, 0, sizeof(svc.resp));
    
    key_value_transfer_t t;
    memset(&t, 0, sizeof(t));
    t.slave_id = _slave_id;
    t.command = kvc_list;
    
    int state = kernel::request_cb(_mod_name.c_str(), MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);

    if (state != 0) {
        // error!
        if(t.error_msg)
            svc.resp.error_message = t.error_msg;
        else
            svc.resp.error_message = strdup(format_string("kernel request failed with %d", state).c_str());
        svc.resp.error_message_len = strlen(svc.resp.error_message);
        intf_error("error %d: %s\n", state, svc.resp.error_message);
    } else {
        svc.resp.keys = t.keys;
        svc.resp.keys_len = t.keys_len;

        svc.resp.names_len = t.values_len;
        svc.resp.names = new _robotkernel_key_value_list_string[svc.resp.names_len];
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


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

//! interface register
/*!
 * \param mod_name module name to register
 * \return interface handle
 */
INTERFACE_HANDLE intf_register(const char *mod_name, const char *dev_name, int slave_id) {
    key_value *s = NULL;

    klog(info, INTFNAME "%s: build by: " BUILD_USER "@" BUILD_HOST "\n", mod_name);
    klog(info, INTFNAME "%s: build date: " BUILD_DATE "\n", mod_name);

    // parsing sercos ring configuration
    try {
        s = new key_value(string(mod_name), string(dev_name), slave_id);
    } catch(exception& e) {
        klog(error, INTFNAME "%s: error constructing interface:\n%s", mod_name, e.what());
        goto ErrorExit;
    }

    return (INTERFACE_HANDLE)s;

ErrorExit:
    if (s)
        delete s;

    return (INTERFACE_HANDLE)NULL;
}

//! interface unregister
/*!
 * \param hdl interface handle
 */
void intf_unregister(INTERFACE_HANDLE hdl) {
    // cast struct
    key_value *s = (key_value *)hdl;

    if (s)
        delete s;
}

#if 0
{
#endif
#ifdef __cplusplus
}
#endif

