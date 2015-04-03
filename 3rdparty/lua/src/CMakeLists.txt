#--------------------------------------------------------------------------------------------------
#
# FILENAME      :CMakeLists.txt
#
# PURPOSE       :FOR COMPILE LUA.
#
# NOTE          :
#
#                               <perturbed@sina.com>
#--------------------------------------------------------------------------------------------------

set(LUA_SOURCE_LIST_liblua lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c lbitlib.c lcorolib.c ldblib.c liolib.c lmathlib.c loslib.c lstrlib.c ltablib.c loadlib.c linit.c)

add_library(lua STATIC ${LUA_SOURCE_LIST_liblua})
#add_library(lua SHARED ${LUA_SOURCE_LIST_liblua})
unset(LUA_SOURCE_LIST_liblua CACHE)