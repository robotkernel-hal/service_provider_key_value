#include <algorithm>
#include <string>

#include <service_provider/key_value/key_value_helper.h>

using namespace std;
using namespace string_util;

key_value_module::key_value_module(string name) : name(name) {
	
}

void key_value_module::add_key_value_slave(key_value_slave* slave, std::string name, unsigned int slave_id) {
	slave->name = name;
	slave->slave_id = slave_id;
	slave->module = this;
	slaves[slave_id] = slave;	
}
void key_value_module::log(robotkernel::loglevel lvl, const char *format, ...) {
	char buf[1024];	
	va_list args;
	va_start(args, format);
	vsnprintf(buf, 1024, format, args);
	klog(lvl, "[key_value_helper|%s] %s", name.c_str(), buf);
}

void key_value_module::debug(const char *format, ...) {
	return;
	char buf[1024];	
	va_list args;
	va_start(args, format);
	vsnprintf(buf, 1024, format, args);
	klog(robotkernel::verbose, "[key_value_helper|%s] %s", name.c_str(), buf);
}

void key_value_module::handle_key_value_request(void* ptr) {
    /*
	key_value_transfer_t* t = (key_value_transfer_t*)ptr;
	t->error_msg = NULL;

	try {
		if(slaves.find(t->slave_id) == slaves.end())
			throw str_exception_tb("unknown slave_id %d for key_value!", t->slave_id);
		key_value_slave* slave = slaves[t->slave_id];

		switch(t->command) {
		case kvc_list: {
			debug("list %d\n", t->slave_id);
		
			t->keys_len = slave->keys.size();
			t->keys = (uint32_t*)malloc(sizeof(uint32_t) * t->keys_len);
			t->values_len = t->keys_len;
			t->values = (char**)malloc(sizeof(char*) * t->values_len);
		
			for(unsigned int k = 0; k < t->keys_len; k++) {
				key_value_key_base* key = slave->keys[k];
					
				t->keys[k] = k;
				t->values[k] = strdup(key->name.c_str());
			}
			break;
		}
		case kvc_read: {
			debug("read %d\n", t->slave_id);

			if(!t->values)
				throw str_exception_tb("key_value_transfer_t::values is NULL!");
			memset(t->values, 0, t->values_len * sizeof(char*));
		
			for(unsigned int i = 0; i < t->keys_len; ++i) {
				unsigned int k = t->keys[i];
				if(k >= slave->keys.size())
					throw str_exception_tb("unknown key_id %d for slave %s (id %d)!", k, slave->name.c_str(), t->slave_id);			
				key_value_key_base* key = slave->keys[k];

				t->values[i] = strdup(key->get_value().c_str());
			}
			break;
		}
		case kvc_write: {
			debug("write %d\n", t->slave_id);
		
			if(!t->keys)
				throw str_exception_tb("key_value_transfer_t::keys is NULL!");
			if(!t->values)
				throw str_exception_tb("key_value_transfer_t::values is NULL!");
			if(t->values_len != t->keys_len)
				throw str_exception_tb("key_value_transfer_t::values_len is %d and ::keys_len is %d!", t->values_len, t->keys_len);
				
			for(unsigned int i = 0; i < t->keys_len; ++i) {
				unsigned int k = t->keys[i];
				if(k >= slave->keys.size())
					throw str_exception_tb("unknown key_id %d for slave %s (id %d)!", k, slave->name.c_str(), t->slave_id);			
				key_value_key_base* key = slave->keys[k];

				key->set_value(t->values[i]);
			}
			break;
		}
		default:
			throw str_exception_tb("invalid key_value_command: %d", t->command);
		}
	}
	catch(const exception& e) {
		string msg = format_string("%s: key_value error:\n%s", name.c_str(), e.what());
		log(robotkernel::error, "%s\n", msg.c_str());
		t->error_msg = strdup(msg.c_str());
		return;
	}
    */
}

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
std::string key_value_repr<string>(string& value) {
	return repr(value);
}


key_value_slave::key_value_slave(const std::string& owner, const std::string& service_prefix)
    : robotkernel::service_interface(owner, service_prefix) 
{
	module = NULL;
}
key_value_slave::~key_value_slave() {
	do_unregister();
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
}
key_value_key_base::~key_value_key_base() {
}
