#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <cstring>
#include "misc/channel_filter.h"
#include "base/plugin_base.h"
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_rare_definitions.h"
#include "teamspeak/clientlib_publicdefinitions.h"

/* ----------------------------------------------------------------------------
* constructor
*/
channel_filter::channel_filter()
{
    this->m_pstTs3Functions     = nullptr;
    this->m_pConfigContainer    = nullptr;
    this->m_nServerID           = 0;
    this->m_nMyClientID         = 0;
}

/* ----------------------------------------------------------------------------
* destructor
*/
channel_filter::~channel_filter()
{
}

/* ----------------------------------------------------------------------------
* set TS3 interface functions
*/
void channel_filter::init(struct TS3Functions *pstTs3Functions, config_container *pConfigContainer, uint64 nServerConnectionHandlerID, anyID nClientID)
{
    this->m_pstTs3Functions     = pstTs3Functions;
    this->m_pConfigContainer    = pConfigContainer;
    this->m_nServerID           = nServerConnectionHandlerID;
    this->m_nMyClientID         = nClientID;

    // get server name
    char* pcServerName;
    this->m_pstTs3Functions->getServerVariableAsString(this->m_nServerID, VIRTUALSERVER_NAME, &pcServerName);
    this->m_sServerName = pcServerName;
    this->m_pstTs3Functions->freeMemory(pcServerName);
}

/* ----------------------------------------------------------------------------
* search for a specific channel within a channel list
*/
int channel_filter::find_channel_in_list(std::vector<channel_info> *plChList, uint64 nChannel)
{
    return this->m_pConfigContainer->s_find_entry(plChList, get_channel_info(nChannel));
}

/* ----------------------------------------------------------------------------
* get list of all channels within a level range
*/
uint64 * channel_filter::filter_channel_from_level(size_t MinChLevel, size_t MaxChLevel, bool bCheckIgnore)
{
    // get channel info
    uint64 nChannelID;
    this->m_pstTs3Functions->getChannelOfClient(this->m_nServerID, this->m_nMyClientID, &nChannelID); //don't check for errors, behaves wrong!

    // filter channel list
    uint64 *pnFilteredList = get_channel_list_from_level(nChannelID, MinChLevel, MaxChLevel, bCheckIgnore);
    return pnFilteredList;
}

/* ----------------------------------------------------------------------------
* get list of all channels within a list + sub channel
*/
uint64 * channel_filter::filter_channel_from_list(std::vector<channel_info>* plChList, bool bSubChannel, bool bCheckIgnore)
{
    uint64* pnFullChannelList;
    uint64* pnFilteredChannelList;
    int     nEntryIndex;
    int     nLastChannelCount = 0;

    // get actual channel list
    if (this->m_pstTs3Functions->getChannelList(this->m_nServerID, &pnFullChannelList) != ERROR_ok)
    {
        return nullptr;
    }
    // create result channel list with the max. size using a copy of a full channel list
    if (this->m_pstTs3Functions->getChannelList(this->m_nServerID, &pnFilteredChannelList) != ERROR_ok)
    {
        this->m_pstTs3Functions->freeMemory(pnFullChannelList);
        return nullptr;
    }

    // prepare filtered channel list
    size_t nCountChannel = get_num_of_channel(pnFilteredChannelList);
    std::memset(pnFilteredChannelList, 0, nCountChannel * sizeof(uint64));

    // find coresponding channels in list (reverse search)
    int jj = 0;
    bool bDuplicateFound = false;

    for (size_t ii = 0; ii < nCountChannel; ii++)
    {
        nEntryIndex = this->m_pConfigContainer->s_find_entry(plChList, get_channel_info(pnFullChannelList[ii]));

        // channel was found...
        if (nEntryIndex > 0)
        {
            // ... check if it is a duplicate ...
            bDuplicateFound = false;
            for (jj = 0; jj < nLastChannelCount; jj++)
                if (pnFilteredChannelList[jj] == pnFullChannelList[ii])
                    bDuplicateFound = true;

            // ... add channel if it's not a duplicate
            if (!bDuplicateFound)
            {
                if ((find_channel_in_list(this->m_pConfigContainer->s_get_IgnoreList(), pnFullChannelList[ii]) < 0) || !bCheckIgnore)
                {
                    pnFilteredChannelList[nLastChannelCount] = pnFullChannelList[ii];
                    nLastChannelCount++;
                }

                //search for subchannel in range
                if (bSubChannel)
                {
                    if (getChildChannelOfParent(this->m_nServerID, pnFullChannelList[ii], pnFullChannelList, pnFilteredChannelList, &nLastChannelCount, 0, bCheckIgnore, true) != ERROR_ok)
                    {
                        this->m_pstTs3Functions->freeMemory(pnFilteredChannelList);
                        return nullptr;
                    }

                }
            }
        }

    }

    //clean up
    this->m_pstTs3Functions->freeMemory(pnFullChannelList);

    // if no entry was found return nullptr
    if (nLastChannelCount == 0)
    {
        this->m_pstTs3Functions->freeMemory(pnFilteredChannelList);
        pnFilteredChannelList = nullptr;
    }

    return pnFilteredChannelList;
}

/* ----------------------------------------------------------------------------
* Verify which channel in list exist and reset InvalidCount otherwise increment InvalidCount.
* If InvalidCount reaches max. delete channel from list.
*/
bool channel_filter::validate_channel_list(std::vector<channel_info>* plChList)
{
    // increment InvalidCount for all entrys
    for (size_t ii = 1; ii < (*plChList).size(); ii++)
        (*plChList)[ii].iInvalidCount++;

    // get actual channel list
    uint64* pnFullChannelList;
    if (this->m_pstTs3Functions->getChannelList(this->m_nServerID, &pnFullChannelList) != ERROR_ok)
    {
        return false;
    }
    size_t nCountChannel = 0;
    while (pnFullChannelList[nCountChannel] != 0)
        nCountChannel++;

    // find coresponding channels in list (reverse search)
    int nEntryIndex;
    for (size_t ii = 0; ii < nCountChannel; ii++)
    {
        nEntryIndex = this->m_pConfigContainer->s_find_entry(plChList, get_channel_info(pnFullChannelList[ii]));

        // channel was found, set InvalidCount to 0
        if (nEntryIndex > 0)
            (*plChList)[nEntryIndex].iInvalidCount = 0;
    }

    //clean up
    this->m_pstTs3Functions->freeMemory(pnFullChannelList);

    // find all entrys that are != 0 and increment them
    for (std::vector<channel_info>::iterator it = plChList->begin(); it != plChList->end();)
    {
        if (it->iInvalidCount >= MAX_INVALIDCOUNT)
        {
            if (DEBUG_LOG) printf("channel %llu deleted from list\n", it->nChannelID);
            plChList->erase(it);
        }
        else
            it++;
    }

    return true;
}

/* ----------------------------------------------------------------------------
* get number of channels in a channel list
*/
size_t channel_filter::get_num_of_channel(uint64* pnChannelList)
{
    size_t nCountChannel = 0;
    if (pnChannelList == nullptr)
    {
        if (this->m_pstTs3Functions->getChannelList(this->m_nServerID, &pnChannelList) != ERROR_ok)
        {
            nCountChannel = get_num_of_channel(pnChannelList);
            this->m_pstTs3Functions->freeMemory(pnChannelList);
        }
    }
    else
    {
        nCountChannel = 1;
        for (; pnChannelList[nCountChannel] != 0; nCountChannel++)
        {
            pnChannelList[nCountChannel - 1] = 0;
        }
    }
    return nCountChannel;
}

/* ----------------------------------------------------------------------------
* get level of the current channel within the server list
*/
size_t channel_filter::get_channel_level(uint64 nChannelID, uint64 *anChannelParentList)
{
    /* prepare all variables */
    size_t iChannelLevel = 0;
    uint64 nChannelParent = nChannelID;

    while ((nChannelParent != 0) && (iChannelLevel < 100))
    {
        if(anChannelParentList != nullptr) anChannelParentList[iChannelLevel] = nChannelParent;
        this->m_pstTs3Functions->getParentChannelOfChannel(this->m_nServerID, nChannelParent, &nChannelParent);
        iChannelLevel++;
    }

    return iChannelLevel;
}

/* ----------------------------------------------------------------------------
* create a vector of channel that are within a given level range, starting from nChannelID
* return value has to be deleted using 
*/
uint64 * channel_filter::get_channel_list_from_level(uint64 nChannelID, size_t nStartLevel, size_t nStopLevel, bool bCheckIgnore)
{
    uint64* pnFullChannelList;
    uint64* pnFilteredChannelList;

    // get actual channel list
    if (this->m_pstTs3Functions->getChannelList(this->m_nServerID, &pnFullChannelList) != ERROR_ok)
    {
        return nullptr;
    }
    // create result channel list with the max. size using a copy of a full channel list
    if (this->m_pstTs3Functions->getChannelList(this->m_nServerID, &pnFilteredChannelList) != ERROR_ok)
    {
        this->m_pstTs3Functions->freeMemory(pnFullChannelList);
        return nullptr;
    }

    // prepare filtered channel list
    size_t nCountChannel = get_num_of_channel(pnFilteredChannelList);
    std::memset(pnFilteredChannelList, 0, nCountChannel * sizeof(uint64));

    // find parent on correct level
    size_t nChannelLevel = get_channel_level(nChannelID, pnFilteredChannelList);
    if (nChannelLevel < nStartLevel)
    {
        this->m_pstTs3Functions->freeMemory(pnFilteredChannelList);
        pnFilteredChannelList = nullptr;
    }
    else
    {
        uint64 nChannelParent = pnFilteredChannelList[nChannelLevel - nStartLevel];
        std::memset(pnFilteredChannelList, 0, nCountChannel * sizeof(uint64));

        // get all channel IDs within starting channel
        int nLastChannelCount = 1;
        pnFilteredChannelList[0] = nChannelParent;
        if (getChildChannelOfParent(this->m_nServerID, nChannelParent, pnFullChannelList, pnFilteredChannelList, &nLastChannelCount, nStopLevel, bCheckIgnore) != ERROR_ok)
        {
            this->m_pstTs3Functions->freeMemory(pnFilteredChannelList);
            pnFilteredChannelList = nullptr;
        }
    }

    //clean up an return
    this->m_pstTs3Functions->freeMemory(pnFullChannelList);
    return pnFilteredChannelList;
}

/* ----------------------------------------------------------------------------
* return the name of an channel
*/
std::string channel_filter::get_channel_name(uint64 nChannelId)
{
    char *pcChName = nullptr;
    std::string sChName;

    this->m_pstTs3Functions->getChannelVariableAsString(this->m_nServerID, nChannelId, CHANNEL_NAME, &pcChName);
    if (pcChName != nullptr)
    {
        sChName = std::string(pcChName);
        this->m_pstTs3Functions->freeMemory(pcChName);
    }

    return sChName;
}


/* ----------------------------------------------------------------------------
* This function searches for all channel starting a specified parent channel recursively
*/
int channel_filter::getChildChannelOfParent(uint64 serverConnectionHandlerID, uint64 channelParent, uint64* fullChannelList, uint64* filteredChannelList, int* lastChannelCount, size_t nStopLevel, bool bCheckIgnore, bool bCheckDuplicate)
{
    bool bDuplicateFound = false;
    uint64 checkParentChannel;

    for (int ii = 0; fullChannelList[ii]; ii++)
    {
        this->m_pstTs3Functions->getParentChannelOfChannel(serverConnectionHandlerID, fullChannelList[ii], &checkParentChannel);
        if (checkParentChannel == channelParent)
        {
            //check if channel is in range
            if ((get_channel_level(fullChannelList[ii]) > nStopLevel) && (nStopLevel != 0))
            {
                printf("check level %zd\n", get_channel_level(fullChannelList[ii]));
                break;
            }

            //serach for duplicates (on demand)
            if (bCheckDuplicate)
                for (int jj = 0; jj < (*lastChannelCount); jj++)
                    if(filteredChannelList[jj] == fullChannelList[ii])
                        bDuplicateFound = true;

            // add entry to list
            if(!bDuplicateFound)
                if ((find_channel_in_list(this->m_pConfigContainer->s_get_IgnoreList(), fullChannelList[ii]) < 0) || !bCheckIgnore)
                    filteredChannelList[(*lastChannelCount)++] = fullChannelList[ii];

            // check if this entry has own children
            getChildChannelOfParent(serverConnectionHandlerID, fullChannelList[ii], fullChannelList, filteredChannelList, lastChannelCount, nStopLevel, bCheckDuplicate, bCheckDuplicate);
        }
    }
    return ERROR_ok;
}


/* ----------------------------------------------------------------------------
* This function gathers all information about a channel to compare it with the list or add it
*/
channel_info channel_filter::get_channel_info(uint64 nChannelID)
{
    channel_info    temp = INVALID_CHANNEL_INFO;
    uint64          nParentChannel;
    int             iIsPermanent;
    char*           pcName;

    // query channel name
    if(this->m_pstTs3Functions->getChannelVariableAsString(this->m_nServerID, nChannelID, CHANNEL_NAME, &pcName) != ERROR_ok)
        return temp;
    
    // find first permanent parent
    nParentChannel = nChannelID;
    do
    {
        this->m_pstTs3Functions->getParentChannelOfChannel(this->m_nServerID, nParentChannel, &nParentChannel);
        this->m_pstTs3Functions->getChannelVariableAsInt(this->m_nServerID, nParentChannel, CHANNEL_FLAG_PERMANENT, &iIsPermanent);
    } while ((nParentChannel != 0) && (iIsPermanent == 0));

    this->m_pstTs3Functions->getChannelVariableAsInt(this->m_nServerID, nChannelID, CHANNEL_FLAG_PERMANENT, &iIsPermanent);

    // set return value
    temp.nChannelID = nChannelID;
    temp.sChannelName = std::string(pcName);
    temp.nChannelParent = nParentChannel;
    temp.bIsPermanent = iIsPermanent;

    //clean up
    this->m_pstTs3Functions->freeMemory(pcName);
    return temp;
}