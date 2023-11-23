import os
from conan import ConanFile, conan_version
from conan.tools.scm import Version


class lnrk_interface_python(ConanFile):
    name = "service_provider_key_value_rkgui"
    description = "python rkgui binding to service_provider_key_value."
    author = "Robert Burger <robert.burgert@dlr.de>"
    license = "GPLv3"

    url = f"https://rmc-github.robotic.dlr.de/robotkernel/service_provider_key_value"
    settings = "os"
    pure_python_folder = os.path.join("bindings", "python")
    exports_sources = os.path.join(pure_python_folder, "*")

    def requirements(self):
        self.requires(f"service_provider_key_value_ln_msgdef/{self.version}@{self.user}/{self.channel}")

    def package(self):
        self.copy(os.path.join(self.pure_python_folder, "*"))

    def package_info(self):
        pypath1 = os.path.join(self.package_folder, os.path.dirname(self.pure_python_folder))
        pypath2 = os.path.join(self.package_folder, self.pure_python_folder)
        if Version(conan_version) < "2.0.0":
            self.env_info.PYTHONPATH.append(pypath1)
            self.env_info.PYTHONPATH.append(pypath2)
        self.runenv_info.append_path("PYTHONPATH", pypath1)
        self.runenv_info.append_path("PYTHONPATH", pypath2)
