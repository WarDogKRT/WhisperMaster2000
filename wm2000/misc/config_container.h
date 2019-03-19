#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/thread.hpp>
#include "teamspeak/public_definitions.h"
#include "misc/error_handler.h"

#define DEFAULT_CHANNEL_INFO { 0,  false, 0, "None", 0 }
#define INVALID_CHANNEL_INFO { 1, false, 0, "",      0 }

#define ERROR_INVALID_POINTER	-100
#define IDNOTFOUND				-1
#define IDFOUND_INVALID_NAME	-2
#define IDFOUND_INVALID_PARENT	-3
#define IDNOTFOUND_NONAME_MATCH	-4
#define IDNOTFOUND_MEM_FULL	    -5

#define DEFAULT_MAXNUMPROFILES  6
#define REAL_MAXNUMPROFILES     20

#define MAX__MAXNUMCHANNEL      1025

#define MAX_FREQUENCY           100
#define MAX__MAXFREQUENCY       10000

#define CONFIG_VERSION          1

struct channel_info
{
	uint64		nChannelID;
	bool        bIsPermanent;
	uint64		nChannelParent;
	std::string sChannelName;
    int32_t     iInvalidCount;
};

enum eProfileType
{
    PROFILE_OFF = 0,
    PROFILE_LEVEL,
    PROFILE_FAVORITE,
    PROFILE_FREQUENCY,
    PROFILE_AUDIO
};

class config_container
{
public:
	config_container();
	~config_container();
	
    // file interaction
	int     s_read_param( const std::string &filename );
    int     s_read_param() { return s_read_param(this->m_sFilePath); };
	void    s_write_param( const std::string &filename );
    void    s_write_param() { s_write_param(this->m_sFilePath); };
    bool	file_exists(const std::string& filename);
    void    set_file_path(std::string filename) { m_sFilePath = filename; };

    // channel list interaction
	size_t  s_delete_entry(std::vector<channel_info> *plReturn, int nEntry);
	int     s_add_entry(std::vector<channel_info> *plReturn, uint64 nChannelID, bool bIsPermanent, uint64 nChannelParent, std::string sChannelName);
	int     s_add_entry(std::vector<channel_info> *plReturn, channel_info eEntry);
	int     s_find_entry(std::vector<channel_info> *plReturn, uint64 nChannelID, bool bIsPermanent, uint64 nChannelParent, std::string sChannelName);
	int     s_find_entry(std::vector<channel_info> *plReturn, channel_info eEntry);
    void    s_clear_vector(std::vector<channel_info> *plReturn);

    std::string     profile_string_from_enum(eProfileType nInput);
    eProfileType    profile_enum_from_string(std::string sInput);

    //interface functions
    bool						s_get_ExpertMode();	                    // (de-)activate expert view in UI (no other settings are changed)
    void						s_set_ExpertMode(bool bValue);
    size_t	                    s_get_MaxNumProfiles();                 // number of profiles to use
    void	                    s_set_MaxNumProfiles(size_t nValue);
    bool						s_get_LGSActive();		                // (de-)activate LogitechGamingSoftware interface
    std::string					s_get_Language();		                // select language (german / english)
    void                        s_set_Language(std::string sValue);     // select language (german / english)
    int                         s_get_MaxNumFreq();                     // max. frequency that can be set
    void                        s_set_MaxNumFreq(int iValue);
    bool						s_get_SaveIgnoreList();	                // (de-)activate saving the ignore channel list
    void						s_set_SaveIgnoreList(bool bValue);
    bool						s_get_UseIgnoreListRx();	            // use ignore list to filter when receiving data
    void						s_set_UseIgnoreListRx(bool bValue);
    bool						s_get_UseMasterRight();	                // enables Master settings
    void						s_set_UseMasterRight(bool bValue);
    std::vector<channel_info>  *s_get_IgnoreList();		                // List of ignored channels (not thread safe)
    channel_info                s_get_IgnoreListEntry(int iIndex);	    // get entry of channel that is ignored
    std::string                 s_get_GenHotKey_reset();                // list of hotkeys used in "general" application
    void                        s_set_GenHotKey_reset(std::string sValue);
    std::string                 s_get_GenHotKey_mute();                 // list of hotkeys used in "general" application
    void                        s_set_GenHotKey_mute(std::string sValue);

    std::string                 s_get_ProfileName(int iProfile);        // name of profile
    void                        s_set_ProfileName(int iProfile, std::string sValue);
    std::string                 get_DefaultProfileName(int iProfile);   // name default name of profile
    eProfileType                s_get_ProfileType(int iProfile);        // type of profile
    void                        s_set_ProfileType(int iProfile, eProfileType eValue);
    bool                        s_get_UseIgnoreListTx(int iProfile);    // use ignore list to filter when sending data of profile
    void                        s_set_UseIgnoreListTx(int iProfile, bool bValue);
    bool                        s_get_AutoActivate(int iProfile);       // activate whisper is key is hit (PTT) of profile
    void                        s_set_AutoActivate(int iProfile, bool bValue);
    bool                        s_get_ActualState(int iProfile);        // State of each profile (internal use only) of profile
    void                        s_set_ActualState(int iProfile, bool bValue);

    std::vector<channel_info>  *s_get_FavoriteList(int iProfile);	    // lists of favorite channels to use in Favorite-Mode of profile
    std::string                 s_get_ServerName(int iProfile);         // name of server that is valid to use Favorite List of profile
    void                        s_set_ServerName(int iProfile, std::string sValue);
    bool                        s_get_UseSubChOfFav(int iProfile);      // use subchannel of all favorite channels of profile
    void                        s_set_UseSubChOfFav(int iProfile, bool bValue);
    size_t                      s_get_MaxChLevel(int iProfile);         // max. channel level to use in Level-Mode of profile
    void                        s_set_MaxChLevel(int iProfile, size_t nValue);
    size_t                      s_get_MinChLevel(int iProfile);         // min. channel level to use in Level-Mode of profile
    void                        s_set_MinChLevel(int iProfile, size_t nValue);
    int                         s_get_ActiveFreq(int iProfile);         // "Frequency" that is used by profile
    void                        s_set_ActiveFreq(int iProfile, int iValue);
    bool                        s_get_MuteFreq(int iProfile);           // marks the profile as listening or not (Rx) of profile
    void                        s_set_MuteFreq(int iProfile, bool bValue);
    bool                        s_get_SquelchFreq(int iProfile);        // marks the profile as listening to priority calls only (Rx) of profile
    void                        s_set_SquelchFreq(int iProfile, bool bValue);
    bool                        s_get_PrioFreq(int iProfile);           // marks the profile as using priority calls (Tx) of profile
    void                        s_set_PrioFreq(int iProfile, bool bValue);
    bool                        s_get_MasterFreq(int iProfile);         // marks the profile as using master calls (Tx) of profile
    void                        s_set_MasterFreq(int iProfile, bool bValue);
    std::string                 s_get_HotKey_down(int iProfile);        // list of hotkeys used in this profile 
    void                        s_set_HotKey_down(int iProfile, std::string sValue);
    std::string                 s_get_HotKey_up(int iProfile);          // list of hotkeys used in this profile 
    void                        s_set_HotKey_up(int iProfile, std::string sValue);

    // channel list interaction (not thread safe)
    void        clear_vector(std::vector<channel_info> *plReturn);
	void        copy_vector(std::vector<channel_info> *plDestination, std::vector<channel_info> *plSource);
    bool        compare_vector(std::vector<channel_info> *plDestination, std::vector<channel_info> *plSource);

    //operator
    bool operator==(config_container const& other);
    bool compare_config(config_container const & other, bool bNeedRestart = false);
    bool operator!=(config_container const& other) { return !(*this == other); };
    config_container& operator=(config_container const& other);

protected:
    void        cleanup_class();


    // memory initialisation (not thread safe)
    void		init_channel_list();

    // data conversion for file interaction
	std::string create_string_from_vector(std::vector<channel_info> *plReturn);
    std::string create_string_from_entry(channel_info eChannel);
	int         create_vector_from_string(std::vector<channel_info> *plReturn, std::string sData);
    

protected:
    boost::mutex                m_cConfigDataMutex; // mutex to read/write client list from different threads

private:
    error_handler               m_cErrHandler;      // link to error handler

    bool                        m_bIsInitialized;   // shows if the class is already filled with valid data
    int                         m_nConfigVersion;   // version that was setup in the config file
    std::string					m_sFilePath;		// path to config file

    //settings memory
	//  general
    bool                        m_bExpertMode;      // user has expert view active or not
    size_t	         		    m_nMaxNumProfiles;  // number of profiles to use
	bool						m_bLGSActive;		// (de-)activate LogitechGamingSoftware interface
	std::string					m_sLanguage;		// select language (german / english)
    int                         m_iMaxNumFreq;      // max. frequency that can be set
	bool						m_bSaveIgnoreList;	// (de-)activate saving the ignore channel list
    bool						m_bUseIgnoreListRx;	// use ignore list to filter when receiving data
    bool						m_bUseMasterRight;	// enables master rights
	std::vector<channel_info>  *m_pvIgnoreList;		// List of ignored channels
    std::string					m_sGenHotKey_reset; // list of hotkeys used in "general" application
    std::string					m_sGenHotKey_mute;  // list of hotkeys used in "general" application

	//  profil    all parameter per profile (array of) 
    std::string                *m_psProfileName;    // name of profile
    eProfileType               *m_pnProfileType;    // type of profile
    bool                       *m_pbUseIgnoreListTx;// use ignore list to filter when sending data
	bool                       *m_pbAutoActivate;   // activate whisper is key is hit (PTT)
    bool                       *m_pbActualState;    // State of each profile (internal use only)

    std::vector<channel_info>  *m_pvFavoriteList;	// lists of favorite channels to use in Favorite-Mode
    std::string                *m_psServerName;     // name of server that is valid to use Favorite List
    bool                       *m_pbUseSubChOfFav;  // use subchannel of all favorite channels
    size_t                     *m_pnMaxChLevel;     // max. channel level to use in Level-Mode
    size_t                     *m_pnMinChLevel;     // min. channel level to use in Level-Mode
    int                        *m_piActiveFreq;     // "Frequency" that is used by profile
    bool                       *m_pbMuteFreq;       // marks the profile as listening or not (Rx)
    bool                       *m_pbSquelchFreq;    // marks the profile as listening to priority calls only (Rx)
    bool                       *m_pbPrioFreq;       // marks the profile as using priority calls (Tx)
    bool                       *m_pbMasterFreq;     // marks the profile as using master calls (Tx)
    std::string				   *m_psProfileHotKey_down;  // list of hotkeys used in this profile
    std::string				   *m_psProfileHotKey_up;    // list of hotkeys used in this profile
};

