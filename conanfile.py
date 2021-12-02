from conans import ConanFile, tools
import os

class MainProject(ConanFile):
    python_requires = "conan_template_ln_generator/[~=5 >=5.0.6]@robotkernel/stable"
    python_requires_extend = "conan_template_ln_generator.RobotkernelLNGeneratorConanFile"

    name = "service_provider_key_value"
    description = "robotkernel service provider for key value devices."
    exports_sources = ["*", "!.gitignore"] + ["!%s" % x for x in tools.Git().excluded_files()]
    requires = "robotkernel/[~=5]@robotkernel/stable"

    def package_info(self):
        base = self.python_requires["conan_template_ln_generator"].module.RobotkernelLNGeneratorConanFile
        base.package_info(self)

        self.env_info.PYTHONPATH.append(os.path.join(self.package_folder, "bindings/python"))

    def config_options(self):
        self.options.generate_ln_mds = True

