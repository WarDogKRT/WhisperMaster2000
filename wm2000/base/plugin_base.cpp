#include "plugin_base.h"
#include <Shlwapi.h>
//#include <synchapi.h>

#if USE_CALL_STACK
#define CALL_STACK if(USE_CALL_STACK) this->m_cErrHandler.message_callstack(__FUNCSIG__);
#else
#define CALL_STACK
#endif

/* ----------------------------------------------------------------------------
* constructor
*/
plugin_base::plugin_base()
{
    CALL_STACK
    try
    {
        this->m_vServerList.reserve(20);
        this->m_pPluginID           = nullptr;
        this->m_nServerConnected    = 0;
        this->m_bServerConnected    = false;
        this->m_pMainUi             = nullptr;
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }
}

/* ----------------------------------------------------------------------------
* destructor
*/
plugin_base::~plugin_base()
{
    CALL_STACK
}

/* ----------------------------------------------------------------------------
* initialize class and read configData
*/
void plugin_base::Init(struct TS3Functions stTs3TempFunc, std::string sPluginPath, char* pluginID)
{
    CALL_STACK
    try
    {
        // copy interface functions
        this->m_stTs3Functions  = stTs3TempFunc;
        this->m_pPluginID       = pluginID;
        this->m_sPluginPath     = sPluginPath;

        //create UI and read config from file
        std::string sConfigPath = this->m_sPluginPath + std::string("WhisperMaster2000/");
        if(this->m_pMainUi == nullptr) this->m_pMainUi = new wm2000_main_ui_actions(&m_cConfigData, nullptr, sConfigPath);
        check_param();

        // set language
        this->m_cTranslate.set_language(this->m_cConfigData.s_get_Language());
        //this->m_cSpeechEngine.set_language(this->m_cConfigData.s_get_Language().c_str());
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }
}

/* ----------------------------------------------------------------------------
* open configure UI on user request
*/
void plugin_base::open_configure_ui()
{
    CALL_STACK
    try
    {
        if (this->m_pMainUi != nullptr)
        {
            if (this->m_pMainUi->isVisible())
            {
                // Window is already displayed somewhere, bring it to the top and give it focus
                this->m_pMainUi->raise();
                this->m_pMainUi->activateWindow();
                this->m_pMainUi->update_box_size();
            }
            else
            {
                // Display window
                this->m_pMainUi->show();
                this->m_pMainUi->update_box_size();
            }
        }
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }
}


/* ----------------------------------------------------------------------------
* clean up class and write actual settings
*/
void plugin_base::Close()
{
    CALL_STACK
    try
    {
        //this->m_cConfigData.s_write_param();

        //close UI
        if (this->m_pMainUi != nullptr)
        {
            this->m_pMainUi->close();
            delete this->m_pMainUi;
            this->m_pMainUi = nullptr;
        }
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }
}


/* ----------------------------------------------------------------------------
* handle connect event
*/
void plugin_base::onConnect(uint64 nServerConnectionHandlerID, int newStatus)
{
    CALL_STACK
    try
    {
        if (newStatus == STATUS_CONNECTED)
        {
            //create server_list entry
            if (find_server_idx(nServerConnectionHandlerID) < 0)
            {
                server_list cServerList;
                cServerList.m_nServerID = nServerConnectionHandlerID;
                if (this->m_stTs3Functions.getClientID(nServerConnectionHandlerID, &cServerList.m_nMyClientID) == ERROR_ok)
                {
                    cServerList.m_pHandler = new plugin_handler(nServerConnectionHandlerID, cServerList.m_nMyClientID, &this->m_stTs3Functions, &this->m_cConfigData, this->m_pPluginID, &this->m_cTranslate);
                    this->m_vServerList.push_back(cServerList);
                }
            }
            else
            {
                //error
            }
            if (DEBUG_LOG) printf("PLUGIN: New server (%d/%zd) connection established\n", this->m_nServerConnected, this->m_vServerList.size());
        }
        else if (newStatus == STATUS_CONNECTION_ESTABLISHED)
        {
            // if client connection is fully established
            if (find_server_idx(nServerConnectionHandlerID) >= 0)
            {
                //update meta data if server is fully connected
                find_server_handler(nServerConnectionHandlerID)->update_meta_data();
                //check server depending parameter
                find_server_handler(nServerConnectionHandlerID)->check_param();
                //setup link to client filter
                if (this->m_pMainUi != nullptr) this->m_pMainUi->add_pointer(find_server_handler(nServerConnectionHandlerID)->get_client_filter(), find_server_handler(nServerConnectionHandlerID)->get_channel_filter());
            }
            else
                if (DEBUG_LOG) printf("Server not found \"plugin_base::onConnect\"\n");

            //mark as connected
            this->m_nServerConnected++;
            this->m_bServerConnected = true;

            //write server name to console
            char* s;
            unsigned int error;
            if ((error = this->m_stTs3Functions.getServerVariableAsString(nServerConnectionHandlerID, VIRTUALSERVER_NAME, &s)) != ERROR_ok)
            {
                if (error != ERROR_not_connected) // Don't spam error in this case (failed to connect)
                    this->m_stTs3Functions.logMessage(TRANSLATE("conn_ErrName"), LogLevel_ERROR, "Whispermaster2000", nServerConnectionHandlerID);
                else
                    this->m_stTs3Functions.logMessage(TRANSLATE("conn_WrongConn"), LogLevel_WARNING, "Whispermaster2000", nServerConnectionHandlerID);
                return;
            }
            if (DEBUG_LOG) printf("PLUGIN: Server (%d/%zd) name: %s\n", this->m_nServerConnected, this->m_vServerList.size(), s);
            this->m_stTs3Functions.freeMemory(s);
        }
        else if (newStatus == STATUS_DISCONNECTED)
        {
            //destroy server_list entry
            int nServerIdx = find_server_idx(nServerConnectionHandlerID);
            if (nServerIdx >= 0)
            {
                //clean up link to client filter
                if (this->m_pMainUi != nullptr) this->m_pMainUi->delete_pointer(find_server_handler(nServerConnectionHandlerID)->get_client_filter(), find_server_handler(nServerConnectionHandlerID)->get_channel_filter());

                // delete handler in list
                delete this->m_vServerList[nServerIdx].m_pHandler;
                this->m_vServerList.erase(this->m_vServerList.begin() + nServerIdx);
            }
            //mark as disconnected
            this->m_nServerConnected--;
            this->m_bServerConnected = (this->m_nServerConnected != 0);

            if (DEBUG_LOG) printf("PLUGIN: Server disconnected %d\n", this->m_nServerConnected);
        }
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }

}



/* ----------------------------------------------------------------------------
* find serverID in server list and return index
*/
int plugin_base::find_server_idx(uint64 nServerConnectionHandlerID)
{
    CALL_STACK
    for (int ii = 0; ii < this->m_vServerList.size(); ii++)
        if (this->m_vServerList[ii].m_nServerID == nServerConnectionHandlerID)
            return ii;

    return -1;
}



/* ----------------------------------------------------------------------------
* find serverID in server list and return ptr to handler
*/
plugin_handler* plugin_base::find_server_handler(uint64 nServerConnectionHandlerID)
{
    CALL_STACK
    for (int ii = 0; ii < this->m_vServerList.size(); ii++)
        if (this->m_vServerList[ii].m_nServerID == nServerConnectionHandlerID)
            return this->m_vServerList[ii].m_pHandler;

    return nullptr;
}


/* ----------------------------------------------------------------------------
* check all parameter of this->configData on init
*/
void plugin_base::check_param()
{
    CALL_STACK
    try
    {
        const size_t nBuffSize = 512;
        char cBuffer[nBuffSize];

        //use actual serverID for logging only
        uint64 serverConnectionHandlerID = this->m_stTs3Functions.getCurrentServerConnectionHandlerID();

        //start init
        this->m_stTs3Functions.logMessage(TRANSLATE("check_Init"), LogLevel_INFO, "WhisperMaster2000", serverConnectionHandlerID);

        //check parameter
        if (this->m_cConfigData.s_get_MaxNumProfiles() > REAL_MAXNUMPROFILES)
        {
            sprintf_s(cBuffer, nBuffSize, TRANSLATE("check_nMaxNumProfiles"), this->m_cConfigData.s_get_MaxNumProfiles(), DEFAULT_MAXNUMPROFILES);
            this->m_stTs3Functions.logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", serverConnectionHandlerID);
            this->m_cConfigData.s_set_MaxNumProfiles(DEFAULT_MAXNUMPROFILES);
        }
        for (int ii = 0; ii < this->m_cConfigData.s_get_MaxNumProfiles(); ii++)
        {
            //check profile name
            if ((this->m_cConfigData.s_get_ProfileName(ii).size() == 0) || (this->m_cConfigData.s_get_ProfileName(ii).size() > 20))
            {
                std::string sTemp;

                if (this->m_cConfigData.s_get_ProfileName(ii).size() == 0)
                {
                    sTemp = std::string("Profile ");
                    sTemp.append(std::to_string(ii + 1));
                }
                else
                    sTemp = this->m_cConfigData.s_get_ProfileName(ii).substr(0, 20);

                this->m_cConfigData.s_get_ProfileName(ii) = sTemp;

                sprintf_s(cBuffer, nBuffSize, TRANSLATE("check_psProfileName"), ii + 1, this->m_cConfigData.s_get_ProfileName(ii).c_str());
                this->m_stTs3Functions.logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", serverConnectionHandlerID);
            }

            // Server name is not equal and channel list is used ==> WARNING
            if ((this->m_cConfigData.s_get_ServerName(ii).compare("") == 0) && ((this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_FAVORITE) || (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_AUDIO)) && (this->m_cConfigData.s_get_FavoriteList(ii)->size() > 1))
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE("check_ValidServer"), ii + 1, ii + 1, ii + 1);
                this->m_stTs3Functions.logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", serverConnectionHandlerID);
                this->m_cConfigData.clear_vector(this->m_cConfigData.s_get_FavoriteList(ii));
            }


            if (this->m_cConfigData.s_get_MinChLevel(ii) > 20)
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE("check_MinChLevel_High"), ii + 1, this->m_cConfigData.s_get_MinChLevel(ii), 2);
                this->m_stTs3Functions.logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", serverConnectionHandlerID);
                this->m_cConfigData.s_set_MinChLevel(ii, 2);
            }
            if (this->m_cConfigData.s_get_MinChLevel(ii) < 1)
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE("check_MinChLevel_Low"), ii + 1, this->m_cConfigData.s_get_MinChLevel(ii), 2);
                this->m_stTs3Functions.logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", serverConnectionHandlerID);
                this->m_cConfigData.s_set_MinChLevel(ii, 2);
            }

            if (this->m_cConfigData.s_get_MaxChLevel(ii) > 20)
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE("check_MaxChLevel_High"), ii + 1, this->m_cConfigData.s_get_MaxChLevel(ii), 0);
                this->m_stTs3Functions.logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", serverConnectionHandlerID);
                this->m_cConfigData.s_set_MaxChLevel(ii, 0);
            }

            if ((this->m_cConfigData.s_get_MinChLevel(ii) > this->m_cConfigData.s_get_MaxChLevel(ii)) && (this->m_cConfigData.s_get_MaxChLevel(ii) > 0))
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE("check_Min2MaxChLevel"), ii + 1, ii + 1, this->m_cConfigData.s_get_MaxChLevel(ii), this->m_cConfigData.s_get_MinChLevel(ii), 0);
                this->m_stTs3Functions.logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", serverConnectionHandlerID);
                this->m_cConfigData.s_set_MaxChLevel(ii, 0);
            }

            if ((this->m_cConfigData.s_get_MaxNumFreq() < 0) || (this->m_cConfigData.s_get_MaxNumFreq() > MAX__MAXFREQUENCY))
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE("check_MaxFreq"), this->m_cConfigData.s_get_MaxNumFreq(), 0, MAX__MAXFREQUENCY, MAX_FREQUENCY);
                this->m_stTs3Functions.logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", serverConnectionHandlerID);
                this->m_cConfigData.s_set_MaxNumFreq(MAX_FREQUENCY);
            }

            if ((this->m_cConfigData.s_get_ActiveFreq(ii) < 0) || (this->m_cConfigData.s_get_ActiveFreq(ii) > this->m_cConfigData.s_get_MaxNumFreq()))
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE("check_ActiveFreq"), ii + 1, this->m_cConfigData.s_get_ActiveFreq(ii), 0, this->m_cConfigData.s_get_MaxNumFreq());
                this->m_stTs3Functions.logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", serverConnectionHandlerID);
                this->m_cConfigData.s_set_ActiveFreq(ii, 0);
            }
        }
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }
}



/* ----------------------------------------------------------------------------
* initialize Hotkeys
*/
void plugin_base::initHotkeys(struct PluginHotkey*** hotkeys)
{
    CALL_STACK
    try
    {
        const size_t nBuffSize = 512;
        char cBuffer_key[nBuffSize];
        char cBuffer_txt[nBuffSize];

        // select menu parts
        size_t nItems = 0;

        for (int ii = 0; ii < this->m_cConfigData.s_get_MaxNumProfiles(); ii++)
        {
            if (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_FAVORITE)
                nItems++;                               // Use / Toggle Level
            else if (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_LEVEL)
                nItems++;                               // Use / Toggle Level
            else if (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_FREQUENCY)
                nItems++;                               // Use / Toggle FREQUENCY
        }
        nItems++;                                       // Reset

        BEGIN_CREATE_HOTKEYS(nItems);  /* Create hotkeys. Size must be correct for allocating memory. */
        for (int ii = 0; ii < this->m_cConfigData.s_get_MaxNumProfiles(); ii++)
        {
            sprintf_s(cBuffer_key, nBuffSize, "profile_%d", ii + 1);
            if (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_FAVORITE)
            {
                if(this->m_cConfigData.s_get_AutoActivate(ii))
                    sprintf_s(cBuffer_txt, nBuffSize, TRANSLATE("hotkey_UseFav"), ii + 1);
                else
                    sprintf_s(cBuffer_txt, nBuffSize, TRANSLATE("hotkey_ToggleFav"), ii + 1);
                CREATE_HOTKEY(cBuffer_key, cBuffer_txt);
            }
            else if (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_LEVEL)
            {
                if (this->m_cConfigData.s_get_AutoActivate(ii))
                    sprintf_s(cBuffer_txt, nBuffSize, TRANSLATE("hotkey_UseLevel"), ii + 1);
                else
                    sprintf_s(cBuffer_txt, nBuffSize, TRANSLATE("hotkey_ToggleLevel"), ii + 1);
                CREATE_HOTKEY(cBuffer_key, cBuffer_txt);
            }
            else if (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_FREQUENCY)
            {
                if (this->m_cConfigData.s_get_AutoActivate(ii))
                    sprintf_s(cBuffer_txt, nBuffSize, TRANSLATE("hotkey_UseFreq"), ii + 1);
                else
                    sprintf_s(cBuffer_txt, nBuffSize, TRANSLATE("hotkey_ToggleFreq"), ii + 1);
                CREATE_HOTKEY(cBuffer_key, cBuffer_txt);
            }
        }
        CREATE_HOTKEY("reset", TRANSLATE("hotkey_Reset"));
        END_CREATE_HOTKEYS;
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }
}



/* ----------------------------------------------------------------------------
* initialize all menus
*/
void plugin_base::InitMenu(struct PluginMenuItem*** menuItems)
{
    CALL_STACK
    try
    {
        const size_t nBuffSize = 512;
        char cBuffer[nBuffSize];

        // select menu parts
        size_t nItems = 0;

            //Client specific
        nItems++;                                       //MENU_ID_CLIENT_1

            //Channel specific
        nItems++;                                       //MENU_ID_CHANNEL_IGNORE
        for (int ii = 0; ii < this->m_cConfigData.s_get_MaxNumProfiles(); ii++)
        {
            if ((this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_FAVORITE) || (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_AUDIO))
                nItems++;                               //MENU_ID_CHANNEL_TOGGLE_P1 - MENU_ID_CHANNEL_TOGGLE_P_Max
            if (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_LEVEL)
                nItems++;                               //MENU_ID_CHANNEL_LEVEL_P1 - MENU_ID_CHANNEL_LEVEL_P_Max
            if ((this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_LEVEL) && (this->m_cConfigData.s_get_MaxChLevel(ii) != 0))
                nItems++;                               //MENU_ID_CHANNEL_LEVEL_END_P1 - MENU_ID_CHANNEL_LEVEL_END_P_Max
        }

            // Global
        nItems++;                                       //MENU_ID_GLOBAL_OPEN_SETTINGS
        nItems++;                                       //MENU_ID_GLOBAL_FREQ_SET
        nItems++;                                       //MENU_ID_GLOBAL_SHOW_ALL
        nItems++;                                       //MENU_ID_GLOBAL_CLEAN_IGNORE
        for (int ii = 0; ii < this->m_cConfigData.s_get_MaxNumProfiles(); ii++)
        {
            if ((this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_FAVORITE) || (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_AUDIO))
                nItems++;                               //MENU_ID_GLOBAL_CLEAN_P1 - MENU_ID_GLOBAL_CLEAN_P_Max
            if ((this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_FREQUENCY))
                nItems++;                               //MENU_ID_GLOBAL_FREQ_MUTE_P1 - MENU_ID_GLOBAL_FREQ_MUTE_P_Max
        }
        nItems++;                                       //MENU_ID_GLOBAL_HELP

        // Create menu
        BEGIN_CREATE_MENUS(nItems);  /* IMPORTANT: Number of menu items must be correct! */
            //Client specific
        CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CLIENT, MENU_ID_CLIENT_1, "Client item 1", "1.png");

            //Channel specific
        sprintf_s(cBuffer, nBuffSize, TRANSLATE("menu_SetIgnore"), m_cConfigData.s_get_SaveIgnoreList() ? "[temp]" : "");
        CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_ID_CHANNEL_IGNORE, cBuffer, "cross.png");

        for (int ii = 0; ii < this->m_cConfigData.s_get_MaxNumProfiles(); ii++)
        {
            if ((this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_FAVORITE) || (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_AUDIO))
            {
                if (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_FAVORITE)
                    sprintf_s(cBuffer, nBuffSize, TRANSLATE("menu_ToggleFav"), this->m_cConfigData.s_get_ProfileName(ii).c_str(), m_cConfigData.s_get_UseSubChOfFav(ii) ? "[+Sub-Channel]" : "");
                else
                    sprintf_s(cBuffer, nBuffSize, TRANSLATE("menu_ToggleAudio"), this->m_cConfigData.s_get_ProfileName(ii).c_str(), m_cConfigData.s_get_UseSubChOfFav(ii) ? "[+Sub-Channel]" : "");
                CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_ID_CHANNEL_TOGGLE_P1 + ii, cBuffer, "create.png");
            }
            else if (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_LEVEL)
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE("menu_SetStartLev"), this->m_cConfigData.s_get_ProfileName(ii).c_str());
                CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_ID_CHANNEL_LEVEL_P1 + ii, cBuffer, "bottom.png");
                if (this->m_cConfigData.s_get_MaxChLevel(ii) != 0)
                {
                    sprintf_s(cBuffer, nBuffSize, TRANSLATE("menu_SetEndLev"), this->m_cConfigData.s_get_ProfileName(ii).c_str());
                    CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_CHANNEL, MENU_ID_CHANNEL_LEVEL_END_P1 + ii, cBuffer, "top.png");
                }
            }
        }

            // Global
        CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_OPEN_SETTINGS, TRANSLATE("menu_OpenSettings"), "settings.png");
        CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_FREQ_SET, TRANSLATE("menu_SetFreq"), "hint.png");
        CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_SHOW_ALL, TRANSLATE("menu_ShowList"), "list.png");
        CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_CLEAN_IGNORE, TRANSLATE("menu_DelIgnore"), "cross.png");
        for (int ii = 0; ii < this->m_cConfigData.s_get_MaxNumProfiles(); ii++)
        {
            sprintf_s(cBuffer, nBuffSize, TRANSLATE("menu_DelProfile"), this->m_cConfigData.s_get_ProfileName(ii).c_str());
            if ((this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_FAVORITE) || (this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_AUDIO))
                CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_CLEAN_P1 + ii, cBuffer, "cross.png");

            sprintf_s(cBuffer, nBuffSize, TRANSLATE("menu_MuteProfile"), this->m_cConfigData.s_get_ProfileName(ii).c_str());
            if ((this->m_cConfigData.s_get_ProfileType(ii) == PROFILE_FREQUENCY))
                CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_FREQ_MUTE_P1 + ii, cBuffer, "mute.png");
        }
        CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_HELP, TRANSLATE("menu_help"), "list2.png");
        CREATE_MENU_ITEM(PLUGIN_MENU_TYPE_GLOBAL, MENU_ID_GLOBAL_ABOUT, TRANSLATE("mUi_MenuAbout"), "about.png");
        END_CREATE_MENUS;  /* Includes an assert checking if the number of menu items matched */
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }
}



/* ----------------------------------------------------------------------------
* interface function
*/
void plugin_base::onHotkeyEvent(const char* keyword)
{
    CALL_STACK
    try
    {
        // serverID is not delivered with this event, get the actual selected ServerTab
        uint64 nServerConnectionHandlerID = this->m_stTs3Functions.getCurrentServerConnectionHandlerID();

        if (find_server_idx(nServerConnectionHandlerID) >= 0)
            find_server_handler(nServerConnectionHandlerID)->onHotkeyEvent(keyword);
        else
            if (DEBUG_LOG) printf("Server not found \"plugin_base::onHotkeyEvent\"\n");
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }
};
/* ----------------------------------------------------------------------------
* interface function
*/
void plugin_base::onUpdateClientEvent(uint64 nServerConnectionHandlerID, anyID nClientID, uint64 nActChannel)
{
    CALL_STACK
    try
    {
        if (find_server_idx(nServerConnectionHandlerID) >= 0)
        {
            find_server_handler(nServerConnectionHandlerID)->onUpdateClientEvent(nClientID, nActChannel);

            if (this->m_pMainUi != nullptr) this->m_pMainUi->update_ui();
        }
        else
            if (DEBUG_LOG) printf("Server not found \"plugin_base::onUpdateClientEvent\"\n");
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }
}


/* ----------------------------------------------------------------------------
* Talk status event handler
*/
void plugin_base::onTalkStatusChangeEvent(uint64 nServerConnectionHandlerID, int iStatus, int iIsReceivedWhisper, anyID nClientID)
{
    CALL_STACK
    try
    {
        if (find_server_idx(nServerConnectionHandlerID) >= 0)
            find_server_handler(nServerConnectionHandlerID)->onTalkStatusChangeEvent(iStatus, iIsReceivedWhisper, nClientID);
        else
            if (DEBUG_LOG) printf("Server not found \"%s\"\n", __FUNCSIG__);
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }
}


/* ----------------------------------------------------------------------------
* interface function
*/
void plugin_base::infoData(uint64 nServerConnectionHandlerID, uint64 id, enum PluginItemType type, char** data)
{
    CALL_STACK
    try
    {
        if (find_server_idx(nServerConnectionHandlerID) >= 0)
            find_server_handler(nServerConnectionHandlerID)->infoData(id, type, data);
        else
            if (DEBUG_LOG) printf("Server not found \"%s\"\n", __FUNCSIG__);
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }
}


/* ----------------------------------------------------------------------------
* handle all menus events
*/
void plugin_base::onMenuItemEvent(uint64 nServerConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID)
{
    CALL_STACK
    try
    {
        const size_t nBuffSize = 512;
        char cBuffer[nBuffSize];
    
        switch (type) {
        case PLUGIN_MENU_TYPE_GLOBAL:
            /* Global menu item was triggered. selectedItemID is unused and set to zero. */
            switch (menuItemID) {
            case MENU_ID_GLOBAL_OPEN_SETTINGS:
                open_configure_ui();
                break;

            case MENU_ID_GLOBAL_FREQ_SET:
                if (this->m_pMainUi != nullptr) this->m_pMainUi->open_freq_ui();
                break;

            case MENU_ID_GLOBAL_SHOW_ALL:
                if (find_server_idx(nServerConnectionHandlerID) >= 0)
                    find_server_handler(nServerConnectionHandlerID)->print_all_lists();
                break;

            case MENU_ID_GLOBAL_CLEAN_IGNORE:
                /* Menu "clean up ignore channel lists" was triggered */
                if (DEBUG_LOG) printf("Clean Ignore\n");
                this->m_cConfigData.clear_vector(this->m_cConfigData.s_get_IgnoreList());
                break;

            case MENU_ID_GLOBAL_HELP:
            {   // Menu "Open help file" was triggered
                std::string  temp = std::string(m_sPluginPath.begin(), m_sPluginPath.end()) + std::string("WhisperMaster2000/TS3_Whispermaster2000_Help.pdf");
                std::wstring temp_w = std::wstring(temp.begin(), temp.end());

                if (this->m_cConfigData.file_exists(temp))
                {
                    ShellExecute(NULL, L"open", temp_w.c_str(), NULL, NULL, SW_SHOW);
                }
                else
                {
                    sprintf_s(cBuffer, nBuffSize, TRANSLATE("menuEv_HelpNotFound"), temp.c_str());
                    this->m_stTs3Functions.logMessage(cBuffer, LogLevel_ERROR, "WhisperMaster2000", nServerConnectionHandlerID);
                }
                break;
            }

            case MENU_ID_GLOBAL_ABOUT:
                if(this->m_pMainUi != nullptr) this->m_pMainUi->open_about_ui();
                break;

            default:
                if ((menuItemID >= MENU_ID_GLOBAL_CLEAN_P1) && (menuItemID <= MENU_ID_GLOBAL_CLEAN_P_Max))
                {
                    // Menu "clean up ignore channel lists" was triggered
                    if (DEBUG_LOG) printf("Clean P%d\n", menuItemID - MENU_ID_GLOBAL_CLEAN_P1 + 1);
                    this->m_cConfigData.clear_vector(this->m_cConfigData.s_get_FavoriteList(menuItemID - MENU_ID_GLOBAL_CLEAN_P1));
                }
                else if ((menuItemID >= MENU_ID_GLOBAL_FREQ_MUTE_P1) && (menuItemID <= MENU_ID_GLOBAL_FREQ_MUTE_P_Max))
                {
                    // Menu "mute frequency" was triggered
                    this->m_cConfigData.s_set_MuteFreq(menuItemID - MENU_ID_GLOBAL_FREQ_MUTE_P1, !this->m_cConfigData.s_get_MuteFreq(menuItemID - MENU_ID_GLOBAL_FREQ_MUTE_P1));
                    for (size_t ii = 0; ii < this->m_vServerList.size(); ii++)
                        this->m_vServerList[ii].m_pHandler->update_meta_data();

                    //update change in ui
                    if (this->m_pMainUi != nullptr) this->m_pMainUi->update_config();
                }
                break;
            }
            break;

        case PLUGIN_MENU_TYPE_CHANNEL:
            /* Channel contextmenu item was triggered. selectedItemID is the channelID of the selected channel */
            switch (menuItemID) {
            case MENU_ID_CHANNEL_IGNORE:
            {
                // add or clear selected channel from ignore list
                //-------------------------------------------------------------------------------------
                if (DEBUG_LOG) printf("PLUGIN: Toggle ignore list\n");
                if (find_server_idx(nServerConnectionHandlerID) >= 0)
                    find_server_handler(nServerConnectionHandlerID)->toggle_ignore(selectedItemID);

                break;
            }
            default:
            {
                if ((menuItemID >= MENU_ID_CHANNEL_TOGGLE_P1) && (menuItemID <= MENU_ID_CHANNEL_TOGGLE_P_Max))
                {
                    // add or clear selected channel from channel list (favorite + audio)
                    //-------------------------------------------------------------------------------------
                    if (DEBUG_LOG) printf("Toggle P%d\n", menuItemID - MENU_ID_CHANNEL_TOGGLE_P1 + 1);
                    if (find_server_idx(nServerConnectionHandlerID) >= 0)
                        find_server_handler(nServerConnectionHandlerID)->toggle_favorite(menuItemID - MENU_ID_CHANNEL_TOGGLE_P1, selectedItemID);
                }
                else if ((menuItemID >= MENU_ID_CHANNEL_LEVEL_P1) && (menuItemID <= MENU_ID_CHANNEL_LEVEL_P_Max))
                {
                    // set new start level, but keep actual end level in mind
                    //-------------------------------------------------------------------------------------
                    if (DEBUG_LOG) printf("Set Level P%d\n", menuItemID - MENU_ID_CHANNEL_LEVEL_P1 + 1);
                    if (find_server_idx(nServerConnectionHandlerID) >= 0)
                        find_server_handler(nServerConnectionHandlerID)->set_profile_level(menuItemID - MENU_ID_CHANNEL_LEVEL_P1, selectedItemID, true);

                }
                else if ((menuItemID >= MENU_ID_CHANNEL_LEVEL_END_P1) && (menuItemID <= MENU_ID_CHANNEL_LEVEL_END_P_Max))
                {
                    // set new end level, but keep actual start level in mind
                    //-------------------------------------------------------------------------------------
                    if (DEBUG_LOG) printf("Set Level end P%d\n", menuItemID - MENU_ID_CHANNEL_LEVEL_END_P1 + 1);
                    if (find_server_idx(nServerConnectionHandlerID) >= 0)
                        find_server_handler(nServerConnectionHandlerID)->set_profile_level(menuItemID - MENU_ID_CHANNEL_LEVEL_END_P1, selectedItemID, false);
                }
                break;
            }
            break;
            }
        case PLUGIN_MENU_TYPE_CLIENT:
            /* Client contextmenu item was triggered. selectedItemID is the clientID of the selected client */
            switch (menuItemID)
            {
            case MENU_ID_CLIENT_1:
                /* Menu client 1 was triggered */
                {
                    //TS3_VECTOR position = { 0.0, 0.0, 0.0 };
                    //position.x = 100.0;
                    //position.y = 200.0;
                    //position.z = 300.0;
                    //
                    //unsigned int nError = this->m_stTs3Functions.channelset3DAttributes(nServerConnectionHandlerID, (anyID)selectedItemID, &position);
                    //printf("Set client (%lld) 3D Audio => 0x%08X\n", selectedItemID, nError);
                }
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    catch (std::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (boost::exception &e)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__, e);
    }
    catch (...)
    {
        this->m_cErrHandler.error_log(__FUNCSIG__);
    }
}


/* ----------------------------------------------------------------------------
* Helper function to create a menu item
*/
struct PluginMenuItem* plugin_base::createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon)
{
    struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
    menuItem->type = type;
    menuItem->id = id;
    strcpy_s(menuItem->text, PLUGIN_MENU_BUFSZ, text);
    strcpy_s(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
    return menuItem;
}


/* ----------------------------------------------------------------------------
* Helper function to create a hotkey
*/
struct PluginHotkey* plugin_base::createHotkey(const char* keyword, const char* description)
{
    struct PluginHotkey* hotkey = (struct PluginHotkey*)malloc(sizeof(struct PluginHotkey));
    strcpy_s(hotkey->keyword, PLUGIN_HOTKEY_BUFSZ, keyword);
    strcpy_s(hotkey->description, PLUGIN_HOTKEY_BUFSZ, description);
    return hotkey;
}
