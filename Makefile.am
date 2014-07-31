ACLOCAL_AMFLAGS = -I m4 

libdir = $(prefix)/lib
lib_LTLIBRARIES = libinterface_key_value.la
BUILT_SOURCES = ln_messages.h
CLEANFILES = ln_messages.h

libinterface_key_value_la_SOURCES  = interface_key_value.cpp
libinterface_key_value_la_CXXFLAGS = -fno-strict-aliasing -Wall -I$(top_srcdir)/src \
										   -I$(top_builddir) @YAMLCPP_CFLAGS@ @LN_CFLAGS@ 
libinterface_key_value_la_LDFLAGS  = -Wall @PTHREAD_LIBS@ @RT_LIBS@ -release $(PACKAGE_VERSION)

VPATH = $(srcdir):$(srcdir)/message_definitions

LN_MESSAGES = \
			  robotkernel/key_value/read \
			  robotkernel/key_value/write \
			  robotkernel/key_value/list

ln_messages.h: Makefile $(LN_MESSAGES)
	@echo creating ln_messages.h
	@LN_GENERATE@ -o $@ $(LN_MESSAGES)
	touch $@

install-exec-hook:
	mkdir -p $(prefix)/share/message_definitions
	cp -r $(srcdir)/message_definitions/* $(prefix)/share/message_definitions/
