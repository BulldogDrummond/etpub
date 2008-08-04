
#include "g_lua.h"

// TODO: aiming for compatibility with ETPro lua mods
// http://wolfwiki.anime.net/index.php/Lua_Mod_API

lua_vm_t * lVM[LUA_NUM_VM];
#define LOG G_LogPrintf

/***************************/
/* Lua et library handlers */
/***************************/

// ET Library Calls {{{
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
// }}}

// Printing {{{
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
// }}}

// Argument Handling {{{
// et.ConcatArgs( index )
static int _et_ConcatArgs(lua_State *L)
{
	int i, off = 0, len;
	char buff[MAX_STRING_CHARS];
	int index = luaL_optint(L, 1, 0);
	int count = trap_Argc()-index;
	
	if (count < 0) count = 0;
	
	for (i=index; i<index+count; i++) {
		trap_Argv(i, buff, sizeof(buff));
		len = strlen(buff);
		if (i < index+count-1 && len < sizeof(buff)-2) {
			buff[len] = ' ';
			buff[len+1] = '\0';
		}
		lua_pushstring(L, buff);
	}
	lua_concat(L, count);
	return 1;
}

// et.trap_Argc()
static int _et_trap_Argc(lua_State *L)
{
	lua_pushinteger(L, trap_Argc());
	return 1;
}

// arg = et.trap_Argv( argnum ) 
static int _et_trap_Argv(lua_State *L)
{
	char buff[MAX_STRING_CHARS];
	int argnum = luaL_checkint(L, 1);
	trap_Argv(argnum, buff, sizeof(buff));
	lua_pushstring(L, buff);
	return 1;
}
// }}}

// Cvars {{{
//cvarvalue = et.trap_Cvar_Get( cvarname ) 
static int _et_trap_Cvar_Get(lua_State *L)
{
	char buff[MAX_CVAR_VALUE_STRING];
	char *cvarname = luaL_checkstring(L, 1);
	trap_Cvar_VariableStringBuffer(cvarname, buff, sizeof(buff));
	lua_pushstring(L, buff);
	return 1;
}

// et.trap_Cvar_Set( cvarname, cvarvalue )
static int _et_trap_Cvar_Set(lua_State *L)
{
	char *cvarname = luaL_checkstring(L, 1);
	char *cvarvalue= luaL_checkstring(L, 2);
	trap_Cvar_Set(cvarname, cvarvalue);
	return 0;
}
// }}}

// Config Strings {{{
// configstringvalue = et.trap_GetConfigstring( index ) 
static int _et_trap_GetConfigstring(lua_State *L)
{
	char buff[MAX_STRING_CHARS];
	int index = luaL_checkint(L, 1);
	trap_GetConfigstring(index, buff, sizeof(buff));
	lua_pushstring(L, buff);
	return 1;
}

// et.trap_SetConfigstring( index, configstringvalue ) 
static int _et_trap_SetConfigstring(lua_State *L)
{
	int index = luaL_checkint(L, 1);
	char *csv = luaL_checkstring(L, 2);
	trap_SetConfigstring(index, csv);
	return 0;
}
// }}}

// Server {{{
// et.trap_SendConsoleCommand( when, command )
static int _et_trap_SendConsoleCommand(lua_State *L)
{
	int when = luaL_checkint(L, 1);
	char *cmd= luaL_checkstring(L, 2);
	trap_SendConsoleCommand(when, cmd);
	return 0;
}
// }}}

// Clients {{{
// et.trap_DropClient( clientnum, reason, ban_time )
static int _et_trap_DropClient(lua_State *L)
{
	int clientnum = luaL_checkint(L, 1);
	char *reason = luaL_checkstring(L, 2);
	int ban = trap_Cvar_VariableIntegerValue("g_defaultBanTime");
	ban = luaL_optint(L, 3, ban);
	trap_DropClient(clientnum, reason, ban);
	return 0;
}

// et.trap_SendServerCommand( clientnum, command )
static int _et_trap_SendServerCommand(lua_State *L)
{
	int clientnum = luaL_checkint(L, 1);
	char *cmd = luaL_checkstring(L, 2);
	trap_SendServerCommand(clientnum, cmd);
	return 0;
}

// et.G_Say( clientNum, mode, text )
static int _et_G_Say(lua_State *L)
{
	int clientnum = luaL_checkint(L, 1);
	int mode = luaL_checkint(L, 2);
	char *text = luaL_checkstring(L, 3);
	G_Say(g_entities+clientnum, NULL, mode, text);
	return 0;
}

// et.ClientUserinfoChanged( clientNum )
static int _et_ClientUserinfoChanged(lua_State *L)
{
	int clientnum = luaL_checkint(L, 1);
	ClientUserinfoChanged(clientnum);
	return 0;
}
// }}}

// Userinfo {{{
// userinfo = et.trap_GetUserinfo( clientnum )
static int _et_trap_GetUserinfo(lua_State *L)
{
	char buff[MAX_STRING_CHARS];
	int clientnum = luaL_checkint(L, 1);
	trap_GetUserinfo(clientnum, buff, sizeof(buff));
	lua_pushstring(L, buff);
	return 1;
}

// et.trap_SetUserinfo( clientnum, userinfo )
static int _et_trap_SetUserinfo(lua_State *L)
{
	int clientnum = luaL_checkint(L, 1);
	char *userinfo = luaL_checkstring(L, 2);
	trap_SetUserinfo(clientnum, userinfo);
	return 0;
}
// }}}

// String Utility Functions {{{
// infostring = et.Info_RemoveKey( infostring, key )
static int _et_Info_RemoveKey(lua_State *L)
{
	char buff[MAX_INFO_STRING];
	char *key = luaL_checkstring(L, 2);
	Q_strncpyz(buff, luaL_checkstring(L, 1), sizeof(buff));
	Info_RemoveKey(buff, key);
	lua_pushstring(L, buff);
	return 1;
}

// infostring = et.Info_SetValueForKey( infostring, key, value )
static int _et_Info_SetValueForKey(lua_State *L)
{
	char *buff[MAX_INFO_STRING];
	char *key = luaL_checkstring(L, 2);
	char *value = luaL_checkstring(L, 3);
	Q_strncpyz(buff, luaL_checkstring(L, 1), sizeof(buff));
	Info_SetValueForKey(buff, key, value);
	lua_pushstring(L, buff);
	return 1;
}

// keyvalue = et.Info_ValueForKey( infostring, key )
static int _et_Info_ValueForKey(lua_State *L)
{
	char *infostring = luaL_checkstring(L, 1);
	char *key = luaL_checkstring(L, 2);
	lua_pushstring(L, Info_ValueForKey(infostring, key));
	return 1;
}

// cleanstring = et.Q_CleanStr( string )
static int _et_Q_CleanStr(lua_State *L)
{
	char buff[MAX_STRING_CHARS];
	Q_strncpyz(buff, luaL_checkstring(L, 1), sizeof(buff));
	Q_CleanStr(buff);
	lua_pushstring(L, buff);
	return 1;
}
// }}}

// ET Filesystem {{{
// fd, len = et.trap_FS_FOpenFile( filename, mode )
static int _et_trap_FS_FOpenFile(lua_State *L)
{
	fileHandle_t fd;
	int len;
	char *filename = luaL_checkstring(L, 1);
	int mode = luaL_checkint(L, 2);
	len = trap_FS_FOpenFile(filename, &fd, mode);
	lua_pushinteger(L, fd);
	lua_pushinteger(L, len);
	return 2;
}

// filedata = et.trap_FS_Read( fd, count )
static int _et_trap_FS_Read(lua_State *L)
{
	fileHandle_t df = 0;//TODO:
	return 1;	
}
// count = et.trap_FS_Write( filedata, count, fd )
// et.trap_FS_Rename( oldname, newname )
// et.trap_FS_FCloseFile( fd )
// }}}

// Indexes {{{
// soundindex = et.G_SoundIndex( filename )
// modelindex = et.G_ModelIndex( filename )
// }}}

// Sound {{{
// et.G_globalSound( sound )
// et.G_Sound( entnum, soundindex )
// }}}

// Miscellaneous {{{
// milliseconds = et.trap_Milliseconds()
// et.G_Damage( target, inflictor, attacker, damage, dflags, mod )
// }}}

// Entities {{{
// entnum = et.G_Spawn()
// entnum = et.G_TempEntity( origin, event )
// et.G_FreeEntity( entnum )
// spawnval = et.G_GetSpawnVar( entnum, key )
// et.G_SetSpawnVar( entnum, key, value )
// integer entnum = et.G_SpawnGEntityFromSpawnVars( string spawnvar, string spawnvalue, ... )
// et.trap_LinkEntity( entnum )
// et.trap_UnlinkEntity( entnum )
// (variable) = et.gentity_get ( entnum, Fieldname ,arrayindex )
// et.gentity_set( entnum, Fieldname, arrayindex, (value) )
// et.G_AddEvent( ent, event, eventparm' )
// }}}






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
	// Argument Handling
	{"ConcatArgs", 		_et_ConcatArgs},
	{"trap_Argc", 		_et_trap_Argc},
	{"trap_Argv", 		_et_trap_Argv},
	// Cvars
	{"trap_Cvar_Get",	_et_trap_Cvar_Get},
	{"trap_Cvar_Set",	_et_trap_Cvar_Set},
	// Config Strings
	{"trap_GetConfigstring", _et_trap_GetConfigstring},
	{"trap_SetConfigstring", _et_trap_SetConfigstring},
	// Server
	{"trap_SendConsoleCommand", _et_trap_SendConsoleCommand},
	// Clients
	{"trap_DropClient",	_et_trap_DropClient},
	{"trap_SendServerCommand",	_et_trap_SendServerCommand},
	{"G_Say",	_et_G_Say},
	{"ClientUserinfoChanged",	_et_ClientUserinfoChanged},
	// String Utility Functions
	{"Info_RemoveKey", _et_Info_RemoveKey},
	{"Info_SetValueForKey", _et_Info_SetValueForKey},
	{"Info_ValueForKey", _et_Info_ValueForKey},
	{"Q_CleanStr",		_et_Q_CleanStr},
	// ET Filesystem
// fd, len = et.trap_FS_FOpenFile( filename, mode )
// filedata = et.trap_FS_Read( fd, count )
// count = et.trap_FS_Write( filedata, count, fd )
// et.trap_FS_Rename( oldname, newname )
// et.trap_FS_FCloseFile( fd ) 
	
	{NULL, NULL},
};

/*************/
/* Lua API   */
/*************/

//{{{
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
//}}}

/*****************************/
/* Lua API hooks / callbacks */
/*****************************/

//{{{

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

/** G_LuaHook_ClientCommand
 * intercepted = et_ClientCommand( clientNum, command ) callback
 */
qboolean G_LuaHook_ClientCommand(int clientNum, char *command)
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_ClientCommand"))
				continue;
			// Arguments
			lua_pushinteger(vm->L, clientNum);
			lua_pushstring(vm->L, command);
			// Call
			if (!G_LuaCall(vm, 2, 1)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
			if (lua_isnumber(vm->L, -1)) {
				if (lua_tointeger(vm->L, -1) == 1) {
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/** G_LuaHook_ConsoleCommand
 * intercepted = et_ConsoleCommand( command ) callback
 */
qboolean G_LuaHook_ConsoleCommand(char *command)
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_ConsoleCommand"))
				continue;
			// Arguments
			lua_pushstring(vm->L, command);
			// Call
			if (!G_LuaCall(vm, 1, 1)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
			if (lua_isnumber(vm->L, -1)) {
				if (lua_tointeger(vm->L, -1) == 1) {
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/** G_LuaHook_UpgradeSkill
 * result = et_UpgradeSkill( cno, skill ) callback
 */
qboolean G_LuaHook_UpgradeSkill(int cno, skillType_t skill)
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_UpgradeSkill"))
				continue;
			// Arguments
			lua_pushinteger(vm->L, cno);
			lua_pushinteger(vm->L, (int)skill);
			// Call
			if (!G_LuaCall(vm, 2, 1)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
			if (lua_isnumber(vm->L, -1)) {
				if (lua_tointeger(vm->L, -1) == -1) {
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/** G_LuaHook_SetPlayerSkill
 * et_SetPlayerSkill( cno, skill ) callback
 */
qboolean G_LuaHook_SetPlayerSkill( int cno, skillType_t skill )
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_SetPlayerSkill"))
				continue;
			// Arguments
			lua_pushinteger(vm->L, cno);
			lua_pushinteger(vm->L, (int)skill);
			// Call
			if (!G_LuaCall(vm, 2, 1)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
			if (lua_isnumber(vm->L, -1)) {
				if (lua_tointeger(vm->L, -1) == -1) {
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/** G_LuaHook_Print
 * et_Print( text ) callback
 */
void G_LuaHook_Print( char *text )
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_Print"))
				continue;
			// Arguments
			lua_pushstring(vm->L, text);
			// Call
			if (!G_LuaCall(vm, 1, 0)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
		}
	}
}

/** G_LuaHook_Obituary
 * et_Obituary( victim, killer, meansOfDeath ) callback
 */
void G_LuaHook_Obituary(int victim, int killer, int meansOfDeath)
{
	int i;
	lua_vm_t *vm;
	for (i=0; i<LUA_NUM_VM; i++) {
		vm = lVM[i];
		if (vm) {
			if (vm->id < 0 || vm->err)
				continue;
			if (!G_LuaGetNamedFunction(vm, "et_Obituary"))
				continue;
			// Arguments
			lua_pushinteger(vm->L, victim);
			lua_pushinteger(vm->L, killer);
			lua_pushinteger(vm->L, meansOfDeath);
			// Call
			if (!G_LuaCall(vm, 3, 0)) {
				G_LuaStopVM(vm);
				continue;
			}
			// Return values
		}
	}
}

//}}}
