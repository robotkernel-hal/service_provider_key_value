#include <algorithm>
#include <string>

#include <service_provider/key_value/key_value_helper.h>

using namespace std;
using namespace string_util;
using namespace service_provider;
using namespace service_provider::key_value;

void key_value_key_base::set_value(std::string repr) {
	_set_value(repr);
	if(after_change_cb) {
		parent->handle_key_cb(name, true, repr);
	}
}

template<>
void key_value_eval<bool>(bool* ptr, std::string repr) {
	std::transform(repr.begin(), repr.end(), repr.begin(), ::tolower);
	if(repr == "true" || repr == "1" || repr == "yes")
 		*ptr = true;
	else
		*ptr = false;
}
template<>
void key_value_eval<float>(float* ptr, std::string repr) {
	*ptr = atof(repr.c_str());
}
template<>
void key_value_eval<int>(int* ptr, std::string repr) {
	*ptr = atoi(repr.c_str());
}
template<>
void key_value_eval<unsigned int>(unsigned int* ptr, std::string repr) {
	*ptr = (unsigned int)strtol(repr.c_str(), (char**)NULL, 10);
}
template<>
void key_value_eval<string>(string* ptr, std::string repr) {
	py_value* v = eval_full(repr);
	*ptr = string(*v);
	delete v;
}

template<>
std::string key_value_repr<bool>(bool& value) {
	if(value)
		return "True";
	return "False";
}
template<>
std::string key_value_repr<float>(float& value) {
	return format_string("%f", value);
}
template<>
std::string key_value_repr<int>(int& value) {
	return format_string("%d", value);
}
template<>
std::string key_value_repr<unsigned int>(unsigned int& value) {
	return format_string("%u", value);
}
template<>
std::string key_value_repr<string>(string& value) {
	return repr(value);
}


key_value_slave::key_value_slave(const std::string& owner, 
        const std::string& service_prefix) : 
    base(owner, service_prefix)
{
}

key_value_slave::~key_value_slave() {
	delete_keys();
}

void key_value_slave::delete_keys() {
	for(keys_t::iterator i = keys.begin(); i != keys.end(); ++i) {
		delete *i;
	}
	keys.clear();
	key_map.clear();
}

key_value_key_base::key_value_key_base(key_value_slave* parent, std::string name, bool after_change_cb)
	: parent(parent), name(name), after_change_cb(after_change_cb) {
	memset(&description, 0, sizeof(description));
}

key_value_key_base::~key_value_key_base() {
}

key_value_key_base& key_value_key_base::describe(string desc) {
	description.description = desc;
	return *this;
}

key_value_key_base& key_value_key_base::unit(string unit) {
    description.unit = unit;
    return *this;
}

key_value_key_base& key_value_key_base::default_value(string default_value) {
    description.default_value = default_value;
	return *this;

}

key_value_key_base& key_value_key_base::format(string format) {
    description.format = format;
	return *this;

}

// read key value pairs
void key_value_slave::key_value_read(key_value_transfer_t& t) {
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
void key_value_slave::key_value_write(const key_value_transfer_t& t) {
    if (t.values.size() != t.keys.size())
        throw str_exception_tb("key_value_write: key_value_transfer_t::values.size() is %d"
                " and ::keys.size() is %d!", t.values.size(), t.keys.size());

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
void key_value_slave::key_value_list(key_value_transfer_t& t) {
    t.keys.resize(keys.size());
    t.values.resize(keys.size());

    for (unsigned i = 0; i < t.keys.size(); ++i) {
        const key_value_key_base* key = keys[i];

        t.keys[i] = i;
        t.values[i] = key->name;
    }
}

// list descriptions
void key_value_slave::key_value_list_descriptions(std::vector<key_value_description_t>& data) {
    data.resize(keys.size());

    for (unsigned i = 0; i < keys.size(); ++i) {
        const key_value_key_base* key = keys[i];
        data[i] = key->description;
    }
}

