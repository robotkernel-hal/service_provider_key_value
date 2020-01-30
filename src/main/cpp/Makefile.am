libdir = $(prefix)/lib
lib_LTLIBRARIES = libservice_provider_key_value.la

<<<<<<< HEAD
extraincludedir = $(includedir)/service_provider/key_value
extrainclude_HEADERS = include/service_provider/key_value/base.h \
					   include/service_provider/key_value/key_value_helper.h

libservice_provider_key_value_la_SOURCES  = provider.cpp
libservice_provider_key_value_la_CXXFLAGS = -fno-strict-aliasing -Wall \
									 -I$(srcdir)/include -I$(top_builddir)  \
									 @ROBOTKERNEL_CFLAGS@ 
libservice_provider_key_value_la_LDFLAGS  = -Wall -release $(PACKAGE_VERSION)

=======
libinterface_key_value_la_SOURCES  = interface_key_value.cpp
libinterface_key_value_la_CXXFLAGS = -fno-strict-aliasing -Wall -I$(srcdir)/include -I$(top_builddir) @ROBOTKERNEL_CFLAGS@ @LIBSTRING_UTIL_CFLAGS@ @YAML_CPP_CFLAGS@ \
                                     -DLN_MD_PREFIX_NS=ln_msg
libinterface_key_value_la_LDFLAGS  = -Wall -release $(PACKAGE_VERSION)

libkey_value_helper_la_SOURCES  = key_value_helper.cpp
libkey_value_helper_la_CXXFLAGS = -fno-strict-aliasing -Wall -I$(srcdir)/include -I$(top_builddir) @ROBOTKERNEL_CFLAGS@ @LIBSTRING_UTIL_CFLAGS@ @YAML_CPP_CFLAGS@ \
                                  -DLN_MD_PREFIX_NS=ln_msg
libkey_value_helper_la_LDFLAGS  = -Wall -release $(PACKAGE_VERSION)

include_headersdir = $(includedir)/interface_key_value
include_headers_HEADERS = include/interface_key_value/module_intf.h include/interface_key_value/key_value_helper.h

VPATH = $(srcdir):$(top_srcdir)/share/ln/message_definitions

LN_MESSAGES = \
        robotkernel/key_value/read \
	robotkernel/key_value/write \
	robotkernel/key_value/list \
	robotkernel/key_value/list_descriptions

ln_messages.h: Makefile $(LN_MESSAGES)
	@echo creating ln_messages.h
	@LN_GENERATE@ -o $@ $(LN_MESSAGES)
	touch $@

install-exec-hook:
	mkdir -p $(prefix)/share
	-cp -rf $(top_srcdir)/share/* $(DESTDIR)$(prefix)/share/
>>>>>>> robotkernel-4.2
