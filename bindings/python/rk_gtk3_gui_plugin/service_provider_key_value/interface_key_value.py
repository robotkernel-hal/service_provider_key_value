#!/usr/bin/python

import os
import sys
import pprint
import math
import traceback
import yaml

import gi
gi.require_version('Gtk', '3.0')
gi.require_version('GLib', '2.0')
from gi.repository import Gtk
from gi.repository import GObject

import helpers

import logging
logger = logging.getLogger(__name__)

class key_value_key():
    def __init__(self, key, name):
        self.key = key
        self.name = name
        self.value = None
        self.desc = None
        self.unit = None
        self.default_value = None

class key_value_device(helpers.svc_wrapper):
    def __init__(self, service_prefix, app, widget, modname, devname):
        logger.debug("initializing key-value client for %s.%s.%s.key_value"
                     % (service_prefix, modname, devname))
        helpers.svc_wrapper.__init__(self, app.clnt,
                "%s.%s.%s.key_value" % (service_prefix, modname, devname))

        self.modname = modname
        self.devname = devname
        self.widget = widget

        self.keys = {}

        self.key_names = []
        self.key_descriptions = []
        self.key_units = []
        self.svc_call_pending = False

        self.current_display_options = {}
        self.current_display_options["expanded"] = set() # [prefix-name] that are expanded

        self.list()
        self.list_descriptions()

    def list(self):
        self.svc_list.call()
        logger.debug("received list: %r" % self.svc_list.resp)

        ret = self.svc_list.resp

        self.key_names = []
        self.n_hex_digits = 2

        for key, name in zip(ret.keys, ret.names):
            real_name = ""
            if hasattr(name, "data"):
                real_name = name.data
            else:
                real_name = name.string

            self.keys[int(key)] = act_key = key_value_key(int(key), real_name)

            self.key_names.append((int(key), real_name))
            key_hex = "%x" % key
            if len(key_hex) > self.n_hex_digits:
                self.n_hex_digits = len(key_hex)
        self.n_hex_digits = int(math.ceil(self.n_hex_digits / 2.)) * 2

    def list_descriptions(self):
        self.svc_list_descriptions.call()
        logger.debug("received descriptions: %r" % self.svc_list_descriptions.resp)
        ret = self.svc_list_descriptions.resp

        self.key_descriptions = []

        if not ret.description:
            return # assume no descriptions

        for field in ret.description:
            if hasattr(field, "data"):
                self.key_descriptions.append(field.data)
            else:
                self.key_descriptions.append(field.string)

        for field in ret.unit:
            if hasattr(field, "data"):
                self.key_units.append(field.data)
            else:
                self.key_units.append(field.string)

    def read(self, keys):
        self.svc_read.req.keys = keys
        self.svc_read.call()
        logger.debug("received for keys %r values = r" % (keys, self.svc_read.resp.values))

        values = self.svc_read.resp.values
        return values

    def write(self, key, value):
        self.svc_write.req.keys = [key]
        if hasattr(self.svc_write.req, "new_vector/string_packet"):
            p = getattr(self.svc_write.req, "new_vector/string_packet")()
            p.data = repr(value)
            p.data_len = len(repr(value))
        else:
            p = getattr(self.svc_write.req, "new_string_packet")()
            p.string = repr(value)
            p.string_len = len(repr(value))
        self.svc_write.req.values = [p]
        logger.debug("for key = %r, writing values = %r" % (key, value))
        self.svc_write.call()

class interface_key_value(helpers.service_provider_view, helpers.builder_base):
    def __init__(self, parent, container):
        fn = os.path.join(os.path.dirname(__file__), 'interface_key_value.ui')
        helpers.builder_base.__init__(self, fn, 'key_value_box', 'unit_conv_popup', 'kv_popup')
        helpers.service_provider_view.__init__(self, parent.app, parent, self.key_value_box, 'list')

        container.pack_start(self.key_value_box, True, True, 0)

        self.devices = {}
        self.current_device = None

        self.active_conversions = {}
        self.tv = tv = self.key_value_tv

        # key_value treeview
        m = self.model = Gtk.TreeStore(
            GObject.TYPE_STRING, # key display
            GObject.TYPE_STRING, # name
            GObject.TYPE_STRING, # value / repr
            GObject.TYPE_PYOBJECT, # real key

            GObject.TYPE_STRING, # unit
            GObject.TYPE_STRING, # description
        )
        #col = tv.insert_column_with_attributes(-1, "Key", Gtk.CellRendererText(), text=0)
        renderer_text = Gtk.CellRendererText()
        col1 = Gtk.TreeViewColumn("Key", renderer_text, text=0)
        treeview.append_column(col1)
        
        col1.set_property("resizable", True)

        #col = tv.insert_column_with_attributes(-1, "Name", Gtk.CellRendererText(), text=1)
        renderer_text = Gtk.CellRendererText()
        col2 = Gtk.TreeViewColumn("Name", renderer_text, text=1)
        treeview.append_column(col2)
        col2.set_property("resizable", True)

        cr = Gtk.CellRendererText()
        cr.set_property("editable", True)
        cr.connect("edited", self.on_edited)
        n = tv.insert_column_with_data_func(-1, "Value", cr, self.on_key_value_data)
        col3 = tv.get_column(n - 1)        

        col3.set_property("resizable", True)
        tv.set_tooltip_column(0)

        cr = Gtk.CellRendererText()
        cr.set_property("editable", False)
        n = tv.insert_column_with_data_func(-1, "Unit", cr, self.on_key_value_unit_data)
        col4 = tv.get_column(n - 1)
        col4.set_property("resizable", True)

        #col = self.description_col = tv.insert_column_with_attributes(-1, "Description", Gtk.CellRendererText(), text=5)
        renderer_text = Gtk.CellRendererText()
        col5 = self.description_col = Gtk.TreeViewColumn("Description", renderer_text, text=5)
        treeview.append_column(col5)
        col5.set_property("resizable", True)

        tv.set_model(m)
        tv.connect("row-activated", self.on_key_value_tv_row_activated)
        tv.set_search_equal_func(func=self.search_func)

        self.kv_refresh_btn.connect("clicked", self.on_refresh)
        #self.all_format_btn.add_events(Gdk.EventMask.BUTTON_PRESS_MASK)
        self.all_format_btn.connect("button_press_event", self.all_format_btn_press)
        self.unit_conv_btn.connect("button_press_event", self.unit_conv_btn_press)

        self._display_formats = ["decimal", "hex_02", "hex_04", "hex_08", "repr"]
        self._display_byte_order = ["native", "16", "32"]
        self.default_display_options = "repr", "native"
        self._display_options_fn = os.path.expanduser("~/.robotkernel/gui/key_value_display_options")
        self._format_strings = {}
        self._load_display_options()
        self.key_names = {}
        self.key_descriptions = {}

        # connect these signals after loading stored conversions
        for A, B, C in self.unit_conversions:
            btn = "uc_%s_%s" % (A, B)
            btn = getattr(self, btn)
            conv = self.active_conversions.get(A)
            if conv:
                btn.set_active(conv[0] == B)
            btn.connect("toggled", self.on_uc_toggled)

        self._read_queue = {}
        self._read_queue_id = None

    def show(self, modname, devname):
        modidx = (modname, devname)
        if modidx not in self.devices:
            self.devices[modidx] = key_value_device(self.service_prefix, self.app, self.tv, modname, devname)

        #self.current_display_options = {}
        #self.current_display_options["expanded"] = set() # [prefix-name] that are expanded
        self.current_device = self.devices[modidx]
        self.current_device.list()
        helpers.service_provider_view.show(self)

        self._show_names()
        return
        #self.show_indices()

        helpers.service_provider_view.show(self)

        if self.current_device != (modname, devname):
            self.current_device = modname, devname
            if modname not in self.display_options:
                self.display_options[modname] = {}
            if devname not in self.display_options[modname]:
                self.display_options[modname][devname] = {}
            self.current_display_options = self.display_options[modname][devname]
            if "expanded" not in self.current_display_options:
                self.current_display_options["expanded"] = set() # [prefix-name] that are expanded
            self._show_names()

    def _find_prefix_name(self, iter):
        m = self.model
        name = []
        while iter:
            name.insert(0, m[iter][1])
            iter = m.iter_parent(iter)
        return ".".join(name)

    def on_all_format_btn_pressed(self, widget, ev):
        logger.warning("on_all_format_btn_pressed(): Handler not implemented, skipping")
        return True

    def on_key_value_tv_row_expanded(self, tv, iter, path):
        name = self._find_prefix_name(iter)
        self.current_device.current_display_options["expanded"].add(name)
        self._schedule_save_display_options()

    def on_key_value_tv_row_collapsed(self, tv, iter, path):
        name = self._find_prefix_name(iter)
        self.current_device.current_display_options["expanded"].remove(name)
        self._schedule_save_display_options()

    def _show_names(self):
        self.current_device.list()

        dev = self.current_device

        key_format = "0x%%0%dx" % dev.n_hex_digits
        m = self.model
        m.clear()

        def parent_iter(parent):
            if not parent:
                return None
            return parent[-1]
        def is_descendant_of(parent, desc):
            for p, d in zip(parent, desc):
                if p != d:
                    return False
            return True
        def is_child_of(parent, child):
            if not is_descendant_of(parent, child):
                return False
            return len(child) == len(parent) + 1
        prefix = []
        parent = []
        descs = dev.key_descriptions
        for key_name, desc, unit in zip(dev.key_names, descs, dev.key_units):
            key, name = key_name
            #desc = descs.get(key, {})
            to_expand = []
            if "." not in name:
                parent = []
                prefix = []
                display_name = name
            else:
                parts = name.split(".")
                while not is_descendant_of(prefix, parts):
                    del prefix[-1]
                    del parent[-1]
                while not is_child_of(prefix, parts):
                    display_name = parts[len(prefix)]
                    new_parent = m.append(parent_iter(parent), ("", display_name, None, None, None, None))
                    prefix_name = ".".join(parts[:len(prefix)+1])
                    if prefix_name in self.current_device.current_display_options["expanded"]:
                        path = m.get_path(new_parent)
                        to_expand.append(path)
                    parent.append(new_parent)
                    prefix.append(display_name)
                display_name = parts[-1]
            new_parent = m.append(parent_iter(parent), (key_format % key, display_name, None, key, unit, desc))
            for p in to_expand:
                self.tv.expand_row(p, False)

            if not parent:
                parent = [new_parent]
                prefix = [display_name]

    def _update_key_names(self):
        method_name = "%s_%s_list" % self.current_device
        if not hasattr(self, method_name):
            self.wrap_service(
                "%s.%s.key_value.list" % self.current_device,
                "gen/%s.%s.%s.key_value.list" % (self.parent.name, self.current_device[0], self.current_device[1]),
                method_name=method_name, call_method="call_gobject")
        ret = getattr(self, method_name)()
        key_names = self.key_names[self.current_device] = []
        self.n_hex_digits = 2
        for key, name in zip(ret["keys"], ret["names"]):
            key_names.append(
                (int(key), name["data"]))
            #print "got key %x: %s" % (key, name.value)
            key_hex = "%x" % key
            if len(key_hex) > self.n_hex_digits:
                self.n_hex_digits = len(key_hex)
        self.n_hex_digits = int(math.ceil(self.n_hex_digits / 2.)) * 2
        #print "have %d key_names" % len(key_names)

        self._update_key_descriptions()

    def _update_key_descriptions(self):
        method_name = "%s_%s_list_descriptions" % self.current_device
        if not hasattr(self, method_name):
            self.wrap_service(
                "%s.%s.key_value.list_descriptions" % self.current_device,
                "gen/%s.%s.%s.key_value.list_descriptions" % (self.parent.name, self.current_device[0], self.current_device[1]),
                method_name=method_name, call_method="call_gobject")
        key_descriptions = self.key_descriptions[self.current_device] = {}
        #print 'key_descriptions: ', key_descriptions
        try:
            ret = getattr(self, method_name)()
            #print 'ret ', ret
        except:
            return # assume no descriptions
        for key, name in self.key_names[self.current_device]:
            kdesc = key_descriptions[key] = {}

            for field in ret:
                if '_len' in field:
                    continue
                #print 'field ', field, ', ret[field] ', ret[field]
                value = ret[field][key]
                if type(value) == dict and "data" in value:
                    kdesc[field] = value["data"]
                else:
                    kdesc[field] = value

    def on_key_value_unit_data(self, col, cr, m, iter):
        row = m[iter]
        key = row[3]
        unit = row[4]
        format, byte_order = self.current_device.current_display_options.get(key, self.default_display_options)
        if unit is None:
            unit = ""
        elif format != "repr" and unit in self.active_conversions:
            unit = self.active_conversions[unit][0]
        cr.set_property("text", unit)
        return True

    def on_key_value_data(self, col, cr, m, iter):
        try:
            self._on_key_value_data(col, cr, m, iter)
        except:
            row = m[iter]
            display = row[2]
            print "exception in data-handler:\n%s\nfor row %r" % (traceback.format_exc(), tuple(row))
            cr.set_property("text", str(display))
        return True

    def _on_key_value_data(self, col, cr, m, iter):
        row = m[iter]
        display = row[2]
        key = row[3]
        unit = row[4]
        if key is None:
            cr.set_property("text", "")
            return
        if display is None:
            ret = self.tv.get_visible_range()
            if ret is not None:
                start_path, end_path = ret
                path = m.get_path(iter)
                if path >= start_path and path <= end_path:
                    self._queue_read(key, path)
            display = ""
        elif display != "":
            format, byte_order = self.current_device.current_display_options.get(key, self.default_display_options)
            if format == "repr":
                display = display # no unit conversion in repr mode
            else:
                # optionally iterate over sequences
                conv = self.active_conversions.get(unit)
                if conv is None:
                    conv = lambda a: a
                else:
                    B, C = conv
                    conv = lambda a: a * C
                format_string = self._format_strings.get(format)
                if format_string is None:
                    if format.startswith("hex"):
                        f = "0x%" + format.split("_", 1)[1] + "x"
                    else:
                        f = "%s"
                    format_string = self._format_strings[format] = f
                if display == "nan":
                    value = float("nan")
                else:
                    value = eval(display)
                if type(value) == str:
                    display = repr(value)
                else:
                    if value in (None, ):
                        display = str(value)
                    elif type(value) in (list, tuple):
                        display = " ".join([format_string % conv(v) for v in value])
                    else:
                        try:
                            display = format_string % conv(value)
                        except:
                            print (format_string, value)
                            raise
            # todo byte_order
        cr.set_property("text", display)
        return True

    def _queue_read(self, key, key_iter, timeout=250):
        self._read_queue[key] = key_iter
        if self._read_queue_id is None:
            self._read_queue_id = GObject.timeout_add(timeout, self._process_read_queue)

    def _process_read_queue(self):
        to_read = self._read_queue
        self._read_queue = {}

        if to_read:
            try:
                self._update_key_values(to_read)
            except:
                print traceback.format_exc()

        if not self._read_queue:
            GObject.source_remove(self._read_queue_id)
            self._read_queue_id = None
            return False
        return True

    def _update_key_values(self, to_read):
        keys = list(to_read.keys())
        values = self.current_device.read(keys)
        for key, value in zip(keys, values):
            path = to_read[key]
            iter = self.model.get_iter(path)
            if hasattr(value, "data"):
                self.model[iter][2] = value.data
            else:
                self.model[iter][2] = value.string
            self.model.row_changed(path, iter)

    def search_func(self, m, col, key, iter):
        if key.lower() in m[iter][1].lower():
            return False
        if m[iter][0].startswith(key):
            return False
        if ("0x%x" % m[iter][3]).startswith(key):
            return False
        try:
            key_int = int(key)
            if key_int == m[iter][3]:
                return False
        except:
            pass
        if ("%x" % m[iter][3]).startswith(key):
            return False
        return True

    def on_key_value_tv_row_activated(self, tv, path, col):
        self.model[path][2] = ""
        self.model.row_changed(path, self.model.get_iter(path))
        self._queue_read(self.model[path][3], path, 50)
        return True

    def on_refresh(self, btn):
        if self.current_device is None:
            return
        #if self.current_device in self.key_names:
        #    del self.key_names[self.current_device]
        self._show_names()

    def on_edited(self, cr, path, value):
        row = self.model[path]
        key = row[3]
        unit = row[4]
        format, byte_order = self.current_device.current_display_options.get(key, self.default_display_options)

        conv = None
        if format != "repr":
            # optionally iterate over sequences
            conv = self.active_conversions.get(unit)
            if conv is not None:
                B, C = conv
                conv = lambda b: b / C

        try:
            if format[0].startswith("hex"):
                if not format[1]:
                    value = eval("0x%s" % value)
                else:
                    value = eval(value)
            else:
                value = eval(value)
            if conv:
                if type(value) in (list, tuple):
                    value = map(conv, value)
                else:
                    value = conv(value)
        except:
            print "warning: invalid input format: %r" % value
            pass

        row[2] = None
        self.model.row_changed(path, self.model.get_iter(path))
        # now write, then read!
        print "write 0x%x to %r" % (key, value)
        #try:
        self._write(key, value)
        #except:
        #    pass
        self._queue_read(self.model[path][3], path, 50)
        return True

    def _write(self, key, value):
        self.current_device.write(key, value)

    def show_exception(self, text=None, reraise=True):
        traceback.print_exc()

        dlg = Gtk.MessageDialog(
            self.app.window,
            Gtk.DialogFlags.DESTROY_WITH_PARENT,
            Gtk.MessageType.ERROR,
            buttons=Gtk.ButtonsType.OK)

        if text is None:
            stack = traceback.format_exc().split("\n")
            stack = "\n".join(stack[2:])
            stack = stack.replace("<", "&lt;").replace(">", "&gt;")
            text = 'exception in\n<span weight="bold" font_family="monospace">%s</span>' % (stack, )

        dlg.label.set_line_wrap(False)
        dlg.set_markup(text)
        dlg.connect("delete-event", lambda w: w.destroy())
        dlg.connect("response", lambda w, rid: w.destroy())
        dlg.show()
        raise

    def on_key_value_tv_button_press_event(self, tv, ev):
        if ev.button != 3:
            return False
        ret = tv.get_path_at_pos(int(ev.x), int(ev.y))
        if ret is None:
            print "no path"
            return False
        path, tvc, x, y = ret
        tv.set_cursor(path, tvc)
        key = self.model[path][3]
        format, byte_order = self.current_device.current_display_options.get(key, self.default_display_options)
        def set_active_group_item(prefix, value, items):
            for item in items:
                i = getattr(self, "%s_%s" % (prefix, item))
                i.set_active(item == value)
        if format[0].startswith("hex"):
            format = format[0]
        set_active_group_item("format", format, self._display_formats)
        set_active_group_item("byteorder", byte_order, self._display_byte_order)
        self.is_format_all = False
        self.kv_popup.popup(None, None, None, ev.button, ev.time)
        return True

    def kv_popup_done(self, *args):
        def get_active_group_item(prefix, items):
            for item in items:
                i = getattr(self, "%s_%s" % (prefix, item))
                if i.get_active():
                    return item
        format = get_active_group_item("format", self._display_formats)
        byte_order = get_active_group_item("byteorder", self._display_byte_order)
        if self.is_format_all:
            m = self.model
            def foreach(iter):
                while iter:
                    row = self.model[iter]
                    key = row[3]
                    if self.current_device.current_display_options.get(key, self.default_display_options) != (format, byte_order):
                        self.current_device.current_display_options[key] = format, byte_order
                        self._schedule_save_display_options()
                        path = self.model.get_path(iter)
                        self.model.row_changed(path, iter)
                    if self.model.iter_has_child(iter):
                        foreach(self.model.iter_children(iter))
                    iter = self.model.iter_next(iter)
            foreach(self.model.get_iter_first())
        else:
            path, col = self.tv.get_cursor()
            key = self.model[path][3]
            #print (key, self.current_device, format, byte_order)
            if self.current_device.current_display_options.get(key, self.default_display_options) != (format, byte_order):
                self.current_device.current_display_options[key] = format, byte_order
                self._schedule_save_display_options()
                self.model.row_changed(path, self.model.get_iter(path))

    def _load_display_options(self):
        if not os.path.isfile(self._display_options_fn):
            self.display_options = {}
            return
        fp = file(self._display_options_fn, "rb")
        self.yaml_loader = yaml.loader.Loader
        def tuple_constructor(loader, node):
            return tuple(loader.construct_sequence(node))
        self.yaml_loader.add_constructor(u'tag:yaml.org,2002:seq', tuple_constructor)
        self.display_options = yaml.load(fp, self.yaml_loader)
        if type(self.display_options) != dict:
            self.display_options = {}

        # restore active conversions
        ac = self.display_options.get("active_conversions", [])
        self.active_conversions = {}
        for A, B in ac:
            if A.endswith("/s"):
                continue
            for A_, B_, C_ in self.unit_conversions:
                if A_ == A and B_ == B:
                    self.active_conversions[A_] = B_, C_
                    self.active_conversions[A_+"/s"] = B_+"/s", C_
                    break
        fp.close()

    def _schedule_save_display_options(self):
        if hasattr(self, "_schedule_save_display_options_id"):
            GObject.source_remove(self._schedule_save_display_options_id)
            del self._schedule_save_display_options_id
        def save_options():
            del self._schedule_save_display_options_id
            dn = os.path.dirname(self._display_options_fn)
            if not os.path.isdir(dn):
                os.makedirs(dn)

            # put active conversions
            ac = self.display_options["active_conversions"] = []
            for A, (B, C) in self.active_conversions.iteritems():
                ac.append((A, B))

            fp = file(self._display_options_fn, "wb")
            self.yaml_dumper = yaml.dumper.Dumper
            def tuple_representer(dumper, data):
                return dumper.represent_list(data)
            self.yaml_dumper.add_representer(tuple, tuple_representer)
            try:
                yaml.dump(self.display_options, fp, self.yaml_dumper)
            except:
                pprint.pprint(self.display_options)
                raise
            fp.close()
            return False

        self._schedule_save_display_options_id = GObject.timeout_add(1000, save_options)

    def all_format_btn_press(self, btn, ev):
        self.is_format_all = True
        self.kv_popup.popup(None, None, None, ev.button, ev.time)
        return False

    def unit_conv_btn_press(self, btn, ev):
        self.unit_conv_popup.popup(None, None, None, ev.button, ev.time)
        return False

    unit_conversions = (
        # convert A to B
        #
        # B = A * C
        # A = B / C
        # A      B     C
        ("rad", "deg", 180 / math.pi),
        ("deg", "rad", math.pi / 180),
        ("mm",  "m",   1e-3),
        ("m",   "mm",  1e3)
    )
    def on_uc_toggled(self, *args):
        # get state from buttons apply and save!
        self.old_conversions = dict(self.active_conversions)
        self.active_conversions = {}
        for A, B, C in self.unit_conversions:
            btn = "uc_%s_%s" % (A, B)
            if getattr(self, btn).get_active():
                self.active_conversions[A] = B, C
                self.active_conversions[A+"/s"] = B+"/s", C

        self._reconvert()
        self._schedule_save_display_options()

    def _reconvert(self):
        m = self.model
        def redisplay(iter):
            while iter:
                row = m[iter]
                unit = row[4]
                if unit in self.active_conversions or unit in self.old_conversions:
                    m.row_changed(m.get_path(iter), iter)
                if m.iter_has_child(iter):
                    redisplay(m.iter_children(iter))
                iter = m.iter_next(iter)
        redisplay(m.get_iter_first())

