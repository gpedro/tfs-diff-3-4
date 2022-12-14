#ifndef __CONSOLE__
	#define ID_KICK 101
	#define ID_BAN 102
	#define ID_ABOUT 104
	#define ID_LOG 105
	#define ID_ICON 106

	#define ID_TRAY_HIDE 118
	#define ID_TRAY_SHUTDOWN 119
	#define ID_STATUS_BAR 120

	#define ID_MENU 200

	#define ID_MENU_MAIN_REJECT 201
	#define ID_MENU_MAIN_ACCEPT 202
	#define ID_MENU_MAIN_CLEARLOG 203
	#define ID_MENU_MAIN_SHUTDOWN 204

	#define ID_MENU_SERVER_WORLDTYPE_NOPVP 205
	#define ID_MENU_SERVER_WORLDTYPE_PVP 206
	#define ID_MENU_SERVER_WORLDTYPE_PVPENFORCED 207

	#define ID_MENU_SERVER_BROADCAST 208
	#define ID_MENU_SERVER_SAVE 209
	#define ID_MENU_SERVER_CLEAN 210
	#define ID_MENU_SERVER_CLOSE 211
	#define ID_MENU_SERVER_OPEN 212

	#define ID_MENU_SERVER_PLAYERBOX 213

	#define ID_MENU_RELOAD_ACTIONS 214
	#define ID_MENU_RELOAD_CONFIG 215
	#define ID_MENU_RELOAD_CREATUREEVENTS 216
	#ifdef __LOGIN_SERVER__
	#define ID_MENU_RELOAD_GAMESERVERS 217
	#endif
	#define ID_MENU_RELOAD_GLOBALEVENTS 218
	#define ID_MENU_RELOAD_HIGHSCORES 219
	#define ID_MENU_RELOAD_HOUSEPRICES 220
	#define ID_MENU_RELOAD_ITEMS 221
	#define ID_MENU_RELOAD_MONSTERS 222
	#define ID_MENU_RELOAD_MOVEMENTS 223
	#define ID_MENU_RELOAD_NPCS 224
	#define ID_MENU_RELOAD_OUTFITS 225
	#define ID_MENU_RELOAD_QUESTS 226
	#define ID_MENU_RELOAD_RAIDS 227
	#define ID_MENU_RELOAD_SPELLS 228
	#define ID_MENU_RELOAD_STAGES 229
	#define ID_MENU_RELOAD_TALKACTIONS 230
	#define ID_MENU_RELOAD_VOCATIONS 231
	#define ID_MENU_RELOAD_WEAPONS 232
	#define ID_MENU_RELOAD_ALL 233
#endif

#define STATUS_SERVER_NAME "The Forgotten Server"
#define STATUS_SERVER_VERSION "0.4"
#define STATUS_SERVER_CODENAME "Alpha 1"
#define STATUS_SERVER_PROTOCOL "8.4"
#define CLIENT_VERSION_MIN 840
#define CLIENT_VERSION_MAX 840
#define CLIENT_VERSION_STRING "Only clients with protocol 8.4 allowed!"
#define LATEST_DB_VERSION 8
