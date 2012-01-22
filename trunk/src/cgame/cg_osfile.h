/*
 * cg_osfile.h
 *
 * This code is taken from No Quarter. All credits go to their team especially Lucel!
 * http://shitstorm.org
 *
 * pheno: small modifications are done
 *
 */

#ifndef __CG_OSFILE_H__
#define __CG_OSFILE_H__

// Allow cross platform access to the CRT stdio in a uniform fashion.
//
// We only need this because some of the shared file code requies servers to be able to access
// files outside of the fs path. Keep all the stdio stuff in one place here to aid in porting.
//
// Lucel

#include "cg_local.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined WIN32
	
	#define Rectangle LCC_Rectangle
	#include <windows.h>
	#undef Rectangle
	#include <io.h>
	#define open _open
	#define close _close
	#define read _read
	#define write _write
	#define stat _stat
	#ifndef S_IRUSR
		#define S_IRUSR _S_IREAD
	#endif
	#ifndef S_IWUSR
		#define S_IWUSR _S_IWRITE
	#endif
	#define S_IFDIR _S_IFDIR
	#define MAX_PATH 260
	
#else
	
	#include <unistd.h>
	#include <dirent.h>
	#include <stdio.h>
	
	#if defined __linux__
		
		#define MAX_PATH 4096
		
	#elif defined __MACOS__
		
		#define MAX_PATH 1024
		
	#endif // __linux__
	
#endif // WIN32

// Fn_IterateDirectory: function pointer used in the G_IterateDirectory function.
// 	- Return qfalse to terminate processing
typedef qboolean(*Fn_IterateDirectory) (char const* filename, char const* fullpath, qboolean directory);

// Functions
extern char* CG_BuildFilePath(char const* path, char const* file, char const* ext, char* dest, int destsz);
//extern void G_IterateDirectory(char const* path, Fn_IterateDirectory handler);
extern int CG_WriteDataToFile(char const* path, char const* buf, int sz);
extern int CG_ReadDataFromFile(char const* path, char* buf, int sz);
extern qboolean CG_IsFile(char const* path);
extern qboolean CG_IsDirectory(char const* path);
extern qboolean CG_DeleteFile(char const* path);
extern qboolean CG_RenameFile(char const* src, char const* dest);

#endif  // __G_OSFILE_H__
