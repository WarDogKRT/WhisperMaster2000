//#include "stdafx.h"
#ifdef _WIN32
#pragma warning (disable : 4100)  /* Disable Unreferenced parameter warning */
#include <Windows.h>
#endif

#include <stringapiset.h>
#include "misc/language_pkg.h"

/* ----------------------------------------------------------------------------
* constructor: set all strings
*/
language_pkg::language_pkg()
{
    this->m_wLanguage = L"de";

    // template
    //-------------------------------------------------------------------------------------
    this->m_cLangPkg.add(L"en.temp", std::wstring(L""));
    this->m_cLangPkg.add(L"de.temp", std::wstring(L""));

    // run specific inits
    init_base();
    init_plugin_para_chk();
    init_plugin_hotkey();
    init_plugin_menu();
    init_plugin_info();
    init_ui_main();
    init_ui_freq();
    init_ui_client_tree();
    init_ui_about();
    init_error_handler();

    // Test
    //-------------------------------------------------------------------------------------

    // test1
    this->m_cLangPkg.add(L"en.test1", std::wstring(L"This is test number %d (\"äöüß\"). There are many tests, but this one is mine.\n"));
    this->m_cLangPkg.add(L"de.test1", std::wstring(L"Das ist der test nummer %d (\"äöüß\"). Es gibt viele tests, aber das ist meiner.\n"));

    // test2
    this->m_cLangPkg.add(L"en.test2", std::wstring(L"Another test %d\n"));
}


/* ----------------------------------------------------------------------------
* set all strings needed by basic functions (called by constructor)
*/
void language_pkg::init_base()
{
    // Base data
    //-------------------------------------------------------------------------------------

    // base_name (dosn't use language based on xml file)
    this->m_cLangPkg.add(L"en.base_name", std::wstring(L"WhisperMaster2000"));
    //this->m_cLangPkg.add(L"de.base_name", std::wstring(L"WhisperMaster2000"));

    // base_user (dosn't use language based on xml file)
    this->m_cLangPkg.add(L"en.base_user", std::wstring(L"wardogmuc | Michael"));
    //this->m_cLangPkg.add(L"de.base_user", std::wstring(L"wardogmuc | Michael"));

    // base_version (dosn't use language based on xml file)
    this->m_cLangPkg.add(L"en.base_version", std::wstring(L"Alpha 00.05"));
    //this->m_cLangPkg.add(L"de.base_version", std::wstring(L"Alpha 00.05"));

    // base_description (dosn't use language based on xml file)
    this->m_cLangPkg.add(L"en.base_description", std::wstring(L"WhisperMaster2000 helps you to master your communication with your friends without seting up whisperlists manually every time the people in your group are changing."));
    this->m_cLangPkg.add(L"de.base_description", std::wstring(L"WhisperMaster2000 hilft dir die Kommunikation mit deinen Teamkameraden zu meistern, ohne jedesmal deine Whisperlisten ändern zu müssen, weil sich die Gruppe verändert."));
}


/* ----------------------------------------------------------------------------
* set all strings needed by parameter check (called by constructor)
*/
void language_pkg::init_plugin_para_chk()
{
    // Parameter check
    //-------------------------------------------------------------------------------------

    // check_Init
    this->m_cLangPkg.add(L"en.check_Init", std::wstring(L"Start initialization."));
    this->m_cLangPkg.add(L"de.check_Init", std::wstring(L"Starte Initialisierung."));

    // conn_ParamCheck
    this->m_cLangPkg.add(L"en.conn_ParamCheck", std::wstring(L"Check on connect"));
    //this->m_cLangPkg.add(L"de.conn_ParamCheck", std::wstring(L""));

    // conn_ErrName
    this->m_cLangPkg.add(L"en.conn_ErrName", std::wstring(L"Unknown error querying server name. => plugin_base::onConnect()"));
    //this->m_cLangPkg.add(L"de.conn_ErrName", std::wstring(L""));

    // conn_WrongConn
    this->m_cLangPkg.add(L"en.conn_WrongConn", std::wstring(L"Status connection established was received, but error 'not connected' occured. => plugin_base::onConnect()"));
    //this->m_cLangPkg.add(L"de.conn_WrongConn", std::wstring(L""));

    // conn_ErrWriteFreq
    this->m_cLangPkg.add(L"en.conn_ErrWriteFreq", std::wstring(L"Initial writing of frequency list to server has returned an error."));
    //this->m_cLangPkg.add(L"de.conn_ErrWriteFreq", std::wstring(L""));

    // check_nMaxNumProfiles
    this->m_cLangPkg.add(L"en.check_nMaxNumProfiles", std::wstring(L"Setting 'general.MaxNumProfiles' (%zd) is too high, set value to default (%d)."));
    this->m_cLangPkg.add(L"de.check_nMaxNumProfiles", std::wstring(L"Einstellung 'general.MaxNumProfiles' (%zd) ist zu hoch, setze Wert auf Standard (%d)."));

    // check_psProfileType
    this->m_cLangPkg.add(L"en.check_psProfileType", std::wstring(L"Setting 'profile%d.ProfileName' is invalid, set new name to \"%s\"."));
    //this->m_cLangPkg.add(L"de.check_psProfileType", std::wstring(L"Einstellung 'm_nMaxNumProfiles' (%zd) ist zu hoch, setze Wert auf Standard (%d)."));

    // check_psProfileName
    this->m_cLangPkg.add(L"en.check_psProfileName", std::wstring(L"Setting 'profile%d.ProfileType' (%s) is invalid, set value to default (%s)."));
    //this->m_cLangPkg.add(L"de.check_psProfileName", std::wstring(L"Einstellung 'm_nMaxNumProfiles' (%zd) ist zu hoch, setze Wert auf Standard (%d)."));

    // check_MinChLevel_High
    this->m_cLangPkg.add(L"en.check_MinChLevel_High", std::wstring(L"Setting 'profile%d.MinChLevel' (%zd) is too high, set value to default (%d)."));
    //this->m_cLangPkg.add(L"de.check_MinChLevel_High", std::wstring(L""));

    // check_MinChLevel_Low
    this->m_cLangPkg.add(L"en.check_MinChLevel_Low", std::wstring(L"Setting 'profile%d.MinChLevel' (%zd) is too low, set value to default (%d)."));
    //this->m_cLangPkg.add(L"de.check_MinChLevel_Low", std::wstring(L""));

    // check_MaxChLevel_High
    this->m_cLangPkg.add(L"en.check_MaxChLevel_High", std::wstring(L"Setting 'profile%d.MaxChLevel' (%zd) is too high, set value to default (%d)."));
    //this->m_cLangPkg.add(L"de.check_MaxChLevel_High", std::wstring(L""));

    // check_Min2MaxChLevel
    this->m_cLangPkg.add(L"en.check_Min2MaxChLevel", std::wstring(L"Setting 'profile%d.MaxChLevel' is lower then 'profile%d.MinChLevel' (%zd < %zd), set 'MaxChLevel' to default (%d)."));
    //this->m_cLangPkg.add(L"de.check_Min2MaxChLevel", std::wstring(L""));

    // check_MaxFreq
    this->m_cLangPkg.add(L"en.check_MaxFreq", std::wstring(L"Setting 'MaxFrequency' %d is out of limits (%d to %d), set to default %d."));
    //this->m_cLangPkg.add(L"de.check_MaxFreq", std::wstring(L""));

    // check_ActiveFreq
    this->m_cLangPkg.add(L"en.check_ActiveFreq", std::wstring(L"Setting 'profile%d.ActiveFreq' %d is out of limits (%d to %d), disable 'ActiveFreq'."));
    //this->m_cLangPkg.add(L"de.check_ActiveFreq", std::wstring(L""));

    // check_ValidServer
    this->m_cLangPkg.add(L"en.check_ValidServer", std::wstring(L"Setting 'profile%d.ProfileType' is favorite/audio, but no 'profile%d.ValidServer' selected. Profile%d will be disabled until you select a channel!"));
    //this->m_cLangPkg.add(L"de.check_ValidServer", std::wstring(L""));

    // check_Connect_ValidServer
    this->m_cLangPkg.add(L"en.check_Connect_ValidServer", std::wstring(L"Setting 'profile%d.ServerName' (%s) missmatch with connected server (%s). Profile%d will be disabled until you select a channel."));
    //this->m_cLangPkg.add(L"de.check_Connect_ValidServer", std::wstring(L""));

    // check_Connect_Hotkey
    this->m_cLangPkg.add(L"en.check_Connect_Hotkey", std::wstring(L"No hotkey is set for profile \"%s\"."));
    //this->m_cLangPkg.add(L"de.check_Connect_Hotkey", std::wstring(L""));
}


/* ----------------------------------------------------------------------------
* set all strings needed by hotkey creation / handling (called by constructor)
*/
void language_pkg::init_plugin_hotkey()
{
    // Hotkeys creation
    //-------------------------------------------------------------------------------------

    // hotkey_UseFav
    this->m_cLangPkg.add(L"en.hotkey_UseFav", std::wstring(L"Use profile %d (favorite) and activate whispering"));
    this->m_cLangPkg.add(L"de.hotkey_UseFav", std::wstring(L"Nutze Profil %d (favorite) und aktiviere whispering"));

    // hotkey_ToggleFav
    this->m_cLangPkg.add(L"en.hotkey_ToggleFav", std::wstring(L"Toggle profile %d (favorite)"));
    this->m_cLangPkg.add(L"de.hotkey_ToggleFav", std::wstring(L"Schalte Profil %d (favorite) um"));

    // hotkey_UseLevel
    this->m_cLangPkg.add(L"en.hotkey_UseLevel", std::wstring(L"Use profile %d (level) and activate whispering"));
    this->m_cLangPkg.add(L"de.hotkey_UseLevel", std::wstring(L"Nutze Profil %d (level) und aktiviere whispering"));

    // hotkey_ToggleLevel
    this->m_cLangPkg.add(L"en.hotkey_ToggleLevel", std::wstring(L"Toggle profile %d (level)"));
    this->m_cLangPkg.add(L"de.hotkey_ToggleLevel", std::wstring(L"Schalte Profil %d (level) um"));

    // hotkey_UseFreq
    this->m_cLangPkg.add(L"en.hotkey_UseFreq", std::wstring(L"Use profile %d (frequency) and activate whispering"));
    this->m_cLangPkg.add(L"de.hotkey_UseFreq", std::wstring(L"Nutze Profil %d (Frequenz) und aktiviere whispering"));

    // hotkey_ToggleFreq
    this->m_cLangPkg.add(L"en.hotkey_ToggleFreq", std::wstring(L"Toggle profile %d (frequency)"));
    this->m_cLangPkg.add(L"de.hotkey_ToggleFreq", std::wstring(L"Schalte Profil %d (Frequenz) um"));

    // hotkey_Reset
    this->m_cLangPkg.add(L"en.hotkey_Reset", std::wstring(L"Deactivate whispering"));
    this->m_cLangPkg.add(L"de.hotkey_Reset", std::wstring(L"Deaktiviere whispering"));



    // Hotkey event handler
    //-------------------------------------------------------------------------------------

    // hotkey_NotConn
    this->m_cLangPkg.add(L"en.hotkey_NotConn", std::wstring(L"Hotkey event was received, but no server is connected. Hotkey will be ignored."));
    this->m_cLangPkg.add(L"de.hotkey_NotConn", std::wstring(L"Hotkey Ereignis wurde empfangen, aber kein Server ist verbunden. Hotkey wird ignoriert."));

    // hotkey_ErrReset
    this->m_cLangPkg.add(L"en.hotkey_ErrReset", std::wstring(L"Unknown error while cleaning up whisperlist. => plugin_base::onHotkeyEvent()"));
    this->m_cLangPkg.add(L"de.hotkey_ErrReset", std::wstring(L"Unbekannter Fehler beim zurücksetzen der whisperliste aufgetretten. => plugin_base::onHotkeyEvent()"));

    // hotkey_ErrActivate1
    this->m_cLangPkg.add(L"en.hotkey_ErrActivate1", std::wstring(L"Error while (de-)activating Push-To-Talk (set variable) [Error code : 0x%04X]"));
    //this->m_cLangPkg.add(L"de.hotkey_ErrActivate", std::wstring(L"Unbekannter Fehler beim zurücksetzen der whisperliste aufgetretten."));

    // hotkey_ErrActivate2
    this->m_cLangPkg.add(L"en.hotkey_ErrActivate2", std::wstring(L"Error while (de-)activating Push-To-Talk (flush). [Error code : 0x%04X]"));
    //this->m_cLangPkg.add(L"de.hotkey_ErrActivate", std::wstring(L"Unbekannter Fehler beim zurücksetzen der whisperliste aufgetretten."));

    // hotkey_MaxNum
    this->m_cLangPkg.add(L"en.hotkey_MaxNum", std::wstring(L"You are using a hotkey of profile %d, but the max. number of profiles is %d. Hotkey will be ignored."));
    this->m_cLangPkg.add(L"de.hotkey_MaxNum", std::wstring(L"Sie verwenden einen Hokey des Profils %d, aber die max. Anzahl der Profile ist %d. Hotkey wird ignoriert."));

    // hotkey_Off
    this->m_cLangPkg.add(L"en.hotkey_Off", std::wstring(L"You are using a hotkey of profile %d, but the type is '%s'. Hotkey will be ignored."));
    this->m_cLangPkg.add(L"de.hotkey_Off", std::wstring(L"Sie verwenden einen Hotkey des Profils %d, aber dessen Typ ist aber '%s'. Hotkey wird ignoriert."));

    // hotkey_LevelOutRange
    this->m_cLangPkg.add(L"en.hotkey_LevelOutRange", std::wstring(L"Selected channel range (%zd - %zd) of profile \"%s\" doesn't match the actual channel level (%zd). No whispertarget can be found."));
    //this->m_cLangPkg.add(L"de.hotkey_LevelOutRange", std::wstring(L"Sie verwenden einen Hotkey des Profils %d, aber dessen Typ ist aber '%s'. Hotkey wird ignoriert."));

    // hotkey_NoActiveClients
    this->m_cLangPkg.add(L"en.hotkey_NoActiveClients", std::wstring(L"Selected frequency (%d) of profile \"%s\" is not active on any other client."));
    //this->m_cLangPkg.add(L"de.hotkey_NoActiveClients", std::wstring(L""));
}


/* ----------------------------------------------------------------------------
* set all strings needed by menu creation / handling (called by constructor)
*/
void language_pkg::init_plugin_menu()
{
    // Menu creation
    //-------------------------------------------------------------------------------------

    // menu_SetIgnore
    this->m_cLangPkg.add(L"en.menu_SetIgnore", std::wstring(L"Ignore channel %s"));
    this->m_cLangPkg.add(L"de.menu_SetIgnore", std::wstring(L"Ignoriere Kanal %s"));

    // menu_ToggleFav
    this->m_cLangPkg.add(L"en.menu_ToggleFav", std::wstring(L"Toggle favorite (%s) %s"));
    this->m_cLangPkg.add(L"de.menu_ToggleFav", std::wstring(L"Schalte Favorit um (%s) %s"));

    // menu_ToggleAudio
    this->m_cLangPkg.add(L"en.menu_ToggleAudio", std::wstring(L"Toggle audio (%s) %s"));
    this->m_cLangPkg.add(L"de.menu_ToggleAudio", std::wstring(L"Schalte Audio um (%s) %s"));

    // menu_SetStartLev
    this->m_cLangPkg.add(L"en.menu_SetStartLev", std::wstring(L"Set start level (%s)"));
    this->m_cLangPkg.add(L"de.menu_SetStartLev", std::wstring(L"Setze Start-Level (%s)"));

    // menu_SetEndLev
    this->m_cLangPkg.add(L"en.menu_SetEndLev", std::wstring(L"Set end level (%s)"));
    this->m_cLangPkg.add(L"de.menu_SetEndLev", std::wstring(L"Setze End-Level (%s)"));

    // menu_OpenSettings
    this->m_cLangPkg.add(L"en.menu_OpenSettings", std::wstring(L"Settings"));
    this->m_cLangPkg.add(L"de.menu_OpenSettings", std::wstring(L"Einstellungen"));

    // menu_ShowList
    this->m_cLangPkg.add(L"en.menu_ShowList", std::wstring(L"Show all lists"));
    this->m_cLangPkg.add(L"de.menu_ShowList", std::wstring(L"Zeige alle Listen"));

    // menu_DelIgnore
    this->m_cLangPkg.add(L"en.menu_DelIgnore", std::wstring(L"Delete list (Ignore)"));
    this->m_cLangPkg.add(L"de.menu_DelIgnore", std::wstring(L"Lösche Liste (Ignorieren)"));

    // menu_DelProfile
    this->m_cLangPkg.add(L"en.menu_DelProfile", std::wstring(L"Delete list (%s)"));
    this->m_cLangPkg.add(L"de.menu_DelProfile", std::wstring(L"Lösche Liste (%s)"));

    // menu_SetFreq
    this->m_cLangPkg.add(L"en.menu_SetFreq", std::wstring(L"Set frequency"));
    //this->m_cLangPkg.add(L"de.menu_SetFreq", std::wstring(L""));

    // menu_MuteProfile
    this->m_cLangPkg.add(L"en.menu_MuteProfile", std::wstring(L"Un-/Mute frequency (%s)"));
    //this->m_cLangPkg.add(L"de.menu_MuteProfile", std::wstring(L""));

    // menu_help
    this->m_cLangPkg.add(L"en.menu_help", std::wstring(L"Help"));
    this->m_cLangPkg.add(L"de.menu_help", std::wstring(L"Hilfe"));



    // Menu event handler
    //-------------------------------------------------------------------------------------

    // menuEv_NameChange
    this->m_cLangPkg.add(L"en.menuEv_NameChange", std::wstring(L"Server name of profile \"%s\" has changed from \"%s\" to \"%s\"."));
    //this->m_cLangPkg.add(L"de.menuEv_NameChange", std::wstring(L""));

    // menuEv_HelpNotFound
    this->m_cLangPkg.add(L"en.menuEv_HelpNotFound", std::wstring(L"Help file not found. (%s)"));
    //this->m_cLangPkg.add(L"de.menuEv_HelpNotFound", std::wstring(L""));

    // menuEv_ListFull
    this->m_cLangPkg.add(L"en.menuEv_ListFull", std::wstring(L"Adding an entry to list \"%s\" not possible. List is full %zd out of %zd entrys."));
    //this->m_cLangPkg.add(L"de.menuEv_ListFull", std::wstring(L""));

    // menuEv_IgnoreInfo
    this->m_cLangPkg.add(L"en.menuEv_IgnoreInfo", std::wstring(L"[B]Ignore list (%zd entries):[/B]\n"));
    //this->m_cLangPkg.add(L"de.menuEv_IgnoreInfo", std::wstring(L""));

    // menuEv_FavoriteInfo
    this->m_cLangPkg.add(L"en.menuEv_FavoriteInfo", std::wstring(L"[B]Favorite list of \"%s\" (%zd entries):[/B]\n"));
    //this->m_cLangPkg.add(L"de.menuEv_FavoriteInfo", std::wstring(L""));

    // menuEv_LevelInfo1
    this->m_cLangPkg.add(L"en.menuEv_LevelInfo1", std::wstring(L"[B]Level of \"%s\" (from level %zd to %zd):[/B]\n"));
    //this->m_cLangPkg.add(L"de.menuEv_LevelInfo1", std::wstring(L""));

    // menuEv_LevelInfo2
    this->m_cLangPkg.add(L"en.menuEv_LevelInfo2", std::wstring(L"[B]Level of \"%s\" (from level %zd):[/B]\n"));
    //this->m_cLangPkg.add(L"de.menuEv_LevelInfo2", std::wstring(L""));

    // menuEv_NoChInRange
    this->m_cLangPkg.add(L"en.menuEv_NoChInRange", std::wstring(L"+____ No channels in range\n"));
    //this->m_cLangPkg.add(L"de.menuEv_NoChInRange", std::wstring(L""));

    // menuEv_NoChInList
    this->m_cLangPkg.add(L"en.menuEv_NoChInList", std::wstring(L"+____ No channels in list\n"));
    //this->m_cLangPkg.add(L"de.menuEv_NoChInList", std::wstring(L""));

    // menuEv_FreqInfo
    this->m_cLangPkg.add(L"en.menuEv_FreqInfo", std::wstring(L"[B]Clients with active freq. %d of \"%s\" (%zd clients)%s:[/B]\n"));
    //this->m_cLangPkg.add(L"de.menuEv_FreqInfo", std::wstring(L""));

    // menuEv_FreqDisabled
    this->m_cLangPkg.add(L"en.menuEv_FreqDisabled", std::wstring(L"+____ Frequency is disabled\n"));
    //this->m_cLangPkg.add(L"de.menuEv_FreqDisabled", std::wstring(L""));

    // menuEv_NoClientInList
    this->m_cLangPkg.add(L"en.menuEv_NoClientInList", std::wstring(L"+____ No clients in list\n"));
    //this->m_cLangPkg.add(L"de.menuEv_NoClientInList", std::wstring(L""));

    // menuEv_FreqChangeInfo
    this->m_cLangPkg.add(L"en.menuEv_FreqChangeInfo", std::wstring(L"Info: Send \"WM2000:freq:%d 10\" to set frequency (10) of the selected profile (%d). Use frequency \"0\" to disable profile. Use \"free\" instead of a frequency number to select next unused frequency."));
    //this->m_cLangPkg.add(L"de.menuEv_FreqChangeInfo", std::wstring(L""));
}

    
/* ----------------------------------------------------------------------------
* set all strings needed by info window handling (called by constructor)
*/
void language_pkg::init_plugin_info()
{
    // INFO sub window
    //-------------------------------------------------------------------------------------

    // info_Title
    this->m_cLangPkg.add(L"en.info_Title", std::wstring(L"WM2000 / Active profile"));
    this->m_cLangPkg.add(L"de.info_Title", std::wstring(L"WM2000 / Aktive Profile"));

    // info_Default
    this->m_cLangPkg.add(L"en.info_Default", std::wstring(L"None"));
    this->m_cLangPkg.add(L"de.info_Default", std::wstring(L"Keine"));

    // info_Freq
    this->m_cLangPkg.add(L"en.info_Freq", std::wstring(L"Active freq.: "));
    this->m_cLangPkg.add(L"de.info_Freq", std::wstring(L"Aktive Freq.: "));

    // info_FreqDisabled
    this->m_cLangPkg.add(L"en.info_FreqDisabled", std::wstring(L"No active freq."));
    this->m_cLangPkg.add(L"de.info_FreqDisabled", std::wstring(L"Keine aktiven Freq."));

    // info_NoWM2000
    this->m_cLangPkg.add(L"en.info_NoWM2000", std::wstring(L"WM2000 not active"));
    this->m_cLangPkg.add(L"de.info_NoWM2000", std::wstring(L"WM2000 nicht aktiv"));

    // info_Ignore
    this->m_cLangPkg.add(L"en.info_Ignore", std::wstring(L"[B]Ignored[/B]"));
    this->m_cLangPkg.add(L"de.info_Ignore", std::wstring(L"[B]Ignoriert[/B]"));
}


/* ----------------------------------------------------------------------------
* set all strings needed by Main UI (called by constructor)
*/
void language_pkg::init_ui_main()
{
    // general config UI
    //-------------------------------------------------------------------------------------

    // gUi_pbOk
    this->m_cLangPkg.add(L"en.gUi_pbOk", std::wstring(L"Ok"));
    //this->m_cLangPkg.add(L"de.gUi_pbOk", std::wstring(L""));

    // gUi_pbCancel
    this->m_cLangPkg.add(L"en.gUi_pbCancel", std::wstring(L"Cancel"));
    this->m_cLangPkg.add(L"de.gUi_pbCancel", std::wstring(L"Abbrechen"));

    // gUi_pbApply
    this->m_cLangPkg.add(L"en.gUi_pbApply", std::wstring(L"Apply"));
    this->m_cLangPkg.add(L"de.gUi_pbApply", std::wstring(L"Übernehmen"));



    // Main config UI
    //-------------------------------------------------------------------------------------

    // mUi_fmTitle
    this->m_cLangPkg.add(L"en.mUi_fmTitle", std::wstring(L"WM2000 plugin main settings"));
    this->m_cLangPkg.add(L"de.mUi_fmTitle", std::wstring(L"WM2000 plugin Einstellungen"));

    // mUi_MenuTitle
    this->m_cLangPkg.add(L"en.mUi_MenuTitle", std::wstring(L"File"));
    this->m_cLangPkg.add(L"de.mUi_MenuTitle", std::wstring(L"Datei"));

    // mUi_MenuLoad
    this->m_cLangPkg.add(L"en.mUi_MenuLoad", std::wstring(L"Load"));
    this->m_cLangPkg.add(L"de.mUi_MenuLoad", std::wstring(L"Lade"));

    // mUi_ToolMenuLoad
    this->m_cLangPkg.add(L"en.mUi_ToolMenuLoad", std::wstring(L"Load default settings"));
    this->m_cLangPkg.add(L"de.mUi_ToolMenuLoad", std::wstring(L"Lade Grundeinstellungen"));

    // mUi_MenuSave
    this->m_cLangPkg.add(L"en.mUi_MenuSave", std::wstring(L"Save"));
    this->m_cLangPkg.add(L"de.mUi_MenuSave", std::wstring(L"Speichern"));

    // mUi_MenuSaveAs
    this->m_cLangPkg.add(L"en.mUi_MenuSaveAs", std::wstring(L"Save as"));
    this->m_cLangPkg.add(L"de.mUi_MenuSaveAs", std::wstring(L"Speichern unter"));

    // mUi_ToolMenuSave
    this->m_cLangPkg.add(L"en.mUi_ToolMenuSave", std::wstring(L"Save settings"));
    this->m_cLangPkg.add(L"de.mUi_ToolMenuSave", std::wstring(L"Speichere Einstellungen"));

    // mUi_MenuClose
    this->m_cLangPkg.add(L"en.mUi_MenuClose", std::wstring(L"Close"));
    this->m_cLangPkg.add(L"de.mUi_MenuClose", std::wstring(L"Beenden"));

    // mUi_ToolMenuClose
    this->m_cLangPkg.add(L"en.mUi_ToolMenuClose", std::wstring(L"Save settings and close window"));
    this->m_cLangPkg.add(L"de.mUi_ToolMenuClose", std::wstring(L"Speichere Einstellungen und schließe das Fenster"));

    // mUi_MenuEditTitle
    this->m_cLangPkg.add(L"en.mUi_MenuEditTitle", std::wstring(L"Edit"));
    this->m_cLangPkg.add(L"de.mUi_MenuEditTitle", std::wstring(L"Bearbeiten"));

    // mUi_MenuUndo
    this->m_cLangPkg.add(L"en.mUi_MenuUndo", std::wstring(L"Undo"));
    this->m_cLangPkg.add(L"de.mUi_MenuUndo", std::wstring(L"Rückgängig"));

    // mUi_ToolMenuUndo
    this->m_cLangPkg.add(L"en.mUi_ToolMenuUndo", std::wstring(L"Undo all changed settings"));
    this->m_cLangPkg.add(L"de.mUi_ToolMenuUndo", std::wstring(L"Mache alle Änderungen rückgängig"));

    // mUi_MenuViewTitle
    this->m_cLangPkg.add(L"en.mUi_MenuViewTitle", std::wstring(L"View"));
    this->m_cLangPkg.add(L"de.mUi_MenuViewTitle", std::wstring(L"Ansicht"));
    
    // mUi_MenuExpert
    this->m_cLangPkg.add(L"en.mUi_MenuExpert", std::wstring(L"Expert"));
    this->m_cLangPkg.add(L"de.mUi_MenuExpert", std::wstring(L"Experte"));

    // mUi_ToolMenuExpert
    this->m_cLangPkg.add(L"en.mUi_ToolMenuExpert", std::wstring(L"Enables/Disables some expert settings"));
    this->m_cLangPkg.add(L"de.mUi_ToolMenuExpert", std::wstring(L"Aktiviert/Deaktiviert einige experten Einstellungen"));

    // mUi_MenuOpenFreq
    this->m_cLangPkg.add(L"en.mUi_MenuOpenFreq", std::wstring(L"Set Frequency"));
    this->m_cLangPkg.add(L"de.mUi_MenuOpenFreq", std::wstring(L"Setze Frequenz"));

    // mUi_ToolMenuOpenFreq
    this->m_cLangPkg.add(L"en.mUi_ToolMenuOpenFreq", std::wstring(L"Opens the \"Set Frequency\" window"));
    this->m_cLangPkg.add(L"de.mUi_ToolMenuOpenFreq", std::wstring(L"Öffnet das \"Setze Frequenz\" Fenster"));

    // mUi_MenuHelpTitle
    this->m_cLangPkg.add(L"en.mUi_MenuHelpTitle", std::wstring(L"Help"));
    this->m_cLangPkg.add(L"de.mUi_MenuHelpTitle", std::wstring(L"Hilfe"));

    // mUi_MenuAbout
    this->m_cLangPkg.add(L"en.mUi_MenuAbout", std::wstring(L"About WM2000"));
    this->m_cLangPkg.add(L"de.mUi_MenuAbout", std::wstring(L"Über WM2000"));

    // mUi_TabGeneral
    this->m_cLangPkg.add(L"en.mUi_TabGeneral", std::wstring(L"General"));
    this->m_cLangPkg.add(L"de.mUi_TabGeneral", std::wstring(L"Allgemein"));

    // mUi_TabGeneralTip
    this->m_cLangPkg.add(L"en.mUi_TabGeneralTip", std::wstring(L"General settings"));
    this->m_cLangPkg.add(L"de.mUi_TabGeneralTip", std::wstring(L"Allgemeine Einstellungen"));

    // mUi_TabGeneralWhat
    this->m_cLangPkg.add(L"en.mUi_TabGeneralWhat", std::wstring(L"Open tab with general settings."));
    this->m_cLangPkg.add(L"de.mUi_TabGeneralWhat", std::wstring(L"Öffne tab mit allgemeneinen Einstellungen."));

    // mUi_NumProfile
    this->m_cLangPkg.add(L"en.mUi_NumProfile", std::wstring(L"Number of profiles:"));
    this->m_cLangPkg.add(L"de.mUi_NumProfile", std::wstring(L"Anzahl Profile:"));

    // mUi_Language
    this->m_cLangPkg.add(L"en.mUi_Language", std::wstring(L"Language:"));
    this->m_cLangPkg.add(L"de.mUi_Language", std::wstring(L"Sprache:"));

    // mUi_LanguageEn
    this->m_cLangPkg.add(L"en.mUi_LanguageEn", std::wstring(L"English"));
    this->m_cLangPkg.add(L"de.mUi_LanguageEn", std::wstring(L"Englisch"));

    // mUi_LanguageDe
    this->m_cLangPkg.add(L"en.mUi_LanguageDe", std::wstring(L"German"));
    this->m_cLangPkg.add(L"de.mUi_LanguageDe", std::wstring(L"Deutsch"));

    // mUi_HotKeyReset
    this->m_cLangPkg.add(L"en.mUi_HotKeyReset", std::wstring(L"Reset hotkeys:"));
    this->m_cLangPkg.add(L"de.mUi_HotKeyReset", std::wstring(L"Reset Hotkeys:"));

    // mUi_HotKeyMute
    this->m_cLangPkg.add(L"en.mUi_HotKeyMute", std::wstring(L"Global mute:"));
    this->m_cLangPkg.add(L"de.mUi_HotKeyMute", std::wstring(L"Globales Mute:"));

    // mUi_HotKeyDefault
    this->m_cLangPkg.add(L"en.mUi_HotKeyDefault", std::wstring(L"No Hotkey"));
    this->m_cLangPkg.add(L"de.mUi_HotKeyDefault", std::wstring(L"Kein Hotkey"));

    // mUi_HotKeyDown
    this->m_cLangPkg.add(L"en.mUi_HotKeyDown", std::wstring(L"Hotkey \"press\":"));
    this->m_cLangPkg.add(L"de.mUi_HotKeyDown", std::wstring(L"Hotkey \"drücken\":"));

    // mUi_HotKeyUp
    this->m_cLangPkg.add(L"en.mUi_HotKeyUp", std::wstring(L"Hotkey \"release\":"));
    this->m_cLangPkg.add(L"de.mUi_HotKeyUp", std::wstring(L"Hotkey \"loslassen\":"));

    // mUi_Ignore
    this->m_cLangPkg.add(L"en.mUi_Ignore", std::wstring(L"Ignore list"));
    this->m_cLangPkg.add(L"de.mUi_Ignore", std::wstring(L"Ignore-Liste"));

    // mUi_DelIgnore
    this->m_cLangPkg.add(L"en.mUi_DelIgnore", std::wstring(L"Delete list"));
    this->m_cLangPkg.add(L"de.mUi_DelIgnore", std::wstring(L"Liste löschen"));

    // mUi_UseForRx
    this->m_cLangPkg.add(L"en.mUi_UseForRx", std::wstring(L"Use for receiving:"));
    this->m_cLangPkg.add(L"de.mUi_UseForRx", std::wstring(L"Verwenden zum Empfangen:"));

    // mUi_AutoSave
    this->m_cLangPkg.add(L"en.mUi_AutoSave", std::wstring(L"Auto save configuration:"));
    this->m_cLangPkg.add(L"de.mUi_AutoSave", std::wstring(L"Automatisch speichern:"));

    // mUi_ProfileName
    this->m_cLangPkg.add(L"en.mUi_ProfileName", std::wstring(L"Name of profile"));
    this->m_cLangPkg.add(L"de.mUi_ProfileName", std::wstring(L"Name des Profils"));

    // mUi_ProfileType
    this->m_cLangPkg.add(L"en.mUi_ProfileType", std::wstring(L"Type of profile:"));
    this->m_cLangPkg.add(L"de.mUi_ProfileType", std::wstring(L"Profiltyp:"));

    // mUi_ProfileOff
    this->m_cLangPkg.add(L"en.mUi_ProfileOff", std::wstring(L"Deactivated"));
    this->m_cLangPkg.add(L"de.mUi_ProfileOff", std::wstring(L"Deaktiviert"));

    // mUi_ProfileFav
    this->m_cLangPkg.add(L"en.mUi_ProfileFav", std::wstring(L"Favorite"));
    this->m_cLangPkg.add(L"de.mUi_ProfileFav", std::wstring(L"Favorit"));

    // mUi_ProfileLevel
    this->m_cLangPkg.add(L"en.mUi_ProfileLevel", std::wstring(L"Level"));
    this->m_cLangPkg.add(L"de.mUi_ProfileLevel", std::wstring(L"Level"));

    // mUi_ProfileFreq
    this->m_cLangPkg.add(L"en.mUi_ProfileFreq", std::wstring(L"Frequency"));
    this->m_cLangPkg.add(L"de.mUi_ProfileFreq", std::wstring(L"Frequenz"));

    // mUi_UseIgnoreTx
    this->m_cLangPkg.add(L"en.mUi_UseIgnoreTx", std::wstring(L"Use ignore list for transmit:"));
    this->m_cLangPkg.add(L"de.mUi_UseIgnoreTx", std::wstring(L"Verwende Ignore-List zum senden:"));

    // mUi_AutoPtt
    this->m_cLangPkg.add(L"en.mUi_AutoPtt", std::wstring(L"Auto activate (PTT):"));
    this->m_cLangPkg.add(L"de.mUi_AutoPtt", std::wstring(L"Auto aktivieren (PTT):"));

    // mUi_ProfileOffGb
    this->m_cLangPkg.add(L"en.mUi_ProfileOffGb", std::wstring(L"Profile deactivated"));
    this->m_cLangPkg.add(L"de.mUi_ProfileOffGb", std::wstring(L"Profil deaktiviert"));

    // mUi_ProfileLevelGb
    this->m_cLangPkg.add(L"en.mUi_ProfileLevelGb", std::wstring(L"Channel Level"));
    this->m_cLangPkg.add(L"de.mUi_ProfileLevelGb", std::wstring(L"Channel-Level"));

    // mUi_LowerLevel
    this->m_cLangPkg.add(L"en.mUi_LowerLevel", std::wstring(L"Lower bound:"));
    this->m_cLangPkg.add(L"de.mUi_LowerLevel", std::wstring(L"Untere Grenze:"));

    // mUi_UpperLevel
    this->m_cLangPkg.add(L"en.mUi_UpperLevel", std::wstring(L"Upper bound (optional):"));
    this->m_cLangPkg.add(L"de.mUi_UpperLevel", std::wstring(L"Obere Grenze (Optional):"));

    // mUi_UseSubCh
    this->m_cLangPkg.add(L"en.mUi_UseSubCh", std::wstring(L"Use sub-channel:"));
    this->m_cLangPkg.add(L"de.mUi_UseSubCh", std::wstring(L"Verwende sub-channel:"));

    // mUi_TransFreq
    this->m_cLangPkg.add(L"en.mUi_TransFreq", std::wstring(L"Transceiver frequency:"));
    this->m_cLangPkg.add(L"de.mUi_TransFreq", std::wstring(L"Sendeempfänger Frequenz:"));

    // mUi_NextFreq
    this->m_cLangPkg.add(L"en.mUi_NextFreq", std::wstring(L"Next free"));
    this->m_cLangPkg.add(L"de.mUi_NextFreq", std::wstring(L"Nächste freie"));

    // mUi_ProfMode
    this->m_cLangPkg.add(L"en.mUi_ProfMode", std::wstring(L"Mode:"));
    this->m_cLangPkg.add(L"de.mUi_ProfMode", std::wstring(L"Modus:"));

    // mUi_ProfModeOff
    this->m_cLangPkg.add(L"en.mUi_ProfModeOff", std::wstring(L"Off"));
    this->m_cLangPkg.add(L"de.mUi_ProfModeOff", std::wstring(L"Off"));

    // mUi_ProfModeNorm
    this->m_cLangPkg.add(L"en.mUi_ProfModeNorm", std::wstring(L"Normal"));
    this->m_cLangPkg.add(L"de.mUi_ProfModeNorm", std::wstring(L"Normal"));

    // mUi_ProfModeMute
    this->m_cLangPkg.add(L"en.mUi_ProfModeMute", std::wstring(L"Mute"));
    this->m_cLangPkg.add(L"de.mUi_ProfModeMute", std::wstring(L"Stumm"));

    // mUi_ProfModeSquelch
    this->m_cLangPkg.add(L"en.mUi_ProfModeSquelch", std::wstring(L"Squelch"));
    this->m_cLangPkg.add(L"de.mUi_ProfModeSquelch", std::wstring(L"Squelch"));

    // mUi_UsePrio
    this->m_cLangPkg.add(L"en.mUi_UsePrio", std::wstring(L"Transmit with priority:"));
    this->m_cLangPkg.add(L"de.mUi_UsePrio", std::wstring(L"Sende mit Priorität:"));

    // mUi_UseMaster
    this->m_cLangPkg.add(L"en.mUi_UseMaster", std::wstring(L"Transmit with master rights:"));
    this->m_cLangPkg.add(L"de.mUi_UseMaster", std::wstring(L"Sende mit Master rechten:"));

    // mUi_NameProfTab
    this->m_cLangPkg.add(L"en.mUi_NameProfTab", std::wstring(L"%d. %s"));
    this->m_cLangPkg.add(L"de.mUi_NameProfTab", std::wstring(L"%d. %s"));

    // mUi_ToolProfTab
    this->m_cLangPkg.add(L"en.mUi_ToolProfTab", std::wstring(L"Show/change settings of profile \"%s\""));
    this->m_cLangPkg.add(L"de.mUi_ToolProfTab", std::wstring(L"Zeige/ändere Einstellungen für Profil \"%s\""));

    // mUi_gb_led
    this->m_cLangPkg.add(L"en.mUi_gb_led", std::wstring(L"LED keyboard backlight"));
    this->m_cLangPkg.add(L"de.mUi_gb_led", std::wstring(L"LED Tastatur Beleuchtung"));

    // mUi_pb_led_color
    this->m_cLangPkg.add(L"en.mUi_pb_led_color", std::wstring(L"Color ..."));
    this->m_cLangPkg.add(L"de.mUi_pb_led_color", std::wstring(L"Farbe ..."));

    // mUi_pb_led_test
    this->m_cLangPkg.add(L"en.mUi_pb_led_test", std::wstring(L"Test"));
    this->m_cLangPkg.add(L"de.mUi_pb_led_test", std::wstring(L"Test"));

    // mUi_cb_led_key
    this->m_cLangPkg.add(L"en.mUi_cb_led_key", std::wstring(L"No key selected"));
    this->m_cLangPkg.add(L"de.mUi_cb_led_key", std::wstring(L"Keine Taste"));

    // mUi_cb_led_type1
    this->m_cLangPkg.add(L"en.mUi_cb_led_type1", std::wstring(L"No LED back light"));
    this->m_cLangPkg.add(L"de.mUi_cb_led_type1", std::wstring(L"Keine LED-Beleuchtung"));

    // mUi_cb_led_type2
    this->m_cLangPkg.add(L"en.mUi_cb_led_type2", std::wstring(L"Logitech (LGS)"));
    this->m_cLangPkg.add(L"de.mUi_cb_led_type2", std::wstring(L"Logitech (LGS)"));

    // mUi_msg_NeedsRestart
    this->m_cLangPkg.add(L"en.mUi_msg_NeedsRestart", std::wstring(L"Some of the changed settings requier a restart of TeamSpeak3.\nPlease restart TeamSpeak3 now!"));
    this->m_cLangPkg.add(L"de.mUi_msg_NeedsRestart", std::wstring(L"Einige der geänderten Einstellung benötigen einen Neustart von TeamSpeak3 um endgültig zu wirken.\nBitte starten sie TeamSpeak3 daher jetzt neu!"));

    // mUi_msg_load_title
    this->m_cLangPkg.add(L"en.mUi_msg_load_title", std::wstring(L"Load configuration"));
    this->m_cLangPkg.add(L"de.mUi_msg_load_title", std::wstring(L"Lade Einstellungen"));

    // mUi_msg_load_text
    this->m_cLangPkg.add(L"en.mUi_msg_load_text", std::wstring(L"Select a configuration to load:"));
    this->m_cLangPkg.add(L"de.mUi_msg_load_text", std::wstring(L"Wähle die zu ladende Konfiguration:"));

    // mUi_msg_firstload_text
    this->m_cLangPkg.add(L"en.mUi_msg_firstload_text", std::wstring(L"You are starting the WhisperMaster2000 for the first time,\nplease select a a basic configuration to load:"));
    this->m_cLangPkg.add(L"de.mUi_msg_firstload_text", std::wstring(L"Du startest den WhisperMaster2000 das erste mal,\nbitte wähle eine Basiskonfiguration:"));

    // mUi_msg_load_nofile
    this->m_cLangPkg.add(L"en.mUi_msg_load_nofile", std::wstring(L"No file found to load configuration from."));
    this->m_cLangPkg.add(L"de.mUi_msg_load_nofile", std::wstring(L"Keine Datei gefunden um Einstellungen zu laden."));

    // mUi_msg_saveas_title
    this->m_cLangPkg.add(L"en.mUi_msg_saveas_title", std::wstring(L"Save configuration to file"));
    this->m_cLangPkg.add(L"de.mUi_msg_saveas_title", std::wstring(L"Speichere Einstellungen in Datei"));

    // mUi_msg_saveas_conf
    this->m_cLangPkg.add(L"en.mUi_msg_saveas_conf", std::wstring(L"Configuration (*.xml)"));
    this->m_cLangPkg.add(L"de.mUi_msg_saveas_conf", std::wstring(L"Konfiguration (*.xml)"));
}


/* ----------------------------------------------------------------------------
* set all strings needed by frequency UI (called by constructor)
*/
void language_pkg::init_ui_freq()
{
    // Freq config UI
    //-------------------------------------------------------------------------------------

    // fUi_fmTitle
    this->m_cLangPkg.add(L"en.fUi_fmTitle", std::wstring(L"Set frequency of profiles"));
    this->m_cLangPkg.add(L"de.fUi_fmTitle", std::wstring(L"Setze Frequenz der Profile"));

    // fUi_tabProfileDummy
    this->m_cLangPkg.add(L"en.fUi_tabProfileDummy", std::wstring(L"0. No \"Frequency\" profile found"));
    this->m_cLangPkg.add(L"de.fUi_tabProfileDummy", std::wstring(L"0. Keine \"Frequency\" Profile gefunden"));

    // fUi_WhatIsTabProfile
    this->m_cLangPkg.add(L"en.fUi_WhatIsTabProfile", std::wstring(L"Selects the <b>Profile</b> to change."));
    this->m_cLangPkg.add(L"de.fUi_WhatIsTabProfile", std::wstring(L"Selektiert das gerade aktive <b>Profil</b>."));

    // fUi_pbSetFreq
    this->m_cLangPkg.add(L"en.fUi_pbSetFreq", std::wstring(L"Set"));
    this->m_cLangPkg.add(L"de.fUi_pbSetFreq", std::wstring(L"Setze"));

    // fUi_WhatIsSetFreq
    this->m_cLangPkg.add(L"en.fUi_WhatIsSetFreq", std::wstring(L"Open a dialog to <b>Set</b> a specific frequency manually."));
    this->m_cLangPkg.add(L"de.fUi_WhatIsSetFreq", std::wstring(L"Öffne einen Dialog um manuell eine bestimte Frequenz zu <b>Setzen</b>"));

    // fUi_WhatIsDialFreq
    this->m_cLangPkg.add(L"en.fUi_WhatIsDialFreq", std::wstring(L"Changes the actual <b>Active frequency</b>."));
    this->m_cLangPkg.add(L"de.fUi_WhatIsDialFreq", std::wstring(L"Verändert die aktuell <b>Aktive Frequenz</b>."));

    // fUi_WhatIsLcdFreq
    this->m_cLangPkg.add(L"en.fUi_WhatIsLcdFreq", std::wstring(L"Shows the actual <b>Active frequency</b>."));
    this->m_cLangPkg.add(L"de.fUi_WhatIsLcdFreq", std::wstring(L"Zeigt die aktuell <b>Aktive Frequenz</b>."));

    // fUi_pbNextFreq
    this->m_cLangPkg.add(L"en.fUi_pbNextFreq", std::wstring(L"Next"));
    this->m_cLangPkg.add(L"de.fUi_pbNextFreq", std::wstring(L"Nächste"));

    // fUi_WhatIsNextFreq
    this->m_cLangPkg.add(L"en.fUi_WhatIsNextFreq", std::wstring(L"Automatically set the <b>Next</b> unused frequency."));
    this->m_cLangPkg.add(L"de.fUi_WhatIsNextFreq", std::wstring(L"Setze automatisch die <b>Nächste</b> unbenutzte Frequenz."));

    // fUi_gbFreq
    this->m_cLangPkg.add(L"en.fUi_gbFreq", std::wstring(L"Frequency"));
    this->m_cLangPkg.add(L"de.fUi_gbFreq", std::wstring(L"Frequenz"));

    // fUi_gbGeneral
    this->m_cLangPkg.add(L"en.fUi_gbGeneral", std::wstring(L"General"));
    this->m_cLangPkg.add(L"de.fUi_gbGeneral", std::wstring(L"Allgemein"));

    // fUi_lbPriority
    this->m_cLangPkg.add(L"en.fUi_lbPriority", std::wstring(L"Priority call"));
    this->m_cLangPkg.add(L"de.fUi_lbPriority", std::wstring(L"Priorisiert senden"));

    // fUi_WhatIsPriority
    this->m_cLangPkg.add(L"en.fUi_WhatIsPriority", std::wstring(L"<b>Priority call</b> can be used to talk to clients that have suppressed normal calls via <b>Squelch</b>-Mode."));
    this->m_cLangPkg.add(L"de.fUi_WhatIsPriority", std::wstring(L"<b>Priorisiertes senden</b> wird verwendet, um clients zu erreichen, die normale funksprüche per <b>Squelch</b>-Mode unterdrückthaben."));

    // fUi_WhatIsDialState
    this->m_cLangPkg.add(L"en.fUi_WhatIsDialState", std::wstring(L"<b>Off</b> disables the communication. In Mode-<b>Normal</b> you can hear all transmissions and talk to all active clients. In Mode-<b>Mute</b> all transmissions are muted, but you cann still talk to all active clients. In Mode-<b>Squelch</b> you can hear only priority calls, but you can talk to all active clients."));
    this->m_cLangPkg.add(L"de.fUi_WhatIsDialState", std::wstring(L"<b>Off</b> deaktiviert sämtliche Kommunikation. Im Modus-<b>Normal</b> kann man mit allen aktiven clients sprechen und diese hören. Im Modus-<b>Mute</b> werden keine Transmissionen empfangen, aber man kann weiterhin mit allen aktiven Clients sprechen. Im Modus-<b>Squelch</b> kann man nur Priorisierte Transmissionen empfangen, während man weiter mit allen aktiven Clients sprechen kann."));

    // fUi_FreqFull
    this->m_cLangPkg.add(L"en.fUi_FreqFull", std::wstring(L"All frequencies are currently in use."));
    //this->m_cLangPkg.add(L"de.fUi_FreqFull", std::wstring(L""));

    // fUi_FreqUsed
    this->m_cLangPkg.add(L"en.fUi_FreqUsed", std::wstring(L"Selected frequency is already in use by another profile."));
    //this->m_cLangPkg.add(L"de.textEv_FreqUsed", std::wstring(L""));
}

/* ----------------------------------------------------------------------------
* set all strings needed by clientTreeWidget UI (called by constructor)
*/
void language_pkg::init_ui_client_tree()
{
    // clientTreeWidget UI
    //-------------------------------------------------------------------------------------
    // ctUi_treeHeaderRx
    this->m_cLangPkg.add(L"en.ctUi_treeHeaderRx", std::wstring(L"Active clients"));
    this->m_cLangPkg.add(L"de.ctUi_treeHeaderRx", std::wstring(L"Aktive Clients"));

    // ctUi_treeHeaderState
    this->m_cLangPkg.add(L"en.ctUi_treeHeaderState", std::wstring(L"State"));
    this->m_cLangPkg.add(L"de.ctUi_treeHeaderState", std::wstring(L"Status"));

    // ctUi_WhatIsTreeItem
    this->m_cLangPkg.add(L"en.ctUi_WhatIsTreeItem", std::wstring(L"This list shows all clients that can be reached at this frequency. Under <b>State</b> you can find the actual Mode that is used by the client."));
    this->m_cLangPkg.add(L"de.ctUi_WhatIsTreeItem", std::wstring(L"In dieser Liste werden die gerade auf dieser Frequenz <b>Aktiven Clients</b> angezeigt. Unter <b>Status</b> wird der vom jeweiligen Client gerade verwendete Betriebsmodus angezeigt."));

    // ctUi_treeItemStateNone
    this->m_cLangPkg.add(L"en.ctUi_treeItemStateNone", std::wstring(L""));
    this->m_cLangPkg.add(L"de.ctUi_treeItemStateNone", std::wstring(L""));

    // ctUi_treeItemStateActive
    this->m_cLangPkg.add(L"en.ctUi_treeItemStateActive", std::wstring(L"Active"));
    this->m_cLangPkg.add(L"de.ctUi_treeItemStateActive", std::wstring(L"Aktiv"));

    // ctUi_treeItemStateMute
    this->m_cLangPkg.add(L"en.ctUi_treeItemStateMute", std::wstring(L"Muted"));
    this->m_cLangPkg.add(L"de.ctUi_treeItemStateMute", std::wstring(L"Muted"));

    // ctUi_treeItemStateSquelch
    this->m_cLangPkg.add(L"en.ctUi_treeItemStateSquelch", std::wstring(L"Squelch"));
    this->m_cLangPkg.add(L"de.ctUi_treeItemStateSquelch", std::wstring(L"Squelch"));

    // ctUi_treeItemStatePriority
    this->m_cLangPkg.add(L"en.ctUi_treeItemStatePriority", std::wstring(L"Priority"));
    this->m_cLangPkg.add(L"de.ctUi_treeItemStatePriority", std::wstring(L"Priorität"));

    // ctUi_treeItemStateError
    this->m_cLangPkg.add(L"en.ctUi_treeItemStateError", std::wstring(L"Error"));
    this->m_cLangPkg.add(L"de.ctUi_treeItemStateError", std::wstring(L"Fehler"));

    // ctUi_contextIgnore
    this->m_cLangPkg.add(L"en.ctUi_contextIgnore", std::wstring(L"Ignore"));
    this->m_cLangPkg.add(L"de.ctUi_contextIgnore", std::wstring(L"Ignorieren"));

    // ctUi_contextIgnoreTool
    this->m_cLangPkg.add(L"en.ctUi_contextIgnoreTool", std::wstring(L"Add selected channel to Ignore list"));
    this->m_cLangPkg.add(L"de.ctUi_contextIgnoreTool", std::wstring(L"Füge selektierten Channel zur Ignorieren Liste hinzu"));

    // ctUi_contextFav
    this->m_cLangPkg.add(L"en.ctUi_contextFav", std::wstring(L"Remove"));
    this->m_cLangPkg.add(L"de.ctUi_contextFav", std::wstring(L"Entfernen"));

    // ctUi_contextFavTool
    this->m_cLangPkg.add(L"en.ctUi_contextFavTool", std::wstring(L"Remove selected channel from Favorite/Ignore list"));
    this->m_cLangPkg.add(L"de.ctUi_contextFavTool", std::wstring(L"Entferne selektierten Channel von der Favoriten/Ignorieren Liste"));


    // ctUi_Level
    this->m_cLangPkg.add(L"en.ctUi_Level", std::wstring(L"Level "));
    this->m_cLangPkg.add(L"de.ctUi_Level", std::wstring(L"Level "));

    // ctUi_Ignore
    this->m_cLangPkg.add(L"en.ctUi_Ignore", std::wstring(L"Ignored"));
    this->m_cLangPkg.add(L"de.ctUi_Ignore", std::wstring(L"Ignoriert"));

    // ctUi_SubChannel
    this->m_cLangPkg.add(L"en.ctUi_SubChannel", std::wstring(L"Sub Channel"));
    this->m_cLangPkg.add(L"de.ctUi_SubChannel", std::wstring(L"Sub Channel"));

    // ctUi_Favorite
    this->m_cLangPkg.add(L"en.ctUi_Favorite", std::wstring(L"Favorite"));
    this->m_cLangPkg.add(L"de.ctUi_Favorite", std::wstring(L"Favorit"));

    // ctUi_NoChannel
    this->m_cLangPkg.add(L"en.ctUi_NoChannel", std::wstring(L"No Channel found"));
    this->m_cLangPkg.add(L"de.ctUi_NoChannel", std::wstring(L"Kein Channel gefunden"));
}

/* ----------------------------------------------------------------------------
* set all strings needed by about UI (called by constructor)
*/
void language_pkg::init_ui_about()
{
    // About UI
    //-------------------------------------------------------------------------------------

    // aUi_fmTitle
    this->m_cLangPkg.add(L"en.aUi_fmTitle", std::wstring(L"About WhisperMaster2000"));
    this->m_cLangPkg.add(L"de.aUi_fmTitle", std::wstring(L"Über WhisperMaster2000"));

    // aUi_Version
    this->m_cLangPkg.add(L"en.aUi_Version", std::wstring(L"Version:"));
    this->m_cLangPkg.add(L"de.aUi_Version", std::wstring(L"Version:"));

    // aUi_Author
    this->m_cLangPkg.add(L"en.aUi_Author", std::wstring(L"Author:"));
    this->m_cLangPkg.add(L"de.aUi_Author", std::wstring(L"Autor:"));

    // aUi_GnuError
    this->m_cLangPkg.add(L"en.aUi_GnuError", std::wstring(L"Something went wrong when trying to read \"./WhisperMaster2000/license_gpl_v3.txt\".\nPlease contact author for more information.\nOr download GNU GPL Version 3 from https://www.gnu.org/licenses/#GPL."));
    this->m_cLangPkg.add(L"de.aUi_GnuError", std::wstring(L"Etwas ist beim laden von \"./WhisperMaster2000/license_gpl_v3.txt\" schief gegangen.\nBitte kontaktieren sie den Autor für mehr informationen.\nOder laden sie die GNU GPL Version 3 von https://www.gnu.org/licenses/#GPL herrunter."));
}


/* ----------------------------------------------------------------------------
* set all strings needed by error handling routines (called by constructor)
*/
void language_pkg::init_error_handler()
{
    // general error messages
    //-------------------------------------------------------------------------------------

    // error_ErrQueryChName
    this->m_cLangPkg.add(L"en.error_ErrQueryChName", std::wstring(L"Error while querying channel name. [%s Error code : 0x%04X]"));
    //this->m_cLangPkg.add(L"de.error_ErrQueryChName", std::wstring(L"Unbekannter Fehler beim zurücksetzen der whisperliste aufgetretten."));

    // error_ErrCreateWhisper
    this->m_cLangPkg.add(L"en.error_ErrCreateWhisper", std::wstring(L"Unknown error while creating whisperlist. [%s Error code : 0x%04X]"));
    //this->m_cLangPkg.add(L"de.error_ErrCreateWhisper", std::wstring(L"Unbekannter Fehler beim zurücksetzen der whisperliste aufgetretten."));


    // Error handler
    //-------------------------------------------------------------------------------------

    // ErrUnknown
    this->m_cLangPkg.add(L"en.ErrUnknown", std::wstring(L"An unknown error was detected in function %s. Please restart client!"));
    //this->m_cLangPkg.add(L"de.ErrUnknown", std::wstring(L""));

    // ErrStdUnknown
    this->m_cLangPkg.add(L"en.ErrStdUnknown", std::wstring(L"An unknown std::exception was detected in function %s. Please restart client! (%s)"));
    //this->m_cLangPkg.add(L"de.ErrStdUnknown", std::wstring(L""));

    // ErrBoostUnknown
    this->m_cLangPkg.add(L"en.ErrBoostUnknown", std::wstring(L"An unknown boost::exception was detected in function %s. Please restart client! (%s)"));
    //this->m_cLangPkg.add(L"de.ErrBoostUnknown", std::wstring(L""));

    // ErrExtUnknown
    this->m_cLangPkg.add(L"en.ErrExtUnknown", std::wstring(L"An unknown error was detected in external function %s. Please restart client!"));
    //this->m_cLangPkg.add(L"de.ErrExtUnknown", std::wstring(L""));

    // ErrExtMessageBox
    this->m_cLangPkg.add(L"en.ErrExtMessageBox", std::wstring(L"There where errors on your last seasion with the WisperMaster2000. Please report the error messages to the developer."));
    //this->m_cLangPkg.add(L"de.ErrExtMessageBox", std::wstring(L""));
}


/* ----------------------------------------------------------------------------
* destructor
*/
language_pkg::~language_pkg()
{
}

/* ----------------------------------------------------------------------------
* set actual language
*/
void language_pkg::set_language(const char* sNewLanguage)
{
    set_language(std::string(sNewLanguage));
}

/* ----------------------------------------------------------------------------
* set actual language
*/
void language_pkg::set_language(std::string sNewLanguage)
{
    if (sNewLanguage.compare("de") == 0 || sNewLanguage.compare("german") == 0 || sNewLanguage.compare("deutsch") == 0)
    {
        this->m_wLanguage = L"de";
    }
    else
    {
        this->m_wLanguage = L"en";
    }

    return;
}


/* ----------------------------------------------------------------------------
* get string in selected language
*/
void language_pkg::translate(const char* sStringName, char* cStrBuffer, size_t nBuffSize)
{
    sprintf_s(cStrBuffer, (size_t)256, "%s", wtranslate(utf8_decode(std::string(sStringName))).c_str());
}

/* ----------------------------------------------------------------------------
* get string in selected language
*/
std::string language_pkg::translate(const char* sStringName)
{
    return wtranslate(utf8_decode(std::string(sStringName)));
}


/* ----------------------------------------------------------------------------
* get wstring in selected language
*/
std::string language_pkg::translate(const wchar_t* sStringName)
{
    return wtranslate(std::wstring(sStringName));
}

/* ----------------------------------------------------------------------------
* get wstring in selected language
*/
std::string language_pkg::wtranslate(std::wstring sStringName)
{
    //try to use user prefered language, but...
    std::wstring temp_lang = m_wLanguage;
    temp_lang.append(L".");
    temp_lang.append(sStringName);

    //...fall back language is always english
    std::wstring temp_default = L"en.";
    temp_default.append(sStringName);

    return utf8_encode(this->m_cLangPkg.get(temp_lang, this->m_cLangPkg.get(temp_default, temp_lang)));
}

/* ----------------------------------------------------------------------------
* Convert a wide Unicode string to an UTF8 string
*/
std::string language_pkg::utf8_encode(const std::wstring &wstr)
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

/* ----------------------------------------------------------------------------
* Convert an UTF8 string to a wide Unicode String
*/
std::wstring language_pkg::utf8_decode(const std::string &str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}