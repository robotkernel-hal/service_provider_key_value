//! robotkernel interface key value requests service definitions
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

#ifndef SERVICE_PROVIDER__KEY_VALUE__PROVIDER__MDS__H
#define SERVICE_PROVIDER__KEY_VALUE__PROVIDER__MDS__H

#include <string>

namespace service_provider {

namespace key_value {

extern const std::string service_definition_read;
extern const std::string service_definition_write;
extern const std::string service_definition_list;
extern const std::string service_definition_list_descriptions;

}; // namespace key_value

}; // namespace service_provider

#endif // SERVICE_PROVIDER__KEY_VALUE__PROVIDER__MDS__H


