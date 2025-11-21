load(":module_mgr.bzl", "create_module_registry")

audio_modules = create_module_registry([":audio_headers"])
# ------------------------------------ AUDIO MODULE DEFINITIONS ---------------------------------
# >>>> AUDIOLITE MODULES <<<<
audio_modules.register(
    name = "ipcc_shmem_test_module",
    config_option = "IS_PMEM_API_BAZEL",
    srcs = [
        "ipcc_shmem_test_module.c"
    ],

)
