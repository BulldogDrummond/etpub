#ifndef _G_LUA_H
#define _G_LUA_H

#include "q_shared.h"
#include "g_local.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define LUA_NUM_VM 16
#define LUA_MAX_FSIZE 1024*1024 // 1MB

typedef struct
{
	int			id;
	char		file_name[MAX_QPATH];
	char		mod_name[MAX_CVAR_VALUE_STRING];
	char		*code;
	int			code_size;
	int			err;
	lua_State 	*L;
} lua_vm_t;

extern lua_vm_t * lVM[LUA_NUM_VM];

// API
qboolean G_LuaInit();
qboolean G_LuaCall(lua_vm_t* vm, int nargs, int nresults);
qboolean G_LuaGetNamedFunction(lua_vm_t *vm, char *name);
qboolean G_LuaStartVM(lua_vm_t* vm);
void G_LuaStopVM(lua_vm_t* vm);
void G_LuaShutdown();
void G_LuaStatus(gentity_t *ent);
lua_vm_t * G_LuaGetVM(lua_State *L);

// Callbacks
void G_LuaHook_InitGame(int levelTime, int randomSeed, int restart);
void G_LuaHook_ShutdownGame(int restart);
void G_LuaHook_RunFrame(int levelTime);
//TODO: test all after this line
qboolean G_LuaHook_ClientConnect(int clientNum, qboolean firstTime, qboolean isBot, char *reason);
void G_LuaHook_ClientDisconnect(int clientNum);
void G_LuaHook_ClientBegin(int clientNum);
void G_LuaHook_ClientUserinfoChanged(int clientNum);
void G_LuaHook_ClientSpawn(int clientNum, qboolean revived, qboolean teamChange, qboolean restoreHealth);
qboolean G_LuaHook_ClientCommand(int clientNum, char *command);
qboolean G_LuaHook_ConsoleCommand(char *command);
qboolean G_LuaHook_UpgradeSkill(int cno, skillType_t skill);
// TODO: this REALLY needs testing!!! (I'm not sure about cno arg)
qboolean G_LuaHook_SetPlayerSkill( int cno, skillType_t skill ); 
void G_LuaHook_Print( char *text );
qboolean G_LuaHook_Obituary(int victim, int killer, int meansOfDeath, char *customObit);


#endif /* ifndef _G_LUA_H */

