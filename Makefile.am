ACLOCAL_AMFLAGS = -I m4
SUBDIRS=src/main/cpp

pkgconfig_DATA = service_provider_key_value.pc

install-exec-hook:
	mkdir -p $(DESTDIR)$(prefix)/bindings/python
	-cp -rauf $(srcdir)/bindings/python/* $(DESTDIR)$(prefix)/bindings/python

