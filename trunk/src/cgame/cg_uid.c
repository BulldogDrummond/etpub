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




#define CURL_STATICLIB
#include "cg_local.h"
#include "md5.h"
#include "cg_osfile.h"
#include "../curl-7.12.2/include/curl/curl.h"
#include "../curl-7.12.2/include/curl/easy.h"


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

int hex2byte(unsigned char *dst,unsigned char *data, int len) {
    int     i,
            t;

    if(len < 0) len = strlen(data);
    len >>= 1;

    for(i = 0; i < len; i++) {
        sscanf(data + (i << 1), "%02x", &t);
        dst[i] = t;
    }
    return(i);
}



int byte2hex(unsigned char *dst, unsigned char *data, int len) {
    static const char   hex[] = "0123456789ABCDEF";
    int     i;

    for(i = 0; i < len; i++) {
        dst[i << 1]       = hex[data[i] >> 4];
        dst[(i << 1) + 1] = hex[data[i] & 15];
    }
    dst[i << 1] = 0;
    return(i);
}

void PB_GUIDFROMETKEY(unsigned char *res, unsigned char *key) {
	MD5_CTX     ctx;
	 MD5Init(&ctx, 0x00b684a3);
    MD5Update(&ctx, key, 18);
    MD5Final(&ctx);
    byte2hex(res, ctx.digest, 16);  // the md5 is performed on the hex string
    MD5Init(&ctx, 0x00051a56);
    MD5Update(&ctx, res, 32);
    MD5Final(&ctx);
    memcpy(res, ctx.digest, 16);
}

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written;
    written = fwrite(ptr, size, nmemb, stream);
    return written;
}
void GUID_test() {
	unsigned char      res[33];
	unsigned char uid[19] = "";
	unsigned char guid[33]= "";
	char homepath[MAX_PATH];
	static char	path[MAX_PATH];
	static char Data[67];

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
		G_BuildFilePath(homepath, "/etmain/etkey","", path, MAX_PATH);
		if(!G_IsFile(path)) {
			trap_Cvar_VariableStringBuffer("fs_homepath", homepath, sizeof(homepath));
			G_BuildFilePath(homepath, "/etmain/etkey","", path, MAX_PATH);
			if(!G_IsFile(path)) {					//no local etkey f ound, get one from etkey.org
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
		G_ReadDataFromFile(path, Data, sizeof(Data));
		memcpy(uid,Data + 10,18);
		uid[66] = 0;
		PB_GUIDFROMETKEY(res, uid);				//Compute GUID by md5 computation
		byte2hex(guid,res,16);
		trap_Cvar_Set("cl_guid",va("%s",guid));
	}
	trap_Cvar_VariableStringBuffer( "cl_guid", buff_tmp, sizeof( buff_tmp ) );
	CG_Printf("Actual client guid %s \n",buff_tmp);

}





