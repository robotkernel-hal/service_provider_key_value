ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libservice_provider_key_value.la

BUILT_SOURCES = $(top_builddir)/src/main/cpp/service_definitions.h
CLEANFILES = $(top_builddir)/src/main/cpp/service_definitions.h

extraincludedir = $(includedir)/service_provider/key_value
extrainclude_HEADERS = include/service_provider/key_value/base.h \
					   include/service_provider/key_value/key_value_helper.h

libservice_provider_key_value_la_SOURCES  = provider.cpp
libservice_provider_key_value_la_CXXFLAGS = -fno-strict-aliasing -Wall \
									 -I$(srcdir)/include -I$(top_builddir)  \
									 @ROBOTKERNEL_CFLAGS@ 
libservice_provider_key_value_la_LDFLAGS  = -Wall -release $(PACKAGE_VERSION)

VPATH = $(srcdir):$(top_srcdir)/share/service_definitions

SERVICE_DEFINITIONS = robotkernel/service_provider_key_value/read \
					  robotkernel/service_provider_key_value/write \
					  robotkernel/service_provider_key_value/list \
					  robotkernel/service_provider_key_value/list_descriptions

$(top_builddir)/src/main/cpp/service_definitions.h: Makefile $(SERVICE_DEFINITIONS)
	install -d $(top_builddir)/src/main/cpp/include/robotkernel
	@echo creating $@
	service_generate --indir $(top_srcdir)/share/service_definitions --out $@ $(SERVICE_DEFINITIONS)
	touch $@


