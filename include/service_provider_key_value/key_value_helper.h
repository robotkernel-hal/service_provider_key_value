//! robotkernel service provider key value helper
/*!
 * author: Florian Schmid <florian.schmidt@dlr.de>
 * author: Robert Burger <robert.burger@dlr.de>
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

#ifndef SERVICE_PROVIDER_KEY_VALUE__KEY_VALUE_HELPER_H
#define SERVICE_PROVIDER_KEY_VALUE__KEY_VALUE_HELPER_H

#include <string>
#include <list>
#include <map>
#include <algorithm>

#include "service_provider_key_value/base.h"

#include <string_util/string_util.h>

namespace service_provider_key_value {

class slave;

class key_base
{	
    public:
        slave* parent;
        std::string name;
        bool after_change_cb;

        service_provider_key_value::description_t description; // if contents are set, they will be free()'d from this detor!

        key_base(slave* parent, std::string name, bool after_change_cb) 
            : parent(parent), name(name), after_change_cb(after_change_cb) {
                //memset(&description, 0, sizeof(description));
            }

        virtual ~key_base() {};

        virtual void _set_value(std::string repr) = 0;
        virtual void _set_value_from_yaml(const YAML::Node& value) = 0;
        void set_value(std::string repr);
        virtual std::string get_value() = 0;
        virtual void* get_void_pointer() = 0;

        virtual key_base& describe(std::string desc);
        virtual key_base& unit(std::string unit);
        virtual key_base& default_value(std::string default_value);
        virtual key_base& format(std::string format);
        virtual key_base& read_only(bool is_read_only) { description.read_only = is_read_only; return *this; }
};

template <typename T> void eval(T* ptr, std::string repr);
template <typename T> std::string repr(T& ptr);

template <typename T>
class key : 
    public key_base 
{	
    public:
        T* ptr;
        key(slave* parent, std::string name, T* ptr, bool after_change_cb)
            : key_base(parent, name, after_change_cb), ptr(ptr) {
                // printf("new key value %s with ptr %#x\n", name.c_str(), ptr);
            }

        virtual ~key() {}

        virtual void _set_value(std::string repr) {
            eval<T>(ptr, repr);			
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
            return repr<T>(*ptr);
        }

        virtual void* get_void_pointer() {
            return (void*)ptr;
        }
};

template <typename T>
class key_read_only : 
    public key_base 
{	
    public:
        T* ptr;
        key_read_only(slave* parent, std::string name, T* ptr, bool after_change_cb)
            : key_base(parent, name, after_change_cb), ptr(ptr) {
                // printf("new key value %s with ptr %#x\n", name.c_str(), ptr);
            }

        virtual ~key_read_only() {}

        virtual void _set_value(std::string repr) {}
        virtual void _set_value_from_yaml(const YAML::Node& value) {}

        virtual std::string get_value() {
            return repr<T>(*ptr);
        }

        virtual void* get_void_pointer() {
            return (void*)ptr;
        }
};

class slave : 
    public service_provider_key_value::base
{
    public:
        typedef std::vector<key_base*> keys_t;
        keys_t keys;
        typedef std::map<std::string, key_base*> key_map_t;
        key_map_t key_map;

        std::string name;

        slave(const std::string& owner, const std::string& service_prefix) :
            base(owner, service_prefix) {}
    
        ~slave()
	    { delete_keys(); }

        void check_exists(std::string name) {
            if(key_map.find(name) != key_map.end())
                throw str_exception_tb("key %s already defined!", string_util::repr(name).c_str());
        }
        void _add_key(key_base* new_key) {
            keys.push_back(new_key);
            key_map[new_key->name] = new_key;
        }

        void delete_keys();

        virtual void handle_key_cb(std::string key, bool op_set, std::string& arg) {
            throw str_exception_tb("slave::handle_key_cb not implemented!");
        }

        void add_key_bool(std::string name, bool* value, bool init, bool after_change_cb=false) {
            *value = init;
            check_exists(name);
            _add_key(new key<bool>(this, name, value, after_change_cb));
        }

        void add_key_float(std::string name, float* value, float init, bool after_change_cb=false) {
            *value = init;
            check_exists(name);
            _add_key(new key<float>(this, name, value, after_change_cb));
        }

        void add_key_no_init_float(std::string name, float* value, bool after_change_cb=false) {
            check_exists(name);
            _add_key(new key<float>(this, name, value, after_change_cb));
        }

        void add_key_string(std::string name, std::string* value, std::string init, bool after_change_cb=false) {
            *value = init;
            check_exists(name);
            _add_key(new key<std::string>(this, name, value, after_change_cb));
        }

        //! read key value pairs
        /*!
         * \param[in,out] transfer  Key value request. Key vector has to be filled
         *                          with keys you want to read, the values will 
         *                          be returned in the values vector.
         */
        void read(service_provider_key_value::transfer_t& transfer);

        //! write key value pairs
        /*!
         * \param[in] transfer      Key value request. Key and values vector has to 
         *                          be filled with keys and values you want to write.
         */
        void write(const service_provider_key_value::transfer_t& transfer);

        //! list keys with names
        /*!
         * \param[out] transfer     Key value request. Key vector will be filled with
         *                          all available key, values vector will be filled 
         *                          with their names.
         */
        void list(service_provider_key_value::transfer_t& transfer);

        //! list descriptions
        /*!
         * \param[out] transfer     Key value request. The data vector will returned the
         *                          descriptions of all available keys.
         */
        void list_descriptions(
                std::vector<service_provider_key_value::description_t>& data);
};

inline void slave::delete_keys() {
	for(keys_t::iterator i = keys.begin(); i != keys.end(); ++i) {
		delete *i;
	}

	keys.clear();
	key_map.clear();
}

// read key value pairs
inline void slave::read(
        service_provider_key_value::transfer_t& t) 
{
    t.values.resize(t.keys.size());

    for (unsigned i = 0; i < t.keys.size(); ++i) {
        uint32_t k = t.keys[i];

        if (k >= keys.size())
            throw str_exception_tb("unknown key_id %d for slave %s!", 
                    k, name.c_str());

        key_base* key = keys[k];
        t.values[i] = key->get_value();
    }
}

// write key value pairs
inline void slave::write(
        const service_provider_key_value::transfer_t& t)
{
    if (t.values.size() != t.keys.size())
        throw str_exception_tb("write: transfer_t::values.size() "
                "is %d and ::keys.size() is %d!", t.values.size(), t.keys.size());

    for (unsigned i = 0; i < t.keys.size(); ++i) {
        uint32_t k = t.keys[i];

        if (k >= keys.size())
            throw str_exception_tb("unknown key_id %d for slave %s!", 
                    k, name.c_str());

        key_base* key = keys[k];
        key->set_value(t.values[i]);
    }
}

// list keys with names
inline void slave::list(
        service_provider_key_value::transfer_t& t) 
{
    t.keys.resize(keys.size());
    t.values.resize(keys.size());

    for (unsigned i = 0; i < t.keys.size(); ++i) {
        const key_base* key = keys[i];

        t.keys[i] = i;
        t.values[i] = key->name;
    }
}

// list descriptions
inline void slave::list_descriptions(
        std::vector<service_provider_key_value::description_t>& data) 
{
    data.resize(keys.size());

    for (unsigned i = 0; i < keys.size(); ++i) {
        const key_base* key = keys[i];
        data[i] = key->description;
    }
}

template<>
inline void eval<bool>(bool* ptr, std::string repr) {
    std::transform(repr.begin(), repr.end(), repr.begin(), 
            [](unsigned char c) -> unsigned char { return std::tolower(c); });
    if(repr == "true" || repr == "1" || repr == "yes")
        *ptr = true;
    else
        *ptr = false;
}

template<>
inline void eval<double>(double* ptr, std::string repr) {
    *ptr = strtod(repr.c_str(), NULL);
}

template<>
inline void eval<float>(float* ptr, std::string repr) {
    *ptr = atof(repr.c_str());
}

template<>
inline void eval<short>(short* ptr, std::string repr) {
    *ptr = atoi(repr.c_str());
}

template<>
inline void eval<unsigned short>(unsigned short* ptr, std::string repr) {
    *ptr = (unsigned short)strtol(repr.c_str(), (char**)NULL, 10);
}

template<>
inline void eval<unsigned char>(unsigned char* ptr, std::string repr) {
    *ptr = (unsigned char)strtol(repr.c_str(), (char**)NULL, 10);
}

template<>
inline void eval<int>(int* ptr, std::string repr) {
    *ptr = atoi(repr.c_str());
}

template<>
inline void eval<unsigned int>(unsigned int* ptr, std::string repr) {
    *ptr = (unsigned int)strtol(repr.c_str(), (char**)NULL, 10);
}

template<>
inline void eval<int64_t>(int64_t* ptr, std::string repr) {
    *ptr = (int64_t)strtoull(repr.c_str(), (char**)NULL, 10);
}

template<>
inline void eval<uint64_t>(uint64_t* ptr, std::string repr) {
    *ptr = (uint64_t)strtoll(repr.c_str(), (char**)NULL, 10);
}

template<>
inline void eval<std::string>(std::string* ptr, std::string repr) {
    string_util::py_value* v = string_util::eval_full(repr);
    *ptr = std::string(*v);
    delete v;
}

template<>
inline std::string repr<bool>(bool& value) {
    if(value)
        return "True";
    return "False";
}

template<>
inline std::string repr<double>(double& value) {
    return string_util::format_string("%f", value);
}

template<>
inline std::string repr<float>(float& value) {
    return string_util::format_string("%f", value);
}

template<>
inline std::string repr<short>(short& value) {
    return string_util::format_string("%hd", value);
}

template<>
inline std::string repr<unsigned short>(unsigned short& value) {
    return string_util::format_string("%hu", value);
}

template<>
inline std::string repr<unsigned char>(unsigned char& value) {
    return string_util::format_string("%hu", value);
}

template<>
inline std::string repr<int>(int& value) {
    return string_util::format_string("%d", value);
}

template<>
inline std::string repr<unsigned int>(unsigned int& value) {
    return string_util::format_string("%u", value);
}

template<>
inline std::string repr<std::string>(std::string& value) {
    return string_util::repr(value);
}

template<>
inline std::string repr<char *>(char *& value) {
    return string_util::format_string("%s", value);
}

template<>
inline std::string repr<int64_t>(int64_t& value) {
    return string_util::format_string("%lld", value);
}

template<>
inline std::string repr<uint64_t>(uint64_t& value) {
    return string_util::format_string("%llu", value);
}

inline void key_base::set_value(std::string repr) {
	_set_value(repr);
	if(after_change_cb) {
		parent->handle_key_cb(name, true, repr);
	}
}

inline key_base& key_base::describe(std::string desc) {
	description.description = desc;
	return *this;
}

inline key_base& key_base::unit(std::string unit) {
    description.unit = unit;
    return *this;
}

inline key_base& key_base::default_value(std::string default_value) {
    description.default_value = default_value;
	return *this;

}

inline key_base& key_base::format(std::string format) {
    description.format = format;
	return *this;

}

}; // namespace service_provider_key_value

#endif // SERVICE_PROVIDER_KEY_VALUE__KEY_VALUE_HELPER_H

