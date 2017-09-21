#ifndef KEY_VALUE_HELPER_H
#define KEY_VALUE_HELPER_H

#include <robotkernel/kernel.h>
#include <string>
#include <map>
#include <interface_key_value/module_intf.h>

#include <string_util/string_util.h>

class key_value_slave;

class key_value_module {
public:
    typedef std::map<unsigned int, key_value_slave*> slaves_t;
    slaves_t slaves;

    std::string name;
	
    key_value_module(std::string name);
	
    void add_key_value_slave(key_value_slave* slave, std::string name, unsigned int slave_id);
    int handle_key_value_request(void* ptr);

    void log(robotkernel::loglevel lvl, const char *format, ...);
    void debug(const char *format, ...);
};


class key_value_key_base {	
public:
    key_value_slave* parent;
    std::string name;
    bool after_change_cb;

    key_value_description_t description; // if contents are set, they will be free()'d from this detor!
	
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

class key_value_slave {
public:
    typedef std::vector<key_value_key_base*> keys_t;
    keys_t keys;
    typedef std::map<std::string, key_value_key_base*> key_map_t;
    key_map_t key_map;

    key_value_module* module;
    std::string name;
    unsigned int slave_id;

    robotkernel::kernel::interface_id_t interface_id;

    key_value_slave();
    ~key_value_slave();
	
    void do_unregister() {
        if(interface_id)
            robotkernel::kernel::unregister_interface_cb(interface_id);
        interface_id = NULL;
    }
    void do_register(const robotkernel::loglevel& ll) {
        if(!module)
            throw str_exception_tb("module is NULL - make sure to call key_value_module::add_key_value_slave() before key_value_slave::do_register()!");
        do_unregister();

        YAML::Node node;
        node["mod_name"] = module->name;
        node["dev_name"] = name;
        node["slave_id"] = slave_id;
        node["loglevel"] = (std::string)ll;
        interface_id = robotkernel::kernel::register_interface_cb(
            "libinterface_key_value.so", node);
    }
	
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
};

#endif // KEY_VALUE_HELPER_H
