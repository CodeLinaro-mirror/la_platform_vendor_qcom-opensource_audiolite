load(":audio_modules.bzl", "audio_modules")
load(":module_mgr.bzl", "define_target_modules")

def define_gen4_gvm():
    define_target_modules(
        target = "autogvm",
        variants = ["consolidate", "perf"],
        registry = audio_modules,
        modules = [
            "ipcc_shmem_test_module",
        ]
    )

def define_audio_target():
    define_gen4_gvm()
