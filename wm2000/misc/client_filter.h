#pragma once
#include "misc/channel_filter.h"
#include "misc/config_container.h"
#include "misc/error_handler.h"
#include "ts3_functions.h"
#include <boost/thread.hpp>

#define INVALID_CHANNEL_ID  0xFFFFFFFFFFFFFFFFll
#define NUM_FREQUENCIES     REAL_MAXNUMPROFILES+1

struct freq_def
{
    uint32_t nFreq      : 24;   // 23:0: Frequency
    uint32_t nReseved24 : 1;    // 24
    uint32_t nReseved25 : 1;    // 25
    uint32_t nReseved26 : 1;    // 26
    uint32_t nReseved27 : 1;    // 27
    uint32_t nPriority  : 1;    // 28: user has activated priority calls
    uint32_t nSquelch   : 1;    // 29: listen to priority calls only
    uint32_t nMute      : 1;    // 30: don't listen to any call
    uint32_t nReseved31 : 1;    // 31
};

union freq_data
{
    freq_def nBit;
    uint32_t nWord;
};

struct client_info
{
    anyID		nClientID;
    std::string sClientName;
    bool        bUseFreqList;
    freq_data   acFreqList[NUM_FREQUENCIES];
    int         iNumFreq;
    uint64      nActualChannelID;
};

class client_filter
{
public:
    client_filter();
    ~client_filter();

    void                init(struct TS3Functions *pstTs3Functions, config_container *pConfigContainer, channel_filter *pCannelFilter, uint64 nServerConnectionHandlerID, uint64 nChannelID, anyID nClientID);// get interfaces after initialization
    bool                set_meta_data();                                                                        // create meta data from profiles and write data to Server

    void                update_client_list(anyID nClientID, uint64 nActChannel);                                // get new info from server (connect + update + disconnect)
    int                 find_client(anyID nClientID);                                                           // find client in m_cClientList

    std::vector<int>    get_client_list_idx(int iFreq, bool bCheckIgnore, bool bCheckSquelch, bool bCheckParam = true);             // get vector of client Idx with active freq. iFreq
    anyID*              get_client_list(int iFreq, bool bCheckIgnore, bool bCheckSquelch);                                          // get 0 terminated list of clients with active freq. iFreq
    int                 find_active_freq(int iClient, int iFreq, bool bCheckIgnore, bool bCheckSquelch, bool bCheckParam = true);   // get index of active freq in freq list

    int                 get_next_free_freq(int iStartFreq);                                                     // return a frequency that is currently unused
    
    std::string         get_channel_name(uint64 nChannelID);                                                    //return name of the given channel ID
    std::string         get_server_name() { return m_sServerName; };                                            //return name of the actual server
    uint64              get_server_id() { return m_nServerID; };                                                //return id of the actual server

protected:
    int  parse_meta_data(std::string sMetaData, freq_data acFreqList[]);                // parse meta data

public:
    boost::mutex                m_cClientListMutex;     // mutex to read/write client list from different threads
    std::vector<client_info>    m_cClientList;          // list of all active clients

private:
    struct TS3Functions        *m_pstTs3Functions;      // TS3 interface functions
    config_container           *m_pConfigContainer;     // link to data container for list management
    channel_filter             *m_pCannelFilter;        // link to ChannelFilter (IgnoreList)
    error_handler               m_cErrHandler;          // link to error handler

    uint64                      m_nServerID;
    std::string                 m_sServerName;
    uint64                      m_nMyChannelID;
    anyID                       m_nMyClientID;
};

