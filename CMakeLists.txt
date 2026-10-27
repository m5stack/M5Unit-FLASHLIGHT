# ESP-IDF component registration for M5Unit-FLASHLIGHT.
# Upstream: https://github.com/m5stack/M5Unit-FLASHLIGHT
#
# The public headers include only M5UnitUnified; M5Utility is used by the sources only.
file(GLOB_RECURSE SRCS "src/*.cpp")
idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "src"
    REQUIRES M5UnitUnified
    PRIV_REQUIRES M5Utility
)
