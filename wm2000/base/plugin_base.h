#pragma once

#include "misc/error_handler.h"
#include "base/plugin_handler.h"
#include "misc/config_container.h"
#include "misc/language_pkg.h"
//#include "speech_engine.h"
#include "misc/channel_filter.h"
#include "misc/client_filter.h"
#include "ui\wm2000_main_ui_actions.h"
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"
#include "ts3_functions.h"

#define PLUGIN_API_VERSION 23

#define PATH_BUFSIZE 512
#define COMMAND_BUFSIZE 128
#define INFODATA_BUFSIZE 128
#define SERVERINFO_BUFSIZE 256
#define CHANNELINFO_BUFSIZE 512
#define RETURNCODE_BUFSIZE 128


/* Some makros to make the code to create menu items a bit more readable */
#define BEGIN_CREATE_MENUS(x) const size_t sz = x + 1; size_t n = 0; *menuItems = (struct PluginMenuItem**)malloc(sizeof(struct PluginMenuItem*) * sz);
#define CREATE_MENU_ITEM(a, b, c, d) (*menuItems)[n++] = createMenuItem(a, b, c, d);
#define END_CREATE_MENUS (*menuItems)[n++] = NULL; assert(n == sz);

/* Some makros to make the code to create hotkeys a bit more readable */
#define BEGIN_CREATE_HOTKEYS(x) const size_t sz = x + 1; size_t n = 0; *hotkeys = (struct PluginHotkey**)malloc(sizeof(struct PluginHotkey*) * sz);
#define CREATE_HOTKEY(a, b) (*hotkeys)[n++] = createHotkey(a, b);
#define END_CREATE_HOTKEYS (*hotkeys)[n++] = NULL; assert(n == sz);

// some helper to use with language_pkg
#define TRANSLATE(a) this->m_cTranslate.translate(a).c_str()

struct server_list
{
    anyID		    m_nMyClientID;
    uint64          m_nServerID;
    plugin_handler *m_pHandler;
};

class plugin_base
{
    /* ----------------------------------------------------------------------------
    * member functions
    */
public:
    plugin_base();
    ~plugin_base();
    void Init(struct TS3Functions stTs3Functions, std::string sPluginPath, char* pluginID);
    void open_configure_ui();

    void onConnect(uint64 serverConnectionHandlerID, int newStatus);

    void InitMenu(struct PluginMenuItem*** menuItems);
    void onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID);

    const char* get_name()          { return TRANSLATE("base_name"); };
    const char* get_user()          { return TRANSLATE("base_user"); };
	const char* get_version()       { return TRANSLATE("base_version"); };
    const char* get_description()   { return TRANSLATE("base_description"); };
    const char* get_infoTitle()     { return TRANSLATE("info_Title"); };
    
    void initHotkeys(struct PluginHotkey*** hotkeys);
    void onHotkeyEvent(const char* keyword);
    void onUpdateClientEvent(uint64 nServerConnectionHandlerID, anyID nClientID, uint64 nActChannel);
    void onTalkStatusChangeEvent(uint64 nServerConnectionHandlerID, int iStatus, int iIsReceivedWhisper, anyID nClientID);
    void infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data);

    void Close();

private:
    void                    check_param();

    struct PluginMenuItem*  createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon);
    struct PluginHotkey*    createHotkey(const char* keyword, const char* description);

    int                     find_server_idx(uint64 nServerConnectionHandlerID);
    plugin_handler*         find_server_handler(uint64 nServerConnectionHandlerID);

/* ----------------------------------------------------------------------------
* member variable
*/
public:
    language_pkg                m_cTranslate;       // language converter
    //speech_engine               m_cSpeechEngine;    // interface to speech engine

private:
    error_handler               m_cErrHandler;      // link to error handler
    std::vector<server_list>    m_vServerList;      // Server list entry TODO
    config_container            m_cConfigData;      // configuration container
    struct TS3Functions         m_stTs3Functions;   // TS3 interface functions
    std::string                 m_sPluginPath;      // path to plugin folder
    char*                       m_pPluginID;        // used for plugin commands

    unsigned int                m_nServerConnected; // save number of connected server
    bool                        m_bServerConnected; // save state of server connection

    wm2000_main_ui_actions     *m_pMainUi;          // pointer to main UI
};