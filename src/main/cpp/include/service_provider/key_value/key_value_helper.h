//! robotkernel service provider key value helper
/*!
 * author: Florian Schmid <florian.schmidt@dlr.de>
 * author: Robert Burger <robert.burger@dlr.de>
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


#ifndef KEY_VALUE_HELPER_H
#define KEY_VALUE_HELPER_H

#include <robotkernel/kernel.h>
#include <string>
#include <list>
#include <map>

#include "service_provider/key_value/base.h"

#include <string_util/string_util.h>

class key_value_slave;

class key_value_key_base {	
public:
	key_value_slave* parent;
	std::string name;
	bool after_change_cb;

    service_provider::key_value::key_value_description_t description; // if contents are set, they will be free()'d from this detor!
	
	key_value_key_base(key_value_slave* parent, std::string name, bool after_change_cb);
	virtual ~key_value_key_base();
	
	virtual void _set_value(std::string repr) = 0;
	virtual void _set_value_from_yaml(const YAML::Node& value) = 0;
	void set_value(std::string repr);
	virtual std::string get_value() = 0;
	virtual void* get_void_pointer() = 0;

	virtual key_value_key_base& describe(std::string desc);
	virtual key_value_key_base& unit(std::string unit);
	virtual key_value_key_base& default_value(std::string default_value);
	virtual key_value_key_base& format(std::string format);
	virtual key_value_key_base& read_only(bool is_read_only) { description.read_only = is_read_only; return *this; }
};

template <typename T> void key_value_eval(T* ptr, std::string repr);
template <typename T> std::string key_value_repr(T& ptr);

template <typename T>
class key_value_key : public key_value_key_base {	
public:
	T* ptr;
	key_value_key(key_value_slave* parent, std::string name, T* ptr, bool after_change_cb)
		: key_value_key_base(parent, name, after_change_cb), ptr(ptr) {
		// printf("new key value %s with ptr %#x\n", name.c_str(), ptr);
	}
	virtual ~key_value_key() {}

	virtual void _set_value(std::string repr) {
		key_value_eval<T>(ptr, repr);			
	}
	virtual void _set_value_from_yaml(const YAML::Node& value) {
#ifdef QNX63
		T dummy;
		value >> dummy;
		*ptr = dummy;
#else
		*ptr = value.as<T>();
#endif
	}
	virtual std::string get_value() {
		return key_value_repr<T>(*ptr);
	}
	virtual void* get_void_pointer() {
		return (void*)ptr;
	}
};

class key_value_slave : 
    public service_provider::key_value::base
{
public:
	typedef std::vector<key_value_key_base*> keys_t;
	keys_t keys;
	typedef std::map<std::string, key_value_key_base*> key_map_t;
	key_map_t key_map;

	std::string name;

	key_value_slave(const std::string& owner, const std::string& service_prefix);
	~key_value_slave();
	
	void check_exists(std::string name) {
		if(key_map.find(name) != key_map.end())
			throw str_exception_tb("key %s already defined!", string_util::repr(name).c_str());
	}
	void _add_key(key_value_key_base* new_key) {
		keys.push_back(new_key);
		key_map[new_key->name] = new_key;
	}

	void delete_keys();

	virtual void handle_key_cb(std::string key, bool op_set, std::string& arg) {
		throw str_exception_tb("key_value_slave::handle_key_cb not implemented!");
	}
	
	void add_key_bool(std::string name, bool* value, bool init, bool after_change_cb=false) {
		*value = init;
		check_exists(name);
		_add_key(new key_value_key<bool>(this, name, value, after_change_cb));
	}
	void add_key_float(std::string name, float* value, float init, bool after_change_cb=false) {
		*value = init;
		check_exists(name);
		_add_key(new key_value_key<float>(this, name, value, after_change_cb));
	}
	void add_key_no_init_float(std::string name, float* value, bool after_change_cb=false) {
		check_exists(name);
		_add_key(new key_value_key<float>(this, name, value, after_change_cb));
	}
	void add_key_string(std::string name, std::string* value, std::string init, bool after_change_cb=false) {
		*value = init;
		check_exists(name);
		_add_key(new key_value_key<std::string>(this, name, value, after_change_cb));
	}
        
        //! read key value pairs
        /*!
         * \param[in,out] transfer  Key value request. Key vector has to be filled
         *                          with keys you want to read, the values will 
         *                          be returned in the values vector.
         */
        void key_value_read(service_provider::key_value::key_value_transfer_t& transfer);

        //! write key value pairs
        /*!
         * \param[in,out] transfer request
         */
        void key_value_write(const service_provider::key_value::key_value_transfer_t& transfer);

        //! list keys with names
        /*!
         * \param[out] transfer request
         */
        void key_value_list(service_provider::key_value::key_value_transfer_t& transfer);
	
        //! list descriptions
        /*!
         * \param[out] data request
         */
        void key_value_list_descriptions(
                std::vector<service_provider::key_value::key_value_description_t>& data);
};

#endif // KEY_VALUE_HELPER_H
