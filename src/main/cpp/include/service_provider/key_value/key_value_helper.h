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
#include <algorithm>

#include "service_provider/key_value/base.h"

#include <string_util/string_util.h>

class key_value_slave;

class key_value_key_base
{	
    public:
        key_value_slave* parent;
        std::string name;
        bool after_change_cb;

        service_provider::key_value::key_value_description_t description; // if contents are set, they will be free()'d from this detor!

        key_value_key_base(key_value_slave* parent, std::string name, bool after_change_cb) 
            : parent(parent), name(name), after_change_cb(after_change_cb) {
                memset(&description, 0, sizeof(description));
            }

        virtual ~key_value_key_base() {};

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
class key_value_key : 
    public key_value_key_base 
{	
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

        key_value_slave(const std::string& owner, const std::string& service_prefix) :
            base(owner, service_prefix) {}
    
        ~key_value_slave()
	    { delete_keys(); }

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
         * \param[in] transfer      Key value request. Key and values vector has to 
         *                          be filled with keys and values you want to write.
         */
        void key_value_write(const service_provider::key_value::key_value_transfer_t& transfer);

        //! list keys with names
        /*!
         * \param[out] transfer     Key value request. Key vector will be filled with
         *                          all available key, values vector will be filled 
         *                          with their names.
         */
        void key_value_list(service_provider::key_value::key_value_transfer_t& transfer);

        //! list descriptions
        /*!
         * \param[out] transfer     Key value request. The data vector will returned the
         *                          descriptions of all available keys.
         */
        void key_value_list_descriptions(
                std::vector<service_provider::key_value::key_value_description_t>& data);
};

inline void key_value_slave::delete_keys() {
	for(keys_t::iterator i = keys.begin(); i != keys.end(); ++i) {
		delete *i;
	}

	keys.clear();
	key_map.clear();
}

// read key value pairs
inline void key_value_slave::key_value_read(
        service_provider::key_value::key_value_transfer_t& t) 
{
    t.values.resize(t.keys.size());

    for (unsigned i = 0; i < t.keys.size(); ++i) {
        uint32_t k = t.keys[i];

        if (k >= keys.size())
            throw str_exception_tb("unknown key_id %d for slave %s!", 
                    k, name.c_str());

        key_value_key_base* key = keys[k];
        t.values[i] = key->get_value();
    }
}

// write key value pairs
inline void key_value_slave::key_value_write(
        const service_provider::key_value::key_value_transfer_t& t)
{
    if (t.values.size() != t.keys.size())
        throw str_exception_tb("key_value_write: key_value_transfer_t::values.size() "
                "is %d and ::keys.size() is %d!", t.values.size(), t.keys.size());

    for (unsigned i = 0; i < t.keys.size(); ++i) {
        uint32_t k = t.keys[i];

        if (k >= keys.size())
            throw str_exception_tb("unknown key_id %d for slave %s!", 
                    k, name.c_str());

        key_value_key_base* key = keys[k];
        key->set_value(t.values[i]);
    }
}

// list keys with names
inline void key_value_slave::key_value_list(
        service_provider::key_value::key_value_transfer_t& t) 
{
    t.keys.resize(keys.size());
    t.values.resize(keys.size());

    for (unsigned i = 0; i < t.keys.size(); ++i) {
        const key_value_key_base* key = keys[i];

        t.keys[i] = i;
        t.values[i] = key->name;
    }
}

// list descriptions
inline void key_value_slave::key_value_list_descriptions(
        std::vector<service_provider::key_value::key_value_description_t>& data) 
{
    data.resize(keys.size());

    for (unsigned i = 0; i < keys.size(); ++i) {
        const key_value_key_base* key = keys[i];
        data[i] = key->description;
    }
}

template<>
inline void key_value_eval<bool>(bool* ptr, std::string repr) {
    std::transform(repr.begin(), repr.end(), repr.begin(), 
            [](unsigned char c) -> unsigned char { return std::tolower(c); });
    if(repr == "true" || repr == "1" || repr == "yes")
        *ptr = true;
    else
        *ptr = false;
}

template<>
inline void key_value_eval<float>(float* ptr, std::string repr) {
    *ptr = atof(repr.c_str());
}

template<>
inline void key_value_eval<int>(int* ptr, std::string repr) {
    *ptr = atoi(repr.c_str());
}

template<>
inline void key_value_eval<unsigned int>(unsigned int* ptr, std::string repr) {
    *ptr = (unsigned int)strtol(repr.c_str(), (char**)NULL, 10);
}

template<>
inline void key_value_eval<std::string>(std::string* ptr, std::string repr) {
    string_util::py_value* v = string_util::eval_full(repr);
    *ptr = std::string(*v);
    delete v;
}

template<>
inline std::string key_value_repr<bool>(bool& value) {
    if(value)
        return "True";
    return "False";
}

template<>
inline std::string key_value_repr<float>(float& value) {
    return string_util::format_string("%f", value);
}

template<>
inline std::string key_value_repr<int>(int& value) {
    return string_util::format_string("%d", value);
}

template<>
inline std::string key_value_repr<unsigned int>(unsigned int& value) {
    return string_util::format_string("%u", value);
}

template<>
inline std::string key_value_repr<std::string>(std::string& value) {
    return string_util::repr(value);
}

inline void key_value_key_base::set_value(std::string repr) {
	_set_value(repr);
	if(after_change_cb) {
		parent->handle_key_cb(name, true, repr);
	}
}

inline key_value_key_base& key_value_key_base::describe(std::string desc) {
	description.description = desc;
	return *this;
}

inline key_value_key_base& key_value_key_base::unit(std::string unit) {
    description.unit = unit;
    return *this;
}

inline key_value_key_base& key_value_key_base::default_value(std::string default_value) {
    description.default_value = default_value;
	return *this;

}

inline key_value_key_base& key_value_key_base::format(std::string format) {
    description.format = format;
	return *this;

}


#endif // KEY_VALUE_HELPER_H

