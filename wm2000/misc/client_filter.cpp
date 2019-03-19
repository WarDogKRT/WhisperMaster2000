#include "client_filter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <cstring>
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"

/* ----------------------------------------------------------------------------
* constructor
*/
client_filter::client_filter()
{
    m_cClientList.reserve(1025);    //dummy value maybe request correct value by calling requestServerVariables => VIRTUALSERVER_MAXCLIENTS??
    this->m_nServerID                   = 0;
    this->m_nMyClientID                 = 0;
    this->m_nMyChannelID                = INVALID_CHANNEL_ID;
    this->m_pConfigContainer            = nullptr;
    this->m_pCannelFilter               = nullptr;
}


/* ----------------------------------------------------------------------------
* destructor
*/
client_filter::~client_filter()
{
}


/* ----------------------------------------------------------------------------
* set TS3 interface functions
*/
void client_filter::init(struct TS3Functions *pstTs3Functions, config_container *pConfigContainer, channel_filter *pCannelFilter, uint64 nServerConnectionHandlerID, uint64 nChannelID, anyID nClientID)
{
    this->m_pstTs3Functions     = pstTs3Functions;
    this->m_pConfigContainer    = pConfigContainer;
    this->m_pCannelFilter       = pCannelFilter;
    this->m_nServerID           = nServerConnectionHandlerID;
    this->m_nMyClientID         = nClientID;
    this->m_nMyChannelID        = nChannelID;

    // get server name
    char* pcServerName;
    this->m_pstTs3Functions->getServerVariableAsString(this->m_nServerID, VIRTUALSERVER_NAME, &pcServerName);
    this->m_sServerName = pcServerName;
    this->m_pstTs3Functions->freeMemory(pcServerName);
}


/* ----------------------------------------------------------------------------
*   get new info from server
*      if nActChannel is unknown when calling this function, set to -1
*/
void client_filter::update_client_list(anyID nClientID, uint64 nActChannel)
{
    //my own client should not be part of this list
    if (this->m_nMyClientID == nClientID)
    {
        if (nActChannel != INVALID_CHANNEL_ID) m_nMyChannelID = nActChannel;

        //TS3_VECTOR position = { 0.0, 0.0, 0.0 };
        //TS3_VECTOR up       = { 0.0, 0.0, 0.0 };
        //TS3_VECTOR forward  = { 0.0, 0.0, 0.0 };
        //if (this->m_nMyChannelID != INVALID_CHANNEL_ID)
        //{
        //    position.x = 0.0;
        //    position.y = 0.0;
        //    position.z = 0.0;
        //
        //    up.x = 0;
        //    up.y = 1;
        //    up.z = 0;
        //
        //    this->m_pstTs3Functions->systemset3DListenerAttributes(this->m_nServerID, &position, NULL, &up);
        //
        //    //this->m_pstTs3Functions->channelset3DAttributes(this->m_nServerID, nClientID, &position);
        //    printf("Set own client 3D Audio (%llu != %llu)\n", nActChannel, this->m_nMyChannelID);
        //}

        return;
    }

    //make sure, no one is working on this
    this->m_cClientListMutex.lock();

    //try to find client
    int nIndex = find_client(nClientID);

    if (nActChannel != 0)
    {
        // user connected or settings changed
        if (nIndex < 0)
        {
            //if client was not found, add new entry
            client_info cNewEntry;

            //initialize client
            cNewEntry.nClientID = nClientID;
            char *pcClientName;
            if (this->m_pstTs3Functions->getClientVariableAsString(this->m_nServerID, nClientID, CLIENT_NICKNAME, &pcClientName) != ERROR_ok)
                this->m_pstTs3Functions->logMessage("Error querying client nickname", LogLevel_DEBUG, "WhisperMaster2000", this->m_nServerID);
            else
            {
                cNewEntry.sClientName = pcClientName;
                this->m_pstTs3Functions->freeMemory(pcClientName);
            }
            
            cNewEntry.bUseFreqList = false;
            cNewEntry.iNumFreq = 0;
            memset(cNewEntry.acFreqList, 0, sizeof(freq_data)*NUM_FREQUENCIES);
            cNewEntry.nActualChannelID = 0;

            //add client
            this->m_cClientList.push_back(cNewEntry);
            nIndex = ((int)this->m_cClientList.size()) - 1;

            printf("Client (%d) added (%zd)\n", nClientID, this->m_cClientList.size());
        }

        //track actual channel (if nActChannel is unknown when calling this function, it is set to -1)
        if ((nActChannel != INVALID_CHANNEL_ID) && (this->m_cClientList[nIndex].nActualChannelID != nActChannel))
        {
            this->m_cClientList[nIndex].nActualChannelID = nActChannel;

            //this->m_pstTs3Functions->allowWhispersFrom(this->m_nServerID, nClientID);
            //this->m_pstTs3Functions->removeFromAllowedWhispersFrom(this->m_nServerID, nClientID);
            //this->m_pstTs3Functions->requestMuteClients(this->m_nServerID, NULL, NULL);
        }

        //track frequency list
        char *pcMetaData;
        int nError;
        if (nError = this->m_pstTs3Functions->getClientVariableAsString(this->m_nServerID, nClientID, CLIENT_META_DATA, &pcMetaData) == ERROR_ok)
        {
            std::string     sMetaData = pcMetaData;
            printf("Check client (%d / %d) Meta Data: %s", nClientID, nIndex, pcMetaData);
            this->m_pstTs3Functions->freeMemory(pcMetaData);

            //check if user uses WhisperMaster and add client if yes
            if (parse_meta_data(sMetaData, nullptr) == 0)
            {
                //parse meta data and write to freq. list
                this->m_cClientList[nIndex].iNumFreq = parse_meta_data(sMetaData, this->m_cClientList[nIndex].acFreqList);
                this->m_cClientList[nIndex].bUseFreqList = true;
                printf(" => valid\n");
            }
            else if (this->m_cClientList[nIndex].bUseFreqList)
            {
                //if client was previously markt as WM2000 user, reset everthing
                memset(this->m_cClientList[nIndex].acFreqList, 0, sizeof(freq_data)*NUM_FREQUENCIES);
                this->m_cClientList[nIndex].iNumFreq = 0;
                this->m_cClientList[nIndex].bUseFreqList = false;
                printf(" => disabled\n");
            }
            else
                printf(" => invalid (%d)\n", parse_meta_data(sMetaData, nullptr));
        }
        else
        {
            printf("FAILED to get client (%d) Meta Data. ERROR: 0x%04X\n", nClientID, nError);
        }
    }
    else
    {
        // user disconnected
        if (nIndex >= 0)
        {
            this->m_cClientList.erase(this->m_cClientList.begin() + nIndex);
            printf("Client (%d) removed (%zd)\n", nClientID, this->m_cClientList.size());
        }
    }

    //release lock
    this->m_cClientListMutex.unlock();
}


/* ----------------------------------------------------------------------------
*   create meta data from profiles
*/
bool client_filter::set_meta_data()
{
    char *pcOldMetaData;
    std::string sOldMetaData, sNewMetaData;

    if (this->m_pstTs3Functions->getClientSelfVariableAsString(this->m_nServerID, CLIENT_META_DATA, &pcOldMetaData) != ERROR_ok)
        return false; //error ocured send this back to caller

    //convert data to string
    sOldMetaData = pcOldMetaData;
    this->m_pstTs3Functions->freeMemory(pcOldMetaData);

    //clean up old string
    size_t nStart = sOldMetaData.find("#WhisperMaster2000[", 0);
    if (nStart != std::string::npos)
    {
        //filter leading char
        sNewMetaData.append(sOldMetaData.substr(0, nStart));

        //filter following char, only if magic word was found
        size_t nEnd = sOldMetaData.find(",];", nStart);
        if (nEnd != std::string::npos)
            sNewMetaData.append(sOldMetaData.substr(nEnd + 3, sOldMetaData.size()));
    }
    else
        sNewMetaData = sOldMetaData;

    //show settings, if string is not empty after clean up
    if (sNewMetaData.size() > 0)
        printf("Old MetaData found => \"%s\"\n", sNewMetaData.c_str());

    //create new string
    int iNumFreq = 0;
    sNewMetaData.append("#WhisperMaster2000[");
    for (int ii = 0; ii < this->m_pConfigContainer->s_get_MaxNumProfiles(); ii++)
    {
        //if                         profile is type FREQUENCY                and                    actual freq is valid
        if ((this->m_pConfigContainer->s_get_ProfileType(ii) == PROFILE_FREQUENCY) && (this->m_pConfigContainer->s_get_ActiveFreq(ii) != 0) )
        {
            //initialize parameter of frequency
            freq_data nTemp;
            nTemp.nWord = 0;
            nTemp.nBit.nFreq     = this->m_pConfigContainer->s_get_ActiveFreq(ii);
            nTemp.nBit.nMute     = this->m_pConfigContainer->s_get_MuteFreq(ii);
            nTemp.nBit.nSquelch  = this->m_pConfigContainer->s_get_SquelchFreq(ii);
            nTemp.nBit.nPriority = this->m_pConfigContainer->s_get_PrioFreq(ii);

            //add to string
            sNewMetaData.append(std::to_string(nTemp.nWord));
            sNewMetaData.append(",");
            iNumFreq++;
        }
    }
    if(iNumFreq == 0)
        sNewMetaData.append(",");
    sNewMetaData.append("];");


    //add new settings to old one
    printf("New MetaData => \"%s\"\n", sNewMetaData.c_str());

    // write data to server
    if (this->m_pstTs3Functions->setClientSelfVariableAsString(this->m_nServerID, CLIENT_META_DATA, sNewMetaData.c_str()) != ERROR_ok)
        return false; //error ocured send this back to caller
    if (this->m_pstTs3Functions->flushClientSelfUpdates(this->m_nServerID, NULL) != ERROR_ok)
        return false; //error ocured send this back to caller

    // tell caller, everthing is fine
    return true;
}


/* ----------------------------------------------------------------------------
*   parse meta data and return frequency list
*/
int client_filter::parse_meta_data(std::string sMetaData, freq_data acFreqList[])
{
    // initialize frequency list
    if (acFreqList != nullptr)
        memset(acFreqList, 0, sizeof(freq_data)*NUM_FREQUENCIES);

    // create sub string of WhisperMaster parameter only
    size_t nStartEnd = 0;
    nStartEnd = sMetaData.find("#WhisperMaster2000[", 0);
    if (nStartEnd == std::string::npos)
        return -1;
    sMetaData = sMetaData.substr(nStartEnd, sMetaData.size() - nStartEnd);
    nStartEnd = sMetaData.find(",];", 0);
    if (nStartEnd == std::string::npos)
        return -2;
    sMetaData = sMetaData.substr(0, nStartEnd + 3);

    // if we just want to know, if user uses Whispermaster, return 1
    if (acFreqList == nullptr)
        return 0;

    // convert string to frequency list
    return sscanf_s(sMetaData.c_str(), "#WhisperMaster2000[%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u];", &acFreqList[0].nWord, &acFreqList[1].nWord, &acFreqList[2].nWord, &acFreqList[3].nWord, &acFreqList[4].nWord, &acFreqList[5].nWord, &acFreqList[6].nWord, &acFreqList[7].nWord, &acFreqList[8].nWord, &acFreqList[9].nWord, &acFreqList[10].nWord, &acFreqList[11].nWord, &acFreqList[12].nWord, &acFreqList[13].nWord, &acFreqList[14].nWord, &acFreqList[15].nWord, &acFreqList[16].nWord, &acFreqList[17].nWord, &acFreqList[18].nWord, &acFreqList[19].nWord);
}


/* ----------------------------------------------------------------------------
*   find client in m_cClientList
*/
int  client_filter::find_client(anyID nClientID)
{
    int nIndex = -1;

    for (int ii = 0; ii < this->m_cClientList.size(); ii++)
    {
        if (this->m_cClientList[ii].nClientID == nClientID)
        {
            nIndex = ii;
            break;
        }
    }

    return nIndex;
}


/* ----------------------------------------------------------------------------
*   find iFreq in freq list of client iClient
*/
int client_filter::find_active_freq(int iClient, int iFreq, bool bCheckIgnore, bool bCheckSquelch, bool bCheckParam)
{
    int iFreqIdx = -1;

    // only if client uses WM2000 and freq list at least one valid entry
    if ((this->m_cClientList[iClient].bUseFreqList) && (this->m_cClientList[iClient].iNumFreq > 0))
    {
        for (int jj = 0; jj < this->m_cClientList[iClient].iNumFreq; jj++)
        {
            if (this->m_cClientList[iClient].acFreqList[jj].nBit.nFreq == iFreq)
            {
                //only if not Muted, not Ignored (when useIgnore) and not Squelche (while not Priority)
                if (!this->m_cClientList[iClient].acFreqList[jj].nBit.nMute || !bCheckParam)
                    if(!this->m_cClientList[iClient].acFreqList[jj].nBit.nSquelch || !bCheckSquelch)
                        if((this->m_pCannelFilter->find_channel_in_list(this->m_pConfigContainer->s_get_IgnoreList(), this->m_cClientList[iClient].nActualChannelID) < 0) || !bCheckIgnore)
                            iFreqIdx = jj;
                break;
            }
        }
    }

    return iFreqIdx;
}

/* ----------------------------------------------------------------------------
*   find a frequency that is currently unused
*/
int client_filter::get_next_free_freq(int iStartFreq)
{
    int iFreeFreq = 0;
    std::vector<int> vActiveClients;

    //make sure, start index within index range
    if (iStartFreq < 1)
        iStartFreq = 1;

    //find first frequency by searching in the actual client list
    for (int ii = iStartFreq; ii <= this->m_pConfigContainer->s_get_MaxNumFreq(); ii++)
    {
        vActiveClients = get_client_list_idx(ii, false, false);
        if (vActiveClients.size() == 0)
        {
            iFreeFreq = ii;
            break;
        }
    }

    //check if this frequency is already in use by our client
    if (iFreeFreq != 0)
    {
        for (int ii = 0; ii < this->m_pConfigContainer->s_get_MaxNumProfiles(); ii++)
        {
            if ((this->m_pConfigContainer->s_get_ProfileType(ii) == PROFILE_FREQUENCY) && (this->m_pConfigContainer->s_get_ActiveFreq(ii) == iFreeFreq))
            {
                //if there is are possible frequencies to search for, try again
                if (iFreeFreq < this->m_pConfigContainer->s_get_MaxNumFreq())
                    iFreeFreq = get_next_free_freq(iFreeFreq + 1);
                else
                {
                    iFreeFreq = 0;
                    break;
                }
            }
        }
    }

    return iFreeFreq;
}

/* ----------------------------------------------------------------------------
* return name of the given channel ID
*/
std::string client_filter::get_channel_name(uint64 nChannelID)
{
    char *pcChName;
    std::string sChannelName = "unknown channel";

    if (this->m_pstTs3Functions->getChannelVariableAsString(this->m_nServerID, nChannelID, CHANNEL_NAME, &pcChName) == ERROR_ok)
    {
        if (pcChName != nullptr)
        {
            sChannelName = pcChName;
            this->m_pstTs3Functions->freeMemory(pcChName);
        }
    }

    return sChannelName;
}

/* ----------------------------------------------------------------------------
*   filter all clients with activ frequency iFreq and return a vector of idx
*   to m_cClientList
*/
std::vector<int> client_filter::get_client_list_idx(int iFreq, bool bCheckIgnore, bool bCheckSquelch, bool bCheckParam)
{
    std::vector<int> vActiveClients;
    vActiveClients.reserve(this->m_cClientList.size());

    for (int ii = 0; ii < this->m_cClientList.size(); ii++)
    {
        if (find_active_freq(ii, iFreq, bCheckIgnore, bCheckSquelch, bCheckParam) >= 0)
        {
            vActiveClients.push_back(ii);
        }
    }

    return vActiveClients;
}

/* ----------------------------------------------------------------------------
*   filter all clients with activ frequency iFreq and return a vector of 
*   client IDs
*   list must be deleted after usage!
*/
anyID * client_filter::get_client_list(int iFreq, bool bCheckIgnore, bool bCheckSquelch)
{
    anyID *pClientList = nullptr;

    //get client list as Idx
    std::vector<int> vActiveClients = get_client_list_idx(iFreq, bCheckIgnore, bCheckSquelch);

    //convert client list to IDs
    if (vActiveClients.size() > 0)
    {
        pClientList = (anyID*)malloc((vActiveClients.size() + 1) * sizeof(anyID));

        //copy client id of all active clients
        for (int ii = 0; ii < vActiveClients.size(); ii++)
            pClientList[ii] = this->m_cClientList[vActiveClients[ii]].nClientID;

        //set last element to 0
        pClientList[vActiveClients.size()] = 0;
    }

    return pClientList;
}
