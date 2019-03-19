#include "wm2000_main_ui.h"

#define TRANSLATE(a) QString(this->m_cTranslate.translate(a).c_str())
#if USE_CALL_STACK
#define CALL_STACK if(USE_CALL_STACK) this->m_cErrHandler.message_callstack(__FUNCSIG__);
#else
#define CALL_STACK
#endif

/*
*   Constructor of UI
*/
wm2000_main_ui::wm2000_main_ui()
{
    CALL_STACK
    this->m_pAboutUi     = nullptr;
    this->m_pFreqUi      = nullptr;
    this->m_pcConfigData = nullptr;
}

/*
*   Constructor of UI
*/
wm2000_main_ui::wm2000_main_ui(config_container *pcConfigData, void(*fUpdate_server)(), std::string sConfigPath, QWidget *parent)
    : QDialog(parent)
{
    CALL_STACK
    //prepare action icons
    this->m_sConfigPath = sConfigPath;
    this->m_pcConfigData = pcConfigData;
    this->m_fUpdate_server = fUpdate_server;
    this->m_cWindowIcon.addFile(QString::fromStdString(sConfigPath + std::string("phone.png")), QSize(), QIcon::Normal, QIcon::Off);
    this->m_cLoadIcon.addFile(QString::fromStdString(sConfigPath + std::string("load.png")), QSize(), QIcon::Normal, QIcon::Off);
    this->m_cSaveIcon.addFile(QString::fromStdString(sConfigPath + std::string("save.png")), QSize(), QIcon::Normal, QIcon::Off);
    this->m_cUndoIcon.addFile(QString::fromStdString(sConfigPath + std::string("undo.png")), QSize(), QIcon::Normal, QIcon::Off);
    this->m_cAboutIcon.addFile(QString::fromStdString(sConfigPath + std::string("about.png")), QSize(), QIcon::Normal, QIcon::Off);
    this->m_cIgnoreIcon.addFile(QString::fromStdString(sConfigPath + std::string("cross.png")), QSize(), QIcon::Normal, QIcon::Off);
    this->m_cFavIcon.addFile(QString::fromStdString(sConfigPath + std::string("create.png")), QSize(), QIcon::Normal, QIcon::Off);
    this->setWindowIcon(this->m_cWindowIcon);

    // create ui with basic settings
    m_cUi.setupUi(this);
}


/*
*   init ui
*     will be called by derived class
*/
void wm2000_main_ui::init_ui(QWidget *parent)
{
    CALL_STACK
    this->m_cLocalConfigData = *this->m_pcConfigData;
    this->m_cTranslate.set_language(this->m_cLocalConfigData.s_get_Language());

    this->m_cUi.treeRxList->init(&m_cLocalConfigData, &m_cIgnoreIcon, &m_cFavIcon);
    m_cUi.tab_group->tabBar()->setStyle(&m_cTabStyle);
    
    //remove all mockup tabs, before creating the new ones
    this->m_cUi.tab_group->removeTab(this->m_cUi.tab_group->indexOf(this->m_cUi.tab_general));
    this->m_cUi.tab_group->removeTab(this->m_cUi.tab_group->indexOf(this->m_cUi.tab_profile));

    // add specific (generic) ui elements and settings
    create_menu();
    create_general_tab();
    for (int ii = 0; ii < REAL_MAXNUMPROFILES; ii++)
        create_profile_tab(ii);
    add_tabs_to_group();
    update_expert_mode();

    //set actual configuration
    update_box_size();

    //create sub UIs
    this->m_pAboutUi = new wm2000_about_ui(this->m_sConfigPath, &m_cWindowIcon, parent);
    this->m_pFreqUi  = new wm2000_freq_ui(this->m_pcConfigData, &m_cWindowIcon, &m_cIgnoreIcon, &m_cFavIcon, parent);

    //set actual configuration
    this->m_pAboutUi->set_language(this->m_pcConfigData->s_get_Language());
    return;
}


/*
*   destructor of UI
*/
wm2000_main_ui::~wm2000_main_ui()
{
    CALL_STACK
    try
    {
        this->m_pAboutUi->close();
        delete this->m_pAboutUi;
        this->m_pAboutUi = nullptr;

        this->m_pFreqUi->close();
        delete this->m_pFreqUi;
        this->m_pFreqUi = nullptr;
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


/*
*   event handler
*   Open about UI
*/
void wm2000_main_ui::open_about_ui()
{
    CALL_STACK
    try
    {
        if (this->m_pAboutUi != nullptr)
        {
            if (this->m_pAboutUi->isVisible())
            {
                // Window is already displayed somewhere, bring it to the top and give it focus
                this->m_pAboutUi->raise();
                this->m_pAboutUi->activateWindow();
            }
            else
            {
                // Display window
                this->m_pAboutUi->show();
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


/*
*   event handler
*   open frequency UI
*/
void wm2000_main_ui::open_freq_ui()
{
    CALL_STACK
    try
    {
        if (this->m_pFreqUi != nullptr)
        {
            if (this->m_pFreqUi->isVisible())
            {
                // Window is already displayed somewhere, bring it to the top and give it focus
                this->m_pFreqUi->raise();
                this->m_pFreqUi->activateWindow();
            }
            else
            {
                // Display window
                this->m_pFreqUi->show();
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


/*
*   init function for general tab
*/
void wm2000_main_ui::create_menu(bool bTransOnly)
{
    CALL_STACK
    this->setWindowTitle(TRANSLATE(L"mUi_fmTitle"));
    if (!bTransOnly) this->setFixedSize(size());
    this->m_cUi.pb_ok->setText(TRANSLATE(L"gUi_pbOk"));
    this->m_cUi.pb_apply->setText(TRANSLATE(L"gUi_pbApply"));
    this->m_cUi.pb_cancel->setText(TRANSLATE(L"gUi_pbCancel"));

    this->m_cUi.menu_file->setTitle(TRANSLATE(L"mUi_MenuTitle"));
    if (!bTransOnly) this->m_pcLoadAction = new QAction(this);
    if (!bTransOnly) this->m_pcLoadAction->setObjectName(QStringLiteral("menu_action_save"));
    this->m_pcLoadAction->setText(TRANSLATE(L"mUi_MenuLoad"));
    this->m_pcLoadAction->setIconText(TRANSLATE(L"mUi_MenuLoad"));
    this->m_pcLoadAction->setToolTip(TRANSLATE(L"mUi_ToolMenuLoad"));
    if (!bTransOnly) this->m_pcLoadAction->setIcon(this->m_cLoadIcon);
    if (!bTransOnly) this->m_pcSaveAction = new QAction(this);
    if (!bTransOnly) this->m_pcSaveAction->setObjectName(QStringLiteral("menu_action_save"));
    this->m_pcSaveAction->setText(TRANSLATE(L"mUi_MenuSave"));
    this->m_pcSaveAction->setIconText(TRANSLATE(L"mUi_MenuSave"));
    this->m_pcSaveAction->setToolTip(TRANSLATE(L"mUi_ToolMenuSave"));
    if (!bTransOnly) this->m_pcSaveAction->setIcon(this->m_cSaveIcon);
    if (!bTransOnly) this->m_pcSaveAsAction = new QAction(this);
    if (!bTransOnly) this->m_pcSaveAsAction->setObjectName(QStringLiteral("menu_action_saveas"));
    this->m_pcSaveAsAction->setText(TRANSLATE(L"mUi_MenuSaveAs"));
    this->m_pcSaveAsAction->setIconText(TRANSLATE(L"mUi_MenuSaveAs"));
    this->m_pcSaveAsAction->setToolTip(TRANSLATE(L"mUi_ToolMenuSave"));
    if (!bTransOnly) this->m_pcSaveAsAction->setIcon(this->m_cSaveIcon);
    if (!bTransOnly) this->m_pcCloseAction = new QAction(this);
    if (!bTransOnly) this->m_pcCloseAction->setObjectName(QStringLiteral("menu_action_close"));
    this->m_pcCloseAction->setText(TRANSLATE(L"mUi_MenuClose"));
    this->m_pcCloseAction->setIconText(TRANSLATE(L"mUi_MenuClose"));
    this->m_pcCloseAction->setToolTip(TRANSLATE(L"mUi_ToolMenuClose"));
    if (!bTransOnly) this->m_cUi.menu_file->addAction(this->m_pcLoadAction);
    if (!bTransOnly) this->m_cUi.menu_file->addAction(this->m_pcSaveAction);
    if (!bTransOnly) this->m_cUi.menu_file->addAction(this->m_pcSaveAsAction);
    if (!bTransOnly) this->m_cUi.menu_file->addSeparator();
    if (!bTransOnly) this->m_cUi.menu_file->addAction(this->m_pcCloseAction);

    this->m_cUi.menu_view->setTitle(TRANSLATE(L"mUi_MenuViewTitle"));
    if (!bTransOnly) this->m_pcExpertAction = new QAction(this);
    if (!bTransOnly) this->m_pcExpertAction->setCheckable(true);
    this->m_pcExpertAction->setChecked(this->m_pcConfigData->s_get_ExpertMode());
    this->m_pcExpertAction->setText(TRANSLATE(L"mUi_MenuExpert"));
    this->m_pcExpertAction->setIconText(TRANSLATE(L"mUi_MenuExpert"));
    this->m_pcExpertAction->setToolTip(TRANSLATE(L"mUi_ToolMenuExpert"));
    if (!bTransOnly) this->m_pcFreqAction = new QAction(this);
    this->m_pcFreqAction->setText(TRANSLATE(L"mUi_MenuOpenFreq"));
    this->m_pcFreqAction->setIconText(TRANSLATE(L"mUi_MenuOpenFreq"));
    this->m_pcFreqAction->setToolTip(TRANSLATE(L"mUi_ToolMenuOpenFreq"));
    if (!bTransOnly) this->m_cUi.menu_view->addAction(this->m_pcExpertAction);
    if (!bTransOnly) this->m_cUi.menu_view->addSeparator();
    if (!bTransOnly) this->m_cUi.menu_view->addAction(this->m_pcFreqAction);

    this->m_cUi.menu_edit->setTitle(TRANSLATE(L"mUi_MenuEditTitle"));
    if (!bTransOnly) this->m_pcUndoAction = new QAction(this);
    this->m_pcUndoAction->setText(TRANSLATE(L"mUi_MenuUndo"));
    this->m_pcUndoAction->setIconText(TRANSLATE(L"mUi_MenuUndo"));
    this->m_pcUndoAction->setToolTip(TRANSLATE(L"mUi_ToolMenuUndo"));
    if (!bTransOnly) this->m_pcUndoAction->setIcon(this->m_cUndoIcon);
    if (!bTransOnly) this->m_cUi.menu_edit->addAction(this->m_pcUndoAction);

    this->m_cUi.menu_help->setTitle(TRANSLATE(L"mUi_MenuHelpTitle"));
    if (!bTransOnly) this->m_pcAboutAction = new QAction(this);
    this->m_pcAboutAction->setText(TRANSLATE(L"mUi_MenuAbout"));
    this->m_pcAboutAction->setIconText(TRANSLATE(L"mUi_MenuAbout"));
    this->m_pcAboutAction->setToolTip(TRANSLATE(L"mUi_MenuAbout"));
    if (!bTransOnly) this->m_pcAboutAction->setIcon(this->m_cAboutIcon);
    if (!bTransOnly) this->m_cUi.menu_help->addAction(this->m_pcAboutAction);

    //initialize all elements
    this->m_cUi.pb_apply->setEnabled(false);
    this->m_pcSaveAction->setEnabled(false);
    this->m_pcUndoAction->setEnabled(false);
    return;
}

/*
*   init function for general tab
*/
void wm2000_main_ui::create_general_tab(bool bTransOnly)
{
    CALL_STACK
    // create General Tab
    if (!bTransOnly) this->m_pcTabGeneral = new QWidget();
    if (!bTransOnly) this->m_pcTabGeneral->setObjectName(QStringLiteral("tab_general"));

    // add group-box for General settings
    if (!bTransOnly) this->m_pcGroupGeneral = new QGroupBox(this->m_pcTabGeneral);
    if (!bTransOnly) this->m_pcGroupGeneral->setObjectName(QStringLiteral("gb_general_general"));
    if (!bTransOnly) this->m_pcGroupGeneral->setGeometry(this->m_cUi.gb_general_general->geometry());
    this->m_pcGroupGeneral->setTitle(TRANSLATE("mUi_TabGeneral"));

    if (!bTransOnly) this->m_pcSpinNumProfile = new QSpinBox(this->m_pcGroupGeneral);
    if (!bTransOnly) this->m_pcSpinNumProfile->setObjectName(QStringLiteral("spin_general_num_profile"));
    if (!bTransOnly) this->m_pcSpinNumProfile->setGeometry(this->m_cUi.spin_general_num_profile->geometry());
    if (!bTransOnly) this->m_pcSpinNumProfile->setMinimum(1);
    if (!bTransOnly) this->m_pcSpinNumProfile->setMaximum(REAL_MAXNUMPROFILES);
    this->m_pcSpinNumProfile->setValue((int)this->m_pcConfigData->s_get_MaxNumProfiles());

    if (!bTransOnly) this->m_pcLabNumProfile = new QLabel(this->m_pcGroupGeneral);
    if (!bTransOnly) this->m_pcLabNumProfile->setObjectName(QStringLiteral("lb_general_num_profile"));
    if (!bTransOnly) this->m_pcLabNumProfile->setGeometry(this->m_cUi.lb_general_num_profile->geometry());
    this->m_pcLabNumProfile->setText(TRANSLATE("mUi_NumProfile"));

    if (!bTransOnly) this->m_pcCombLanguage = new QComboBox(this->m_pcGroupGeneral);
    if (!bTransOnly) this->m_pcCombLanguage->setObjectName(QStringLiteral("cb_general_language"));
    if (!bTransOnly) this->m_pcCombLanguage->setGeometry(this->m_cUi.cb_general_language->geometry());
    if (!bTransOnly)
    {
        this->m_pcCombLanguage->insertItems(0, QStringList()
            << QStringLiteral("0")
            << QStringLiteral("1"));
    }
    this->m_pcCombLanguage->setItemText(0, TRANSLATE("mUi_LanguageDe"));
    this->m_pcCombLanguage->setItemText(1, TRANSLATE("mUi_LanguageEn"));
    if (this->m_pcConfigData->s_get_Language().compare("german") == 0)
        this->m_pcCombLanguage->setCurrentIndex(0); //german
    else
        this->m_pcCombLanguage->setCurrentIndex(1); //english

    if (!bTransOnly) this->m_pcLabLanguage = new QLabel(this->m_pcGroupGeneral);
    if (!bTransOnly) this->m_pcLabLanguage->setObjectName(QStringLiteral("lb_general_language"));
    if (!bTransOnly) this->m_pcLabLanguage->setGeometry(this->m_cUi.lb_general_language->geometry());
    this->m_pcLabLanguage->setText(TRANSLATE("mUi_Language"));

    if (!bTransOnly) this->m_pcEdtHotkey_reset = new QLineEdit(this->m_pcGroupGeneral);
    if (!bTransOnly) this->m_pcEdtHotkey_reset->setObjectName(QStringLiteral("ed_generel_key"));
    if (!bTransOnly) this->m_pcEdtHotkey_reset->setEnabled(false);
    if (!bTransOnly) this->m_pcEdtHotkey_reset->setGeometry(this->m_cUi.ed_generel_key->geometry());
    this->m_pcEdtHotkey_reset->setPlaceholderText(TRANSLATE("mUi_HotKeyDefault"));
    this->m_pcEdtHotkey_reset->setText(QString::fromStdString(this->m_pcConfigData->s_get_GenHotKey_reset()));

    if (!bTransOnly) this->m_pcLabHotkey_reset = new QLabel(this->m_pcGroupGeneral);
    if (!bTransOnly) this->m_pcLabHotkey_reset->setObjectName(QStringLiteral("lb_general_key"));
    if (!bTransOnly) this->m_pcLabHotkey_reset->setGeometry(this->m_cUi.lb_general_key->geometry());
    this->m_pcLabHotkey_reset->setText(TRANSLATE("mUi_HotKeyReset"));

    if (!bTransOnly) this->m_pcEdtHotkey_mute = new QLineEdit(this->m_pcGroupGeneral);
    if (!bTransOnly) this->m_pcEdtHotkey_mute->setObjectName(QStringLiteral("ed_generel_key_2"));
    if (!bTransOnly) this->m_pcEdtHotkey_mute->setEnabled(false);
    if (!bTransOnly) this->m_pcEdtHotkey_mute->setGeometry(this->m_cUi.ed_generel_key_2->geometry());
    this->m_pcEdtHotkey_mute->setPlaceholderText(TRANSLATE("mUi_HotKeyDefault"));
    this->m_pcEdtHotkey_mute->setText(QString::fromStdString(this->m_pcConfigData->s_get_GenHotKey_mute()));

    if (!bTransOnly) this->m_pcLabHotkey_mute = new QLabel(this->m_pcGroupGeneral);
    if (!bTransOnly) this->m_pcLabHotkey_mute->setObjectName(QStringLiteral("lb_general_key_2"));
    if (!bTransOnly) this->m_pcLabHotkey_mute->setGeometry(this->m_cUi.lb_general_key_2->geometry());
    this->m_pcLabHotkey_mute->setText(TRANSLATE("mUi_HotKeyMute"));

    // add group-box for Ignore settings
    if (!bTransOnly) this->m_pcGroupIgnore = new QGroupBox(this->m_pcTabGeneral);
    if (!bTransOnly) this->m_pcGroupIgnore->setObjectName(QStringLiteral("gb_general_ignore"));
    if (!bTransOnly) this->m_pcGroupIgnore->setGeometry(this->m_cUi.gb_general_ignore->geometry());
    this->m_pcGroupIgnore->setTitle(TRANSLATE("mUi_Ignore"));

    if (!bTransOnly) this->m_pcPbDelIgnore = new QPushButton(this->m_pcGroupIgnore);
    if (!bTransOnly) this->m_pcPbDelIgnore->setObjectName(QStringLiteral("pb_general_delete"));
    if (!bTransOnly) this->m_pcPbDelIgnore->setGeometry(this->m_cUi.pb_general_delete->geometry());
    this->m_pcPbDelIgnore->setText(TRANSLATE("mUi_DelIgnore"));

    if (!bTransOnly) this->m_pcSwIgnoreUseRx = new SwitchButton(this->m_pcGroupIgnore);
    if (!bTransOnly) this->m_pcSwIgnoreUseRx->setObjectName(QStringLiteral("sw_general_use_rx"));
    if (!bTransOnly) this->m_pcSwIgnoreUseRx->setGeometry(this->m_cUi.sw_general_use_rx->geometry());
    this->m_pcSwIgnoreUseRx->setValue(this->m_pcConfigData->s_get_UseIgnoreListRx());

    if (!bTransOnly) this->m_pcLabIgnoreUseRx = new QLabel(this->m_pcGroupIgnore);
    if (!bTransOnly) this->m_pcLabIgnoreUseRx->setObjectName(QStringLiteral("lb_general_use_rx"));
    if (!bTransOnly) this->m_pcLabIgnoreUseRx->setGeometry(this->m_cUi.lb_general_use_rx->geometry());
    this->m_pcLabIgnoreUseRx->setText(TRANSLATE("mUi_UseForRx"));

    if (!bTransOnly) this->m_pcSwAutoSave = new SwitchButton(this->m_pcGroupIgnore);
    if (!bTransOnly) this->m_pcSwAutoSave->setObjectName(QStringLiteral("sw_general__auto_save"));
    if (!bTransOnly) this->m_pcSwAutoSave->setGeometry(this->m_cUi.sw_general__auto_save->geometry());
    this->m_pcSwAutoSave->setValue(this->m_pcConfigData->s_get_SaveIgnoreList());

    if (!bTransOnly) this->m_pcLabAutoSave = new QLabel(this->m_pcGroupIgnore);
    if (!bTransOnly) this->m_pcLabAutoSave->setObjectName(QStringLiteral("lb_general_auto_save"));
    if (!bTransOnly) this->m_pcLabAutoSave->setGeometry(this->m_cUi.lb_general_auto_save->geometry());
    this->m_pcLabAutoSave->setText(TRANSLATE("mUi_AutoSave"));

    // add group-box for LED background lighting
    create_led_group(this->m_pcTabGeneral, this->m_pcGroupLED, this->m_pcPbLedColor, this->m_pcPbLedTest, this->m_pcCbKey, this->m_pcCbType, this->m_pcLbColor, bTransOnly);
    return;
}


/*
*   init function for profile tabs
*/
void wm2000_main_ui::create_profile_tab(int nIndex, bool bTransOnly)
{
    CALL_STACK

    //create tab
    if (!bTransOnly) this->m_acTabProfile[nIndex] = new QWidget();
    if (!bTransOnly) this->m_acTabProfile[nIndex]->setObjectName(QString("tab_profile_") + QString::number(nIndex));

    //create general settings
    if (!bTransOnly) this->m_acGroupGeneral[nIndex] = new QGroupBox(this->m_acTabProfile[nIndex]);
    if (!bTransOnly) this->m_acGroupGeneral[nIndex]->setObjectName(QString("gb_profile_general_") + QString::number(nIndex));
    if (!bTransOnly) this->m_acGroupGeneral[nIndex]->setGeometry(this->m_cUi.gb_profile_general->geometry());
    this->m_acGroupGeneral[nIndex]->setTitle(TRANSLATE("mUi_TabGeneral"));

    if (!bTransOnly) this->m_edProfileName[nIndex] = new QLineEdit(this->m_acGroupGeneral[nIndex]);
    if (!bTransOnly) this->m_edProfileName[nIndex]->setObjectName(QString("ed_profile_name_") + QString::number(nIndex));
    if (!bTransOnly) this->m_edProfileName[nIndex]->setGeometry(this->m_cUi.ed_profile_name->geometry());
    if (!bTransOnly) this->m_edProfileName[nIndex]->setMaxLength(15);
    this->m_edProfileName[nIndex]->setPlaceholderText(TRANSLATE("mUi_ProfileName"));
    if (this->m_pcConfigData->get_DefaultProfileName(nIndex).compare(this->m_pcConfigData->s_get_ProfileName(nIndex)) != 0)
        this->m_edProfileName[nIndex]->setText(QString::fromStdString(this->m_pcConfigData->s_get_ProfileName(nIndex)));

    if (!bTransOnly) this->m_lbProfileType[nIndex] = new QLabel(this->m_acGroupGeneral[nIndex]);
    if (!bTransOnly) this->m_lbProfileType[nIndex]->setObjectName(QString("lb_profile_type_") + QString::number(nIndex));
    if (!bTransOnly) this->m_lbProfileType[nIndex]->setGeometry(this->m_cUi.lb_profile_type->geometry());
    this->m_lbProfileType[nIndex]->setText(TRANSLATE("mUi_ProfileType"));

    if (!bTransOnly) this->m_cbProfileType[nIndex] = new QComboBox(this->m_acGroupGeneral[nIndex]);
    if (!bTransOnly) this->m_cbProfileType[nIndex]->setObjectName(QString("cb_profile_type_") + QString::number(nIndex));
    if (!bTransOnly) this->m_cbProfileType[nIndex]->setGeometry(this->m_cUi.cb_profile_type->geometry());
    if (!bTransOnly)
    {
        this->m_cbProfileType[nIndex]->insertItems(0, QStringList()
            << QStringLiteral("0")
            << QStringLiteral("1")
            << QStringLiteral("2")
            << QStringLiteral("3"));
    }
    this->m_cbProfileType[nIndex]->setItemText(0, TRANSLATE("mUi_ProfileOff"));
    this->m_cbProfileType[nIndex]->setItemText(1, TRANSLATE("mUi_ProfileLevel"));
    this->m_cbProfileType[nIndex]->setItemText(2, TRANSLATE("mUi_ProfileFav"));
    this->m_cbProfileType[nIndex]->setItemText(3, TRANSLATE("mUi_ProfileFreq"));
    this->m_cbProfileType[nIndex]->setCurrentIndex( (this->m_pcConfigData->s_get_ProfileType(nIndex) < PROFILE_AUDIO) ? this->m_pcConfigData->s_get_ProfileType(nIndex) : PROFILE_OFF);
    bool bProfileActive = (this->m_pcConfigData->s_get_ProfileType(nIndex) != PROFILE_OFF);

    if (!bTransOnly) this->m_lbProfileIgnoreTx[nIndex] = new QLabel(this->m_acGroupGeneral[nIndex]);
    if (!bTransOnly) this->m_lbProfileIgnoreTx[nIndex]->setObjectName(QString("lb_profile_ignore_tx_") + QString::number(nIndex));
    if (!bTransOnly) this->m_lbProfileIgnoreTx[nIndex]->setGeometry(this->m_cUi.lb_profile_ignore_tx->geometry());
    this->m_lbProfileIgnoreTx[nIndex]->setText(TRANSLATE("mUi_UseIgnoreTx"));
    this->m_lbProfileIgnoreTx[nIndex]->setEnabled(bProfileActive);

    if (!bTransOnly) this->m_swProfileIgnoreTx[nIndex] = new SwitchButton(this->m_acGroupGeneral[nIndex]);
    if (!bTransOnly) this->m_swProfileIgnoreTx[nIndex]->setObjectName(QString("sw_profile_ignore_tx_") + QString::number(nIndex));
    if (!bTransOnly) this->m_swProfileIgnoreTx[nIndex]->setGeometry(this->m_cUi.sw_profile_ignore_tx->geometry());
    this->m_swProfileIgnoreTx[nIndex]->setEnabled(bProfileActive);
    this->m_swProfileIgnoreTx[nIndex]->setValue(this->m_pcConfigData->s_get_UseIgnoreListTx(nIndex));

    if (!bTransOnly) this->m_lbProfilePtt[nIndex] = new QLabel(this->m_acGroupGeneral[nIndex]);
    if (!bTransOnly) this->m_lbProfilePtt[nIndex]->setObjectName(QString("lb_profile_ptt_") + QString::number(nIndex));
    if (!bTransOnly) this->m_lbProfilePtt[nIndex]->setGeometry(this->m_cUi.lb_profile_ptt->geometry());
    this->m_lbProfilePtt[nIndex]->setText(TRANSLATE("mUi_AutoPtt"));
    this->m_lbProfilePtt[nIndex]->setEnabled(bProfileActive);

    if (!bTransOnly) this->m_swProfilePtt[nIndex] = new SwitchButton(this->m_acGroupGeneral[nIndex]);
    if (!bTransOnly) this->m_swProfilePtt[nIndex]->setObjectName(QString("sw_profile_ptt_") + QString::number(nIndex));
    if (!bTransOnly) this->m_swProfilePtt[nIndex]->setGeometry(this->m_cUi.sw_profile_ptt->geometry());
    this->m_swProfilePtt[nIndex]->setEnabled(bProfileActive);
    this->m_swProfilePtt[nIndex]->setValue(this->m_pcConfigData->s_get_AutoActivate(nIndex));

    if (!bTransOnly) this->m_lbProfileKeyDw[nIndex] = new QLabel(this->m_acGroupGeneral[nIndex]);
    if (!bTransOnly) this->m_lbProfileKeyDw[nIndex]->setObjectName(QString("lb_profile_key_") + QString::number(nIndex));
    if (!bTransOnly) this->m_lbProfileKeyDw[nIndex]->setGeometry(this->m_cUi.lb_profile_key->geometry());
    this->m_lbProfileKeyDw[nIndex]->setText(TRANSLATE("mUi_HotKeyDown"));
    this->m_lbProfileKeyDw[nIndex]->setEnabled(bProfileActive);

    if (!bTransOnly) this->m_edProfileKeyDw[nIndex] = new QLineEdit(this->m_acGroupGeneral[nIndex]);
    if (!bTransOnly) this->m_edProfileKeyDw[nIndex]->setObjectName(QString("ed_profile_key_") + QString::number(nIndex));
    if (!bTransOnly) this->m_edProfileKeyDw[nIndex]->setEnabled(false);
    if (!bTransOnly) this->m_edProfileKeyDw[nIndex]->setGeometry(this->m_cUi.ed_profile_key->geometry());
    this->m_edProfileKeyDw[nIndex]->setPlaceholderText(TRANSLATE("mUi_HotKeyDefault"));
    this->m_edProfileKeyDw[nIndex]->setEnabled(false);
    this->m_edProfileKeyDw[nIndex]->setText(QString::fromStdString( this->m_pcConfigData->s_get_HotKey_down(nIndex)));

    if (!bTransOnly) this->m_lbProfileKeyUp[nIndex] = new QLabel(this->m_acGroupGeneral[nIndex]);
    if (!bTransOnly) this->m_lbProfileKeyUp[nIndex]->setObjectName(QString("lb_profile_key_2_") + QString::number(nIndex));
    if (!bTransOnly) this->m_lbProfileKeyUp[nIndex]->setGeometry(this->m_cUi.lb_profile_key_2->geometry());
    this->m_lbProfileKeyUp[nIndex]->setText(TRANSLATE("mUi_HotKeyUp"));
    this->m_lbProfileKeyUp[nIndex]->setEnabled(bProfileActive);
    this->m_lbProfileKeyUp[nIndex]->setVisible(this->m_pcConfigData->s_get_AutoActivate(nIndex)); // active if we are in PTT mode

    if (!bTransOnly) this->m_edProfileKeyUp[nIndex] = new QLineEdit(this->m_acGroupGeneral[nIndex]);
    if (!bTransOnly) this->m_edProfileKeyUp[nIndex]->setObjectName(QString("ed_profile_key_2_") + QString::number(nIndex));
    if (!bTransOnly) this->m_edProfileKeyUp[nIndex]->setEnabled(false);
    if (!bTransOnly) this->m_edProfileKeyUp[nIndex]->setGeometry(this->m_cUi.ed_profile_key_2->geometry());
    this->m_edProfileKeyUp[nIndex]->setPlaceholderText(TRANSLATE("mUi_HotKeyDefault"));
    this->m_edProfileKeyUp[nIndex]->setEnabled(false);
    this->m_edProfileKeyUp[nIndex]->setVisible(this->m_pcConfigData->s_get_AutoActivate(nIndex)); // active if we are in PTT mode
    this->m_edProfileKeyUp[nIndex]->setText(QString::fromStdString(this->m_pcConfigData->s_get_HotKey_up(nIndex)));

    //create profile type specific settings for...
    if (!bTransOnly) this->m_acStackedTyp[nIndex] = new QStackedWidget(this->m_acTabProfile[nIndex]);
    if (!bTransOnly) this->m_acStackedTyp[nIndex]->setObjectName(QString("stack_profile_tab") + QString::number(nIndex));
    if (!bTransOnly) this->m_acStackedTyp[nIndex]->setGeometry(this->m_cUi.stack_profile_tab->geometry());

    // no type selected
    if (!bTransOnly) this->m_widgetNoProfile[nIndex] = new QWidget();
    if (!bTransOnly) this->m_widgetNoProfile[nIndex]->setGeometry(QRect(10, 210, 511, 180));
    if (!bTransOnly) this->m_widgetNoProfile[nIndex]->setObjectName(QString("page_profile_empty") + QString::number(nIndex));

    if (!bTransOnly) this->m_acGroupNoProfile[nIndex] = new QGroupBox(this->m_widgetNoProfile[nIndex]);
    if (!bTransOnly) this->m_acGroupNoProfile[nIndex]->setObjectName(QString("gb_profile_empty") + QString::number(nIndex));
    if (!bTransOnly) this->m_acGroupNoProfile[nIndex]->setGeometry(this->m_cUi.gb_profile_empty->geometry());
    this->m_acGroupNoProfile[nIndex]->setTitle(TRANSLATE("mUi_ProfileOffGb"));

    if (!bTransOnly) this->m_acStackedTyp[nIndex]->addWidget(this->m_widgetNoProfile[nIndex]);

    // ...Level is selected
    if (!bTransOnly) this->m_widgetLevel[nIndex] = new QWidget();
    if (!bTransOnly) this->m_widgetLevel[nIndex]->setGeometry(QRect(10, 210, 511, 180));
    if (!bTransOnly) this->m_widgetLevel[nIndex]->setObjectName(QString("page_profile_level") + QString::number(nIndex));

    if (!bTransOnly) this->m_acGroupLevel[nIndex] = new QGroupBox(this->m_widgetLevel[nIndex]);
    if (!bTransOnly) this->m_acGroupLevel[nIndex]->setObjectName(QString("gb_profile_lev") + QString::number(nIndex));
    if (!bTransOnly) this->m_acGroupLevel[nIndex]->setGeometry(this->m_cUi.gb_profile_empty->geometry());
    this->m_acGroupLevel[nIndex]->setTitle(TRANSLATE("mUi_ProfileLevelGb"));

    if (!bTransOnly) this->m_lbLevLower[nIndex] = new QLabel(this->m_acGroupLevel[nIndex]);
    if (!bTransOnly) this->m_lbLevLower[nIndex]->setObjectName(QString("lb_profile_lev_lower") + QString::number(nIndex));
    if (!bTransOnly) this->m_lbLevLower[nIndex]->setGeometry(this->m_cUi.lb_profile_lev_lower->geometry());
    this->m_lbLevLower[nIndex]->setText(TRANSLATE("mUi_LowerLevel"));
    this->m_lbLevLower[nIndex]->setEnabled(bProfileActive);

    if (!bTransOnly) this->m_spLevLower[nIndex] = new QSpinBox(this->m_acGroupLevel[nIndex]);
    if (!bTransOnly) this->m_spLevLower[nIndex]->setObjectName(QString("spin_profile_lev_lower") + QString::number(nIndex));
    if (!bTransOnly) this->m_spLevLower[nIndex]->setGeometry(this->m_cUi.spin_profile_lev_lower->geometry());
    if (!bTransOnly) this->m_spLevLower[nIndex]->setMinimum(1);
    this->m_spLevLower[nIndex]->setEnabled(bProfileActive);
    this->m_spLevLower[nIndex]->setValue((int)this->m_pcConfigData->s_get_MinChLevel(nIndex));

    if (!bTransOnly) this->m_lbLevUpper[nIndex] = new QLabel(this->m_acGroupLevel[nIndex]);
    if (!bTransOnly) this->m_lbLevUpper[nIndex]->setObjectName(QString("lb_profile_lev_upper") + QString::number(nIndex));
    if (!bTransOnly) this->m_lbLevUpper[nIndex]->setGeometry(this->m_cUi.lb_profile_lev_upper->geometry());
    this->m_lbLevUpper[nIndex]->setText(TRANSLATE("mUi_UpperLevel"));
    this->m_lbLevUpper[nIndex]->setEnabled(bProfileActive);

    if (!bTransOnly) this->m_spLevUpper[nIndex] = new QSpinBox(this->m_acGroupLevel[nIndex]);
    if (!bTransOnly) this->m_spLevUpper[nIndex]->setObjectName(QString("spin_profile_lev_upper") + QString::number(nIndex));
    if (!bTransOnly) this->m_spLevUpper[nIndex]->setGeometry(this->m_cUi.spin_profile_lev_upper->geometry());
    if (!bTransOnly) this->m_spLevUpper[nIndex]->setMinimum(1);
    if (!bTransOnly) this->m_spLevUpper[nIndex]->setMaximum(1000);
    this->m_spLevUpper[nIndex]->setEnabled( bProfileActive && (this->m_pcConfigData->s_get_MaxChLevel(nIndex) != 0) );
    this->m_spLevUpper[nIndex]->setValue((this->m_pcConfigData->s_get_MaxChLevel(nIndex) == 0) ? (int)this->m_pcConfigData->s_get_MinChLevel(nIndex) : (int)this->m_pcConfigData->s_get_MaxChLevel(nIndex));

    if (!bTransOnly) this->m_cbLevUpper[nIndex] = new QCheckBox(this->m_acGroupLevel[nIndex]);
    if (!bTransOnly) this->m_cbLevUpper[nIndex]->setObjectName(QString("check_profile_lev_upper") + QString::number(nIndex));
    if (!bTransOnly) this->m_cbLevUpper[nIndex]->setGeometry(this->m_cUi.check_profile_lev_upper->geometry());
    this->m_cbLevUpper[nIndex]->setText(QStringLiteral(""));
    this->m_cbLevUpper[nIndex]->setEnabled( bProfileActive );
    this->m_cbLevUpper[nIndex]->setCheckState( (this->m_pcConfigData->s_get_MaxChLevel(nIndex) == 0) ? Qt::Unchecked : Qt::Checked );

    if (!bTransOnly) this->m_acStackedTyp[nIndex]->addWidget(this->m_widgetLevel[nIndex]);

    // ...Favorite is selected
    if (!bTransOnly) this->m_widgetFavorite[nIndex] = new QWidget();
    if (!bTransOnly) this->m_widgetFavorite[nIndex]->setGeometry(QRect(10, 210, 511, 180));
    if (!bTransOnly) this->m_widgetFavorite[nIndex]->setObjectName(QString("page_profile_favorite") + QString::number(nIndex));

    if (!bTransOnly) this->m_acGroupFavorite[nIndex] = new QGroupBox(this->m_widgetFavorite[nIndex]);
    if (!bTransOnly) this->m_acGroupFavorite[nIndex]->setObjectName(QString("gb_profile_favorite") + QString::number(nIndex));
    if (!bTransOnly) this->m_acGroupFavorite[nIndex]->setGeometry(this->m_cUi.gb_profile_empty->geometry());
    this->m_acGroupFavorite[nIndex]->setTitle(TRANSLATE("mUi_ProfileFav"));

    if (!bTransOnly) this->m_pbDeleteFav[nIndex] = new QPushButton(this->m_acGroupFavorite[nIndex]);
    if (!bTransOnly) this->m_pbDeleteFav[nIndex]->setObjectName(QString("pb_profile_fav_delete") + QString::number(nIndex));
    if (!bTransOnly) this->m_pbDeleteFav[nIndex]->setGeometry(this->m_cUi.pb_profile_fav_delete->geometry());
    this->m_pbDeleteFav[nIndex]->setText(TRANSLATE("mUi_DelIgnore"));
    this->m_pbDeleteFav[nIndex]->setEnabled(bProfileActive);

    if (!bTransOnly) this->m_lbFavUseSub[nIndex] = new QLabel(this->m_acGroupFavorite[nIndex]);
    if (!bTransOnly) this->m_lbFavUseSub[nIndex]->setObjectName(QString("lb_profile_fav_use_sub") + QString::number(nIndex));
    if (!bTransOnly) this->m_lbFavUseSub[nIndex]->setGeometry(this->m_cUi.lb_profile_fav_use_sub->geometry());
    this->m_lbFavUseSub[nIndex]->setText(TRANSLATE("mUi_UseSubCh"));
    this->m_lbFavUseSub[nIndex]->setEnabled(bProfileActive);

    if (!bTransOnly) this->m_swFavUseSub[nIndex] = new SwitchButton(this->m_acGroupFavorite[nIndex]);
    if (!bTransOnly) this->m_swFavUseSub[nIndex]->setObjectName(QString("sw_profile_fav_use_sub") + QString::number(nIndex));
    if (!bTransOnly) this->m_swFavUseSub[nIndex]->setGeometry(this->m_cUi.sw_profile_fav_use_sub->geometry());
    this->m_swFavUseSub[nIndex]->setEnabled(bProfileActive);
    this->m_swFavUseSub[nIndex]->setValue(this->m_pcConfigData->s_get_UseSubChOfFav(nIndex));

    if (!bTransOnly) this->m_acStackedTyp[nIndex]->addWidget(this->m_widgetFavorite[nIndex]);

    // ...Frequency is selected
    if (!bTransOnly) this->m_widgetFreq[nIndex] = new QWidget();
    if (!bTransOnly) this->m_widgetFreq[nIndex]->setGeometry(QRect(10, 210, 511, 180));
    if (!bTransOnly) this->m_widgetFreq[nIndex]->setObjectName(QString("page_profile_freq") + QString::number(nIndex));

    if (!bTransOnly) this->m_acGroupFreq[nIndex] = new QGroupBox(this->m_widgetFreq[nIndex]);
    if (!bTransOnly) this->m_acGroupFreq[nIndex]->setObjectName(QString("gb_profile_freq") + QString::number(nIndex));
    if (!bTransOnly) this->m_acGroupFreq[nIndex]->setGeometry(this->m_cUi.gb_profile_empty->geometry());
    this->m_acGroupFreq[nIndex]->setTitle(TRANSLATE("fUi_gbFreq"));

    if (!bTransOnly) this->m_lbFreqFreq[nIndex] = new QLabel(this->m_acGroupFreq[nIndex]);
    if (!bTransOnly) this->m_lbFreqFreq[nIndex]->setObjectName(QString("lb_profile_freq_freq") + QString::number(nIndex));
    if (!bTransOnly) this->m_lbFreqFreq[nIndex]->setGeometry(this->m_cUi.lb_profile_freq_freq->geometry());
    this->m_lbFreqFreq[nIndex]->setText(TRANSLATE("mUi_TransFreq"));
    this->m_lbFreqFreq[nIndex]->setEnabled(bProfileActive);

    if (!bTransOnly) this->m_spFreqFreq[nIndex] = new QSpinBox(this->m_acGroupFreq[nIndex]);
    if (!bTransOnly) this->m_spFreqFreq[nIndex]->setObjectName(QString("spin_profile_freq_freq") + QString::number(nIndex));
    if (!bTransOnly) this->m_spFreqFreq[nIndex]->setGeometry(this->m_cUi.spin_profile_freq_freq->geometry());
    if (!bTransOnly) this->m_spFreqFreq[nIndex]->setMinimum(1);
    if (!bTransOnly) this->m_spFreqFreq[nIndex]->setMaximum(this->m_pcConfigData->s_get_MaxNumFreq()-1);
    this->m_spFreqFreq[nIndex]->setEnabled(bProfileActive);
    this->m_spFreqFreq[nIndex]->setValue((this->m_pcConfigData->s_get_ActiveFreq(nIndex) == 0) ? 1 : this->m_pcConfigData->s_get_ActiveFreq(nIndex));

    if (!bTransOnly) this->m_pbFreqNext[nIndex] = new QPushButton(this->m_acGroupFreq[nIndex]);
    if (!bTransOnly) this->m_pbFreqNext[nIndex]->setObjectName(QString("pb_profile_freq_next") + QString::number(nIndex));
    if (!bTransOnly) this->m_pbFreqNext[nIndex]->setGeometry(this->m_cUi.pb_profile_freq_next->geometry());
    this->m_pbFreqNext[nIndex]->setText(TRANSLATE("mUi_NextFreq"));
    this->m_pbFreqNext[nIndex]->setEnabled(bProfileActive);

    if (!bTransOnly) this->m_lbFreqMode[nIndex] = new QLabel(this->m_acGroupFreq[nIndex]);
    if (!bTransOnly) this->m_lbFreqMode[nIndex]->setObjectName(QString("lb_profile_freq_mode") + QString::number(nIndex));
    if (!bTransOnly) this->m_lbFreqMode[nIndex]->setGeometry(this->m_cUi.lb_profile_freq_mode->geometry());
    this->m_lbFreqMode[nIndex]->setText(TRANSLATE("mUi_ProfMode"));
    this->m_lbFreqMode[nIndex]->setEnabled(bProfileActive);

    if (!bTransOnly) this->m_cbFreqMode[nIndex] = new QComboBox(this->m_acGroupFreq[nIndex]);
    if (!bTransOnly) this->m_cbFreqMode[nIndex]->setObjectName(QString("cb_profile_freq_mode") + QString::number(nIndex));
    if (!bTransOnly) this->m_cbFreqMode[nIndex]->setGeometry(this->m_cUi.cb_profile_freq_mode->geometry());
    if (!bTransOnly)
    {
        this->m_cbFreqMode[nIndex]->insertItems(0, QStringList()
            << QStringLiteral("0")
            << QStringLiteral("1")
            << QStringLiteral("2")
            << QStringLiteral("3"));
    }
    this->m_cbFreqMode[nIndex]->setItemText(wm2000_freq_ui::State::STATE_OFF,     TRANSLATE("mUi_ProfModeOff"));
    this->m_cbFreqMode[nIndex]->setItemText(wm2000_freq_ui::State::STATE_NORMAL,  TRANSLATE("mUi_ProfModeNorm"));
    this->m_cbFreqMode[nIndex]->setItemText(wm2000_freq_ui::State::STATE_MUTE,    TRANSLATE("mUi_ProfModeMute"));
    this->m_cbFreqMode[nIndex]->setItemText(wm2000_freq_ui::State::STATE_SQUELCH, TRANSLATE("mUi_ProfModeSquelch"));
    this->m_cbFreqMode[nIndex]->setEnabled(bProfileActive);
    if (this->m_pcConfigData->s_get_ActiveFreq(nIndex) == 0)
        this->m_cbFreqMode[nIndex]->setCurrentIndex(wm2000_freq_ui::State::STATE_OFF);
    else if (this->m_pcConfigData->s_get_MuteFreq(nIndex))
        this->m_cbFreqMode[nIndex]->setCurrentIndex(wm2000_freq_ui::State::STATE_MUTE);
    else if (this->m_pcConfigData->s_get_SquelchFreq(nIndex))
        this->m_cbFreqMode[nIndex]->setCurrentIndex(wm2000_freq_ui::State::STATE_SQUELCH);
    else
        this->m_cbFreqMode[nIndex]->setCurrentIndex(wm2000_freq_ui::State::STATE_NORMAL);
    
    if (!bTransOnly) this->m_lbFreqPrio[nIndex] = new QLabel(this->m_acGroupFreq[nIndex]);
    if (!bTransOnly) this->m_lbFreqPrio[nIndex]->setObjectName(QString("lb_profile_freq_prio") + QString::number(nIndex));
    if (!bTransOnly) this->m_lbFreqPrio[nIndex]->setGeometry(this->m_cUi.lb_profile_freq_prio->geometry());
    this->m_lbFreqPrio[nIndex]->setText(TRANSLATE("mUi_UsePrio"));
    this->m_lbFreqPrio[nIndex]->setEnabled(bProfileActive);

    if (!bTransOnly) this->m_swFreqPrio[nIndex] = new SwitchButton(this->m_acGroupFreq[nIndex]);
    if (!bTransOnly) this->m_swFreqPrio[nIndex]->setObjectName(QString("sw_profile_freq_prio") + QString::number(nIndex));
    if (!bTransOnly) this->m_swFreqPrio[nIndex]->setGeometry(this->m_cUi.sw_profile_freq_prio->geometry());
    this->m_swFreqPrio[nIndex]->setEnabled(bProfileActive);
    // in update_expert_mode ==> this->m_swFreqPrio[nIndex]->setValue();

    if (!bTransOnly) this->m_pLabelMaster[nIndex] = new QLabel(this->m_acGroupFreq[nIndex]);
    if (!bTransOnly) this->m_pLabelMaster[nIndex]->setObjectName(QString("lb_profile_freq_prio") + QString::number(nIndex));
    if (!bTransOnly) this->m_pLabelMaster[nIndex]->setGeometry(this->m_cUi.lb_profile_freq_master->geometry());
    this->m_pLabelMaster[nIndex]->setText(TRANSLATE("mUi_UseMaster"));
    this->m_pLabelMaster[nIndex]->setEnabled(bProfileActive);

    if (!bTransOnly) this->m_pSwitchMaster[nIndex] = new SwitchButton(this->m_acGroupFreq[nIndex], SwitchButton::Style::ONOFF, QColor(205, 55, 45));
    if (!bTransOnly) this->m_pSwitchMaster[nIndex]->setObjectName(QString("sw_profile_freq_prio") + QString::number(nIndex));
    if (!bTransOnly) this->m_pSwitchMaster[nIndex]->setGeometry(this->m_cUi.sw_profile_freq_master->geometry());
    this->m_pSwitchMaster[nIndex]->setEnabled(bProfileActive);
    // in update_expert_mode ==> this->m_pSwitchMaster[nIndex]->setValue();

    // add group-box for LED background lighting
    create_led_group(this->m_acTabProfile[nIndex], this->m_acGroupLED[nIndex], this->m_acPbLedColor[nIndex], this->m_acPbLedTest[nIndex], this->m_acCbKey[nIndex], this->m_acCbType[nIndex], this->m_acLbColor[nIndex], bTransOnly);

    //set profile type
    if (!bTransOnly) this->m_acStackedTyp[nIndex]->addWidget(this->m_widgetFreq[nIndex]);
    this->m_acStackedTyp[nIndex]->setCurrentIndex((this->m_pcConfigData->s_get_ProfileType(nIndex) < PROFILE_AUDIO) ? this->m_pcConfigData->s_get_ProfileType(nIndex) : PROFILE_OFF);
}


/*
*   add all "active" tabs to tabgroup
*/
void wm2000_main_ui::add_tabs_to_group()
{
    CALL_STACK
    //remove all tabs from tab-group
    this->m_cUi.tab_group->clear();

    // add generated tab to tab-group
    this->m_cUi.tab_group->addTab(this->m_pcTabGeneral, QString());
    this->m_cUi.tab_group->setTabText(this->m_cUi.tab_group->indexOf(this->m_pcTabGeneral), TRANSLATE("mUi_TabGeneral"));
    this->m_cUi.tab_group->setTabToolTip(this->m_cUi.tab_group->indexOf(this->m_pcTabGeneral), TRANSLATE("mUi_TabGeneralTip"));
    this->m_cUi.tab_group->setTabWhatsThis(this->m_cUi.tab_group->indexOf(this->m_pcTabGeneral), TRANSLATE("mUi_TabGeneralWhat"));

    // add generated tab to tab-group
    for (int ii = 0; ii < this->m_cLocalConfigData.s_get_MaxNumProfiles(); ii++)
    {
        const size_t buffer_size = 128;
        char buffer[buffer_size];

        this->m_cUi.tab_group->addTab(this->m_acTabProfile[ii], QString());
        sprintf_s(buffer, buffer_size, TRANSLATE("mUi_NameProfTab").toUtf8(), ii + 1, this->m_cLocalConfigData.s_get_ProfileName(ii));
        this->m_cUi.tab_group->setTabText(this->m_cUi.tab_group->indexOf(this->m_acTabProfile[ii]), QString(buffer));
        sprintf_s(buffer, buffer_size, TRANSLATE("mUi_ToolProfTab").toUtf8(), this->m_cLocalConfigData.s_get_ProfileName(ii));
        this->m_cUi.tab_group->setTabToolTip(this->m_cUi.tab_group->indexOf(this->m_acTabProfile[ii]), QString(buffer));
    }
}


/*
*   update all "Expert Mode" related settings
*     if in Expert Mode, make sure this value is in default state
*/
void wm2000_main_ui::update_expert_mode()
{
    CALL_STACK
    bool bExpert = this->m_pcConfigData->s_get_ExpertMode();

    // menu
    this->m_pcSaveAsAction->setVisible(bExpert);

    // general Tab

    // profile Tabs
    for (int ii = 0; ii < REAL_MAXNUMPROFILES; ii++)
    {
        this->m_lbFreqPrio[ii]->setVisible(bExpert);
        this->m_pcConfigData->s_set_PrioFreq(ii,    bExpert ? false : this->m_pcConfigData->s_get_PrioFreq(ii));
        this->m_cLocalConfigData.s_set_PrioFreq(ii, bExpert ? false : this->m_cLocalConfigData.s_get_PrioFreq(ii));
        this->m_swFreqPrio[ii]->setValue(this->m_pcConfigData->s_get_PrioFreq(ii));
        this->m_swFreqPrio[ii]->setVisible(bExpert);

        this->m_pcConfigData->s_set_MasterFreq(ii, bExpert ? false : this->m_pcConfigData->s_get_MasterFreq(ii));
        this->m_cLocalConfigData.s_set_MasterFreq(ii, bExpert ? false : this->m_cLocalConfigData.s_get_MasterFreq(ii));
        this->m_pSwitchMaster[ii]->setValue(this->m_pcConfigData->s_get_MasterFreq(ii));
        this->m_pSwitchMaster[ii]->setVisible(bExpert && this->m_pcConfigData->s_get_UseMasterRight());
        this->m_pLabelMaster[ii]->setVisible(bExpert && this->m_pcConfigData->s_get_UseMasterRight());
    }
}


/*
*   update width of GroupBox depending on Tab width
*/
void wm2000_main_ui::update_box_size()
{
    CALL_STACK
    const int cWidthOffset = 20;

    //find minimum size
    QRect cSize = this->m_pcTabGeneral->geometry();
    int iMinWidth = cSize.width();
    for (int ii = 0; ii < this->m_cLocalConfigData.s_get_MaxNumProfiles(); ii++)
    {
        cSize = this->m_acTabProfile[ii]->geometry();
        iMinWidth = (iMinWidth < cSize.width()) ? iMinWidth : cSize.width();
    }

    // general Tab
    set_box_size(this->m_pcGroupGeneral, iMinWidth - cWidthOffset);
    set_box_size(this->m_pcGroupIgnore, iMinWidth - cWidthOffset);
    set_box_size(this->m_pcGroupLED, iMinWidth - cWidthOffset);

    // profile Tabs
    for (int ii = 0; ii < REAL_MAXNUMPROFILES; ii++)
    {
        set_box_size(this->m_acGroupGeneral[ii], iMinWidth - cWidthOffset);
        set_box_size(this->m_acGroupNoProfile[ii], iMinWidth - cWidthOffset);
        set_box_size(this->m_acGroupFavorite[ii], iMinWidth - cWidthOffset);
        set_box_size(this->m_acGroupLevel[ii], iMinWidth - cWidthOffset);
        set_box_size(this->m_acGroupFreq[ii], iMinWidth - cWidthOffset);
        set_box_size(this->m_acGroupLED[ii], iMinWidth - cWidthOffset);
    }
}


/*
*   set width of specific GroupBox
*/
void wm2000_main_ui::set_box_size(QGroupBox* &pGroupBox, int dNewWidth)
{
    CALL_STACK
    QRect cActSize;
    cActSize = pGroupBox->geometry();
    cActSize.setWidth(dNewWidth);
    pGroupBox->setGeometry(cActSize);
    return;
}

/*
*   init function for LED handler group box
*/
void wm2000_main_ui::create_led_group(QWidget* pTab, QGroupBox* &pGroupBox, QPushButton* &pPbLedColor, QPushButton* &pPbLedTest, QComboBox* &pCbKey, QComboBox* &pCbType, QLabel* &pLbColor, bool bTransOnly)
{
    CALL_STACK
    
    bool bProfileActive = false;

    QFont font_test;
    font_test.setBold(true);
    font_test.setWeight(75);

    if (!bTransOnly) pGroupBox = new QGroupBox(pTab);
    if (!bTransOnly) pGroupBox->setObjectName(QStringLiteral("gb_led"));
    if (!bTransOnly) pGroupBox->setGeometry(this->m_cUi.gb_general_led->geometry());
    pGroupBox->setTitle(TRANSLATE(L"mUi_gb_led"));

    if (!bTransOnly) pPbLedColor = new QPushButton(pGroupBox);
    if (!bTransOnly) pPbLedColor->setObjectName(QStringLiteral("pb_led_color"));
    if (!bTransOnly) pPbLedColor->setGeometry(this->m_cUi.pb_general_led_color->geometry());
    pPbLedColor->setText(TRANSLATE(L"mUi_pb_led_color"));
    pPbLedColor->setEnabled(bProfileActive);

    if (!bTransOnly) pCbKey = new QComboBox(pGroupBox);
    if (!bTransOnly) pCbKey->setObjectName(QStringLiteral("cb_key"));
    if (!bTransOnly) pCbKey->setGeometry(this->m_cUi.cb_general_key->geometry());
    if (!bTransOnly)
    {
        pCbKey->insertItems(0, QStringList()
            << QStringLiteral("0")
            << QStringLiteral("F1")
            << QStringLiteral("F2") );
    }
    pCbKey->setItemText(0, TRANSLATE("mUi_cb_led_key"));
    pCbKey->setEnabled(bProfileActive);

    if (!bTransOnly) pPbLedTest = new QPushButton(pGroupBox);
    if (!bTransOnly) pPbLedTest->setObjectName(QStringLiteral("pb_led_test"));
    if (!bTransOnly) pPbLedTest->setGeometry(this->m_cUi.pb_general_led_test->geometry());
    if (!bTransOnly) pPbLedTest->setFont(font_test);
    pPbLedTest->setText(TRANSLATE(L"mUi_pb_led_test"));
    pPbLedTest->setEnabled(bProfileActive);

    if (!bTransOnly) pCbType = new QComboBox(pGroupBox);
    if (!bTransOnly) pCbType->setObjectName(QStringLiteral("cb_led_type"));
    if (!bTransOnly) pCbType->setGeometry(this->m_cUi.cb_general_led_type->geometry());
    if (!bTransOnly)
    {
        pCbType->insertItems(0, QStringList()
            << QStringLiteral("0")
            << QStringLiteral("1") );
    }
    pCbType->setItemText(0, TRANSLATE("mUi_cb_led_type1"));
    pCbType->setItemText(1, TRANSLATE("mUi_cb_led_type2"));
    pCbType->setEnabled(bProfileActive);

    if (!bTransOnly) pLbColor = new QLabel(pGroupBox);
    if (!bTransOnly) pLbColor->setObjectName(QStringLiteral("lb_color"));
    if (!bTransOnly) pLbColor->setGeometry(this->m_cUi.lb_general_color->geometry());
    if (!bTransOnly) set_led_color_preview(pLbColor, QColor(255, 85, 0, 255));
    if (!bTransOnly) pLbColor->setAutoFillBackground(true);
    pLbColor->setText(QStringLiteral(""));
    pLbColor->setEnabled(bProfileActive);
}


/*
*   set LED color preview
*/
void wm2000_main_ui::set_led_color_preview(QLabel* pLbColor, QColor cColor)
{
    CALL_STACK
    QPalette palette;
    QBrush brush(cColor);
    brush.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active,   QPalette::Window, brush);
    palette.setBrush(QPalette::Inactive, QPalette::Window, brush);
    palette.setBrush(QPalette::Disabled, QPalette::Window, brush);
    pLbColor->setPalette(palette);
}

/*
*   update language where needed
*    additionally all parameter are set to last stored value
*/
void wm2000_main_ui::update_language()
{
    CALL_STACK
    
    // use actual setting to update language
    this->m_cTranslate.set_language(this->m_pcConfigData->s_get_Language());
    
    //set actual configuration
    create_menu(true);
    create_general_tab(true);
    for (int ii = 0; ii < REAL_MAXNUMPROFILES; ii++)
        create_profile_tab(ii, true);

    this->m_pAboutUi->set_language(this->m_cLocalConfigData.s_get_Language());
    return;
}