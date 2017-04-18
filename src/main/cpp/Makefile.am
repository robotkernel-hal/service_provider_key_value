
libdir = $(prefix)/lib
lib_LTLIBRARIES = libservice_provider_key_value.la libkey_value_helper.la

extraincludedir = $(includedir)/service_provider/key_value
extrainclude_HEADERS = include/service_provider/key_value/base.h \
					   include/service_provider/key_value/key_value_helper.h

libservice_provider_key_value_la_SOURCES  = provider.cpp
libservice_provider_key_value_la_CXXFLAGS = -fno-strict-aliasing -Wall \
									 -I$(srcdir)/include -I$(top_builddir)  \
									 @ROBOTKERNEL_CFLAGS@ 					\
									 @LIBSTRING_UTIL_CFLAGS@ 				\
									 @YAML_CPP_CFLAGS@
libservice_provider_key_value_la_LDFLAGS  = -Wall -release $(PACKAGE_VERSION)

libkey_value_helper_la_SOURCES  = key_value_helper.cpp
libkey_value_helper_la_CXXFLAGS = -fno-strict-aliasing -Wall  \
								  -I$(srcdir)/include -I$(top_builddir)   \
								  @ROBOTKERNEL_CFLAGS@                    \
								  @LIBSTRING_UTIL_CFLAGS@                 \
								  @YAML_CPP_CFLAGS@
libkey_value_helper_la_LDFLAGS  = -Wall -release $(PACKAGE_VERSION)

