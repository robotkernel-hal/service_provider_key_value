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

SERVICE_PROVIDER_DEF(key_value, interface_key_value::key_value);

using namespace std;
using namespace std::placeholders;
using namespace robotkernel;
using namespace interface_key_value;

const char* interface_key_value::key_value_sp_magic = "key_value"; 

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

//! handler construction
key_value_handler::key_value_handler(
		std::string mod_name, std::string dev_name, int slave_id) : 
	log_base(mod_name, (mod_name + "." + dev_name + ".key_value")), 
	mod_name(mod_name), dev_name(dev_name), slave_id(slave_id) {
	kernel& k = *kernel::get_instance();

	stringstream base;
	base << mod_name << "." << dev_name << ".key_value.";

	k.add_service(mod_name, base.str() + "read", service_definition_read,
			std::bind(&key_value_handler::service_read, this, _1, _2));
	k.add_service(mod_name, base.str() + "write", service_definition_write,
			std::bind(&key_value_handler::service_write, this, _1, _2));
	k.add_service(mod_name, base.str() + "list", service_definition_list,
			std::bind(&key_value_handler::service_list, this, _1, _2));
}

//! handler destruction
key_value_handler::~key_value_handler() {
	kernel& k = *kernel::get_instance();

	stringstream base;
	base << mod_name << "." << dev_name << ".key_value.";
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
int key_value_handler::service_read(const robotkernel::service_arglist_t& request, 
		robotkernel::service_arglist_t& response) {
	// vectors for keys and values
#define READ_REQ_KEYS	0
	std::vector<rk_type> keys = request[READ_REQ_KEYS];
	std::vector<rk_type> values(keys.size());
	string error_message;

	std::vector<uint32_t> tmp_keys(keys.size());
	for (unsigned i = 0; i < keys.size(); ++i)
		tmp_keys[i] = keys[i];

	key_value_transfer_t t;
	memset(&t, 0, sizeof(t));
	t.slave_id      = slave_id;
	t.command       = kvc_read;
	t.keys          = &tmp_keys[0];
	t.keys_len      = tmp_keys.size();
	t.values        = new char*[tmp_keys.size()];
	t.values_len    = tmp_keys.size();

	int state = kernel::request_cb(mod_name.c_str(), 
			MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);

	if (state != 0) {
		// error!
		if (t.error_msg)
			error_message = t.error_msg;
		else {
			stringstream ss;
			ss << "kernel request failed with " << state;
			error_message = ss.str(); 
		}
	} else { 
		for (unsigned i = 0; i < t.values_len; ++i) {
			values[i] = string(t.values[i]);

			if (t.values[i])
				free(t.values[i]);
		}
	}

	delete[] t.values;

#define READ_RESP_VALUES			0
#define READ_RESP_ERROR_MESSAGE		1
	response.resize(2);
	response[READ_RESP_VALUES]			= values;
	response[READ_RESP_ERROR_MESSAGE]	= error_message;

	return 0;
}

const std::string key_value_handler::service_definition_read = 
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
int key_value_handler::service_write(const robotkernel::service_arglist_t& request, 
		robotkernel::service_arglist_t& response) {
	// vectors for keys and values
#define WRITE_REQ_KEYS	0
#define WRITE_REQ_VALUES	1
	std::vector<rk_type> keys = request[WRITE_REQ_KEYS];
	std::vector<rk_type> values = request[WRITE_REQ_VALUES]; 
	std::vector<uint32_t> tmp_keys(keys.size());
	int state;

	// default response values
	string error_message = "";

	if (keys.size() != values.size()) {
		stringstream ss;
		ss << "error keys_len is " << keys.size() 
			<< ", values_len is " << + values.size();
		error_message = ss.str();
		goto write_exit;
	}

	for (unsigned i = 0; i < keys.size(); ++i)
		tmp_keys[i] = keys[i];

	key_value_transfer_t t;
	memset(&t, 0, sizeof(t));
	t.slave_id      = slave_id;
	t.command       = kvc_write;
	t.keys          = &tmp_keys[0];
	t.keys_len      = tmp_keys.size();
	t.values        = new char*[tmp_keys.size()];
	t.values_len    = tmp_keys.size();

	for (unsigned i = 0; i < t.values_len; ++i) {
		string tmp = values[i];
		t.values[i] = strdup(tmp.c_str());
	}

	state = kernel::request_cb(mod_name.c_str(), 
			MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);

	if (state != 0) {
		// error!
		if (t.error_msg)
			error_message = t.error_msg;
		else {
			stringstream ss;
			ss << "kernel request failed with " << state;
			error_message = ss.str();
		}
	} 

	for (unsigned i = 0; i < t.values_len; ++i) {
		if (t.values[i])
			free(t.values[i]);
	}
	delete[] t.values;

write_exit:
#define WRITE_RESP_ERROR_MESSAGE	0
	response.resize(1);
	response[WRITE_RESP_ERROR_MESSAGE] = error_message;

	return 0;
}

const std::string key_value_handler::service_definition_write = 
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
int key_value_handler::service_list(const robotkernel::service_arglist_t& request, 
		robotkernel::service_arglist_t& response) {
	std::vector<rk_type> keys;
	std::vector<rk_type> names;
	string error_message = "";

	key_value_transfer_t t;
	memset(&t, 0, sizeof(t));
	t.slave_id = slave_id;
	t.command = kvc_list;

	int state = kernel::request_cb(mod_name.c_str(), 
			MOD_REQUEST_KEY_VALUE_TRANSFER, (void *)&t);

	if (state != 0) {
		// error!
		if (t.error_msg)
			error_message = t.error_msg;
		else {
			stringstream ss;
			ss << "kernel request failed with " << state;
			error_message = ss.str(); 
		}
	} else {
		keys.resize(t.keys_len);
		names.resize(t.values_len);

		for (unsigned i = 0; i < t.keys_len; ++i) 
			keys[i] = (uint32_t)(t.keys[i]);

		for (unsigned i = 0; i < t.values_len; ++i)
			names[i] = string(t.values[i]); 
	}

	if (t.keys)
		free(t.keys);

	for (unsigned i = 0; i < t.values_len; ++i)
		if (t.values[i])
			free(t.values[i]);

#define LIST_RESP_KEYS			0
#define LIST_RESP_NAMES			1
#define LIST_RESP_ERROR_MESSAGE	2
	response.resize(3);
	response[LIST_RESP_KEYS]			= keys;
	response[LIST_RESP_NAMES]			= names;
	response[LIST_RESP_ERROR_MESSAGE]	= error_message;

	return 0;
}

const std::string key_value_handler::service_definition_list =
"response:\n"
"   uint32_t*: keys\n"
"   string*: names\n"
"   string: error_message\n";

