ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libservice_provider_key_value.la

BUILT_SOURCES = $(top_builddir)/src/service_definitions.h
CLEANFILES = $(top_builddir)/src/service_definitions.h

extraincludedir = $(includedir)/service_provider_key_value
extrainclude_HEADERS = $(top_srcdir)/include/service_provider_key_value/base.h \
					   $(top_srcdir)/include/service_provider_key_value/key_value_helper.h

libservice_provider_key_value_la_SOURCES  = provider.cpp
libservice_provider_key_value_la_CXXFLAGS = -fno-strict-aliasing -Wall \
									 -I$(top_srcdir)/include -I$(top_builddir)/src  \
									 @ROBOTKERNEL_CFLAGS@ 
libservice_provider_key_value_la_LDFLAGS  = -Wall -release $(PACKAGE_VERSION)

VPATH = $(srcdir):$(top_srcdir)/share/service_definitions

SERVICE_DEFINITIONS = service_provider_key_value/read \
					  service_provider_key_value/write \
					  service_provider_key_value/list \
					  service_provider_key_value/list_descriptions

$(top_builddir)/src/service_definitions.h: Makefile $(SERVICE_DEFINITIONS)
	@echo creating $@
	service_generate --indir $(top_srcdir)/share/service_definitions --out $@ $(SERVICE_DEFINITIONS)
	touch $@


