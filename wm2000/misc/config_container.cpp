//#include "stdafx.h"
#include "config_container.h"
#include <Shlwapi.h>

#if USE_CALL_STACK
#define CALL_STACK if(USE_CALL_STACK) this->m_cErrHandler.message_callstack(__FUNCSIG__);
#else
#define CALL_STACK
#endif

/* ----------------------------------------------------------------------------
* constructor: initialize variables and set all pointer to NULL
*/
config_container::config_container()
{
    CALL_STACK
    this->m_bIsInitialized      = false;
	//general
    this->m_bExpertMode         = false;
	this->m_bLGSActive		    = false;
	this->m_sLanguage			= "german";
    this->m_iMaxNumFreq         = 100;
	this->m_bSaveIgnoreList	    = false;
    this->m_bUseIgnoreListRx    = false;
	this->m_nMaxNumProfiles	    = DEFAULT_MAXNUMPROFILES;
	this->m_pvIgnoreList        = nullptr; // will be initialized within read_param()

	//profile
    this->m_psProfileName       = nullptr; // will be initialized within read_param()
    this->m_pnProfileType       = nullptr;
    this->m_pbUseIgnoreListTx   = nullptr;
    this->m_pbAutoActivate      = nullptr;
    this->m_pbActualState       = nullptr;
    this->m_pvFavoriteList      = nullptr;
    this->m_psServerName        = nullptr;
    this->m_pbUseSubChOfFav     = nullptr;
    this->m_pnMaxChLevel        = nullptr;
    this->m_pnMinChLevel        = nullptr;
    this->m_piActiveFreq        = nullptr;
    this->m_pbMuteFreq          = nullptr;
    this->m_pbSquelchFreq       = nullptr;
    this->m_pbPrioFreq          = nullptr;
    this->m_pbMasterFreq        = nullptr;
    this->m_psProfileHotKey_down= nullptr;
    this->m_psProfileHotKey_up  = nullptr;
}

/* ----------------------------------------------------------------------------
* compare data with another container
*/
bool config_container::operator==(config_container const & other)
{
    CALL_STACK
    return compare_config(other);
}

bool config_container::compare_config(config_container const & other, bool bNeedRestart)
{
    CALL_STACK
    bool bResult = true;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    //first check if both container are initialize before checking the other parameter
    if (!bNeedRestart) bResult &= this->m_bIsInitialized;
    if (!bNeedRestart) bResult &= other.m_bIsInitialized;
    if (bResult)
    {
        //check general settings
        if (!bNeedRestart) bResult &= (this->m_bExpertMode == other.m_bExpertMode);
        if (!bNeedRestart) bResult &= (this->m_bLGSActive == other.m_bLGSActive);
        bResult &= (this->m_sLanguage == other.m_sLanguage);
        if (!bNeedRestart) bResult &= (this->m_iMaxNumFreq == other.m_iMaxNumFreq);
        if (!bNeedRestart) bResult &= (this->m_bSaveIgnoreList == other.m_bSaveIgnoreList);
        if (!bNeedRestart) bResult &= (this->m_bUseIgnoreListRx == other.m_bUseIgnoreListRx);
        if (!bNeedRestart) bResult &= (this->m_bUseMasterRight == other.m_bUseMasterRight);
        bResult &= (this->m_nMaxNumProfiles == other.m_nMaxNumProfiles);
        if (!bNeedRestart) bResult &= compare_vector(this->m_pvIgnoreList, other.m_pvIgnoreList);
        if (bResult)
        {
            //check profile settings
            for (size_t ii = 0; ii < this->m_nMaxNumProfiles; ii++)
            {
                bResult &= (this->m_psProfileName[ii].compare(other.m_psProfileName[ii]) == 0);
                bResult &= (this->m_pnProfileType[ii]    == other.m_pnProfileType[ii]);
                if (!bNeedRestart) bResult &= (this->m_pbUseIgnoreListTx[ii]== other.m_pbUseIgnoreListTx[ii]);
                bResult &= (this->m_pbAutoActivate[ii]   == other.m_pbAutoActivate[ii]);
                if (!bNeedRestart) bResult &= compare_vector(this->m_pvFavoriteList + ii, other.m_pvFavoriteList + ii);
                if (!bNeedRestart) bResult &= (this->m_psServerName[ii].compare(other.m_psServerName[ii]) == 0);
                bResult &= (this->m_pbUseSubChOfFav[ii]  == other.m_pbUseSubChOfFav[ii]);
                if (!bNeedRestart) bResult &= (this->m_pnMaxChLevel[ii]     == other.m_pnMaxChLevel[ii]);
                if (bNeedRestart && ((this->m_pnMaxChLevel[ii]==0) || (other.m_pnMaxChLevel[ii] == 0)) )
                bResult &= (this->m_pnMaxChLevel[ii]     == other.m_pnMaxChLevel[ii]);
                if (!bNeedRestart) bResult &= (this->m_pnMinChLevel[ii]     == other.m_pnMinChLevel[ii]);
                if (!bNeedRestart) bResult &= (this->m_piActiveFreq[ii]     == other.m_piActiveFreq[ii]);
                if (!bNeedRestart) bResult &= (this->m_pbMuteFreq[ii]       == other.m_pbMuteFreq[ii]);
                if (!bNeedRestart) bResult &= (this->m_pbSquelchFreq[ii]    == other.m_pbSquelchFreq[ii]);
                if (!bNeedRestart) bResult &= (this->m_pbPrioFreq[ii]       == other.m_pbPrioFreq[ii]);
                if (!bNeedRestart) bResult &= (this->m_pbMasterFreq[ii]     == other.m_pbMasterFreq[ii]);
            }
        }
    }

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

/* ----------------------------------------------------------------------------
* copy data from one container to the other
*/
config_container& config_container::operator=(config_container const & other)
{
    CALL_STACK
    //if data are equal, skip copy
    if (*this == other)
        return *this;

    //thread safe begin
    this->m_cConfigDataMutex.lock();

    // if this contains valid data, clean up first
    if (this->m_bIsInitialized && (this->m_nMaxNumProfiles != other.m_nMaxNumProfiles))
        this->cleanup_class();


    //setup general data
    this->m_bExpertMode         = other.m_bExpertMode;
    this->m_nConfigVersion      = other.m_nConfigVersion;
    this->m_bLGSActive          = other.m_bLGSActive;
    this->m_sLanguage           = other.m_sLanguage;
    this->m_iMaxNumFreq         = other.m_iMaxNumFreq;
    this->m_bSaveIgnoreList     = other.m_bSaveIgnoreList;
    this->m_bUseIgnoreListRx    = other.m_bUseIgnoreListRx;
    this->m_bUseMasterRight     = other.m_bUseMasterRight;
    this->m_nMaxNumProfiles     = other.m_nMaxNumProfiles;

    //setup profile specific data
    // if this has not been initialized, do this
    if (this->m_pvIgnoreList == nullptr)
        this->init_channel_list();

    copy_vector(this->m_pvIgnoreList, other.m_pvIgnoreList);
    this->m_sGenHotKey_reset      = other.m_sGenHotKey_reset;
    this->m_sGenHotKey_mute       = other.m_sGenHotKey_mute;

    // copy profiles
    for (size_t ii = 0; ii < this->m_nMaxNumProfiles; ii++)
    {
        this->m_psProfileName[ii]       = other.m_psProfileName[ii];
        this->m_pnProfileType[ii]       = other.m_pnProfileType[ii];

        this->m_pbUseIgnoreListTx[ii]   = other.m_pbUseIgnoreListTx[ii];
        this->m_pbAutoActivate[ii]      = other.m_pbAutoActivate[ii];
        copy_vector(this->m_pvFavoriteList + ii, other.m_pvFavoriteList + ii);
        this->m_psServerName[ii]        = other.m_psServerName[ii];
        this->m_pbUseSubChOfFav[ii]     = other.m_pbUseSubChOfFav[ii];
        this->m_pnMaxChLevel[ii]        = other.m_pnMaxChLevel[ii];
        this->m_pnMinChLevel[ii]        = other.m_pnMinChLevel[ii];
        this->m_piActiveFreq[ii]        = other.m_piActiveFreq[ii];
        this->m_pbMuteFreq[ii]          = other.m_pbMuteFreq[ii];
        this->m_pbSquelchFreq[ii]       = other.m_pbSquelchFreq[ii];
        this->m_pbPrioFreq[ii]          = other.m_pbPrioFreq[ii];
        this->m_pbMasterFreq[ii]        = other.m_pbMasterFreq[ii];
        this->m_psProfileHotKey_down[ii]= other.m_psProfileHotKey_down[ii];
        this->m_psProfileHotKey_up[ii]  = other.m_psProfileHotKey_up[ii];
    }

    //mark as initialized
    this->m_bIsInitialized = true;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return *this;
}

/* ----------------------------------------------------------------------------
* create arrays depending on this->m_nMaxNumProfiles
*/
void config_container::init_channel_list()
{
    CALL_STACK
    //general
	this->m_pvIgnoreList        = new std::vector<channel_info>;
    clear_vector(this->m_pvIgnoreList);

    //profile   => create data depending on m_nMaxNumProfiles
    this->m_psProfileName       = new std::string[REAL_MAXNUMPROFILES];
    this->m_pnProfileType       = new eProfileType[REAL_MAXNUMPROFILES];
    this->m_pbUseIgnoreListTx   = new bool[REAL_MAXNUMPROFILES];
    this->m_pbAutoActivate      = new bool[REAL_MAXNUMPROFILES];
    this->m_pbActualState       = new bool[REAL_MAXNUMPROFILES];
    this->m_pvFavoriteList      = new std::vector<channel_info>[REAL_MAXNUMPROFILES];
    this->m_psServerName        = new std::string[REAL_MAXNUMPROFILES];
    this->m_pbUseSubChOfFav     = new bool[REAL_MAXNUMPROFILES];
    this->m_pnMaxChLevel        = new size_t[REAL_MAXNUMPROFILES];
    this->m_pnMinChLevel        = new size_t[REAL_MAXNUMPROFILES];
    this->m_piActiveFreq        = new int[REAL_MAXNUMPROFILES];
    this->m_pbMuteFreq          = new bool[REAL_MAXNUMPROFILES];
    this->m_pbSquelchFreq       = new bool[REAL_MAXNUMPROFILES];
    this->m_pbPrioFreq          = new bool[REAL_MAXNUMPROFILES];
    this->m_pbMasterFreq        = new bool[REAL_MAXNUMPROFILES];
    this->m_psProfileHotKey_down= new std::string[REAL_MAXNUMPROFILES];
    this->m_psProfileHotKey_up  = new std::string[REAL_MAXNUMPROFILES];

    //          => initialize all profiles
    this->m_sGenHotKey_reset    = "";
    this->m_sGenHotKey_mute     = "";
    for (int ii = 0; ii < REAL_MAXNUMPROFILES; ii++)
    {
        this->m_psProfileName[ii]     = get_DefaultProfileName(ii);
        this->m_pnProfileType[ii]     = PROFILE_OFF;  // type of profile

        this->m_pbUseIgnoreListTx[ii] = false;        // use ignore list to filter
        this->m_pbAutoActivate[ii]    = false;        // activate whisper is key is hit (PTT)

        clear_vector(this->m_pvFavoriteList + ii);
        this->m_psServerName[ii]        = "";           // name of server that is valid to use Favorite List
        this->m_pbUseSubChOfFav[ii]     = false;        // use subchannel of all favorite channels
        this->m_pnMaxChLevel[ii]        = 0;            // max. channel level to use in Level-Mode
        this->m_pnMinChLevel[ii]        = 0;            // min. channel level to use in Level-Mode
        this->m_piActiveFreq[ii]        = 0;            // "Frequency" that is used by profile
        this->m_pbMuteFreq[ii]          = false;        // profile is active by default
        this->m_pbSquelchFreq[ii]       = false;        // don't listen with squelch
        this->m_pbPrioFreq[ii]          = false;        // don't use priority calls
        this->m_pbMasterFreq[ii]        = false;        // don't use master calls
        this->m_psProfileHotKey_down[ii]= "";           // will be updated by plugin_base::check_param later
        this->m_psProfileHotKey_up[ii]  = "";           // will be updated by plugin_base::check_param later
    }

    this->m_bIsInitialized = false;
    return;
}

/* ----------------------------------------------------------------------------
* delete all generated arrays and set default values
*/
void config_container::cleanup_class()
{
    CALL_STACK
    //general
    if (this->m_pvIgnoreList != nullptr)       delete this->m_pvIgnoreList;        this->m_pvIgnoreList = nullptr;

    //profile
    if (this->m_psProfileName != nullptr)      delete[] this->m_psProfileName;     this->m_psProfileName = nullptr;
    if (this->m_pnProfileType != nullptr)      delete[] this->m_pnProfileType;     this->m_pnProfileType = nullptr;
    if (this->m_pbUseIgnoreListTx != nullptr)  delete[] this->m_pbUseIgnoreListTx; this->m_pbUseIgnoreListTx = nullptr;
    if (this->m_pbAutoActivate != nullptr)     delete[] this->m_pbAutoActivate;    this->m_pbAutoActivate = nullptr;
    if (this->m_pbActualState != nullptr)      delete[] this->m_pbActualState;     this->m_pbActualState = nullptr;
    if (this->m_pvFavoriteList != nullptr)     delete[] this->m_pvFavoriteList;    this->m_pvFavoriteList = nullptr;
    if (this->m_psServerName != nullptr)       delete[] this->m_psServerName;      this->m_psServerName = nullptr;
    if (this->m_pbUseSubChOfFav != nullptr)    delete[] this->m_pbUseSubChOfFav;   this->m_pbUseSubChOfFav = nullptr;
    if (this->m_pnMaxChLevel != nullptr)       delete[] this->m_pnMaxChLevel;      this->m_pnMaxChLevel = nullptr;
    if (this->m_pnMinChLevel != nullptr)       delete[] this->m_pnMinChLevel;      this->m_pnMinChLevel = nullptr;
    if (this->m_piActiveFreq != nullptr)       delete[] this->m_piActiveFreq;      this->m_piActiveFreq = nullptr;
    if (this->m_pbMuteFreq != nullptr)         delete[] this->m_pbMuteFreq;        this->m_pbMuteFreq = nullptr;
    if (this->m_pbSquelchFreq != nullptr)      delete[] this->m_pbSquelchFreq;     this->m_pbSquelchFreq = nullptr;
    if (this->m_pbPrioFreq != nullptr)         delete[] this->m_pbPrioFreq;        this->m_pbPrioFreq = nullptr;
    if (this->m_pbMasterFreq != nullptr)       delete[] this->m_pbMasterFreq;      this->m_pbMasterFreq = nullptr;
    if (this->m_psProfileHotKey_down != nullptr) delete[] this->m_psProfileHotKey_down; this->m_psProfileHotKey_down = nullptr;
    if (this->m_psProfileHotKey_up != nullptr) delete[] this->m_psProfileHotKey_up; this->m_psProfileHotKey_up = nullptr;

    this->m_bIsInitialized = false;
    return;
}

/* ----------------------------------------------------------------------------
* destructor: delete all generated arrays
*/
config_container::~config_container()
{
    CALL_STACK
    cleanup_class();
}

/* ----------------------------------------------------------------------------
* read data from selected XML file
*/
int config_container::s_read_param( const std::string &filename )
{
    CALL_STACK
    channel_info temp = DEFAULT_CHANNEL_INFO;
	int nConverted = 0;

	if (!file_exists(filename))
	{ //if config file does not exist, write default data
		printf("PLUGIN: config file doesn't exist, write default parameter\n");
        init_channel_list();
		this->s_write_param(filename);
        this->s_read_param(filename);
	}
	else
	{
		// Create an empty property tree object.
		boost::property_tree::ptree tree;
        try
        {
		    boost::property_tree::read_xml(filename, tree, boost::property_tree::xml_parser::trim_whitespace);
        }
        catch (boost::property_tree::xml_parser_error &e)
        {
            printf("PLUGIN: ERROR: read_param / read_xml => %s (%s)\n", e.message().c_str(), filename.c_str());
        }

        //thread safe begin
        this->m_cConfigDataMutex.lock();

		//get number of profiles and initialize all vectors
		this->m_nMaxNumProfiles   = tree.get("general.MaxNumProfiles", DEFAULT_MAXNUMPROFILES);
		init_channel_list();

		//general
        this->m_bExpertMode       = tree.get("general.Expert",        false);
        this->m_nConfigVersion    = tree.get("general.ConfigVersion", (int)0);
		this->m_bLGSActive        = tree.get("general.LgsActive",     false);
		this->m_sLanguage         = tree.get("general.Language",      std::string("german"));
        this->m_iMaxNumFreq       = tree.get("general.MaxFrequency",  (int)MAX_FREQUENCY);
		this->m_bSaveIgnoreList   = tree.get("general.SaveIgnoreList", false);
        this->m_bUseIgnoreListRx  = tree.get("general.UseIgnoreListRx", false);
        this->m_bUseMasterRight   = tree.get("general.UseMasterRights", false);
        if (this->m_bSaveIgnoreList)
        {
            nConverted = create_vector_from_string(this->m_pvIgnoreList, tree.get("general.IgnoreList", create_string_from_entry(temp)));
            if (nConverted != 4) clear_vector(this->m_pvIgnoreList);
        }

		//profiles
        char buffer[100];
        for (int ii = 0; (ii < this->m_nMaxNumProfiles) && (ii < REAL_MAXNUMPROFILES); ii++)
        {
            sprintf_s(buffer, "profile.profile%d.ProfileName", ii + 1);
            this->m_psProfileName[ii] = tree.get(buffer, get_DefaultProfileName(ii));
            sprintf_s(buffer, "profile.profile%d.ProfileType", ii + 1);
            this->m_pnProfileType[ii] = profile_enum_from_string(tree.get(buffer, "off"));
            sprintf_s(buffer, "profile.profile%d.UseIgnoreListTx", ii + 1);
            this->m_pbUseIgnoreListTx[ii] = tree.get(buffer, false);
            sprintf_s(buffer, "profile.profile%d.AutoActivate", ii + 1);
            this->m_pbAutoActivate[ii] = tree.get(buffer, false);
            sprintf_s(buffer, "profile.profile%d.FavoriteList", ii + 1);
            nConverted = create_vector_from_string(this->m_pvFavoriteList + ii, tree.get(buffer, create_string_from_entry(temp)));
            if (nConverted != 5) clear_vector(this->m_pvFavoriteList + ii);
            sprintf_s(buffer, "profile.profile%d.ValidServer", ii + 1);
            this->m_psServerName[ii] = tree.get(buffer, "");
            sprintf_s(buffer, "profile.profile%d.UseFavSubCh", ii + 1);
            this->m_pbUseSubChOfFav[ii] = tree.get(buffer, false);
            sprintf_s(buffer, "profile.profile%d.MaxChLevel", ii + 1);
            this->m_pnMaxChLevel[ii] = tree.get(buffer, 0);
            sprintf_s(buffer, "profile.profile%d.MinChLevel", ii + 1);
            this->m_pnMinChLevel[ii] = tree.get(buffer, 2);
            sprintf_s(buffer, "profile.profile%d.ActiveFreq", ii + 1);
            this->m_piActiveFreq[ii] = tree.get(buffer, 0);
            sprintf_s(buffer, "profile.profile%d.Squelch", ii + 1);
            this->m_pbSquelchFreq[ii] = tree.get(buffer, false);
            sprintf_s(buffer, "profile.profile%d.Priority", ii + 1);
            this->m_pbPrioFreq[ii] = tree.get(buffer, false);
            sprintf_s(buffer, "profile.profile%d.Master", ii + 1);
            this->m_pbMasterFreq[ii] = tree.get(buffer, false);
        }

        //thread safe end
        this->m_cConfigDataMutex.unlock();
	}
    this->m_bIsInitialized = true;
	return nConverted;
}


/* ----------------------------------------------------------------------------
* write data to selected XML file
*/
void config_container::s_write_param(const std::string &filename)
{
    CALL_STACK
	// Create an empty property tree object.
	boost::property_tree::ptree tree;

    if (this->m_pvIgnoreList != nullptr)
    {
        //thread safe begin
        this->m_cConfigDataMutex.lock();

        //general
        tree.put("general.ConfigVersion", (int)CONFIG_VERSION);
        tree.put("general.Expert", this->m_bExpertMode);
        tree.put("general.LgsActive", this->m_bLGSActive);
        tree.put("general.Language", this->m_sLanguage);
        tree.put("general.SaveIgnoreList", this->m_bSaveIgnoreList);
        tree.put("general.UseIgnoreListRx", this->m_bUseIgnoreListRx);
        if(this->m_bSaveIgnoreList) tree.put("general.IgnoreList", create_string_from_vector(this->m_pvIgnoreList));
        if(this->m_bUseMasterRight) tree.put("general.UseMasterRights", this->m_bUseMasterRight);
        if (this->m_nMaxNumProfiles != DEFAULT_MAXNUMPROFILES) tree.put("general.MaxNumProfiles", this->m_nMaxNumProfiles); // hidden parameter, hold if it was set by the user
        if (this->m_iMaxNumFreq != MAX_FREQUENCY) tree.put("general.MaxFrequency", this->m_iMaxNumFreq);                              // hidden parameter, hold if it was set by the user
        this->m_nMaxNumProfiles = (this->m_nMaxNumProfiles <= (REAL_MAXNUMPROFILES+1)) ? this->m_nMaxNumProfiles : REAL_MAXNUMPROFILES;   // make sure, size is not too high. Real check has to be external

        //profiles
        char buffer[100];
        for (size_t ii = 0; ii < this->m_nMaxNumProfiles; ii++)
        {
            sprintf_s(buffer, "profile.profile%zd.ProfileName", ii + 1);
            tree.put(buffer, this->m_psProfileName[ii]);
            sprintf_s(buffer, "profile.profile%zd.ProfileType", ii + 1);
            tree.put(buffer, profile_string_from_enum(this->m_pnProfileType[ii]));
            sprintf_s(buffer, "profile.profile%zd.UseIgnoreListTx", ii + 1);
            tree.put(buffer, this->m_pbUseIgnoreListTx[ii]);
            sprintf_s(buffer, "profile.profile%zd.AutoActivate", ii + 1);
            tree.put(buffer, this->m_pbAutoActivate[ii]);
            sprintf_s(buffer, "profile.profile%zd.FavoriteList", ii + 1);
            tree.put(buffer, create_string_from_vector(this->m_pvFavoriteList + ii));
            sprintf_s(buffer, "profile.profile%zd.ValidServer", ii + 1);
            tree.put(buffer, this->m_psServerName[ii]);
            sprintf_s(buffer, "profile.profile%zd.UseFavSubCh", ii + 1);
            tree.put(buffer, this->m_pbUseSubChOfFav[ii]);
            sprintf_s(buffer, "profile.profile%zd.MaxChLevel", ii + 1);
            tree.put(buffer, this->m_pnMaxChLevel[ii]);
            sprintf_s(buffer, "profile.profile%zd.MinChLevel", ii + 1);
            tree.put(buffer, this->m_pnMinChLevel[ii]);
            sprintf_s(buffer, "profile.profile%zd.ActiveFreq", ii + 1);
            tree.put(buffer, this->m_piActiveFreq[ii]);
            sprintf_s(buffer, "profile.profile%zd.Squelch", ii + 1);
            tree.put(buffer, this->m_pbSquelchFreq[ii]);
            sprintf_s(buffer, "profile.profile%zd.Priority", ii + 1);
            tree.put(buffer, this->m_pbPrioFreq[ii]);
            sprintf_s(buffer, "profile.profile%zd.Master", ii + 1);
            if (this->m_bUseMasterRight)
                tree.put(buffer, this->m_pbMasterFreq[ii]);
        }
        //thread safe end
        this->m_cConfigDataMutex.unlock();
    }

	boost::property_tree::xml_writer_settings<std::string> settings('\t', 1);
    try
    {
        boost::property_tree::write_xml(filename, tree, std::locale(), settings);
    }
    catch (boost::property_tree::xml_parser_error &e)
    {
        printf("PLUGIN: ERROR: write_param / write_xml => %s (%s)\n", e.message().c_str(), filename.c_str());
    }
}

/* ----------------------------------------------------------------------------
* check if a file exists
*/
bool config_container::file_exists( const std::string& filename )
{
    CALL_STACK
	std::ifstream f(filename.c_str());
	return f.good();
}


/* ----------------------------------------------------------------------------
* interface for channel list manipulation
*/
size_t config_container::s_delete_entry(std::vector<channel_info> *plReturn, int nEntry)
{
    CALL_STACK
    size_t nResult = ERROR_INVALID_POINTER;
	if (plReturn == nullptr) return nResult;

    //thread safe begin
    this->m_cConfigDataMutex.lock();

    if (nEntry <= 0 || nEntry >= plReturn->size())
        nResult=  IDNOTFOUND;
    else
    {
        plReturn->erase(plReturn->begin() + nEntry);
        nResult = plReturn->size();
    }

    //thread safe end
    this->m_cConfigDataMutex.unlock();
	return nResult;
}



/* ----------------------------------------------------------------------------
* add an entry to a channel_info list
*/
int config_container::s_add_entry(std::vector<channel_info> *plReturn, uint64 nChannelID, bool bIsPermanent, uint64 nChannelParent, std::string sChannelName)
{
    CALL_STACK
	channel_info temp = { nChannelID, bIsPermanent , nChannelParent, sChannelName, 0 };
	return s_add_entry(plReturn, temp);
}

/* ----------------------------------------------------------------------------
* add an entry to a channel_info list
*/
int config_container::s_add_entry(std::vector<channel_info> *plReturn, channel_info eEntry)
{
    CALL_STACK
    int iResult = ERROR_INVALID_POINTER;
	if (plReturn == nullptr) return iResult;
	// try to find if entry allready exists
	int nEntryfound = s_find_entry(plReturn, eEntry);
	if (nEntryfound > 0)
		return nEntryfound;

    // force invalid count to 0
    eEntry.iInvalidCount = 0;

    //thread safe begin
    this->m_cConfigDataMutex.lock();

    if (plReturn->size() == plReturn->capacity())
        iResult = IDNOTFOUND_MEM_FULL;
    else
    {
        printf("actual utilization of list %zd / %zd\n", plReturn->size(), plReturn->capacity() - 1);

        //add only if value doesn't exist
        plReturn->push_back(eEntry);
        iResult = IDNOTFOUND;
    }
    //thread safe end
    this->m_cConfigDataMutex.unlock();
	return iResult;
}



/* ----------------------------------------------------------------------------
* find an entry within a channel_info list
*/
int config_container::s_find_entry(std::vector<channel_info> *plReturn, uint64 nChannelID, bool bIsPermanent, uint64 nChannelParent, std::string sChannelName)
{
    CALL_STACK
	channel_info temp = { nChannelID, bIsPermanent , nChannelParent, sChannelName, 0 };
	return s_find_entry(plReturn, temp);
}

/* ----------------------------------------------------------------------------
* find an entry within a channel_info list
*/
int config_container::s_find_entry(std::vector<channel_info> *plReturn, channel_info eEntry)
{
    CALL_STACK
	if (plReturn == nullptr) return ERROR_INVALID_POINTER;

	int nResult = IDNOTFOUND;

    //thread safe begin
    this->m_cConfigDataMutex.lock();

	// search in array for an matching channel ID
	for (int ii = 0; ii < plReturn->size(); ii++)
	{
		//if channel ID matches, check other parameter
		if ((*plReturn)[ii].nChannelID == eEntry.nChannelID)
		{
			if ((*plReturn)[ii].nChannelParent != eEntry.nChannelParent)
			{
                printf("IDFOUND_INVALID_PARENT\n");
				nResult = IDFOUND_INVALID_PARENT;
				break;
			}
			if ((*plReturn)[ii].sChannelName.compare(eEntry.sChannelName) != 0)
			{
                printf("IDFOUND_INVALID_NAME\n");
				nResult = IDFOUND_INVALID_NAME;
				break;
			}

            // if channel matches, push result
			nResult = ii;
            break;
		}
	}

    //if channel ID was not found or invalid, but the channel is not permanent, try to find it using the name
    if ((nResult < 0) && !eEntry.bIsPermanent)
    {
        nResult = IDNOTFOUND_NONAME_MATCH;
        // search in array for an matching channel name
        for (int ii = 0; ii < plReturn->size(); ii++)
        {
            // if name matches, check parent
            if ((*plReturn)[ii].sChannelName.compare(eEntry.sChannelName) == 0)
            {
                if (((*plReturn)[ii].nChannelParent == eEntry.nChannelParent) && ((*plReturn)[ii].bIsPermanent == eEntry.bIsPermanent))
                {
                    // replace channel ID, with the new one
                    (*plReturn)[ii].nChannelID = eEntry.nChannelID;
                    
                    printf("Channel found by name\n");

                    // push result
                    nResult = ii;
                    break;
                }
            }
        }
    }

    //if channel was found, reset invalid counter
    if(nResult > 0)
        (*plReturn)[nResult].iInvalidCount = 0;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
	return nResult;
}


/* ----------------------------------------------------------------------------
* convert a channel_info entry to a string
*/
std::string config_container::create_string_from_entry(channel_info eChannel)
{
    std::string temp;
    temp += std::to_string(eChannel.nChannelID) + "," + std::to_string(eChannel.bIsPermanent) + "," + std::to_string(eChannel.nChannelParent) + "," + std::to_string(eChannel.iInvalidCount) + "," + eChannel.sChannelName + ";";
    return temp;
}

/* ----------------------------------------------------------------------------
* convert a channel_info list to a string
*/
std::string config_container::create_string_from_vector(std::vector<channel_info> *plReturn)
{
	if (plReturn == nullptr) return "";
	std::string temp;
	for (size_t ii = plReturn->size()-1;  ; ii--)
	{
        if ((*plReturn)[ii].nChannelID != 0 || plReturn->size() == 1)
            temp += create_string_from_entry((*plReturn)[ii]);

		if (ii == 0)
			break;
	}

	return temp;
}


/* ----------------------------------------------------------------------------
* clear vector and reinitialize with DEFAULT_CHANNEL_INFO
*/
void config_container::s_clear_vector(std::vector<channel_info> *plReturn)
{
    if (plReturn == nullptr) return;

    //thread safe begin
    this->m_cConfigDataMutex.lock();

    clear_vector(plReturn);

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

/* ----------------------------------------------------------------------------
* clear vector and reinitialize with DEFAULT_CHANNEL_INFO
*/
void config_container::clear_vector(std::vector<channel_info> *plReturn)
{
    CALL_STACK
    if (plReturn == nullptr) return;

    // delete all entrys
    plReturn->clear();
    
    // alocate enough memory ==> problem with automatic reallocation!
    plReturn->reserve(MAX__MAXNUMCHANNEL);

    channel_info temp = DEFAULT_CHANNEL_INFO;
    plReturn->push_back(temp);
}

/* ----------------------------------------------------------------------------
* copy channel info vector
*/
void config_container::copy_vector(std::vector<channel_info>* plDestination, std::vector<channel_info>* plSource)
{
    CALL_STACK
    if ((plDestination == nullptr) || (plSource == nullptr)) return;

    //delete old vector
    plDestination->clear();

    //copy element by element
    for (std::vector<channel_info>::iterator it = plSource->begin(); it != plSource->end(); ++it)
        plDestination->push_back(*it);

    return;
}

/* ----------------------------------------------------------------------------
* compare channel info vector
*/
bool config_container::compare_vector(std::vector<channel_info>* plDestination, std::vector<channel_info>* plSource)
{
    CALL_STACK
    bool bResult = true;

    //if the pointer are the same, they are equal
    if ((plDestination == plSource) || (plDestination == nullptr) || (plSource == nullptr))
        return true;

    //if the vectors have different size, they are different
    if (plDestination->size() != plSource->size())
        return false;

    //copy element by element
    for (int ii = 0; ii < plSource->size(); ++ii)
    {
        channel_info cSource        = (*plSource)[ii];
        channel_info cDestination   = (*plDestination)[ii];
        bResult &= (cSource.nChannelID      == cDestination.nChannelID);
        bResult &= (cSource.bIsPermanent    == cDestination.bIsPermanent);
        bResult &= (cSource.nChannelParent  == cDestination.nChannelParent);
        bResult &= (cSource.sChannelName.compare(cDestination.sChannelName) == 0);
        bResult &= (cSource.iInvalidCount   == cDestination.iInvalidCount);
    }

    return bResult;
}


/* ----------------------------------------------------------------------------
*
*/
int config_container::create_vector_from_string(std::vector<channel_info> *plReturn, std::string sData )
{
	if (plReturn == nullptr) return ERROR_INVALID_POINTER;
    clear_vector(plReturn);

	uint64		nChannelID;
	int         nIsPermanent;
	uint64		nChannelParent;
	char		sChannelName[100];
    int32_t     iInvalidCount;

	int nConverted = 0;
	size_t nStart  = 0;
	size_t nLength = 0;
    size_t nStartName = 0;
	std::string sPart;
    channel_info temp;

	do
	{
        // create sub string of a single channel
		nLength = sData.find(";", nStart);
		sPart = sData.substr(nStart, nLength-nStart);
		nStart += (nLength - nStart)+1;

        // convert sub string to parameter
		nConverted = sscanf_s(sPart.c_str(), "%llu,%d,%llu,%d,%s", &nChannelID, &nIsPermanent, &nChannelParent, &iInvalidCount, sChannelName, (unsigned int)sizeof(sChannelName));
		if (nConverted != 5)
			break;
        
        // extract name again to cover all special cases like a space
        nStartName = sPart.find_last_of(",");
        sPart = sPart.substr(nStartName + 1);

        // write result to list
		temp = { nChannelID, (nIsPermanent == 0) , nChannelParent, sPart, iInvalidCount };
		if(nChannelID != 0)
			plReturn->push_back(temp);

	} while (nStart < sData.length());

	return nConverted;
}



/* ----------------------------------------------------------------------------
* convert profile type enum to string to store in xml file
*/
std::string config_container::profile_string_from_enum(eProfileType nInput)
{
    std::string sResult = "off";

    if (nInput == PROFILE_LEVEL)
        sResult = "level";
    if (nInput == PROFILE_FAVORITE)
        sResult = "favorite";
    else if (nInput == PROFILE_AUDIO)
        sResult = "audio";
    else if (nInput == PROFILE_FREQUENCY)
        sResult = "frequency";

    return sResult;
}

/* ----------------------------------------------------------------------------
* convert string from xml file to profile type enum
*/
eProfileType config_container::profile_enum_from_string(std::string sInput)
{
    eProfileType nResult = PROFILE_OFF;

    if      (sInput.compare("level") == 0)
        nResult = PROFILE_LEVEL;
    else if (sInput.compare("favorite") == 0)
        nResult = PROFILE_FAVORITE;
    else if(sInput.compare("audio") == 0)
        nResult = PROFILE_AUDIO;
    else if ((sInput.compare("frequency") == 0) || (sInput.compare("freq") == 0))
        nResult = PROFILE_FREQUENCY;

    return nResult;
}


/* ----------------------------------------------------------------------------
* interface functions to config data
*/
bool config_container::s_get_ExpertMode()
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_bExpertMode;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

void config_container::s_set_ExpertMode(bool bValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_bExpertMode = bValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return;
}

size_t config_container::s_get_MaxNumProfiles()
{
    size_t nResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    nResult = this->m_nMaxNumProfiles;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return nResult;
}

void config_container::s_set_MaxNumProfiles(size_t nValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_nMaxNumProfiles = nValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

bool config_container::s_get_LGSActive()
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_bLGSActive;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

std::string config_container::s_get_Language()
{
    std::string sResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    sResult = this->m_sLanguage;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return sResult;
}

void config_container::s_set_Language(std::string sNewLanguage)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    if (sNewLanguage.compare("de") == 0 || sNewLanguage.compare("german") == 0 || sNewLanguage.compare("deutsch") == 0 || sNewLanguage.compare("German") == 0 || sNewLanguage.compare("Deutsch") == 0)
    {
        this->m_sLanguage = std::string("german");
    }
    else
    {
        this->m_sLanguage = std::string("english");
    }

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return;
}

int config_container::s_get_MaxNumFreq()
{
    int iResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    iResult = this->m_iMaxNumFreq;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return iResult;
}

void config_container::s_set_MaxNumFreq(int iValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_iMaxNumFreq = iValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

bool config_container::s_get_SaveIgnoreList()
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_bSaveIgnoreList;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

void config_container::s_set_SaveIgnoreList(bool bValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_bSaveIgnoreList = bValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return;
}

bool config_container::s_get_UseIgnoreListRx()
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_bUseIgnoreListRx;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

void config_container::s_set_UseIgnoreListRx(bool bValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_bUseIgnoreListRx = bValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return;
}

bool config_container::s_get_UseMasterRight()
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_bUseMasterRight;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

void config_container::s_set_UseMasterRight(bool bValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_bUseMasterRight = bValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return;
}

std::vector<channel_info>* config_container::s_get_IgnoreList()
{
    //TODO: How to handle access to the List???

    std::vector<channel_info>* pResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    pResult = this->m_pvIgnoreList;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return pResult;
}

channel_info config_container::s_get_IgnoreListEntry(int iIndex)
{
    channel_info cResult = INVALID_CHANNEL_INFO;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    if (this->m_pvIgnoreList != nullptr)
        cResult = (*this->m_pvIgnoreList)[iIndex];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return cResult;
}

std::string config_container::s_get_ProfileName(int iProfile)
{
    std::string sResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    sResult = this->m_psProfileName[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return sResult;
}

void config_container::s_set_ProfileName(int iProfile, std::string sValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_psProfileName[iProfile] = sValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return;
}

std::string config_container::get_DefaultProfileName(int iProfile)
{
    std::string sResult = std::string("Profile ") + std::to_string(iProfile + 1);
    return sResult;
}

eProfileType config_container::s_get_ProfileType(int iProfile)
{
    eProfileType cResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    cResult = this->m_pnProfileType[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return cResult;
}

void config_container::s_set_ProfileType(int iProfile, eProfileType eValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_pnProfileType[iProfile] = eValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return;
}

bool config_container::s_get_UseIgnoreListTx(int iProfile)
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_pbUseIgnoreListTx[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

void config_container::s_set_UseIgnoreListTx(int iProfile, bool bValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_pbUseIgnoreListTx[iProfile] = bValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return;
}

bool config_container::s_get_AutoActivate(int iProfile)
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_pbAutoActivate[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

void config_container::s_set_AutoActivate(int iProfile, bool bValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_pbAutoActivate[iProfile] = bValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return;
}

bool config_container::s_get_ActualState(int iProfile)
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_pbActualState[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

void config_container::s_set_ActualState(int iProfile, bool bValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_pbActualState[iProfile] = bValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

std::vector<channel_info>* config_container::s_get_FavoriteList(int iProfile)
{
    std::vector<channel_info>* pResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    pResult = this->m_pvFavoriteList + iProfile;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return pResult;
}

std::string config_container::s_get_ServerName(int iProfile)
{
    std::string sResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    sResult = this->m_psServerName[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return sResult;
}

void config_container::s_set_ServerName(int iProfile, std::string sValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    //if server name changes, cleanup favorite list
    if (sValue.compare(this->m_psServerName[iProfile]) != 0)
        clear_vector(this->m_pvFavoriteList + iProfile);

    this->m_psServerName[iProfile] = sValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

bool config_container::s_get_UseSubChOfFav(int iProfile)
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_pbUseSubChOfFav[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

void config_container::s_set_UseSubChOfFav(int iProfile, bool bValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_pbUseSubChOfFav[iProfile] = bValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return;
}

size_t config_container::s_get_MaxChLevel(int iProfile)
{
    size_t nResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    nResult = this->m_pnMaxChLevel[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return nResult;
}

void config_container::s_set_MaxChLevel(int iProfile, size_t nValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_pnMaxChLevel[iProfile] = nValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

size_t config_container::s_get_MinChLevel(int iProfile)
{
    size_t nResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    nResult = this->m_pnMinChLevel[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return nResult;
}

void config_container::s_set_MinChLevel(int iProfile, size_t nValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_pnMinChLevel[iProfile] = nValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

int config_container::s_get_ActiveFreq(int iProfile)
{
    int iResult;

    //thread safe begin
    this->m_cConfigDataMutex.lock();

    iResult = this->m_piActiveFreq[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return iResult;
}

void config_container::s_set_ActiveFreq(int iProfile, int iValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_piActiveFreq[iProfile] = iValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

bool config_container::s_get_MuteFreq(int iProfile)
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_pbMuteFreq[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

void config_container::s_set_MuteFreq(int iProfile, bool bValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_pbMuteFreq[iProfile] = bValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

bool config_container::s_get_SquelchFreq(int iProfile)
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_pbSquelchFreq[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

void config_container::s_set_SquelchFreq(int iProfile, bool bValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_pbSquelchFreq[iProfile] = bValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

bool config_container::s_get_PrioFreq(int iProfile)
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_pbPrioFreq[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

void config_container::s_set_PrioFreq(int iProfile, bool bValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_pbPrioFreq[iProfile] = bValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

bool config_container::s_get_MasterFreq(int iProfile)
{
    bool bResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    bResult = this->m_pbMasterFreq[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();
    return bResult;
}

void config_container::s_set_MasterFreq(int iProfile, bool bValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_pbMasterFreq[iProfile] = bValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

std::string config_container::s_get_GenHotKey_reset()
{
    std::string sResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    sResult = this->m_sGenHotKey_reset;

    //thread safe end
    this->m_cConfigDataMutex.unlock();

    return sResult;
}

void config_container::s_set_GenHotKey_reset(std::string sValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_sGenHotKey_reset = sValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

std::string config_container::s_get_GenHotKey_mute()
{
    std::string sResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    sResult = this->m_sGenHotKey_mute;

    //thread safe end
    this->m_cConfigDataMutex.unlock();

    return sResult;
}

void config_container::s_set_GenHotKey_mute(std::string sValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_sGenHotKey_mute = sValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

std::string config_container::s_get_HotKey_down(int iProfile)
{
    std::string sResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    sResult = this->m_psProfileHotKey_down[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();

    return sResult;
}

void config_container::s_set_HotKey_down(int iProfile, std::string sValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_psProfileHotKey_down[iProfile] = sValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}

std::string config_container::s_get_HotKey_up(int iProfile)
{
    std::string sResult;
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    sResult = this->m_psProfileHotKey_up[iProfile];

    //thread safe end
    this->m_cConfigDataMutex.unlock();

    return sResult;
}

void config_container::s_set_HotKey_up(int iProfile, std::string sValue)
{
    //thread safe begin
    this->m_cConfigDataMutex.lock();

    this->m_psProfileHotKey_up[iProfile] = sValue;

    //thread safe end
    this->m_cConfigDataMutex.unlock();
}