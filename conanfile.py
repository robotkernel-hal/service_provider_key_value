from conan import ConanFile
import os


class MainProject(ConanFile):
    python_requires = "conan_template/[~5]@robotkernel/stable"
    python_requires_extend = "conan_template.RobotkernelConanFile"

    name = "service_provider_key_value"
    url = "https://rmc-github.robotic.dlr.de/robotkernel/service_provider_key_value"
    description = "robotkernel service provider for key value devices."
    exports_sources = ["*", "!.gitignore", "!bindings"]

    tool_requires = ["robotkernel_service_helper/[~6]@robotkernel/snapshot"]

    def source(self):
        self.run(f"sed 's/AC_INIT(.*/AC_INIT([service_provider_key_value], [{self.version}], [{self.author}])/' configure.ac.in > configure.ac")

    def requirements(self):
        self.requires(f"{self.name}_ln_msgdef/{self.version}@{self.user}/{self.channel}")
        self.requires("robotkernel/[~6]@robotkernel/snapshot")
