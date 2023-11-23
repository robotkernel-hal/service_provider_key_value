import os
from conan import ConanFile, conan_version
from conan.tools.files import copy, chdir
from conan.tools.scm import Version


class MainProject(ConanFile):
    name = "service_provider_key_value_ln_msgdef"
    url = "https://rmc-github.robotic.dlr.de/robotkernel/service_provider_key_value"
    author = "Robert Burger robert.burger@dlr.de"
    license = "GPLv3"
    description = "service_provider_key_value ln message definitions"
    settings = None
    exports_sources = [
        "share/*",
    ]

    tool_requires = ["robotkernel_ln_helper/[*]@robotkernel/stable"]
    generators = "VirtualBuildEnv"

    def package(self):
        svc_def_dir = "share/service_definitions"
        ln_msg_dir = "share/ln/message_definitions"

        svc_def_files = []
        with chdir(self, os.path.join(self.build_folder, svc_def_dir)):
            for dirpath, dirnames, filenames in os.walk("."):
                svc_def_files.extend(os.path.join(dirpath, filename) for filename in filenames)

        self.run(
            "service_generate --indir %s --outdir %s %s" % (svc_def_dir, ln_msg_dir, " ".join(svc_def_files)), env="conanbuild"
        )

        copy(self, "share/ln/message_definitions/*", self.build_folder, self.package_folder)

    def package_info(self):
        msgdef_dir = os.path.join(self.package_folder, "share/ln/message_definitions")
        if Version(conan_version) < "2.0.0":
            self.env_info.LN_MESSAGE_DEFINITION_DIRS.append(msgdef_dir)
        self.runenv_info.append_path("LN_MESSAGE_DEFINITION_DIRS", msgdef_dir)
        self.buildenv_info.append_path("LN_MESSAGE_DEFINITION_DIRS", msgdef_dir)
