#include "plugin_handler.h"
#include <Shlwapi.h>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"

/* ----------------------------------------------------------------------------
* default constructor
*/
plugin_handler::plugin_handler()
{
    //init member on creation
    this->m_nServerID       = 0;
    this->m_nMyClientID     = 0;
    this->m_pPluginID       = nullptr;

    this->m_pstTs3Functions = nullptr;
    this->m_pcConfigData    = nullptr;
    this->m_pcTranslate     = nullptr;

    this->m_nActualActiveProfile = 0;
    this->m_bPttState            = false;
}

/* ----------------------------------------------------------------------------
* constructor
*/
plugin_handler::plugin_handler(uint64 nServerID, anyID nMyClientID, TS3Functions *pstTs3TempFunc, config_container *configData, char *pPluginID, language_pkg* pcTranslate)
{
    //init member on creation
    this->m_nServerID       = nServerID;
    this->m_nMyClientID     = nMyClientID;
    this->m_pPluginID       = pPluginID;

    this->m_pstTs3Functions = pstTs3TempFunc;
    this->m_pcConfigData    = configData;
    this->m_pcTranslate     = pcTranslate;

    this->m_nActualActiveProfile = 0;
    this->m_bPttState            = false;

    // set interface to channel filter
    this->m_cChannelFilter.init(this->m_pstTs3Functions, this->m_pcConfigData, this->m_nServerID, this->m_nMyClientID);

    // set interface to client filter
    this->m_cClientFilter.init(this->m_pstTs3Functions, this->m_pcConfigData, &this->m_cChannelFilter, this->m_nServerID, INVALID_CHANNEL_ID, this->m_nMyClientID);

    // set own 3D audio settings
    const TS3_VECTOR position   = { 0.0, 0.0, 0.0 };    //I'm at the center, ...
    const TS3_VECTOR forward    = { 1.0, 0.0, 0.0 };    // ...I look in X direction...
    const TS3_VECTOR up         = { 0.0, 0.0, 1.0 };    // ...and Z is upwards
    if( this->m_pstTs3Functions->systemset3DListenerAttributes(this->m_nServerID, &position, &forward, &up) != ERROR_ok )
        if (DEBUG_LOG) printf("PLUGIN: error while init 3D audio\n");

    if (DEBUG_LOG) printf("PLUGIN: plugin_handler was created (nServerID %llu, nMyClientID %d)\n", nServerID, nMyClientID);
}


/* ----------------------------------------------------------------------------
* destructor
*/
plugin_handler::~plugin_handler()
{
    if (this->m_pstTs3Functions == nullptr)
        return;

    // if actual profile is selected profile, deselect whisperlist and deactivate PTT (on demand)
    if (this->m_nActualActiveProfile != 0)
    {
        if (DEBUG_LOG) printf("PLUGIN: deactivate profile %d on disconnect\n", this->m_nActualActiveProfile);
        reset_WhisperlList();
        if (this->m_pcConfigData->s_get_AutoActivate(this->m_nActualActiveProfile - 1)) activate_PTT(INPUT_DEACTIVATED);
        this->m_nActualActiveProfile = 0;
        return;
    }

    // IDs + references are invalid now
    this->m_nServerID       = this->m_pstTs3Functions->getCurrentServerConnectionHandlerID();
    this->m_nMyClientID     = 0;

    this->m_pstTs3Functions = nullptr;
    this->m_pcConfigData    = nullptr;
    this->m_pcTranslate     = nullptr;
}


/* ----------------------------------------------------------------------------
* check all parameter of this->configData
*/
void plugin_handler::check_param()
{
    const size_t nBuffSize = 512;
    char cBuffer[nBuffSize];
    char cBuffer_key[nBuffSize];
    char cBuffer_hot[nBuffSize];
    const char*     temp_key = (const char*)cBuffer_key;
    char*           temp_hot = (char*)&cBuffer_hot;

    if (this->m_nServerID != 0)
    {
        char* s;
        std::string sServerName;

        this->m_pstTs3Functions->logMessage(TRANSLATE_PTR("conn_ParamCheck"), LogLevel_INFO, "WhisperMaster2000", this->m_nServerID);

        if (this->m_pstTs3Functions->getServerVariableAsString(this->m_nServerID, VIRTUALSERVER_NAME, &s) == ERROR_ok)
        {
            sServerName = std::string(s);
            this->m_pstTs3Functions->freeMemory(s);
        }

        // check if channel of ignore list exists
        if (this->m_pcConfigData->s_get_SaveIgnoreList())
            this->m_cChannelFilter.validate_channel_list(this->m_pcConfigData->s_get_IgnoreList());

        //check profile specific parameter
        for (int ii = 0; ii < this->m_pcConfigData->s_get_MaxNumProfiles(); ii++)
        {
            // Server name is not equal and channel list is used ==> WARNING
            if ((this->m_pcConfigData->s_get_ServerName(ii).compare(sServerName) != 0) && ((this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_FAVORITE) || (this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_AUDIO)) && (this->m_pcConfigData->s_get_FavoriteList(ii)->size() > 1))
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("check_Connect_ValidServer"), ii + 1, this->m_pcConfigData->s_get_ServerName(ii).c_str(), sServerName.c_str(), ii + 1);
                this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", this->m_nServerID);
            }

            // Server name is equal and channel list is used ==> check channel list
            if ((this->m_pcConfigData->s_get_ServerName(ii).compare(sServerName) == 0) && ((this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_FAVORITE) || (this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_AUDIO)))
            {
                this->m_cChannelFilter.validate_channel_list(this->m_pcConfigData->s_get_FavoriteList(ii));
            }

            // check if profile has any hotkey
            if (((this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_LEVEL) || (this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_FAVORITE) || (this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_FREQUENCY)) && (this->m_pPluginID != nullptr))
            {
                sprintf_s(cBuffer_key, nBuffSize, "profile_%d", ii + 1);
                sprintf_s(cBuffer_hot, nBuffSize, "");
                if (this->m_pstTs3Functions->getHotkeyFromKeyword(this->m_pPluginID, &temp_key, &temp_hot, 1, nBuffSize) != ERROR_ok)
                    break;

                std::string sData = std::string(cBuffer_hot);
                if (sData.size() == 0)
                {
                    sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("check_Connect_Hotkey"), this->m_pcConfigData->s_get_ProfileName(ii).c_str());
                    this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", 0);
                }
                this->m_pcConfigData->s_set_HotKey_down(ii, sData);

                if (DEBUG_LOG) printf("Hotkey for %s = \"%s\" %llu\n", cBuffer_key, sData.c_str(), sData.size());
            }
        }

        // check if "general" hotkey is available
        sprintf_s(cBuffer_key, nBuffSize, "reset");
        sprintf_s(cBuffer_hot, nBuffSize, "");
        if (this->m_pstTs3Functions->getHotkeyFromKeyword(this->m_pPluginID, &temp_key, &temp_hot, 1, nBuffSize) == ERROR_ok)
        {
            std::string sData = std::string(cBuffer_hot);
            this->m_pcConfigData->s_set_GenHotKey_reset(sData);
        }
    }

}


/*----------------------------------------------------------------------------
* reset whisperlist
*/
void plugin_handler::reset_WhisperlList()
{
    if (this->m_pstTs3Functions->requestClientSetWhisperList(this->m_nServerID, this->m_nMyClientID, NULL, NULL, NULL) != ERROR_ok)
        this->m_pstTs3Functions->logMessage(TRANSLATE_PTR("hotkey_ErrReset"), LogLevel_ERROR, "WhisperMaster2000", this->m_nServerID);
}


/*----------------------------------------------------------------------------
* (de-)activate PTT on demand, but only if it is not already active
*/
void plugin_handler::activate_PTT(int iState)
{
    uint32_t nError = ERROR_ok;
    const size_t nBuffSize = 512;
    char    cBuffer[nBuffSize];

    bool bNewPttState = (iState == INPUT_ACTIVE);

    if (this->m_bPttState != bNewPttState)
    {
        if (nError = this->m_pstTs3Functions->setClientSelfVariableAsInt(this->m_nServerID, CLIENT_INPUT_DEACTIVATED, iState) != ERROR_ok)
        {
            sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("hotkey_ErrActivate1"), nError);
            this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_ERROR, "WhisperMaster2000", this->m_nServerID);
        }
        else
        {
            this->m_bPttState = bNewPttState;
            nError = this->m_pstTs3Functions->flushClientSelfUpdates(this->m_nServerID, NULL);
            if ((nError != ERROR_ok) && (nError != ERROR_undefined) && (nError != ERROR_ok_no_update))
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("hotkey_ErrActivate2"), nError);
                this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", this->m_nServerID);
                if (iState == INPUT_ACTIVE) activate_PTT(INPUT_DEACTIVATED);
            }

        }
    }
}

/* ----------------------------------------------------------------------------
* handle Hotkey events
*/
void plugin_handler::onHotkeyEvent(const char* keyword)
{
    int nError;

    if (DEBUG_LOG) printf("Hotkey %s incomming\n", keyword);

    if (strcmp(keyword, "reset") == 0)
    {
        // on reset event, just restore whisperlist to default and deactivate PTT on demand
        //-------------------------------------------------------------------------------------
        if (DEBUG_LOG) printf("Hotkey reset erkannt\n");

        reset_WhisperlList();
        if (this->m_pcConfigData->s_get_AutoActivate(this->m_nActualActiveProfile - 1)) activate_PTT(INPUT_DEACTIVATED);
        this->m_pcConfigData->s_set_ActualState(this->m_nActualActiveProfile, false);
        this->m_nActualActiveProfile = 0;
    }
    else
    {
        int     nConverted;
        int     iHotkeyIndex;
        const size_t nBuffSize = 512;
        char    cBuffer[nBuffSize];

        //filter profile index and check keyword
        nConverted = sscanf_s(keyword, "%[a-zA-Z]_%d", cBuffer, (unsigned int)nBuffSize, &iHotkeyIndex);
        if (nConverted != 2)
        {
            if (DEBUG_LOG) printf("Hotkey falscher count %d erkannt\n", nConverted);
            return;
        }
        if (strcmp(cBuffer, "profile") != 0)
        {
            if (DEBUG_LOG) printf("Hotkey falscher string %s erkannt\n", cBuffer);
            return;
        }
        if (iHotkeyIndex > this->m_pcConfigData->s_get_MaxNumProfiles())
        {
            sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("hotkey_MaxNum"), iHotkeyIndex, this->m_pcConfigData->s_get_MaxNumProfiles());
            this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", this->m_nServerID);
            return;
        }
        printf("Hotkey %d erkannt\n", iHotkeyIndex);


        if (this->m_nActualActiveProfile == iHotkeyIndex)
        {
            // if actual profile is selected profile, deselect whisperlist and deactivate PTT (on demand)
            //-------------------------------------------------------------------------------------
            if (DEBUG_LOG) printf("deactivate profile %d\n", iHotkeyIndex);
            reset_WhisperlList();
            if (this->m_pcConfigData->s_get_AutoActivate(iHotkeyIndex - 1)) activate_PTT(INPUT_DEACTIVATED);
            this->m_nActualActiveProfile = 0;
            this->m_pcConfigData->s_set_ActualState(iHotkeyIndex - 1, false);
            return;
        }
        else if (this->m_nActualActiveProfile != 0)
        {
            // key was pressed (but not released) until another hotkey was pressed...
            if (this->m_pcConfigData->s_get_ActualState(iHotkeyIndex - 1) && this->m_pcConfigData->s_get_AutoActivate(iHotkeyIndex - 1))
            {
                //... if AutoActive, just restore first key, but hold actual setting (key release event).
                this->m_pcConfigData->s_set_ActualState(iHotkeyIndex - 1, false);
                return;
            }

            //... non AutoActive, prepare for new setup.
            reset_WhisperlList();
            if (this->m_pcConfigData->s_get_AutoActivate(this->m_nActualActiveProfile - 1))
                activate_PTT(INPUT_DEACTIVATED);
        }

        // get default parameter
        uint64 nChannelID;
        this->m_pstTs3Functions->getChannelOfClient(this->m_nServerID, this->m_nMyClientID, &nChannelID); //don't check for errors, behaves wrong!

        anyID  *pnFilteredClientList = nullptr;
        uint64 *pnFilteredChannelList = nullptr;
        if ((this->m_pcConfigData->s_get_ProfileType(iHotkeyIndex - 1) == PROFILE_OFF) || (this->m_pcConfigData->s_get_ProfileType(iHotkeyIndex - 1) == PROFILE_AUDIO))
        {
            // these types don't need any action
            //-------------------------------------------------------------------------------------
            sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("hotkey_Off"), iHotkeyIndex, this->m_pcConfigData->profile_string_from_enum(this->m_pcConfigData->s_get_ProfileType(iHotkeyIndex - 1)).c_str());
            this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", this->m_nServerID);
            return;
        }
        else if (this->m_pcConfigData->s_get_ProfileType(iHotkeyIndex - 1) == PROFILE_FAVORITE)
        {
            // if profile is in favorite mode, activate all channels in list
            //-------------------------------------------------------------------------------------
            if (DEBUG_LOG) printf("Hotkey %d => favorite erkannt\n", iHotkeyIndex);

            // filter channel list
            pnFilteredChannelList = this->m_cChannelFilter.filter_channel_from_list(this->m_pcConfigData->s_get_FavoriteList(iHotkeyIndex - 1), this->m_pcConfigData->s_get_UseSubChOfFav(iHotkeyIndex - 1), this->m_pcConfigData->s_get_UseIgnoreListTx(iHotkeyIndex - 1));

            if (pnFilteredChannelList == nullptr)
            {
                sprintf_s(cBuffer, nBuffSize, "No active channel in favorite list of \"%s\"", this->m_pcConfigData->s_get_ProfileName(iHotkeyIndex - 1).c_str());
                this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", this->m_nServerID);
            }
        }
        else if (this->m_pcConfigData->s_get_ProfileType(iHotkeyIndex - 1) == PROFILE_LEVEL)
        {
            // if profile is in level mode, activate all channels in the selected channel range
            //-------------------------------------------------------------------------------------
            if (DEBUG_LOG) printf("Hotkey %d => level erkannt\n", iHotkeyIndex);

            // filter channel list
            pnFilteredChannelList = this->m_cChannelFilter.filter_channel_from_level(this->m_pcConfigData->s_get_MinChLevel(iHotkeyIndex - 1), this->m_pcConfigData->s_get_MaxChLevel(iHotkeyIndex - 1), this->m_pcConfigData->s_get_UseIgnoreListTx(iHotkeyIndex - 1));

            if (pnFilteredChannelList == nullptr)
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("hotkey_LevelOutRange"), this->m_pcConfigData->s_get_MinChLevel(iHotkeyIndex - 1), this->m_pcConfigData->s_get_MaxChLevel(iHotkeyIndex - 1), this->m_pcConfigData->s_get_ProfileName(iHotkeyIndex - 1).c_str(), this->m_cChannelFilter.get_channel_level(nChannelID));
                this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", this->m_nServerID);
            }
        }
        else if (this->m_pcConfigData->s_get_ProfileType(iHotkeyIndex - 1) == PROFILE_FREQUENCY)
        {
            // if profile is in frequency mode, activate clients instead of channels
            //-------------------------------------------------------------------------------------
            if (DEBUG_LOG) printf("Hotkey %d => frequency erkannt\n", iHotkeyIndex);

            pnFilteredClientList = this->m_cClientFilter.get_client_list(this->m_pcConfigData->s_get_ActiveFreq(iHotkeyIndex - 1), this->m_pcConfigData->s_get_UseIgnoreListTx(iHotkeyIndex - 1), !this->m_pcConfigData->s_get_PrioFreq(iHotkeyIndex - 1));

            if (pnFilteredClientList == nullptr)
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("hotkey_NoActiveClients"), this->m_pcConfigData->s_get_ActiveFreq(iHotkeyIndex - 1), this->m_pcConfigData->s_get_ProfileName(iHotkeyIndex - 1).c_str());
                this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_WARNING, "WhisperMaster2000", this->m_nServerID);
            }
        }

        //mark active profile
        this->m_nActualActiveProfile = iHotkeyIndex;
        this->m_pcConfigData->s_set_ActualState(iHotkeyIndex - 1, true);

        //activate whisperlist
        if ((pnFilteredChannelList != nullptr) || (pnFilteredClientList != nullptr))
        {
            // set filter list
            if (nError = this->m_pstTs3Functions->requestClientSetWhisperList(this->m_nServerID, this->m_nMyClientID, pnFilteredChannelList, pnFilteredClientList, NULL) != ERROR_ok)
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("error_ErrCreateWhisper"), "plugin_base::onHotkeyEvent", nError);
                this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_ERROR, "WhisperMaster2000", this->m_nServerID);
            }

            // Activate PTT on demand. Or deactivate it, if it was active before.
            if (this->m_pcConfigData->s_get_AutoActivate(iHotkeyIndex - 1)) activate_PTT(INPUT_ACTIVE);

            // print all filtered channel names (DEBUG)
            if (DEBUG_LOG && (pnFilteredChannelList != nullptr))
            {
                char* s;
                printf("PLUGIN: filtered channels:\n");
                for (int i = 0; pnFilteredChannelList[i]; i++)
                {
                    // Query channel name
                    if (nError = this->m_pstTs3Functions->getChannelVariableAsString(this->m_nServerID, pnFilteredChannelList[i], CHANNEL_NAME, &s) != ERROR_ok)
                    {
                        sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("error_ErrQueryChName"), "plugin_base::onHotkeyEvent", nError);
                        this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_ERROR, "WhisperMaster2000", this->m_nServerID);
                    }
                    printf("PLUGIN: Channel ID = %llu, name = %s\n", (long long unsigned int)pnFilteredChannelList[i], s);
                    this->m_pstTs3Functions->freeMemory(s);
                }
            }
            else if (DEBUG_LOG && (pnFilteredClientList != nullptr))
            {
                char *pcClientName;
                printf("PLUGIN: Clients selected\n");
                for (int i = 0; pnFilteredClientList[i]; i++)
                {
                    if (this->m_pstTs3Functions->getClientVariableAsString(this->m_nServerID, pnFilteredClientList[i], CLIENT_NICKNAME, &pcClientName) != ERROR_ok)
                        this->m_pstTs3Functions->logMessage("Error querying client nickname", LogLevel_DEBUG, "WhisperMaster2000", this->m_nServerID);
                    else
                    {
                        printf("PLUGIN: Client ID = %u, name = %s\n", pnFilteredClientList[i], pcClientName);
                        this->m_pstTs3Functions->freeMemory(pcClientName);
                    }
                }
            }

            // free filtered list after function is done
            if (pnFilteredChannelList != nullptr)
                this->m_pstTs3Functions->freeMemory(pnFilteredChannelList);
            if (pnFilteredClientList != nullptr)
                free(pnFilteredClientList);
        }
//        else
//        {
//            //communicate to user if no one is active
//            if (this->m_pcConfigData->s_get_ProfileType(iHotkeyIndex - 1) == PROFILE_FREQUENCY)
//                this->m_cSpeechEngine.say("NoClientFreq");
//            else
//                this->m_cSpeechEngine.say("NoClientPofile");
//        }

    }
    return;
}


/* ----------------------------------------------------------------------------
* handle request for info data to display in the right subwindow
*/
void plugin_handler::infoData(uint64 id, PluginItemType type, char ** data)
{
    int nIndex = 0;
    bool bUseSubChOfFav = false;
    std::string sText = "";
    uint64 nChannelID = id;
    uint64 *pnFilteredList;

    /* For demonstration purpose, display the name of the currently selected server, channel or client. */
    switch (type) {
    case PLUGIN_SERVER:
        sText.append(TRANSLATE_PTR("info_Default"));
        break;
    case PLUGIN_CLIENT:
    {
        //get ID of actual channel
        this->m_pstTs3Functions->getChannelOfClient(this->m_nServerID, (anyID)id, &nChannelID); //don't check for errors, behaves wrong!

        nIndex = this->m_cClientFilter.find_client((anyID)id);
        if (DEBUG_LOG && false)
        {
            //prepare debug client info
            char buff[100];
            if (nIndex >= 0)
                sprintf_s(buff, sizeof(buff), "%s (Id=%d, Ch=%llu, Entry=%d)", this->m_cClientFilter.m_cClientList[nIndex].sClientName.c_str(), this->m_cClientFilter.m_cClientList[nIndex].nClientID, this->m_cClientFilter.m_cClientList[nIndex].nActualChannelID, nIndex);
            else
                sprintf_s(buff, sizeof(buff), "missing client info");
            sText.append(buff);
        }

        //print all active frequencies of client
        if (nIndex >= 0)
        {
            if (sText.size() != 0) sText.append("\n");
            if (this->m_cClientFilter.m_cClientList[nIndex].bUseFreqList)
            {
                if (this->m_cClientFilter.m_cClientList[nIndex].iNumFreq > 0)
                {
                    sText.append(TRANSLATE_PTR("info_Freq"));
                    for (int ii = 0; ((ii < NUM_FREQUENCIES) && (this->m_cClientFilter.m_cClientList[nIndex].acFreqList[ii].nBit.nFreq > 0)); ii++)
                    {
                        if (ii > 0) sText.append(", ");
                        sText.append(std::to_string(this->m_cClientFilter.m_cClientList[nIndex].acFreqList[ii].nBit.nFreq));
                        if(this->m_cClientFilter.m_cClientList[nIndex].acFreqList[ii].nBit.nMute) sText.append(" [muted]");
                    }
                }
                else
                    sText.append(TRANSLATE_PTR("info_FreqDisabled"));
            }
            else
                sText.append(TRANSLATE_PTR("info_NoWM2000"));
        }
    }
    case PLUGIN_CHANNEL:
        for (int ii = 0; ii < this->m_pcConfigData->s_get_MaxNumProfiles(); ii++)
        {
            // generate channel list based on same filter like hotkey
            if ((this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_FAVORITE) || (this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_AUDIO))
            {
                pnFilteredList = this->m_cChannelFilter.filter_channel_from_list(this->m_pcConfigData->s_get_FavoriteList(ii), this->m_pcConfigData->s_get_UseSubChOfFav(ii), false);
                if (this->m_pcConfigData->s_get_UseSubChOfFav(ii)) bUseSubChOfFav = true;
            }
            else if (this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_LEVEL)
            {
                pnFilteredList = this->m_cChannelFilter.filter_channel_from_level(this->m_pcConfigData->s_get_MinChLevel(ii), this->m_pcConfigData->s_get_MaxChLevel(ii), false);
            }
            else if ((this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_FREQUENCY) && (type == PLUGIN_CLIENT) && (nIndex >= 0))
            {
                //print profile info if frequency of profile can be found in client FreqList
                bool bFreqFound = false;
                bool bMuted = false;
                bool bIgnored = false;
                for (int jj = 0; ((jj < NUM_FREQUENCIES) && (this->m_cClientFilter.m_cClientList[nIndex].acFreqList[jj].nBit.nFreq > 0)); jj++)
                {
                    if (this->m_cClientFilter.m_cClientList[nIndex].acFreqList[jj].nBit.nFreq == this->m_pcConfigData->s_get_ActiveFreq(ii))
                    {
                        bFreqFound = true;
                        bMuted = this->m_cClientFilter.m_cClientList[nIndex].acFreqList[jj].nBit.nMute;
                        break;
                    }
                }
                if ((this->m_cChannelFilter.find_channel_in_list(this->m_pcConfigData->s_get_IgnoreList(), this->m_cClientFilter.m_cClientList[nIndex].nActualChannelID) >= 0) && this->m_pcConfigData->s_get_UseIgnoreListTx(ii))
                    bIgnored = true;

                if (bFreqFound)
                {
                    if (sText.size() != 0) sText.append("\n");
                    sText.append("[I]\"");
                    sText.append(this->m_pcConfigData->s_get_ProfileName(ii));
                    sText.append("\"[/I] (Freq. ");
                    sText.append(std::to_string(this->m_pcConfigData->s_get_ActiveFreq(ii)));
                    if (bMuted) sText.append(" [muted]");
                    if (bIgnored) sText.append(" [ignored]");
                    sText.append(")");
                }
                continue;
            }
            else
                pnFilteredList = nullptr;


            //check if selected filter is part of channel list
            if (pnFilteredList != nullptr)
            {
                for (int jj = 0; pnFilteredList[jj] != 0; jj++)
                {
                    if (pnFilteredList[jj] == nChannelID)
                    {
                        bool bIgnored = false;
                        if (sText.size() != 0) sText.append("\n");
                        sText.append("[I]\"");
                        sText.append(this->m_pcConfigData->s_get_ProfileName(ii));
                        sText.append("\"[/I]");

                        //if subchannel are used, try to find it in the master list
                        if (bUseSubChOfFav)
                            if (this->m_pcConfigData->s_find_entry(this->m_pcConfigData->s_get_FavoriteList(ii), this->m_cChannelFilter.get_channel_info(nChannelID)) < 0)
                                sText.append(" (sub channel)");

                        if ((this->m_cChannelFilter.find_channel_in_list(this->m_pcConfigData->s_get_IgnoreList(), nChannelID) >= 0) && this->m_pcConfigData->s_get_UseIgnoreListTx(ii))
                            bIgnored = true;

                        //if it is a level based profile, add actual level
                        if (this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_LEVEL)
                            sText.append(std::string(" (level ") + std::to_string(this->m_cChannelFilter.get_channel_level(nChannelID)) + std::string(")"));

                        if (bIgnored) sText.append(" [ignored]");
                        break;
                    }
                }

                // free filtered list after function is done
                this->m_pstTs3Functions->freeMemory(pnFilteredList);
            }
        }

        // check ignore list
        if (this->m_pcConfigData->s_find_entry(this->m_pcConfigData->s_get_IgnoreList(), this->m_cChannelFilter.get_channel_info(nChannelID)) > 0)
        {
            if (sText.size() != 0) sText.append("\n");
            sText.append(TRANSLATE_PTR("info_Ignore"));
        }

        //if no profile matches, write default string
        if (sText.size() == 0)
            sText.append(TRANSLATE_PTR("info_Default"));

        break;
    default:
        printf("Invalid item type: %d\n", type);
        data = NULL;  /* Ignore */
        return;
    }

    // limit length of string
    if (sText.size() > 120)
    {
        printf("reduce info length %zd => 120\n", sText.size());
        sText = sText.substr(0, 120);
        sText.append("\n...");
    }

    // allocate memory for the buffer
    *data = (char*)malloc(INFODATA_BUFSIZE * sizeof(char));
    // copy string to buffer
    sprintf_s(*data, INFODATA_BUFSIZE, "%s", sText.c_str());  // bbCode is supported. HTML is not supported
    return;
}


/*
* Talk status event
*/
void plugin_handler::onTalkStatusChangeEvent(int iStatus, int iIsReceivedWhisper, anyID nClientID)
{
    // Demonstrate usage of getClientDisplayName
    int iClientIdx = this->m_cClientFilter.find_client(nClientID);
    if (iClientIdx >= 0)
    {
        if (iStatus == STATUS_TALKING)
        {
            printf("--> %s starts %s\n", this->m_cClientFilter.m_cClientList[iClientIdx].sClientName.c_str(), iIsReceivedWhisper == 0 ? "talking" : "whispering");
        }
        else
        {
            printf("--> %s stops %s\n", this->m_cClientFilter.m_cClientList[iClientIdx].sClientName.c_str(), iIsReceivedWhisper == 0 ? "talking" : "whispering");
        }
    }
}


/* 
* Menu "Show all lists" was triggered
*/
void plugin_handler::print_all_lists()
{
    const size_t nBuffSize = 512;
    char cBuffer[nBuffSize];
    char *pcChName;

    // add some new lines
    sprintf_s(cBuffer, nBuffSize, "\n\n");
    this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);

    //print ignore list
    sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_IgnoreInfo"), m_pcConfigData->s_get_IgnoreList()->size() - 1);
    this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);
    if (m_pcConfigData->s_get_IgnoreList()->size() > 1)
    {
        for (int ii = 1; ii < m_pcConfigData->s_get_IgnoreList()->size(); ii++)
        {
            sprintf_s(cBuffer, nBuffSize, "+____%3d: %s\n", ii, (*m_pcConfigData->s_get_IgnoreList())[ii].sChannelName.c_str());
            this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);
        }
    }
    else
    {
        sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_NoChInList"));
        this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);
    }

    //print profiles
    for (int ii = 0; ii < this->m_pcConfigData->s_get_MaxNumProfiles(); ii++)
    {
        // Type "favorite" + "audio" lists
        if ((this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_FAVORITE) || (this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_AUDIO))
        {
            sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_FavoriteInfo"), this->m_pcConfigData->s_get_ProfileName(ii).c_str(), this->m_pcConfigData->s_get_FavoriteList(ii)->size() - 1);
            this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);

            // print channel list based on same filter like hotkey
            uint64 *pnFilteredList = this->m_cChannelFilter.filter_channel_from_list(this->m_pcConfigData->s_get_FavoriteList(ii), this->m_pcConfigData->s_get_UseSubChOfFav(ii), false);
            if (pnFilteredList != nullptr)
            {
                for (int jj = 0; pnFilteredList[jj] != 0; jj++)
                {
                    bool bIsSubChannel = false;
                    bool bIgnored = false;

                    //if subchannel are used, try to find it in the master list
                    if (this->m_pcConfigData->s_get_UseSubChOfFav(ii))
                        if (this->m_pcConfigData->s_find_entry(this->m_pcConfigData->s_get_FavoriteList(ii), this->m_cChannelFilter.get_channel_info(pnFilteredList[jj])) < 0)
                            bIsSubChannel = true;;

                    if ((this->m_cChannelFilter.find_channel_in_list(this->m_pcConfigData->s_get_IgnoreList(), pnFilteredList[jj]) >= 0) && this->m_pcConfigData->s_get_UseIgnoreListTx(ii))
                        bIgnored = true;

                    this->m_pstTs3Functions->getChannelVariableAsString(this->m_nServerID, pnFilteredList[jj], CHANNEL_NAME, &pcChName);
                    if (pcChName != nullptr)
                    {
                        sprintf_s(cBuffer, nBuffSize, "+____%3d: %s%s%s\n", jj + 1, pcChName, bIsSubChannel ? "[I](sub channel)[/I]" : "", bIgnored ? " [ignored]" : "");
                        this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);
                        this->m_pstTs3Functions->freeMemory(pcChName);
                    }
                }
                // free filtered list after function is done
                this->m_pstTs3Functions->freeMemory(pnFilteredList);
            }
            else
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_NoChInList"));
                this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);
            }
        }
        else if (this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_LEVEL)
        {
            // Type "Level" depending on selected range and position
            if (this->m_pcConfigData->s_get_MaxChLevel(ii) != 0)
                sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_LevelInfo1"), this->m_pcConfigData->s_get_ProfileName(ii).c_str(), this->m_pcConfigData->s_get_MinChLevel(ii), this->m_pcConfigData->s_get_MaxChLevel(ii));
            else
                sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_LevelInfo2"), this->m_pcConfigData->s_get_ProfileName(ii).c_str(), this->m_pcConfigData->s_get_MinChLevel(ii));
            this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);

            // print channel list based on same filter like hotkey
            uint64 *pnFilteredList = this->m_cChannelFilter.filter_channel_from_level(this->m_pcConfigData->s_get_MinChLevel(ii), this->m_pcConfigData->s_get_MaxChLevel(ii), false);
            if (pnFilteredList != nullptr)
            {
                for (int jj = 0; pnFilteredList[jj] != 0; jj++)
                {
                    bool bIgnored = false;

                    if ((this->m_cChannelFilter.find_channel_in_list(this->m_pcConfigData->s_get_IgnoreList(), pnFilteredList[jj]) >= 0) && this->m_pcConfigData->s_get_UseIgnoreListTx(ii))
                        bIgnored = true;

                    this->m_pstTs3Functions->getChannelVariableAsString(this->m_nServerID, pnFilteredList[jj], CHANNEL_NAME, &pcChName);
                    if (pcChName != nullptr)
                    {
                        sprintf_s(cBuffer, nBuffSize, "+____%3d: %s [I](level %zd)[/I]%s\n", jj + 1, pcChName, this->m_cChannelFilter.get_channel_level(pnFilteredList[jj]), bIgnored ? " [ignored]" : "");
                        this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);
                        this->m_pstTs3Functions->freeMemory(pcChName);
                    }
                }
                // free filtered list after function is done
                this->m_pstTs3Functions->freeMemory(pnFilteredList);
            }
            else
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_NoChInRange"));
                this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);
            }
        }
        else if (this->m_pcConfigData->s_get_ProfileType(ii) == PROFILE_FREQUENCY)
        {
            // Type "Frequency" depending on selected frequency
            sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_FreqInfo"), this->m_pcConfigData->s_get_ActiveFreq(ii), this->m_pcConfigData->s_get_ProfileName(ii).c_str(), this->m_cClientFilter.m_cClientList.size(), this->m_pcConfigData->s_get_MuteFreq(ii) ? " => muted" : "");
            this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);

            if (this->m_pcConfigData->s_get_ActiveFreq(ii) != 0)
            {
                //if frequency is valid, try to find other clients on same frequency
                std::vector<int> vActiveClient = this->m_cClientFilter.get_client_list_idx(this->m_pcConfigData->s_get_ActiveFreq(ii), false, false);
                if (vActiveClient.size() > 0)
                {
                    //print all active clients
                    for (int jj = 0; jj < vActiveClient.size(); jj++)
                    {
                        //check if client is muted/ignored or not
                        bool bMuted = false;
                        bool bIgnored = false;
                        for (int kk = 0; ((kk < NUM_FREQUENCIES) && (this->m_cClientFilter.m_cClientList[vActiveClient[jj]].acFreqList[kk].nBit.nFreq > 0)); kk++)
                        {
                            if (this->m_cClientFilter.m_cClientList[vActiveClient[jj]].acFreqList[kk].nBit.nFreq == this->m_pcConfigData->s_get_ActiveFreq(ii))
                            {
                                bMuted = this->m_cClientFilter.m_cClientList[vActiveClient[jj]].acFreqList[kk].nBit.nMute;
                                break;
                            }
                        }
                        if((this->m_cChannelFilter.find_channel_in_list(this->m_pcConfigData->s_get_IgnoreList(), this->m_cClientFilter.m_cClientList[vActiveClient[jj]].nActualChannelID) >= 0) && this->m_pcConfigData->s_get_UseIgnoreListTx(ii))
                            bIgnored = true;

                        //print name and state of client
                        sprintf_s(cBuffer, nBuffSize, "+____%3d: %s%s%s\n", jj + 1, this->m_cClientFilter.m_cClientList[vActiveClient[jj]].sClientName.c_str(), bMuted ? " [muted]" : "", bIgnored ? " [ignored]" : "");
                        this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);
                    }
                }
                else
                {
                    sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_NoClientInList"));
                    this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);
                }
            }
            else
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_FreqDisabled"));
                this->m_pstTs3Functions->printMessageToCurrentTab(cBuffer);
            }
        }
    }
}


/*
* update meta data of client
*/
void plugin_handler::update_meta_data()
{
    this->m_cClientFilter.set_meta_data();
}


/*
* update ignore list
*/
void plugin_handler::toggle_ignore(uint64 nChannel)
{
    const size_t nBuffSize = 512;
    char cBuffer[nBuffSize];

    // check if new channel is in list...
    channel_info stChInfo = this->m_cChannelFilter.get_channel_info(nChannel);
    int nEntry = this->m_pcConfigData->s_find_entry(this->m_pcConfigData->s_get_IgnoreList(), stChInfo);
    if (nEntry < 0) //... and add it, if it was not found...
    {
        if (this->m_pcConfigData->s_add_entry(this->m_pcConfigData->s_get_IgnoreList(), stChInfo) == IDNOTFOUND_MEM_FULL)
        {
            sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_ListFull"), "Ignore", this->m_pcConfigData->s_get_IgnoreList()->size() - 1, this->m_pcConfigData->s_get_IgnoreList()->capacity() - 1);
            this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_ERROR, "WhisperMaster2000", this->m_nServerID);
        }
    }
    else //... or delete it from the list
        this->m_pcConfigData->s_delete_entry(this->m_pcConfigData->s_get_IgnoreList(), nEntry);

    if (DEBUG_LOG) printf("Id %llu, IsPermanent %d, Parent %llu, Name %s, Invalid %d (Entry %d)\n", stChInfo.nChannelID, stChInfo.bIsPermanent, stChInfo.nChannelParent, stChInfo.sChannelName.c_str(), stChInfo.iInvalidCount, nEntry);
}

/*
* update favorite list
* add or clear selected channel from channel list (favorite + audio)
*/
void plugin_handler::toggle_favorite(int nProfile, uint64 nChannel)
{
    const size_t nBuffSize = 512;
    char cBuffer[nBuffSize];

    if ((this->m_pcConfigData->s_get_ProfileType(nProfile) != PROFILE_FAVORITE) && (this->m_pcConfigData->s_get_ProfileType(nProfile) != PROFILE_AUDIO))
    {
        if (DEBUG_LOG) printf("PLUGIN: Falsches menue fuer config favorite / audio\n");
    }
    else
    {
        // get server name
        char* pcServerName;
        std::string sServerName;
        this->m_pstTs3Functions->getServerVariableAsString(this->m_nServerID, VIRTUALSERVER_NAME, &pcServerName);
        sServerName = pcServerName;
        this->m_pstTs3Functions->freeMemory(pcServerName);

        //compare server name with profile
        if (this->m_pcConfigData->s_get_ServerName(nProfile).compare(sServerName) != 0)
        {
            //if not equal, clean up and set new server name
            if (this->m_pcConfigData->s_get_ServerName(nProfile).compare("") != 0)
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_NameChange"), this->m_pcConfigData->s_get_ProfileName(nProfile).c_str(), this->m_pcConfigData->s_get_ServerName(nProfile).c_str(), sServerName.c_str());
                this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_INFO, "WhisperMaster2000", this->m_nServerID);
            }

            this->m_pcConfigData->s_get_ServerName(nProfile) = sServerName;
            this->m_pcConfigData->clear_vector(this->m_pcConfigData->s_get_FavoriteList(nProfile));
        }

        // check if new channel is in list...
        channel_info stChInfo = this->m_cChannelFilter.get_channel_info(nChannel);
        int nEntry = this->m_pcConfigData->s_find_entry(this->m_pcConfigData->s_get_FavoriteList(nProfile), stChInfo);
        if (nEntry < 0) //... and add it, if it was not found...
        {
            if (this->m_pcConfigData->s_add_entry(this->m_pcConfigData->s_get_FavoriteList(nProfile), stChInfo) == IDNOTFOUND_MEM_FULL)
            {
                sprintf_s(cBuffer, nBuffSize, TRANSLATE_PTR("menuEv_ListFull"), this->m_pcConfigData->s_get_ProfileName(nProfile).c_str(), this->m_pcConfigData->s_get_FavoriteList(nProfile)->size() - 1, this->m_pcConfigData->s_get_FavoriteList(nProfile)->capacity() - 1);
                this->m_pstTs3Functions->logMessage(cBuffer, LogLevel_ERROR, "WhisperMaster2000", this->m_nServerID);
            }
        }
        else //... or delete it from the list
            this->m_pcConfigData->s_delete_entry(this->m_pcConfigData->s_get_FavoriteList(nProfile), nEntry);

        if (DEBUG_LOG) printf("Id %llu, IsPermanent %d, Parent %llu, Name %s, Invalid %d (Entry %d)\n", stChInfo.nChannelID, stChInfo.bIsPermanent, stChInfo.nChannelParent, stChInfo.sChannelName.c_str(), stChInfo.iInvalidCount, nEntry);
    }
}


/*
* set new channel level of profile
*/
void plugin_handler::set_profile_level(int nProfile, uint64 nChannel, bool bIsStart)
{
    if(bIsStart)
    {
        // set new start level, but keep actual end level in mind
        //-------------------------------------------------------------------------------------
        if (this->m_pcConfigData->s_get_ProfileType(nProfile) != PROFILE_LEVEL)
        {
            if (DEBUG_LOG) printf("PLUGIN: Falsches menue fuer config level\n");
        }
        else
        {
            // filter channel level
            size_t nNewLevel = m_cChannelFilter.get_channel_level(nChannel);
            if (nNewLevel >= 1)
            {
                this->m_pcConfigData->s_set_MinChLevel(nProfile, nNewLevel);
                if ((nNewLevel > this->m_pcConfigData->s_get_MaxChLevel(nProfile)) && (this->m_pcConfigData->s_get_MaxChLevel(nProfile) != 0))
                    this->m_pcConfigData->s_set_MaxChLevel(nProfile, nNewLevel);
            }
            if (DEBUG_LOG) printf("Set Level P%d range %zd - %zd\n", nProfile + 1, this->m_pcConfigData->s_get_MinChLevel(nProfile), this->m_pcConfigData->s_get_MaxChLevel(nProfile));
        }
    }
    else
    {
        // set new end level, but keep actual start level in mind
        //-------------------------------------------------------------------------------------
        if ((this->m_pcConfigData->s_get_ProfileType(nProfile) != PROFILE_LEVEL) || (this->m_pcConfigData->s_get_MaxChLevel(nProfile) == 0))
        {
            if (DEBUG_LOG) printf("PLUGIN: Falsches menue fuer config level\n");
        }
        else
        {
            // filter channel level
            size_t nNewLevel = m_cChannelFilter.get_channel_level(nChannel);
            if (nNewLevel >= 1)
            {
                this->m_pcConfigData->s_set_MaxChLevel(nProfile, nNewLevel);
                if (nNewLevel < this->m_pcConfigData->s_get_MinChLevel(nProfile))
                    this->m_pcConfigData->s_set_MaxChLevel(nProfile, this->m_pcConfigData->s_get_MinChLevel(nProfile));
            }
            if (DEBUG_LOG) printf("Set Level P%d range %zd - %zd\n", nProfile + 1, this->m_pcConfigData->s_get_MinChLevel(nProfile), this->m_pcConfigData->s_get_MaxChLevel(nProfile));
        }
    }
}



/*
*   Debug only
*/
void plugin_handler::internal_write_err(const char* pFuncName)
{
    FILE *pFile;

    std::string sLogPath = "c:/Users/micha/AppData/Roaming/TS3Client/plugins/";
    sLogPath = sLogPath + "WhisperMaster2000/wm2000_error.log";

    fopen_s(&pFile, sLogPath.c_str(), "a");
    if (pFile != nullptr)
    {
        fprintf_s(pFile, "DEBUG: %s\n", pFuncName);
        fclose(pFile);
    }
}