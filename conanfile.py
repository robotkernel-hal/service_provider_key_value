from conans import tools, python_requires

base = python_requires("conan_template/[~=5]@robotkernel/stable")

class MainProject(base.RobotkernelConanFile):
    name = "service_provider_key_value"
    description = "robotkernel-5 service provider for key value devices."
    exports_sources = ["*", "!.gitignore"] + ["!%s" % x for x in tools.Git().excluded_files()]
    requires = "robotkernel/[~=5]@robotkernel/stable"

