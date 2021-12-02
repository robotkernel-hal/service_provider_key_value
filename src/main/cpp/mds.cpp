#include "mds.h"
#include <list>

const std::string service_provider::key_value::service_definition_read = 
"name: service_provider/key_value/read\n"
"request:\n"
"- vector/uint32_t: keys\n"
"response:\n"
"- vector/string: values\n"
"- string: error_message\n";

const std::string service_provider::key_value::service_definition_write = 
"name: service_provider/key_value/write\n"
"request:\n"
"- vector/uint32_t: keys\n"
"- vector/string: values\n"
"response:\n"
"- string: error_message\n";

const std::string service_provider::key_value::service_definition_list =
"name: service_provider/key_value/list\n"
"response:\n"
"- vector/uint32_t: keys\n"
"- vector/string: names\n"
"- string: error_message\n";

const std::string service_provider::key_value::service_definition_list_descriptions =
"name: service_provider/key_value/list_descriptions\n"
"response:\n"
"- vector/string: description\n"
"- vector/string: unit\n"
"- vector/string: default_value\n"
"- vector/string: format\n"
"- vector/uint8_t: read_only\n";

typedef void (*get_sd_t)(std::list<std::string>& sd_list);
extern "C" void get_sd(std::list<std::string>& sd_list) {
    sd_list.push_back(service_provider::key_value::service_definition_read);
    sd_list.push_back(service_provider::key_value::service_definition_write);
    sd_list.push_back(service_provider::key_value::service_definition_list);
    sd_list.push_back(service_provider::key_value::service_definition_list_descriptions);
}

