/*
  Automatic give guid file

This file by calling GUID_test(),
cl_guid cvars is verified,
if the case of content is unvalid,
	an etkey file is search on the disk
	if an etkey file is not found a new one is downlaoded from etkey.org site
    a guid is computed and the cl_guid cvar is updated

In all case, the computed guid is pb one like (computed in the same way)
 Y are free to use and modify this code
 Do not alter this notice
!Grats to schnogg for give the way to found info about guid compuattion
!Grats to 7killer to have code that

*/

#include "cg_local.h"
#include "cg_osfile.h"

#include "md5.h"

#include "curl/curl.h"
#include "curl/easy.h"

#define PB_KEY_LENGTH 18

// pheno: PunkBuster compatible MD5 hash algorithm
unsigned char *CG_PBCompatibleMD5( unsigned char *data, int len, int seed )
{
	MD5_CTX						ctx;
	unsigned char				*p;
	int							i;
	static unsigned char		hash[PB_GUID_LENGTH + 1];
	static const unsigned char	hex[] = "0123456789abcdef";

	MD5Init( &ctx, seed );
	MD5Update( &ctx, data, len );
	MD5Final( &ctx );

	p = hash;

	for( i = 0; i < 16; i++ ) {
		*p++ = hex[ctx.digest[i] >> 4];
		*p++ = hex[ctx.digest[i] & 15];
	}

	*p = 0;

	return hash;
}

// pheno: returns exact the same GUID like PunkBuster does
const char *CG_GenerateGUIDFromKey( unsigned char *key )
{
	unsigned char	*hash;
	int				i;

	hash = CG_PBCompatibleMD5( key, PB_KEY_LENGTH, 0x00b684a3 );
	hash = CG_PBCompatibleMD5( hash, PB_GUID_LENGTH, 0x00051a56 );

	// hash is lowercased after md5sums, we must to change case to upper
	for( i = 0; hash[i]; i++ ) {
		hash[i] = toupper( hash[i] );
	}

	return ( const char * )hash;
}

// returns qtrue if the given guid is a valid one
qboolean CG_IsValidGUID( char *guid )
{
	int i;

	if( !guid )  {
		return qfalse;
	}

	if( !Q_stricmp( guid, "" ) ||
		!Q_stricmp( guid, "unknown" ) ||
		!Q_stricmp( guid, "NO_GUID" ) ) {
		return qfalse;
	}

	if( strlen( guid ) <= 0 || strlen( guid ) > PB_GUID_LENGTH ) {
		return qfalse;
	}

	for( i = 0 ; i < PB_GUID_LENGTH ; i++ ) {
		if( guid[i] < 48 || ( guid[i] > 57 && guid[i] < 65 ) || guid[i] > 70 ) {
			return qfalse;
		}
	}

	return qtrue;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void GUID_test()
{
	unsigned char	key[PB_KEY_LENGTH + 1] = "";
	const char		*guid;
	char homepath[MAX_PATH],
		path[MAX_PATH],
		buf[PB_KEY_LENGTH + 11];
#ifdef WIN32
	OSVERSIONINFO	osvi;
	char winpath[MAX_PATH];
#endif // WIN32

	CURL *curl;
	char *url = "www.etkey.org/etkey.php";	//Url to get the etkey file
	FILE *fp;
	char buff_tmp[128];
	memset( buff_tmp, 0, sizeof( buff_tmp ) );
	trap_Cvar_VariableStringBuffer( "cl_guid", buff_tmp, sizeof( buff_tmp ) );		//Copy actual guid to tempory buffer

	if(!CG_IsValidGUID(buff_tmp)) {
		CG_Printf ("Searching etkey file...\n");
		trap_Cvar_VariableStringBuffer("fs_homepath", homepath, sizeof(homepath));
		CG_BuildFilePath(homepath, "/etmain/etkey","", path, MAX_PATH);

#ifdef WIN32
		if(!CG_IsFile(path)) {
			osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
			GetVersionEx( &osvi );

			if( osvi.dwMajorVersion == 6 ) { // Windows Vista, Windows Server 2008 and Windows 7
				CG_BuildFilePath( va( "%s\\AppData\\Local\\PunkBuster\\ET\\etmain",
					getenv( "USERPROFILE" ) ), "etkey", "", winpath, MAX_PATH );
				if(CG_IsFile(winpath)) {
					//if the file exists, use winpath as path, else leave it
					memcpy(path, winpath, MAX_PATH);
				}
			}
		}
#endif // WIN32

		if(!CG_IsFile(path)) {
			//open for writing
			fp = fopen(path,"wb");

#ifdef WIN32
			//no write access, so try winpath
			if(fp == NULL) {
				memcpy(path, winpath, MAX_PATH);
				fp = fopen(path,"wb");
			}
#endif // WIN32

			//no need to download if you cant write data
			if(fp != NULL) {
				curl = curl_easy_init();
				if (curl) {
						CG_Printf ("Downloading etkey file...\n");
						curl_easy_setopt(curl, CURLOPT_URL, url);
						curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
						curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
						curl_easy_perform(curl);
						curl_easy_cleanup(curl);
						fclose(fp);
				}
			}
		}
		if(CG_IsFile(path)) {
			CG_Printf ("ETkey file found, loadind GUID...\n");
			CG_ReadDataFromFile( path, buf, PB_KEY_LENGTH + 10);
			memcpy( key, buf + 10, PB_KEY_LENGTH );
			guid = CG_GenerateGUIDFromKey( key );
			trap_Cvar_Set("cl_guid",va("%s",guid));
		} else {
			CG_Printf ( "You need an etkey... Automatic download system failed. Visit etkey.org to obtain one!\n");
			return;
		}
	} else {
		trap_Cvar_VariableStringBuffer( "cl_guid", buff_tmp, sizeof( buff_tmp ) );
		CG_Printf("Actual client guid %s \n",buff_tmp);
	}
}





