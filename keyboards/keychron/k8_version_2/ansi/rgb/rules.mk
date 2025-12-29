# OpenRGB Support (wired only)
RAW_ENABLE = yes
OPENRGB_ENABLE = yes
VIA_OPENRGB_HYBRID = yes

OPT_DEFS += -DOPENRGB_ENABLE -DVIA_OPENRGB_HYBRID

SRC += openrgb.c ../../../common/hybrid_switch_animation.c
