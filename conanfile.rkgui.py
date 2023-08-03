import os
from conans import ConanFile, AutoToolsBuildEnvironment

class service_provider_key_value_python_rkgui(ConanFile):
    name = "service_provider_key_value_rkgui"
    description = "python rkgui binding to service_provider_key_value."
    author = "Robert Burger <robert.burgert@dlr.de>"
    license = "GPLv3"
    
    url = f"https://rmc-github.robotic.dlr.de/robotkernel/service_provider_key_value"
    settings = "os"
    pure_python_folder = os.path.join("bindings","python")
    exports_sources = os.path.join(pure_python_folder, "*")
    
    def requirements(self):
        self.requires(f"service_provider_key_value_ln_msgdef/{self.version}@{self.user}/{self.channel}")

    def package(self):
        self.copy(os.path.join(self.pure_python_folder, "*"))

    def package_info(self):
        self.env_info.PYTHONPATH.append(os.path.join(self.package_folder, os.path.dirname(self.pure_python_folder)))
        self.env_info.PYTHONPATH.append(os.path.join(self.package_folder, self.pure_python_folder))
    
