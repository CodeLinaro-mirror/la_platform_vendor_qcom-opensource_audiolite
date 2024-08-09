ifeq ($(RPM_BUILD), true)
dtb-y += sa8775p_audiolite_common.dtbo
dtb-$(CONFIG_AUDIOLITE_OVERLAY) += sa8775p_audiolite_overlay.dtbo
include ${PWD}/Makefile.rpmbuild
else
include ${PWD}/Makefile.bbbuild
endif
