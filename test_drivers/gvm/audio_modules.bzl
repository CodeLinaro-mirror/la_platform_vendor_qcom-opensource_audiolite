load(":module_mgr.bzl", "create_module_registry")

audio_modules = create_module_registry([":audio_headers"])
# ------------------------------------ AUDIO MODULE DEFINITIONS ---------------------------------
# >>>> AUDIOLITE MODULES <<<<
audio_modules.register(
    name = "ipcc_shmem_test_module",
    srcs = [
        "ipcc_shmem_test_module.c"
    ],

)
