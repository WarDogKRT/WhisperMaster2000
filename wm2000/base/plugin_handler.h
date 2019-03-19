#pragma once
#include "misc/error_handler.h"
#include "misc/config_container.h"
#include "misc/language_pkg.h"
//#include "speech_engine.h"
#include "misc/channel_filter.h"
#include "misc/client_filter.h"
#include "ts3_functions.h"

#define INFODATA_BUFSIZE 128

// some helper to use with language_pkg
#define TRANSLATE_PTR(a) this->m_pcTranslate->translate(a).c_str()


class plugin_handler
{
public:
    plugin_handler();
    plugin_handler(uint64 nServerID, anyID nMyClientID, TS3Functions *pstTs3TempFunc, config_container *configData, char *pPluginID, language_pkg* pcTranslate);
    ~plugin_handler();
    
    // handler init
    void                 check_param();

    //event functions
    void onUpdateClientEvent(anyID nClientID, uint64 nActChannel) { this->m_cClientFilter.update_client_list(nClientID, nActChannel); };
    void onHotkeyEvent(const char* keyword);
    void infoData(uint64 id, enum PluginItemType type, char** data);
    void onTalkStatusChangeEvent(int iStatus, int iIsReceivedWhisper, anyID nClientID);

    // menu helper functions
    void print_all_lists();
    void update_meta_data();
    void toggle_ignore(uint64 nChannel);
    void toggle_favorite(int nProfile, uint64 nChannel);
    void set_profile_level(int nProfile, uint64 nChannel, bool bIsStart);

    int get_next_free_freq(int iStartFreq) { return this->m_cClientFilter.get_next_free_freq(iStartFreq); }; // return a frequency that is currently unused

    // interface
    client_filter*      get_client_filter()  { return &this->m_cClientFilter; };
    channel_filter*     get_channel_filter() { return &this->m_cChannelFilter; };

protected:
    void                reset_WhisperlList();
    void                activate_PTT(int iState);
    void                internal_write_err(const char* pFuncName);

private:
//    speech_engine        m_cSpeechEngine;       // interface to speech engine

    uint64               m_nServerID;           // ID of connected Server
    anyID                m_nMyClientID;         // ID of own client on this Server
    language_pkg        *m_pcTranslate;         // language converter
    config_container    *m_pcConfigData;        // configuration container
    error_handler        m_cErrHandler;         // link to error handler

    struct TS3Functions *m_pstTs3Functions;     // TS3 interface functions
    char*                m_pPluginID;           // used for plugin commands

    int                  m_nActualActiveProfile;// number of the actual used profile (0 => off)
    bool                 m_bPttState;           // state of PTT activation

    client_filter        m_cClientFilter;       // helper class to filter clients
    channel_filter       m_cChannelFilter;      // helper class to filter channel lists
};


enum eMenuID
{
    MENU_ID_CLIENT_1 = 1,
    MENU_ID_CHANNEL_IGNORE,
    MENU_ID_CHANNEL_TOGGLE_P1,
    MENU_ID_CHANNEL_TOGGLE_P_Max = MENU_ID_CHANNEL_TOGGLE_P1 + REAL_MAXNUMPROFILES,
    MENU_ID_CHANNEL_LEVEL_P1,
    MENU_ID_CHANNEL_LEVEL_P_Max = MENU_ID_CHANNEL_LEVEL_P1 + REAL_MAXNUMPROFILES,
    MENU_ID_CHANNEL_LEVEL_END_P1,
    MENU_ID_CHANNEL_LEVEL_END_P_Max = MENU_ID_CHANNEL_LEVEL_END_P1 + REAL_MAXNUMPROFILES,
    MENU_ID_GLOBAL_OPEN_SETTINGS,
    MENU_ID_GLOBAL_SHOW_ALL,
    MENU_ID_GLOBAL_CLEAN_IGNORE,
    MENU_ID_GLOBAL_CLEAN_P1,
    MENU_ID_GLOBAL_CLEAN_P_Max = MENU_ID_GLOBAL_CLEAN_P1 + REAL_MAXNUMPROFILES,
    MENU_ID_GLOBAL_FREQ_MUTE_P1,
    MENU_ID_GLOBAL_FREQ_MUTE_P_Max = MENU_ID_GLOBAL_FREQ_MUTE_P1 + REAL_MAXNUMPROFILES,
    MENU_ID_GLOBAL_FREQ_SET,
    MENU_ID_GLOBAL_HELP,
    MENU_ID_GLOBAL_ABOUT
};