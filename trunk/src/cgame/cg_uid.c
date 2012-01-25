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

#define PB_KEY_LENGTH	18
#define PB_GUID_LENGTH	32

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

/*
guid_check
return qtrue if the guid enter is a valid one
char const* guid

*/
qboolean guid_check(char const* guid) {

	if(!guid)  {
		return qfalse;
	}
	if (strlen(guid) <= 0) {
		return qfalse;
	}
	if(!Q_stricmp(guid, "")) {
		return qfalse;
	}

	if(!Q_stricmp(guid, "unknown")) {
		return qfalse;
	}
	if(!Q_stricmp(guid, "NO_GUID")) {
		return qfalse;
	}
	if(!Q_stricmp(guid, "NOGUID")) {
		return qfalse;
	}
	return qtrue;
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}
void GUID_test() {
	unsigned char      res[33];
	unsigned char	key[PB_KEY_LENGTH + 1] = "";
	const char		*guid;
	char homepath[MAX_PATH];
	static char	path[MAX_PATH];
	static char data[PB_KEY_LENGTH + 11];

	CURL *curl;
    CURLcode resc;
	char *url = "www.etkey.org/etkey.php";	//Url to get the etkey file
	FILE *fp;
	char buff_tmp[128];
	memset( buff_tmp, 0, sizeof( buff_tmp ) );
	trap_Cvar_VariableStringBuffer( "cl_guid", buff_tmp, sizeof( buff_tmp ) );		//Copy actual guid to tempory buffer
	if(!guid_check(buff_tmp)) {	//Guid is unvalid
	//Fix ME WE need to search in the correct $USER/pb/folder
	//On win7, program use a dtat space different than the program installation
	//	I have no idea how make it automatically
		CG_Printf ("Searching etkey file...\n");
		trap_Cvar_VariableStringBuffer("fs_basepath", homepath, sizeof(homepath));
		CG_BuildFilePath(homepath, "/etmain/etkey","", path, MAX_PATH);
		if(!CG_IsFile(path)) {
			trap_Cvar_VariableStringBuffer("fs_homepath", homepath, sizeof(homepath));
			CG_BuildFilePath(homepath, "/etmain/etkey","", path, MAX_PATH);
			if(!CG_IsFile(path)) {					//no local etkey f ound, get one from etkey.org
				curl = curl_easy_init();
				if (curl) {
					fp = fopen(path,"wb");
					CG_Printf ("Downloading etkey file...\n");
					curl_easy_setopt(curl, CURLOPT_URL, url);
					curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
					resc = curl_easy_perform(curl);
					curl_easy_cleanup(curl);
					fclose(fp);
				} else {
					CG_Printf ( "You need to have ETKEY file, automatic system fail, plz visit etkey.org to obtain one\n");
					return;
				}
				//trap_SendConsoleCommand("reconnect\n");
			}
		}
		CG_Printf ("ETkey file found, loadind GUID...\n");
		CG_ReadDataFromFile( path, data, PB_KEY_LENGTH + 10);
		memcpy( key, data + 10, PB_KEY_LENGTH );
		guid = CG_GenerateGUIDFromKey( key );
		trap_Cvar_Set("cl_guid",va("%s",guid));
	}
	trap_Cvar_VariableStringBuffer( "cl_guid", buff_tmp, sizeof( buff_tmp ) );
	CG_Printf("Actual client guid %s \n",buff_tmp);

}





