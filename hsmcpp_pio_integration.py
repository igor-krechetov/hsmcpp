# Copyright (C) 2023 Igor Krechetov
# Distributed under MIT license. See file LICENSE for details

Import("env", "projenv")
import os
from tools.scxml2gen import scxml2gen

# Modify project environment (files located in the `src` folder)
projenv.Append(CPPDEFINES=[("PLATFORM_ARDUINO", 1), ("HSM_BUILD_HSMBUILD_DISPATCHER_ARDUINO", 1)])


# HSM files generation
projectOptions = env.GetProjectOptions(as_dict=True)

if ("custom_hsm_files" in projectOptions):
    if ("custom_hsm_gendir" in projectOptions):
        if "PROJECT_DIR" in env:
            PROJECT_DIR = env["PROJECT_DIR"]
            hsmFiles = env.GetProjectOption("custom_hsm_files")
            genDir = f"{PROJECT_DIR}/{env.GetProjectOption('custom_hsm_gendir')}"

            if hsmFiles:
                os.makedirs(genDir, exist_ok=True)
                hsmFiles = hsmFiles.strip().split("\n")

                for scxmlConfig in hsmFiles:
                    cfg = scxmlConfig.strip().split(":")

                    if len(cfg) == 3:
                        scxml2gen.generate_code(scxmlPath=f"{PROJECT_DIR}/{cfg[0]}",
                                                dest_dir=genDir, 
                                                class_name=cfg[1], 
                                                class_suffix=cfg[2], 
                                                template_hpp="./tools/scxml2gen/template.hpp",
                                                template_cpp="./tools/scxml2gen/template.cpp")
                    else:
                        print(f"[hsmcpp] ERROR: values in 'custom_hsm_files' must be in '<scxml path>:<class name>:<class suffix>' format. Found value: ({scxmlConfig})")
                        exit(1)
        else:
            print("[hsmcpp] ERROR: PROJECT_DIR not found in pio environment. Normally this should"
                  " not happen and probably indicates compatibility issue with new version of pio")
            exit(2)
    else:
        print("[hsmcpp] ERROR: 'custom_hsm_gendir' project option is mandatory if 'custom_hsm_files' is specified")
        exit(1)
