
#include "g_lua.h"

// TODO: aiming for compatibility with ETPro lua mods
// http://wolfwiki.anime.net/index.php/Lua_Mod_API

lua_vm_t * lVM[LUA_NUM_VM];
#define LOG G_LogPrintf

/***************************/
/* Lua et library handlers */
/***************************/

// ET Library Calls
// et.RegisterModname( modname )
static int _et_RegisterModname(lua_State *L)
{
	char * modname = luaL_checkstring(L, 1);
	if (modname) {
		lua_vm_t * vm = G_LuaGetVM(L);
		if (vm) {
			Q_strncpyz(vm->mod_name, modname, sizeof(vm->mod_name));
		}
	}
	return 0;
}

// vmnumber = et.FindSelf() 
static int _et_FindSelf(lua_State *L)
{
	lua_vm_t * vm = G_LuaGetVM(L);
	if (vm) {
		lua_pushinteger(L, vm->id);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

// modname, signature = et.FindMod( vmnumber ) 
static int _et_FindMod(lua_State *L)
{
	int vmnumber = luaL_checkint(L, 1);
	lua_vm_t * vm = lVM[vmnumber];
	if ( vm ) {
		lua_pushstring(L, vm->mod_name);
		lua_pushstring(L, ""); //TODO: sha1 signature
	} else {
		lua_pushnil(L);
		lua_pushnil(L);
	}
	return 2;
}

// success = et.IPCSend( vmnumber, message ) 
static int _et_IPCSend(lua_State *L)
{
	int vmnumber = luaL_checkint(L, 1);
	char *message = luaL_checkstring(L, 2);
	
	lua_vm_t *sender = G_LuaGetVM(L);
	lua_vm_t *vm = lVM[vmnumber];
	
	if (!vm || vm->err) {
		lua_pushinteger(L, 0);
		return 1;
	}
	
	// Find callback
	if (!G_LuaGetNamedFunction(vm, "et_IPCReceive")) {
		lua_pushinteger(L, 0);
		return 1;
	}
	
	// Arguments
	if (sender) {
		lua_pushinteger(vm->L, sender->id);
	} else {
		lua_pushnil(vm->L);
	}
	lua_pushstring(vm->L, message);
	
	// Call
	if (!G_LuaCall(vm, 2, 0)) {
		G_LuaStopVM(vm);
		lua_pushinteger(L, 0);
		return 1;
	}
	
	// Success
	lua_pushinteger(L, 1);
	return 1;
}

// Printing
// et.G_Print( text )
static int _et_G_Print(lua_State *L)
{
	char * text = luaL_checkstring(L, 1);
	if ( text )
		G_Printf(text);
	return 0;
}

// et.G_LogPrint( text ) 
static int _et_G_LogPrint(lua_State *L)
{
	char * text = luaL_checkstring(L, 1);
	if ( text ) {
		G_LogPrintf(text);
	}
	return 0;
}



// et library initialisation array
static const luaL_Reg etlib[] = {
// ET Library Calls
  {"RegisterModname", 	_et_RegisterModname},
  {"FindSelf", 			_et_FindSelf},
  {"FindMod",			_et_FindMod},
  {"IPCSend",			_et_IPCSend},
 // Printing
  {"G_Print", 			_et_G_Print},
  {"G_LogPrint",		_et_G_LogPrint},
  {NULL, NULL},
};

/*************/
/* Lua API   */
/*************/

/** G_LuaInit()
 * Initialises the Lua API interface 
 */
qboolean G_LuaInit()
{
	int i, num_vm = 0, len, flen = 0;
	char buff[MAX_CVAR_VALUE_STRING], *crt, *code;
	fileHandle_t f;
	lua_vm_t *vm;
	
	if (!lua_modules.string[0])
		return qtrue;
	
	Q_strncpyz(buff, lua_modules.string, sizeof(buff));
	len = strlen(buff);
	crt = buff;
	
	for (i=0; i<LUA_NUM_VM; i++)
		lVM[i] = NULL;
	
	for (i=0; i<=len; i++)
		if (buff[i] == ' ' || buff[i] == '\0' || buff[i] == ',' || buff[i] == ';') {
			buff[i] = '\0';
			
			// try to open lua file
			flen = trap_FS_FOpenFile(crt, &f, FS_READ);
			if (flen < 0) {
				LOG("Lua API: can not open file %s\n", crt);
			} else if (flen > LUA_MAX_FSIZE) {
				// quad: Let's not load arbitrarily big files to memory.
				// If your lua file exceeds the limit, let me know.
				LOG("Lua API: ignoring file %s (too big)\n", crt);
				trap_FS_FCloseFile(f);
			} else {
				code = malloc(flen + 1);
				trap_FS_Read(code, flen, f);
				*(code + flen) = '\0';
				trap_FS_FCloseFile(f);
				
				// Init lua_vm_t struct
				vm = (lua_vm_t*) malloc(sizeof(lua_vm_t));
				vm->id = -1;
				Q_strncpyz(vm->file_name, crt, sizeof(vm->file_name));
				Q_strncpyz(vm->mod_name, "", sizeof(vm->mod_name));
				vm->code = code;
				vm->code_size = flen;
				vm->err = 0;
				
				// Start lua virtual machine
				if (G_LuaStartVM(vm) == qfalse) {
					G_LuaStopVM(vm);
					vm = NULL;
				} else {
					vm->id = num_vm;
					lVM[num_vm] = vm;
					num_vm++;
				}
			}
			
			// prepare for next iteration
			if (i+1 < len)
				crt = buff + i + 1;
			else
				crt = NULL;
			if (num_vm >= LUA_NUM_VM) {
				LOG("Lua API: too many lua files specified, only the first %d have been loaded\n", LUA_NUM_VM);
				break;
			}
		}
	return qtrue;
}

/** G_LuaCall( vm, nargs, nresults )
 * Calls a function already on the stack.
 */
qboolean G_LuaCall(lua_vm_t* vm, int nargs, int nresults)
{
	int res = lua_pcall(vm->L, nargs, nresults, 0);
	if (res == LUA_ERRRUN) {
		LOG("Lua API: run-time error ( %s ) %s\n", vm->file_name, lua_tostring(vm->L, -1) );
		lua_pop(vm->L, 1);
		vm->err++;
		return qfalse;
	} else if (res == LUA_ERRMEM) {
		LOG("Lua API: memory allocation error #2 ( %s )\n", vm->file_name);
		vm->err++;
		return qfalse;
	} else if (res == LUA_ERRERR) {
		LOG("Lua API: traceback error ( %s )\n", vm->file_name);
		vm->err++;
		return qfalse;
	}
	return qtrue;
}

/** G_LuaGetNamedFunction( vm, name )
 * Finds a function by name and puts it onto the stack.
 * If the function does not exist, returns qfalse.
 */
qboolean G_LuaGetNamedFunction(lua_vm_t *vm, char *name)
{
	if (vm->L) {
		lua_getglobal(vm->L, name);
		if (lua_isfunction(vm->L, -1)) {
			return qtrue;
		} else {
			lua_pop(vm->L, 1);
			return qfalse;
		}
	}
	return qfalse;
}

/** G_LuaStartVM( vm )
 * Starts one individual virtual machine.
 */
qboolean G_LuaStartVM(lua_vm_t* vm)
{
	int res = 0;
	
	// Open a new lua state
	vm->L = luaL_newstate();
	if (! vm->L ) {
		LOG("Lua API: Lua failed to initialise.\n");
		return qfalse;
	}
	
	// Initialise the lua state
	luaL_openlibs(vm->L);
	luaL_register(vm->L, "et", etlib);
	
	// Load the code
	res = luaL_loadbuffer(vm->L, vm->code, vm->code_size, vm->file_name);
	if (res == LUA_ERRSYNTAX) {
		LOG("Lua API: syntax error during pre-compilation ( %s )\n", vm->file_name);
		vm->err++;
		return qfalse;
	} else if (res == LUA_ERRMEM) {
		LOG("Lua API: memory allocation error #1 ( %s )\n", vm->file_name);
		vm->err++;
		return qfalse;
	}
	
	// Execute the code
	if (!G_LuaCall(vm, 0, 0))
		return qfalse;
	
	LOG("Lua API: Loaded script %s\n", vm->file_name);
	return qtrue;
}

/** G_LuaStopVM( vm )
 * Stops one virtual machine, and calls its et_Quit callback. 
 */
void G_LuaStopVM(lua_vm_t *vm)
{
	if (vm == NULL)
		return;
	if (vm->code != NULL) {
		free(vm->code);
		vm->code = NULL;
	}
	if (vm->L) {
		if (G_LuaGetNamedFunction(vm, "et_Quit"))
			G_LuaCall(vm, 0, 0);
		lua_close(vm->L);
		vm->L = NULL;
	}
	if (vm->id >= 0) {
		if (lVM[vm->id] == vm)
			lVM[vm->id] = NULL;
		if (!vm->err) {
			LOG("Lua API: Unloaded script %s\n", vm->file_name);
		}
	}
	free(vm);
}

/** G_LuaShutdown()
 * Shuts down everything related to Lua API.
 */
void G_LuaShutdown()
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			G_LuaStopVM(vm);
		}
	}
}

/** G_LuaStatus( ent )
 * Prints information on the Lua virtual machines.
 */
void G_LuaStatus(gentity_t *ent)
{
	int i, cnt = 0;
	for (i=0; i<LUA_NUM_VM; i++)
		if (lVM[i])
			cnt++;
	
	if (cnt == 0) {
		G_refPrintf(ent, "Lua API: no scripts loaded.");
		return;
	} else if (cnt == 1) {
		G_refPrintf(ent, "Lua API: showing lua information ( 1 script loaded )");
	} else {
		G_refPrintf(ent, "Lua API: showing lua information ( %d scripts loaded )", cnt);
	}
	G_refPrintf(ent, "%-2s %-24s %-24s", "VM", "Mod name", "File name");
	G_refPrintf(ent, "-- ------------------------ ------------------------");
	for (i=0; i<LUA_NUM_VM; i++) {
		if (lVM[i]) {
			G_refPrintf(ent, "%-2d %-24s %-24s", i, lVM[i]->mod_name, lVM[i]->file_name);
		}
	}
	G_refPrintf(ent, "-- ------------------------ ------------------------");
	
}

/** G_LuaGetVM
 * Retrieves the VM for a given lua_State
 */
lua_vm_t * G_LuaGetVM(lua_State *L)
{
	int i;
	for (i=0; i<LUA_NUM_VM; i++)
		if (lVM[i] && lVM[i]->L == L)
			return lVM[i];
	return NULL;
}

/*****************************/
/* Lua API hooks / callbacks */
/*****************************/

/** G_LuaHook_InitGame
 * et_InitGame( levelTime, randomSeed, restart ) callback 
 */
void G_LuaHook_InitGame(int levelTime, int randomSeed, int restart)
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_InitGame"))
				continue;
			// Arguments
			lua_pushinteger(vm->L, levelTime);
			lua_pushinteger(vm->L, randomSeed);
			lua_pushinteger(vm->L, restart);
			// Call
			if (!G_LuaCall(vm, 3, 0)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
		}
	}
}

/** G_LuaHook_ShutdownGame
 * et_ShutdownGame( restart )  callback 
 */
void G_LuaHook_ShutdownGame(int restart)
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_ShutdownGame"))
				continue;
			// Arguments
			lua_pushinteger(vm->L, restart);
			// Call
			if (!G_LuaCall(vm, 1, 0)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
		}
	}
}

/** G_LuaHook_RunFrame
 * et_RunFrame( levelTime )  callback
 */
void G_LuaHook_RunFrame(int levelTime)
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_RunFrame"))
				continue;
			// Arguments
			lua_pushinteger(vm->L, levelTime);
			// Call
			if (!G_LuaCall(vm, 1, 0)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
		}
	}
}

/** G_LuaHook_ClientConnect
 * rejectreason = et_ClientConnect( clientNum, firstTime, isBot ) callback
 */
qboolean G_LuaHook_ClientConnect(int clientNum, qboolean firstTime, qboolean isBot, char *reason)
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_ClientConnect"))
				continue;
			// Arguments
			lua_pushinteger(vm->L, clientNum);
			lua_pushinteger(vm->L, (int)firstTime);
			lua_pushinteger(vm->L, (int)isBot);
			// Call
			if (!G_LuaCall(vm, 3, 1)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
			if (lua_isstring(vm->L, -1)) {
				Q_strncpyz(reason, lua_tostring(vm->L, -1), MAX_STRING_CHARS);
				return qtrue;
			}
		}
	}
	return qfalse;
}

/** G_LuaHook_ClientDisconnect
 * et_ClientDisconnect( clientNum ) callback
 */
void G_LuaHook_ClientDisconnect(int clientNum)
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_ClientDisconnect"))
				continue;
			// Arguments
			lua_pushinteger(vm->L, clientNum);
			// Call
			if (!G_LuaCall(vm, 1, 0)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
		}
	}
}

/** G_LuaHook_ClientBegin
 * et_ClientBegin( clientNum ) callback
 */
void G_LuaHook_ClientBegin(int clientNum)
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_ClientBegin"))
				continue;
			// Arguments
			lua_pushinteger(vm->L, clientNum);
			// Call
			if (!G_LuaCall(vm, 1, 0)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
		}
	}
}

/** void G_LuaHook_ClientUserinfoChanged(int clientNum);
 * et_ClientUserinfoChanged( clientNum ) callback
 */
void G_LuaHook_ClientUserinfoChanged(int clientNum)
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_ClientUserinfoChanged"))
				continue;
			// Arguments
			lua_pushinteger(vm->L, clientNum);
			// Call
			if (!G_LuaCall(vm, 1, 0)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
		}
	}
}

/** G_LuaHook_ClientSpawn
 * et_ClientSpawn( clientNum, revived, teamChange, restoreHealth ) callback
 */
void G_LuaHook_ClientSpawn(int clientNum, qboolean revived, qboolean teamChange, qboolean restoreHealth)
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_ClientSpawn"))
				continue;
			// Arguments
			lua_pushinteger(vm->L, clientNum);
			lua_pushinteger(vm->L, (int)revived);
			lua_pushinteger(vm->L, (int)teamChange);
			lua_pushinteger(vm->L, (int)restoreHealth);
			// Call
			if (!G_LuaCall(vm, 4, 0)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
		}
	}
}

