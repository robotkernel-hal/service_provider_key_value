#from numpy import *
import links_and_nodes as ln

class key_value_wrapper(ln.services_wrapper):
    def __init__(self, clnt, kvname):
        ln.services_wrapper.__init__(self, clnt, "%s.key_value" % kvname)
        
        self.default_call_method = "call_gobject"
        
        self._kv = None
        self._keys = None        
        self.wrap_service("list", 'robotkernel/key_value/list')
        self.wrap_service("write", 'robotkernel/key_value/write')
        self.wrap_service("read", 'robotkernel/key_value/read')
        self.service_prefix = kvname
        
    def get_kvlist(self):
        if self._kv is not None:
            return self._kv
        kv = self.list()
        self._kv = {}
        self._keys = []
        for i, name in enumerate(kv["names"]):
            self._kv[name["value"]] = kv["keys"][i]
            self._keys.append(name["value"])
        return self._kv
        
    def write_key(self, name, value):
        self.get_kvlist()
        key = self._kv[name]        
        vals = [ self.write.svc.req.new_string_packet() ]
        vals[0].value = repr(value)
        self.write(keys=[key], values=vals)
        
    def read_key(self, which):
        self.get_kvlist()
        key = self._kv[which]
        ret = self.read(keys=[key])
        return eval(ret[0]["value"])

    def dump_all_keys(self):
        self.get_kvlist()
        to_read = []
        for key in self._keys:
            to_read.append(self._kv[key])
        ret = self.read(keys=to_read)
        for key, value in zip(self._keys, ret):
            v = eval(value["value"])
            if "_ptr" in key or "_flags" in key:
                v = hex(v)
            print "%-30.30s: %s" % (key, v)
