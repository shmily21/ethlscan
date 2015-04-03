#--------------------------------------------------------------------------------------------------
#
# FILENAME      :CMakeLists.txt
#
# PURPOSE       :FOR COMPILE LUA
#
# NOTE          :PRESET PARAMETER.
#
#                               <perturbed@sina.com>
#--------------------------------------------------------------------------------------------------

if(UNIX)
    add_definitions("-DLUA_COMPAT_ALL -DLUA_USE_LINUX")
endif(UNIX)

add_subdirectory(src)
