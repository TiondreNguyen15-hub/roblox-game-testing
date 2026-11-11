import ctypes,sys
lua=ctypes.CDLL('liblua5.4.so.0');lua.luaL_newstate.restype=ctypes.c_void_p
lua.luaL_openlibs.argtypes=[ctypes.c_void_p];lua.luaL_loadfilex.argtypes=[ctypes.c_void_p,ctypes.c_char_p,ctypes.c_char_p]
lua.lua_pcallk.argtypes=[ctypes.c_void_p,ctypes.c_int,ctypes.c_int,ctypes.c_int,ctypes.c_longlong,ctypes.c_void_p]
lua.lua_tolstring.argtypes=[ctypes.c_void_p,ctypes.c_int,ctypes.c_void_p];lua.lua_tolstring.restype=ctypes.c_char_p;lua.lua_close.argtypes=[ctypes.c_void_p]
s=lua.luaL_newstate();lua.luaL_openlibs(s)
r=lua.luaL_loadfilex(s,sys.argv[1].encode(),None)
if not r:r=lua.lua_pcallk(s,0,-1,0,0,None)
if r: print(lua.lua_tolstring(s,-1,None).decode());sys.exit(1)
lua.lua_close(s)
