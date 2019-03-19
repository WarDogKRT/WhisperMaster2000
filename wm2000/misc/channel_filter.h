#pragma once
#include "misc/config_container.h"
#include "misc/error_handler.h"
#include "ts3_functions.h"

#define MAX_INVALIDCOUNT 100

class channel_filter
{
public:
    channel_filter();
    ~channel_filter();
    void init(struct TS3Functions *pstTs3Functions, config_container *pConfigContainer, uint64 nServerConnectionHandlerID, anyID nClientID); // get interfaces after initialization

    uint64*         filter_channel_from_level(size_t MinChLevel, size_t MaxChLevel, bool bCheckIgnore);
    uint64*         filter_channel_from_list(std::vector<channel_info> *plChList, bool bSubChannel, bool bCheckIgnore);
    int             find_channel_in_list(std::vector<channel_info> *plChList, uint64 nChannel);

    size_t          get_num_of_channel(uint64* pnChannelList = nullptr);
    size_t          get_channel_level(uint64 nChannelID, uint64 *anChannelParentList = nullptr);
    uint64*         get_channel_list_from_level(uint64 nChannelID, size_t nStartLevel, size_t nStopLevel, bool bCheckIgnore);
    channel_info    get_channel_info(uint64 nChannelID);

    bool            validate_channel_list(std::vector<channel_info>* plChList);
    void            free_channel_list(uint64* pnChList) { m_pstTs3Functions->freeMemory(pnChList); };

    std::string     get_server_name() { return m_sServerName; };
    uint64          get_server_id() { return m_nServerID; };
    std::string     get_channel_name(uint64 nChannelId);

private:
    int             getChildChannelOfParent(uint64 serverConnectionHandlerID, uint64 channelParent, uint64* fullChannelList, uint64* filteredChannelList, int* lastChannelCount, size_t nStopLevel, bool bCheckIgnore, bool bCheckDuplicate = false);

private:
    struct TS3Functions *m_pstTs3Functions;     // TS3 interface functions
    config_container    *m_pConfigContainer;    // link to data container for list management
    error_handler        m_cErrHandler;         // link to error handler

    uint64               m_nServerID;           // ID of connected Server
    std::string          m_sServerName;         // Name of the Server
    anyID                m_nMyClientID;         // ID of own client on this Server
};

