/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2006-2009 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "client.h"

#if defined(USE_JAVA)

#include "../qcommon/vm_java.h"
//#include "java/xreal_client_Client.h"

/*
====================
GetClientState
====================
*/
static void GetClientState(uiClientState_t * state)
{
	state->connectPacketCount = clc.connectPacketCount;
	state->connState = cls.state;
	Q_strncpyz(state->servername, cls.servername, sizeof(state->servername));
	Q_strncpyz(state->updateInfoString, cls.updateInfoString, sizeof(state->updateInfoString));
	Q_strncpyz(state->messageString, clc.serverMessage, sizeof(state->messageString));
	state->clientNum = cl.snap.ps.clientNum;
}

/*
====================
LAN_LoadCachedServers
====================
*/
void LAN_LoadCachedServers(void)
{
	int             size;
	fileHandle_t    fileIn;

	cls.numglobalservers = cls.numfavoriteservers = 0;
	cls.numGlobalServerAddresses = 0;
	if(FS_SV_FOpenFileRead("servercache.dat", &fileIn))
	{
		FS_Read(&cls.numglobalservers, sizeof(int), fileIn);
		FS_Read(&cls.numfavoriteservers, sizeof(int), fileIn);
		FS_Read(&size, sizeof(int), fileIn);
		if(size == sizeof(cls.globalServers) + sizeof(cls.favoriteServers))
		{
			FS_Read(&cls.globalServers, sizeof(cls.globalServers), fileIn);
			FS_Read(&cls.favoriteServers, sizeof(cls.favoriteServers), fileIn);
		}
		else
		{
			cls.numglobalservers = cls.numfavoriteservers = 0;
			cls.numGlobalServerAddresses = 0;
		}
		FS_FCloseFile(fileIn);
	}
}

/*
====================
LAN_SaveServersToCache
====================
*/
void LAN_SaveServersToCache(void)
{
	int             size;
	fileHandle_t    fileOut = FS_SV_FOpenFileWrite("servercache.dat");
	FS_Write(&cls.numglobalservers, sizeof(int), fileOut);
	FS_Write(&cls.numfavoriteservers, sizeof(int), fileOut);
	size = sizeof(cls.globalServers) + sizeof(cls.favoriteServers);
	FS_Write(&size, sizeof(int), fileOut);
	FS_Write(&cls.globalServers, sizeof(cls.globalServers), fileOut);
	FS_Write(&cls.favoriteServers, sizeof(cls.favoriteServers), fileOut);
	FS_FCloseFile(fileOut);
}


/*
====================
LAN_ResetPings
====================
*/
static void LAN_ResetPings(int source)
{
	int             count, i;
	serverInfo_t   *servers = NULL;

	count = 0;

	switch (source)
	{
		case AS_LOCAL:
			servers = &cls.localServers[0];
			count = MAX_OTHER_SERVERS;
			break;
		case AS_MPLAYER:
		case AS_GLOBAL:
			servers = &cls.globalServers[0];
			count = MAX_GLOBAL_SERVERS;
			break;
		case AS_FAVORITES:
			servers = &cls.favoriteServers[0];
			count = MAX_OTHER_SERVERS;
			break;
	}
	if(servers)
	{
		for(i = 0; i < count; i++)
		{
			servers[i].ping = -1;
		}
	}
}

/*
====================
LAN_AddServer
====================
*/
static int LAN_AddServer(int source, const char *name, const char *address)
{
	int             max, *count, i;
	netadr_t        adr;
	serverInfo_t   *servers = NULL;

	max = MAX_OTHER_SERVERS;
	count = NULL;

	switch (source)
	{
		case AS_LOCAL:
			count = &cls.numlocalservers;
			servers = &cls.localServers[0];
			break;
		case AS_MPLAYER:
		case AS_GLOBAL:
			max = MAX_GLOBAL_SERVERS;
			count = &cls.numglobalservers;
			servers = &cls.globalServers[0];
			break;
		case AS_FAVORITES:
			count = &cls.numfavoriteservers;
			servers = &cls.favoriteServers[0];
			break;
	}
	if(servers && *count < max)
	{
		NET_StringToAdr(address, &adr, NA_IP);
		for(i = 0; i < *count; i++)
		{
			if(NET_CompareAdr(servers[i].adr, adr))
			{
				break;
			}
		}
		if(i >= *count)
		{
			servers[*count].adr = adr;
			Q_strncpyz(servers[*count].hostName, name, sizeof(servers[*count].hostName));
			servers[*count].visible = qtrue;
			(*count)++;
			return 1;
		}
		return 0;
	}
	return -1;
}

/*
====================
LAN_RemoveServer
====================
*/
static void LAN_RemoveServer(int source, const char *addr)
{
	int            *count, i;
	serverInfo_t   *servers = NULL;

	count = NULL;
	switch (source)
	{
		case AS_LOCAL:
			count = &cls.numlocalservers;
			servers = &cls.localServers[0];
			break;
		case AS_MPLAYER:
		case AS_GLOBAL:
			count = &cls.numglobalservers;
			servers = &cls.globalServers[0];
			break;
		case AS_FAVORITES:
			count = &cls.numfavoriteservers;
			servers = &cls.favoriteServers[0];
			break;
	}
	if(servers)
	{
		netadr_t        comp;

		NET_StringToAdr(addr, &comp, NA_IP);
		for(i = 0; i < *count; i++)
		{
			if(NET_CompareAdr(comp, servers[i].adr))
			{
				int             j = i;

				while(j < *count - 1)
				{
					Com_Memcpy(&servers[j], &servers[j + 1], sizeof(servers[j]));
					j++;
				}
				(*count)--;
				break;
			}
		}
	}
}


/*
====================
LAN_GetServerCount
====================
*/
static int LAN_GetServerCount(int source)
{
	switch (source)
	{
		case AS_LOCAL:
			return cls.numlocalservers;
			break;
		case AS_MPLAYER:
		case AS_GLOBAL:
			return cls.numglobalservers;
			break;
		case AS_FAVORITES:
			return cls.numfavoriteservers;
			break;
	}
	return 0;
}

/*
====================
LAN_GetLocalServerAddressString
====================
*/
static void LAN_GetServerAddressString(int source, int n, char *buf, int buflen)
{
	switch (source)
	{
		case AS_LOCAL:
			if(n >= 0 && n < MAX_OTHER_SERVERS)
			{
				Q_strncpyz(buf, NET_AdrToStringwPort(cls.localServers[n].adr), buflen);
				return;
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL:
			if(n >= 0 && n < MAX_GLOBAL_SERVERS)
			{
				Q_strncpyz(buf, NET_AdrToStringwPort(cls.globalServers[n].adr), buflen);
				return;
			}
			break;
		case AS_FAVORITES:
			if(n >= 0 && n < MAX_OTHER_SERVERS)
			{
				Q_strncpyz(buf, NET_AdrToStringwPort(cls.favoriteServers[n].adr), buflen);
				return;
			}
			break;
	}
	buf[0] = '\0';
}

/*
====================
LAN_GetServerInfo
====================
*/
static void LAN_GetServerInfo(int source, int n, char *buf, int buflen)
{
	char            info[MAX_STRING_CHARS];
	serverInfo_t   *server = NULL;

	info[0] = '\0';
	switch (source)
	{
		case AS_LOCAL:
			if(n >= 0 && n < MAX_OTHER_SERVERS)
			{
				server = &cls.localServers[n];
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL:
			if(n >= 0 && n < MAX_GLOBAL_SERVERS)
			{
				server = &cls.globalServers[n];
			}
			break;
		case AS_FAVORITES:
			if(n >= 0 && n < MAX_OTHER_SERVERS)
			{
				server = &cls.favoriteServers[n];
			}
			break;
	}
	if(server && buf)
	{
		buf[0] = '\0';
		Info_SetValueForKey(info, "hostname", server->hostName);
		Info_SetValueForKey(info, "mapname", server->mapName);
		Info_SetValueForKey(info, "clients", va("%i", server->clients));
		Info_SetValueForKey(info, "sv_maxclients", va("%i", server->maxClients));
		Info_SetValueForKey(info, "ping", va("%i", server->ping));
		Info_SetValueForKey(info, "minping", va("%i", server->minPing));
		Info_SetValueForKey(info, "maxping", va("%i", server->maxPing));
		Info_SetValueForKey(info, "game", server->game);
		Info_SetValueForKey(info, "gametype", va("%i", server->gameType));
		Info_SetValueForKey(info, "nettype", va("%i", server->netType));
		Info_SetValueForKey(info, "addr", NET_AdrToString(server->adr));
		Q_strncpyz(buf, info, buflen);
	}
	else
	{
		if(buf)
		{
			buf[0] = '\0';
		}
	}
}

/*
====================
LAN_GetServerPing
====================
*/
static int LAN_GetServerPing(int source, int n)
{
	serverInfo_t   *server = NULL;

	switch (source)
	{
		case AS_LOCAL:
			if(n >= 0 && n < MAX_OTHER_SERVERS)
			{
				server = &cls.localServers[n];
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL:
			if(n >= 0 && n < MAX_GLOBAL_SERVERS)
			{
				server = &cls.globalServers[n];
			}
			break;
		case AS_FAVORITES:
			if(n >= 0 && n < MAX_OTHER_SERVERS)
			{
				server = &cls.favoriteServers[n];
			}
			break;
	}
	if(server)
	{
		return server->ping;
	}
	return -1;
}

/*
====================
LAN_GetServerPtr
====================
*/
static serverInfo_t *LAN_GetServerPtr(int source, int n)
{
	switch (source)
	{
		case AS_LOCAL:
			if(n >= 0 && n < MAX_OTHER_SERVERS)
			{
				return &cls.localServers[n];
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL:
			if(n >= 0 && n < MAX_GLOBAL_SERVERS)
			{
				return &cls.globalServers[n];
			}
			break;
		case AS_FAVORITES:
			if(n >= 0 && n < MAX_OTHER_SERVERS)
			{
				return &cls.favoriteServers[n];
			}
			break;
	}
	return NULL;
}

/*
====================
LAN_CompareServers
====================
*/
static int LAN_CompareServers(int source, int sortKey, int sortDir, int s1, int s2)
{
	int             res;
	serverInfo_t   *server1, *server2;

	server1 = LAN_GetServerPtr(source, s1);
	server2 = LAN_GetServerPtr(source, s2);
	if(!server1 || !server2)
	{
		return 0;
	}

	res = 0;
	switch (sortKey)
	{
		case SORT_HOST:
			res = Q_stricmp(server1->hostName, server2->hostName);
			break;

		case SORT_MAP:
			res = Q_stricmp(server1->mapName, server2->mapName);
			break;
		case SORT_CLIENTS:
			if(server1->clients < server2->clients)
			{
				res = -1;
			}
			else if(server1->clients > server2->clients)
			{
				res = 1;
			}
			else
			{
				res = 0;
			}
			break;
		case SORT_GAME:
			if(server1->gameType < server2->gameType)
			{
				res = -1;
			}
			else if(server1->gameType > server2->gameType)
			{
				res = 1;
			}
			else
			{
				res = 0;
			}
			break;
		case SORT_PING:
			if(server1->ping < server2->ping)
			{
				res = -1;
			}
			else if(server1->ping > server2->ping)
			{
				res = 1;
			}
			else
			{
				res = 0;
			}
			break;
	}

	if(sortDir)
	{
		if(res < 0)
			return 1;
		if(res > 0)
			return -1;
		return 0;
	}
	return res;
}

/*
====================
LAN_GetPingQueueCount
====================
*/
static int LAN_GetPingQueueCount(void)
{
	return (CL_GetPingQueueCount());
}

/*
====================
LAN_ClearPing
====================
*/
static void LAN_ClearPing(int n)
{
	CL_ClearPing(n);
}

/*
====================
LAN_GetPing
====================
*/
static void LAN_GetPing(int n, char *buf, int buflen, int *pingtime)
{
	CL_GetPing(n, buf, buflen, pingtime);
}

/*
====================
LAN_GetPingInfo
====================
*/
static void LAN_GetPingInfo(int n, char *buf, int buflen)
{
	CL_GetPingInfo(n, buf, buflen);
}

/*
====================
LAN_MarkServerVisible
====================
*/
static void LAN_MarkServerVisible(int source, int n, qboolean visible)
{
	if(n == -1)
	{
		int             count = MAX_OTHER_SERVERS;
		serverInfo_t   *server = NULL;

		switch (source)
		{
			case AS_LOCAL:
				server = &cls.localServers[0];
				break;
			case AS_MPLAYER:
			case AS_GLOBAL:
				server = &cls.globalServers[0];
				count = MAX_GLOBAL_SERVERS;
				break;
			case AS_FAVORITES:
				server = &cls.favoriteServers[0];
				break;
		}
		if(server)
		{
			for(n = 0; n < count; n++)
			{
				server[n].visible = visible;
			}
		}

	}
	else
	{
		switch (source)
		{
			case AS_LOCAL:
				if(n >= 0 && n < MAX_OTHER_SERVERS)
				{
					cls.localServers[n].visible = visible;
				}
				break;
			case AS_MPLAYER:
			case AS_GLOBAL:
				if(n >= 0 && n < MAX_GLOBAL_SERVERS)
				{
					cls.globalServers[n].visible = visible;
				}
				break;
			case AS_FAVORITES:
				if(n >= 0 && n < MAX_OTHER_SERVERS)
				{
					cls.favoriteServers[n].visible = visible;
				}
				break;
		}
	}
}


/*
=======================
LAN_ServerIsVisible
=======================
*/
static int LAN_ServerIsVisible(int source, int n)
{
	switch (source)
	{
		case AS_LOCAL:
			if(n >= 0 && n < MAX_OTHER_SERVERS)
			{
				return cls.localServers[n].visible;
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL:
			if(n >= 0 && n < MAX_GLOBAL_SERVERS)
			{
				return cls.globalServers[n].visible;
			}
			break;
		case AS_FAVORITES:
			if(n >= 0 && n < MAX_OTHER_SERVERS)
			{
				return cls.favoriteServers[n].visible;
			}
			break;
	}
	return qfalse;
}

/*
=======================
LAN_UpdateVisiblePings
=======================
*/
qboolean LAN_UpdateVisiblePings(int source)
{
	return CL_UpdateVisiblePings_f(source);
}

/*
====================
LAN_GetServerStatus
====================
*/
int LAN_GetServerStatus(char *serverAddress, char *serverStatus, int maxLen)
{
	return CL_ServerStatus(serverAddress, serverStatus, maxLen);
}

/*
====================
CL_GetClipboardData
====================
*/
static void CL_GetClipboardData(char *buf, int buflen)
{
	char           *cbd;

	cbd = Sys_GetClipboardData();

	if(!cbd)
	{
		*buf = 0;
		return;
	}

	Q_strncpyz(buf, cbd, buflen);

	Z_Free(cbd);
}






// ====================================================================================

/*
 * Class:     xreal_client_Client
 * Method:    getConfigString
 * Signature: (I)Ljava/lang/String;
 */
jstring JNICALL Java_xreal_client_Client_getConfigString(JNIEnv *env, jclass cls, jint index)
{
	int             offset;
	char			buf[MAX_INFO_STRING];

	if(index < 0 || index >= MAX_CONFIGSTRINGS)
		return NULL;

	offset = cl.gameState.stringOffsets[index];
	if(!offset)
	{
		return NULL;
	}

	Q_strncpyz(buf, cl.gameState.stringData + offset, sizeof(buf));

	return (*env)->NewStringUTF(env, buf);
}

/*
 * Class:     xreal_client_Client
 * Method:    getKeyCatchers
 * Signature: ()I
 */
jint JNICALL Java_xreal_client_Client_getKeyCatchers(JNIEnv *env, jclass cls)
{
	return Key_GetCatcher();
}

/*
 * Class:     xreal_client_Client
 * Method:    setKeyCatchers
 * Signature: (I)V
 */
void JNICALL Java_xreal_client_Client_setKeyCatchers(JNIEnv *env, jclass cls, jint catchers)
{
	// Don't allow the modules to close the console
	Key_SetCatcher(catchers | (Key_GetCatcher() & KEYCATCH_CONSOLE));
}

/*
 * Class:     xreal_client_Client
 * Method:    getKeyBinding
 * Signature: (I)Ljava/lang/String;
 */
jstring JNICALL Java_xreal_client_Client_getKeyBinding(JNIEnv *env, jclass cls, jint keynum)
{
	char           *value;

	value = Key_GetBinding(keynum);
	if(value && *value)
	{
		return (*env)->NewStringUTF(env, value);
	}

	return NULL;
}

/*
 * Class:     xreal_client_Client
 * Method:    setKeyBinding
 * Signature: (ILjava/lang/String;)V
 */
void JNICALL Java_xreal_client_Client_setKeyBinding(JNIEnv *env, jclass cls, jint keynum, jstring jbinding)
{
	char           *binding;

	binding = (char *)((*env)->GetStringUTFChars(env, jbinding, 0));

	Key_SetBinding(keynum, binding);

	(*env)->ReleaseStringUTFChars(env, jbinding, binding);
}

/*
 * Class:     xreal_client_Client
 * Method:    isKeyDown
 * Signature: (I)Z
 */
jboolean JNICALL Java_xreal_client_Client_isKeyDown(JNIEnv *env, jclass cls, jint keynum)
{
	return Key_IsDown(keynum);
}

/*
 * Class:     xreal_client_Client
 * Method:    clearKeyStates
 * Signature: ()V
 */
void JNICALL Java_xreal_client_Client_clearKeyStates(JNIEnv *env, jclass cls)
{
	Key_ClearStates();
}

/*
 * Class:     xreal_client_Client
 * Method:    registerSound
 * Signature: (Ljava/lang/String;)I
 */
jint JNICALL Java_xreal_client_Client_registerSound(JNIEnv *env, jclass cls, jstring jname)
{
	char           *name;
	sfxHandle_t		handle;

	name = (char *)((*env)->GetStringUTFChars(env, jname, 0));

	handle = S_RegisterSound(name);

	(*env)->ReleaseStringUTFChars(env, jname, name);

	return handle;
}

/*
 * Class:     xreal_client_Client
 * Method:    startSound
 * Signature: (FFFIII)V
 */
// void startSound(float posX, float posY, float posZ, int entityNum, int entityChannel, int sfxHandle);
void JNICALL Java_xreal_client_Client_startSound(JNIEnv *env, jclass cls, jfloat posX, jfloat posY, jfloat posZ, jint entityNum, jint entityChannel, jint sfxHandle)
{
	vec3_t position;

	VectorSet(position, posX, posY, posZ);

	S_StartSound(position, entityNum, entityChannel, sfxHandle);
}

/*
 * Class:     xreal_client_Client
 * Method:    startLocalSound
 * Signature: (II)V
 */
void JNICALL Java_xreal_client_Client_startLocalSound(JNIEnv *env, jclass cls, jint sfxHandle, jint channelNum)
{
	S_StartLocalSound(sfxHandle, channelNum);
}

/*
 * Class:     xreal_client_Client
 * Method:    addLoopingSound
 * Signature: (IFFFFFFI)V
 */
// void addLoopingSound(int entityNum, float posX, float posY, float posZ, float velX, float velY, float velZ, int sfxHandle);
void JNICALL Java_xreal_client_Client_addLoopingSound(JNIEnv *env, jclass cls, jint entityNum, jfloat posX, jfloat posY, jfloat posZ, jfloat velX, jfloat velY, jfloat velZ, jint sfxHandle)
{
	vec3_t position;
	vec3_t velocity;

	VectorSet(position, posX, posY, posZ);
	VectorSet(velocity, velX, velY, velZ);

	S_AddLoopingSound(entityNum, position, velocity, sfxHandle);
}

/*
 * Class:     xreal_client_Client
 * Method:    addRealLoopingSound
 * Signature: (IFFFFFFI)V
 */
// void addRealLoopingSound(int entityNum, float posX, float posY, float posZ, float velX, float velY, float velZ, int sfxHandle);
void JNICALL Java_xreal_client_Client_addRealLoopingSound(JNIEnv *env, jclass cls, jint entityNum, jfloat posX, jfloat posY, jfloat posZ, jfloat velX, jfloat velY, jfloat velZ, jint sfxHandle)
{
	vec3_t position;
	vec3_t velocity;

	VectorSet(position, posX, posY, posZ);
	VectorSet(velocity, velX, velY, velZ);

	S_AddRealLoopingSound(entityNum, position, velocity, sfxHandle);
}

/*
 * Class:     xreal_client_Client
 * Method:    updateEntitySoundPosition
 * Signature: (IFFF)V
 */
// void updateEntitySoundPosition(int entityNum, float posX, float posY, float posZ);
void JNICALL Java_xreal_client_Client_updateEntitySoundPosition(JNIEnv *env, jclass cls, jint entityNum, jfloat posX, jfloat posY, jfloat posZ)
{
	vec3_t position;

	VectorSet(position, posX, posY, posZ);

	S_UpdateEntityPosition(entityNum, position);
}

/*
 * Class:     xreal_client_Client
 * Method:    stopLoopingSound
 * Signature: (I)V
 */
void JNICALL Java_xreal_client_Client_stopLoopingSound(JNIEnv *env, jclass cls, jint entityNum)
{
	S_StopLoopingSound(entityNum);
}

/*
 * Class:     xreal_client_Client
 * Method:    clearLoopingSounds
 * Signature: (Z)V
 */
void JNICALL Java_xreal_client_Client_clearLoopingSounds(JNIEnv *env, jclass cls, jboolean killall)
{
	S_ClearLoopingSounds(killall);
}

/*
 * Class:     xreal_client_Client
 * Method:    respatialize
 * Signature: (IFFFFFFFZ)V
 */
// void respatialize(int entityNum, float posX, float posY, float posZ, float quatX, float quatY, float quatZ, float quatW, boolean inWater);
void JNICALL Java_xreal_client_Client_respatialize(JNIEnv *env, jclass cls, jint entityNum, jfloat posX, jfloat posY, jfloat posZ, jfloat quatX, jfloat quatY, jfloat quatZ, jfloat quatW, jboolean inWater)
{
	vec3_t position;
	quat_t quat;
	axis_t axis;

	VectorSet(position, posX, posY, posZ);
	QuatSet(quat, quatX, quatY, quatZ, quatZ);

	QuatToAxis(quat, axis);

	S_Respatialize(entityNum, position, axis, inWater);
}

/*
 * Class:     xreal_client_Client
 * Method:    startBackgroundTrack
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
void JNICALL Java_xreal_client_Client_startBackgroundTrack(JNIEnv *env, jclass cls, jstring jintro, jstring jloop)
{
	char           *intro;
	char           *loop;

	intro = (char *)((*env)->GetStringUTFChars(env, jintro, 0));
	loop = (char *)((*env)->GetStringUTFChars(env, jloop, 0));

	S_StartBackgroundTrack(intro, loop);

	(*env)->ReleaseStringUTFChars(env, jintro, intro);
	(*env)->ReleaseStringUTFChars(env, jloop, loop);
}

/*
 * Class:     xreal_client_Client
 * Method:    stopBackgroundTrack
 * Signature: ()V
 */
void JNICALL Java_xreal_client_Client_stopBackgroundTrack(JNIEnv *env, jclass cls)
{
	S_StopBackgroundTrack();
}

// handle to Client class
static jclass   class_Client = NULL;
static JNINativeMethod Client_methods[] = {
	{"getConfigString", "(I)Ljava/lang/String;", Java_xreal_client_Client_getConfigString},

	{"getKeyCatchers", "()I", Java_xreal_client_Client_getKeyCatchers},
	{"setKeyCatchers", "(I)V", Java_xreal_client_Client_setKeyCatchers},
	{"getKeyBinding", "(I)Ljava/lang/String;", Java_xreal_client_Client_getKeyBinding},
	{"setKeyBinding", "(ILjava/lang/String;)V", Java_xreal_client_Client_setKeyBinding},
	{"isKeyDown", "(I)Z", Java_xreal_client_Client_isKeyDown},
	{"clearKeyStates", "()V", Java_xreal_client_Client_clearKeyStates},

	{"registerSound", "(Ljava/lang/String;)I", Java_xreal_client_Client_registerSound},
	{"startSound", "(FFFIII)V", Java_xreal_client_Client_startSound},
	{"startLocalSound", "(II)V", Java_xreal_client_Client_startLocalSound},
	{"addLoopingSound", "(IFFFFFFI)V", Java_xreal_client_Client_addLoopingSound},
	{"addRealLoopingSound", "(IFFFFFFI)V", Java_xreal_client_Client_addRealLoopingSound},
	{"updateEntitySoundPosition", "(IFFF)V", Java_xreal_client_Client_updateEntitySoundPosition},
	{"stopLoopingSound", "(I)V", Java_xreal_client_Client_stopLoopingSound},
	{"clearLoopingSounds", "(Z)V", Java_xreal_client_Client_clearLoopingSounds},
	{"respatialize", "(IFFFFFFFZ)V", Java_xreal_client_Client_respatialize},
	{"startBackgroundTrack", "(Ljava/lang/String;Ljava/lang/String;)V", Java_xreal_client_Client_startBackgroundTrack},
	{"stopBackgroundTrack", "()V", Java_xreal_client_Client_stopBackgroundTrack},
};

void Client_javaRegister()
{
	Com_Printf("Client_javaRegister()\n");

	class_Client = (*javaEnv)->FindClass(javaEnv, "xreal/client/Client");
	if(CheckException() || !class_Client)
	{
		Com_Error(ERR_FATAL, "Couldn't find xreal.client.Client");
	}

	(*javaEnv)->RegisterNatives(javaEnv, class_Client, Client_methods, sizeof(Client_methods) / sizeof(Client_methods[0]));
	if(CheckException())
	{
		Com_Error(ERR_FATAL, "Couldn't register native methods for xreal.client.Client");
	}
}


void Client_javaDetach()
{
	Com_Printf("Client_javaDetach()\n");

	if(javaEnv)
	{
		if(class_Client)
		{
			(*javaEnv)->UnregisterNatives(javaEnv, class_Client);
			(*javaEnv)->DeleteLocalRef(javaEnv, class_Client);
			class_Client = NULL;
		}
	}
}


// ====================================================================================


static jclass class_Glyph = NULL;
static jmethodID method_Glyph_ctor = NULL;

static jclass   class_Font = NULL;
static jmethodID method_Font_ctor = NULL;

void Font_javaRegister()
{
	class_Glyph = (*javaEnv)->FindClass(javaEnv, "xreal/client/renderer/Glyph");
	if(CheckException() || !class_Glyph)
	{
		Com_Error(ERR_FATAL, "Couldn't find xreal.client.renderer.Glyph");
	}

	/*
		public Glyph(int height, int top, int bottom, int pitch, int skip, int imageWidth, int imageHeight, float s, float t, float s2, float t2, int glyph,
				String materialName) {
	 */

	method_Glyph_ctor = (*javaEnv)->GetMethodID(javaEnv, class_Glyph, "<init>", "(IIIIIIIFFFFILjava/lang/String;)V");
	if(CheckException())
	{
		Com_Error(ERR_FATAL, "Couldn't find constructor of xreal.client.renderer.Glyph");
	}


	class_Font = (*javaEnv)->FindClass(javaEnv, "xreal/client/renderer/Font");
	if(CheckException() || !class_Font)
	{
		Com_Error(ERR_FATAL, "Couldn't find xreal.client.renderer.Font");
	}

	// public Font(Glyph[] glyphs, float glyphScale, String name)
	method_Font_ctor = (*javaEnv)->GetMethodID(javaEnv, class_Font, "<init>", "([Lxreal/client/renderer/Glyph;FLjava/lang/String;)V");

	if(CheckException())
	{
		Com_Error(ERR_FATAL, "Couldn't find constructor of xreal.client.renderer.Font");
	}
}

void Font_javaDetach()
{
	if(class_Glyph)
	{
		(*javaEnv)->DeleteLocalRef(javaEnv, class_Glyph);
		class_Glyph = NULL;
	}

	if(class_Font)
	{
		(*javaEnv)->DeleteLocalRef(javaEnv, class_Font);
		class_Font = NULL;
	}
}

jobject Java_NewGlyph(const glyphInfo_t * glyph)
{
	jobject obj = NULL;

	if(class_Glyph)
	{
		jstring name = (*javaEnv)->NewStringUTF(javaEnv, glyph->shaderName);

		/*
		public Glyph(int height, int top, int bottom, int pitch, int skip, int imageWidth, int imageHeight, float s, float t, float s2, float t2, int glyph,
					String materialName) {
		*/
		obj = (*javaEnv)->NewObject(javaEnv, class_Glyph, method_Glyph_ctor,
				glyph->height,
				glyph->top,
				glyph->bottom,
				glyph->pitch,
				glyph->xSkip,
				glyph->imageWidth,
				glyph->imageHeight,
				glyph->s,
				glyph->t,
				glyph->s2,
				glyph->t2,
				glyph->glyph,
				name);
	}

	return obj;
}

jobject Java_NewFont(const fontInfo_t * font)
{
	jobject obj = NULL;

	if(class_Glyph && class_Font)
	{
		int i;
		jobjectArray glyphsArray;
		jstring name;

		glyphsArray = (*javaEnv)->NewObjectArray(javaEnv, GLYPHS_PER_FONT, class_Glyph, NULL);

		for(i = 0; i < GLYPHS_PER_FONT; i++) {
			jobject glyph = Java_NewGlyph(&font->glyphs[i]);

			(*javaEnv)->SetObjectArrayElement(javaEnv, glyphsArray, i, glyph);
		}

		name = (*javaEnv)->NewStringUTF(javaEnv, font->name);

		/*
		public Font(Glyph[] glyphs, float glyphScale, String name) {
		*/
		obj = (*javaEnv)->NewObject(javaEnv, class_Font, method_Font_ctor,
				glyphsArray,
				font->glyphScale,
				name);

		(*javaEnv)->DeleteLocalRef(javaEnv, glyphsArray);

		CheckException();
	}

	return obj;
}

// ====================================================================================



/*
 * Class:     xreal_client_renderer_Renderer
 * Method:    setColor
 * Signature: (FFFF)V
 */
void JNICALL Java_xreal_client_renderer_Renderer_setColor(JNIEnv *env, jclass cls, jfloat red, jfloat green, jfloat blue, jfloat alpha)
{
	vec4_t color;

	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	re.SetColor(color);
}

/*
 * Class:     xreal_client_renderer_Renderer
 * Method:    drawStretchPic
 * Signature: (FFFFFFFFI)V
 */
//drawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, int hShader);
void JNICALL Java_xreal_client_renderer_Renderer_drawStretchPic(JNIEnv *env, jclass cls, jfloat x, jfloat y, jfloat w, jfloat h, jfloat s1, jfloat t1, jfloat s2, jfloat t2, jint hMaterial)
{
	re.DrawStretchPic(x, y, w, h, s1, t1, s2, t2, hMaterial);
}

/*
 * Class:     xreal_client_renderer_Renderer
 * Method:    registerFont
 * Signature: (Ljava/lang/String;I)Lxreal/client/renderer/Font;
 */
jobject JNICALL Java_xreal_client_renderer_Renderer_registerFont(JNIEnv *env, jclass cls, jstring jname, jint pointSize)
{
	fontInfo_t      font;
	char           *name;

	name = (char *)((*env)->GetStringUTFChars(env, jname, 0));

	re.RegisterFont(name, pointSize, &font);

	(*env)->ReleaseStringUTFChars(env, jname, name);

	return Java_NewFont(&font);
}

/*
 * Class:     xreal_client_renderer_Renderer
 * Method:    registerMaterial
 * Signature: (Ljava/lang/String;)I
 */
jint JNICALL Java_xreal_client_renderer_Renderer_registerMaterial(JNIEnv *env, jclass cls, jstring jname)
{
	char           *name;
	qhandle_t		handle;

	name = (char *)((*env)->GetStringUTFChars(env, jname, 0));

	handle = re.RegisterShader(name);

	(*env)->ReleaseStringUTFChars(env, jname, name);

	return handle;
}

/*
 * Class:     xreal_client_renderer_Renderer
 * Method:    registerMaterialNoMip
 * Signature: (Ljava/lang/String;)I
 */
jint JNICALL Java_xreal_client_renderer_Renderer_registerMaterialNoMip(JNIEnv *env, jclass cls, jstring jname)
{
	char           *name;
	qhandle_t		handle;

	name = (char *)((*env)->GetStringUTFChars(env, jname, 0));

	handle = re.RegisterShaderNoMip(name);

	(*env)->ReleaseStringUTFChars(env, jname, name);

	return handle;
}

/*
 * Class:     xreal_client_renderer_Renderer
 * Method:    registerMaterialLightAttenuation
 * Signature: (Ljava/lang/String;)I
 */
jint JNICALL Java_xreal_client_renderer_Renderer_registerMaterialLightAttenuation(JNIEnv *env, jclass cls, jstring jname)
{
	char           *name;
	qhandle_t		handle;

	name = (char *)((*env)->GetStringUTFChars(env, jname, 0));

	handle = re.RegisterShaderLightAttenuation(name);

	(*env)->ReleaseStringUTFChars(env, jname, name);

	return handle;
}

/*
 * Class:     xreal_client_renderer_Renderer
 * Method:    registerModel
 * Signature: (Ljava/lang/String;Z)I
 */
jint JNICALL Java_xreal_client_renderer_Renderer_registerModel(JNIEnv *env, jclass cls, jstring jname, jboolean forceStatic)
{
	char           *name;
	qhandle_t		handle;

	name = (char *)((*env)->GetStringUTFChars(env, jname, 0));

	handle = re.RegisterModel(name, forceStatic);

	(*env)->ReleaseStringUTFChars(env, jname, name);

	return handle;
}

/*
 * Class:     xreal_client_renderer_Renderer
 * Method:    registerAnimation
 * Signature: (Ljava/lang/String;)I
 */
jint JNICALL Java_xreal_client_renderer_Renderer_registerAnimation(JNIEnv *env, jclass cls, jstring jname)
{
	char           *name;
	qhandle_t		handle;

	name = (char *)((*env)->GetStringUTFChars(env, jname, 0));

	handle = re.RegisterAnimation(name);

	(*env)->ReleaseStringUTFChars(env, jname, name);

	return handle;
}

/*
 * Class:     xreal_client_renderer_Renderer
 * Method:    registerSkin
 * Signature: (Ljava/lang/String;)I
 */
jint JNICALL Java_xreal_client_renderer_Renderer_registerSkin(JNIEnv *env, jclass cls, jstring jname)
{
	char           *name;
	qhandle_t		handle;

	name = (char *)((*env)->GetStringUTFChars(env, jname, 0));

	handle = re.RegisterSkin(name);

	(*env)->ReleaseStringUTFChars(env, jname, name);

	return handle;
}

// handle to Renderer class
static jclass   class_Renderer = NULL;
static JNINativeMethod Renderer_methods[] = {
	{"setColor", "(FFFF)V", Java_xreal_client_renderer_Renderer_setColor},
	{"drawStretchPic", "(FFFFFFFFI)V", Java_xreal_client_renderer_Renderer_drawStretchPic},
	{"registerFont", "(Ljava/lang/String;I)Lxreal/client/renderer/Font;", Java_xreal_client_renderer_Renderer_registerFont},
	{"registerMaterial", "(Ljava/lang/String;)I", Java_xreal_client_renderer_Renderer_registerMaterial},
	{"registerMaterialNoMip", "(Ljava/lang/String;)I", Java_xreal_client_renderer_Renderer_registerMaterialNoMip},
	{"registerMaterialLightAttenuation", "(Ljava/lang/String;)I", Java_xreal_client_renderer_Renderer_registerMaterialLightAttenuation},
	{"registerModel", "(Ljava/lang/String;Z)I", Java_xreal_client_renderer_Renderer_registerModel},
	{"registerAnimation", "(Ljava/lang/String;)I", Java_xreal_client_renderer_Renderer_registerAnimation},
	{"registerSkin", "(Ljava/lang/String;)I", Java_xreal_client_renderer_Renderer_registerSkin},
};

void Renderer_javaRegister()
{
	Com_Printf("Renderer_javaRegister()\n");

	class_Renderer = (*javaEnv)->FindClass(javaEnv, "xreal/client/renderer/Renderer");
	if(CheckException() || !class_Renderer)
	{
		Com_Error(ERR_FATAL, "Couldn't find xreal.client.renderer.Renderer");
	}

	(*javaEnv)->RegisterNatives(javaEnv, class_Renderer, Renderer_methods, sizeof(Renderer_methods) / sizeof(Renderer_methods[0]));
	if(CheckException())
	{
		Com_Error(ERR_FATAL, "Couldn't register native methods for xreal.client.renderer.Renderer");
	}
}


void Renderer_javaDetach()
{
	Com_Printf("Renderer_javaDetach()\n");

	if(javaEnv)
	{
		if(class_Renderer)
		{
			(*javaEnv)->UnregisterNatives(javaEnv, class_Renderer);
			(*javaEnv)->DeleteLocalRef(javaEnv, class_Renderer);
			class_Renderer = NULL;
		}
	}
}

// ====================================================================================


// handle to UserInterface class
static jclass   class_UserInterface = NULL;
static jobject  object_UserInterface = NULL;
static jclass   interface_UserInterfaceListener;
static jmethodID method_UserInterface_ctor;
static jmethodID method_UserInterface_initUserInterface;
static jmethodID method_UserInterface_shutdownUserInterface;
static jmethodID method_UserInterface_keyEvent;
static jmethodID method_UserInterface_mouseEvent;
static jmethodID method_UserInterface_refresh;
static jmethodID method_UserInterface_isFullscreen;
static jmethodID method_UserInterface_setActiveMenu;
static jmethodID method_UserInterface_consoleCommand;
static jmethodID method_UserInterface_drawConnectScreen;

void UserInterface_javaRegister()
{
	Com_Printf("UserInterface_javaRegister()\n");

	// load the interface UserInterfaceListener
	interface_UserInterfaceListener = (*javaEnv)->FindClass(javaEnv, "xreal/client/ui/UserInterfaceListener");
	if(CheckException() || !interface_UserInterfaceListener)
	{
		Com_Error(ERR_DROP, "Couldn't find class xreal.client.ui.UserInterfaceListener");
		cls.uiStarted = qtrue;
	}

	// load the class UserInterface
	class_UserInterface = (*javaEnv)->FindClass(javaEnv, "xreal/client/ui/UserInterface");
	if(CheckException() || !class_UserInterface)
	{
		Com_Error(ERR_DROP, "Couldn't find class xreal.client.ui.UserInterface");
		cls.uiStarted = qtrue;
	}

	// check class UserInterface against interface UserInterfaceListener
	if(!((*javaEnv)->IsAssignableFrom(javaEnv, class_UserInterface, interface_UserInterfaceListener)))
	{
		Com_Error(ERR_DROP, "The specified UserInterface class doesn't implement xreal.client.ui.UserInterfaceListener");
		cls.uiStarted = qtrue;
	}

	// remove old game if existing
	(*javaEnv)->DeleteLocalRef(javaEnv, interface_UserInterfaceListener);

	// load game interface methods
	method_UserInterface_initUserInterface = (*javaEnv)->GetMethodID(javaEnv, class_UserInterface, "initUserInterface", "()V");
	method_UserInterface_shutdownUserInterface = (*javaEnv)->GetMethodID(javaEnv, class_UserInterface, "shutdownUserInterface", "()V");
	method_UserInterface_keyEvent = (*javaEnv)->GetMethodID(javaEnv, class_UserInterface, "keyEvent", "(IIZ)V");
	method_UserInterface_mouseEvent = (*javaEnv)->GetMethodID(javaEnv, class_UserInterface, "mouseEvent", "(III)V");
	method_UserInterface_refresh = (*javaEnv)->GetMethodID(javaEnv, class_UserInterface, "refresh", "(I)V");
	method_UserInterface_isFullscreen = (*javaEnv)->GetMethodID(javaEnv, class_UserInterface, "isFullscreen", "()Z");
	method_UserInterface_setActiveMenu = (*javaEnv)->GetMethodID(javaEnv, class_UserInterface, "setActiveMenu", "(I)V");
	method_UserInterface_refresh = (*javaEnv)->GetMethodID(javaEnv, class_UserInterface, "refresh", "(I)V");
	method_UserInterface_drawConnectScreen = (*javaEnv)->GetMethodID(javaEnv, class_UserInterface, "drawConnectScreen", "(Z)V");
	method_UserInterface_consoleCommand = (*javaEnv)->GetMethodID(javaEnv, class_UserInterface, "consoleCommand", "(I)Z");
	if(CheckException())
	{
		Com_Error(ERR_DROP, "Problem getting handle for one or more of the UserInterface methods\n");
		cls.uiStarted = qtrue;
	}

	// load constructor
	method_UserInterface_ctor = (*javaEnv)->GetMethodID(javaEnv, class_UserInterface, "<init>", "(IIF)V");

	object_UserInterface = (*javaEnv)->NewObject(javaEnv, class_UserInterface, method_UserInterface_ctor, cls.glconfig.vidWidth, cls.glconfig.vidHeight, cls.glconfig.windowAspect);
	if(CheckException())
	{
		Com_Error(ERR_DROP, "Couldn't create instance of the class UserInterface");
		cls.uiStarted = qtrue;
	}
}


void UserInterface_javaDetach()
{
	Com_Printf("UserInterface_javaDetach()\n");

	if(javaEnv)
	{
		if(class_UserInterface)
		{
			(*javaEnv)->DeleteLocalRef(javaEnv, class_UserInterface);
			class_UserInterface = NULL;
		}

		if(object_UserInterface)
		{
			(*javaEnv)->DeleteLocalRef(javaEnv, object_UserInterface);
			object_UserInterface = NULL;
		}
	}
}

void Java_UI_Init(void)
{
	if(!object_UserInterface)
		return;

	//Com_Printf("Java_UI_Init\n");

	(*javaEnv)->CallVoidMethod(javaEnv, object_UserInterface, method_UserInterface_initUserInterface);

	if(CheckException())
	{
		Com_Error(ERR_DROP, "Java exception occurred during Java_UI_Init()");
	}
}

void Java_UI_Shutdown(void)
{
	if(!object_UserInterface)
		return;

	//Com_Printf("Java_UI_Shutdown\n");

	(*javaEnv)->CallVoidMethod(javaEnv, object_UserInterface, method_UserInterface_shutdownUserInterface);

	if(CheckException())
	{
		Com_Error(ERR_DROP, "Java exception occurred during Java_UI_Shutdown()");
	}
}

void Java_UI_KeyEvent(int key, qboolean down)
{
	if(!object_UserInterface)
		return;

	//Com_Printf("Java_UI_KeyEvent(key = %i, down = %i)\n", key, down);

	(*javaEnv)->CallVoidMethod(javaEnv, object_UserInterface, method_UserInterface_keyEvent, cls.realtime, key, down);

	if(CheckException())
	{
		Com_Error(ERR_DROP, "Java exception occurred during Java_UI_KeyEvent(key = %i, down = %i)", key, down);
	}
}

void Java_UI_MouseEvent(int dx, int dy)
{
	if(!object_UserInterface)
		return;

	//Com_Printf("Java_UI_MouseEvent(dx = %i, dy = %i)\n", dx, dy);

	(*javaEnv)->CallVoidMethod(javaEnv, object_UserInterface, method_UserInterface_mouseEvent, cls.realtime, dx, dy);

	if(CheckException())
	{
		Com_Error(ERR_DROP, "Java exception occurred during Java_UI_MouseEvent(dx = %i, dy = %i)", dx, dy);
	}
}

void Java_UI_Refresh(int time)
{
	if(!object_UserInterface)
		return;

	//Com_Printf("Java_UI_Refresh(time = %i)\n", time);

	(*javaEnv)->CallVoidMethod(javaEnv, object_UserInterface, method_UserInterface_refresh, time);

	if(CheckException())
	{
		Com_Error(ERR_DROP, "Java exception occurred during Java_UI_Refresh(time = %i)", time);
	}
}

qboolean Java_UI_IsFullscreen(void)
{
	jboolean result;

	if(!object_UserInterface)
		return qfalse;

	//Com_Printf("Java_UI_IsFullscreen\n");

	result = (*javaEnv)->CallBooleanMethod(javaEnv, object_UserInterface, method_UserInterface_isFullscreen);

	if(CheckException())
	{
		Com_Error(ERR_DROP, "Java exception occurred during Java_UI_isFullscreen()");
	}

	return result;
}

void Java_UI_SetActiveMenu(uiMenuCommand_t menu)
{
	if(!object_UserInterface)
		return;

	//Com_Printf("Java_UI_SetActiveMenu(menu = %i)\n", menu);

	(*javaEnv)->CallVoidMethod(javaEnv, object_UserInterface, method_UserInterface_setActiveMenu, menu);

	if(CheckException())
	{
		Com_Error(ERR_DROP, "Java exception occurred during Java_UI_SetActiveMenu(menu = %i)", menu);
	}
}

void Java_UI_DrawConnectScreen(qboolean overlay)
{
	if(!object_UserInterface)
		return;

	//Com_Printf("Java_UI_DrawConnectScreen(overlay = %i)\n", overlay);

	(*javaEnv)->CallVoidMethod(javaEnv, object_UserInterface, method_UserInterface_drawConnectScreen, overlay);

	if(CheckException())
	{
		Com_Error(ERR_DROP, "Java exception occurred during Java_UI_DrawConnectScreen(overlay = %i)", overlay);
	}
}

qboolean Java_UI_ConsoleCommand(int realTime)
{
	jboolean result;

	if(!object_UserInterface)
		return qfalse;

	//Com_Printf("Java_UI_ConsoleCommand(realTime = %i)\n", realTime);

	result = (*javaEnv)->CallBooleanMethod(javaEnv, object_UserInterface, method_UserInterface_consoleCommand, realTime);

	if(CheckException())
	{
		Com_Error(ERR_DROP, "Java exception occurred during Java_UI_ConsoleCommand(realTime = %i)", realTime);
	}

	return result;
}

// ====================================================================================



/*
====================
CL_ShutdownUI
====================
*/
void CL_ShutdownUI(void)
{
	Key_SetCatcher(Key_GetCatcher() & ~KEYCATCH_UI);
	cls.uiStarted = qfalse;

	if(!javaEnv)
	{
		Com_Printf("Can't stop Java user interface module, javaEnv pointer was null\n");
		return;
	}

	CheckException();

	Java_UI_Shutdown();

	CheckException();

	Client_javaDetach();
	Renderer_javaDetach();
	Font_javaDetach();
	UserInterface_javaDetach();
}

/*
====================
CL_InitUI
====================
*/
void CL_InitUI(void)
{
	Client_javaRegister();
	Renderer_javaRegister();
	Font_javaRegister();
	UserInterface_javaRegister();

	// init for this gamestate
	Java_UI_Init(/*(cls.state >= CA_AUTHORIZING && cls.state < CA_ACTIVE)*/);

	// reset any CVAR_CHEAT cvars registered by ui
	if(!clc.demoplaying && !cl_connectedToCheatServer)
		Cvar_SetCheatState();
}

/*
====================
UI_GameCommand

See if the current console command is claimed by the ui
====================
*/
qboolean UI_GameCommand(void)
{
	return Java_UI_ConsoleCommand(cls.realtime);
}






#endif
