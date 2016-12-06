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

#ifndef __INTERFACE_KEY_VALUE_MODULE_INTF_H__
#define __INTERFACE_KEY_VALUE_MODULE_INTF_H__

#define MOD_REQUEST_KEY_VALUE_MAGIC  0x22
#define MOD_REQUEST_KEY_VALUE(x, s) \
    __MOD_REQUEST((MOD_REQUEST_KEY_VALUE_MAGIC), (x), __MOD_REQUEST_TYPE(s))

typedef enum key_value_command {
    kvc_read  = 0,      //!< read key value pairs
    kvc_write = 1,      //!< write key value pairs
    kvc_list  = 2,      //!< list available key value pairs
    kvc_list_descriptions  = 3 //!< list descriptions for each key
} key_value_command_t;

typedef struct {
    const char* description;
    const char* unit;
    const char* default_value;
    const char* format;
    uint8_t read_only;
} key_value_description_t;

typedef struct key_value_transfer {
    int slave_id;
    key_value_command_t command;

    uint32_t *keys;     //! kvc_read, kvc_write: provided by caller
                        //! kvc_list: provided by callee            
    size_t keys_len;    //! length of keys

    char **values;      //! each values[x] is a \0-terminated string
                        //! kvc_read, kvc_write: values is provided by caller,
                        //! kvc_list: values is provided by callee,
                        //! kvc_read, kvc_list: values[x] are provided by callee, 
                        //!      caller has to free() values[x],
                        //! kvc_write: values[x] are provided by caller
    size_t values_len;
    char *error_msg;    //! kvc_read, kvc_write, kvc_list: provided by callee or 
                        //!      NULL, caller has to free() error_msg

    key_value_description_t* descriptions; /* only used for kvc_list_descriptions,
                                              length is returned via keys_len,
                                              descriptions must be freed by caller,
                                              descriptions[x] must also be freed by caller
                                           */
} key_value_transfer_t;

#define MOD_REQUEST_KEY_VALUE_TRANSFER  MOD_REQUEST_KEY_VALUE(0x0001, key_value_transfer_t)

#endif // __INTERFACE_KEY_VALUE_MODULE_INTF_H__

