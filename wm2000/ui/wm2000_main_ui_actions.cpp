#include "wm2000_main_ui_actions.h"
#include <QMessageBox>
#include <qinputdialog.h>
#include <qdiriterator.h>
#include <qfiledialog.h>

#define TRANSLATE(a) QString(this->m_cTranslate.translate(a).c_str())
#if USE_CALL_STACK
#define CALL_STACK if(USE_CALL_STACK) this->m_cErrHandler.message_callstack(__FUNCSIG__);
#else
#define CALL_STACK
#endif

/*
*   constructor
*/
wm2000_main_ui_actions::wm2000_main_ui_actions(config_container *pcConfigData, void(*fUpdate_server)(), std::string sConfigPath, QWidget *parent)
    : wm2000_main_ui(pcConfigData, fUpdate_server, sConfigPath, parent)
{
    CALL_STACK
    // load stored settings and set interface up
    std::string sXmlPath = sConfigPath + std::string("config.xml");
    pcConfigData->set_file_path(sXmlPath);
    if (pcConfigData->file_exists(sXmlPath))
    {
        pcConfigData->s_read_param();
    }
    else
    {
        pcConfigData->s_read_param();                   //first, create default settings file (in case no predefined configs can be found or user cancels)
        this->m_cTranslate.set_language(pcConfigData->s_get_Language());
        load_base_config(pcConfigData, true);           //try to load predefined config
    }
    init_ui(parent);
}

/*
*   init ui
*     has to call base class init
*/
void wm2000_main_ui_actions::init_ui(QWidget *parent)
{
    CALL_STACK
    //call init of base class first
    wm2000_main_ui::init_ui(parent);

    // link menu events to handler
    connect(this->m_cUi.pb_ok,          SIGNAL(pressed()),              this, SLOT(handler_pbOk_clicked()));
    connect(this->m_pcLoadAction,       SIGNAL(triggered()),            this, SLOT(handler_Load_clicked()));
    connect(this->m_pcCloseAction,      SIGNAL(triggered()),            this, SLOT(handler_pbOk_clicked()));
    connect(this->m_cUi.pb_apply,       SIGNAL(pressed()),              this, SLOT(handler_pbApply_clicked()));
    connect(this->m_pcSaveAction,       SIGNAL(triggered()),            this, SLOT(handler_pbApply_clicked()));
    connect(this->m_pcSaveAsAction,     SIGNAL(triggered()),            this, SLOT(handler_SaveAs_clicked()));
    connect(this->m_cUi.pb_cancel,      SIGNAL(pressed()),              this, SLOT(handler_pbCancel_clicked()));
    connect(this->m_pcUndoAction,       SIGNAL(triggered()),            this, SLOT(handler_Undo_clicked()));
    connect(this->m_pFreqUi,            SIGNAL(config_updated()),       this, SLOT(handler_Undo_clicked()));
    connect(this->m_cUi.tab_group,      SIGNAL(currentChanged(int)),    this, SLOT(handler_tabProfile_currentItemChanged(int)));
    connect(this->m_pcExpertAction,     SIGNAL(triggered()),            this, SLOT(handler_select_Expert()));
    connect(this->m_pcFreqAction,       SIGNAL(triggered()),            this, SLOT(open_freq_ui()));
    connect(this->m_pcAboutAction,      SIGNAL(triggered()),            this, SLOT(open_about_ui()));

    connect(this->m_cUi.treeRxList,     SIGNAL(channel_ignore_triggered(int,uint64)),  this, SLOT(handler_add_to_ignore_clicked(int, uint64)));
    connect(this->m_cUi.treeRxList,     SIGNAL(channel_remove_triggered(int, uint64)), this, SLOT(handler_delete_from_list_clicked(int, uint64)));

    // link general tab events to handler
    connect(this->m_pcSpinNumProfile,   SIGNAL(valueChanged(int)),      this, SLOT(handler_select_NumProfile(int)));
    connect(this->m_pcCombLanguage,     SIGNAL(currentIndexChanged(const QString&)), this, SLOT(handler_select_Language(const QString&)));
    connect(this->m_pcSwIgnoreUseRx,    SIGNAL(valueChanged(bool)),     this, SLOT(handler_select_IgnoreRx(bool)));
    connect(this->m_pcSwAutoSave,       SIGNAL(valueChanged(bool)),     this, SLOT(handler_select_IgnoreSave(bool)));
    connect(this->m_pcPbDelIgnore,      SIGNAL(pressed()),              this, SLOT(handler_push_DelIgnore()));


    // link profile tab events to handler
    for (int ii = 0; ii < REAL_MAXNUMPROFILES; ii++)
    {
        connect(this->m_cbProfileType[ii],      SIGNAL(activated(int)),                 this, SLOT(handler_select_ProfileType(int)));
        connect(this->m_edProfileName[ii],      SIGNAL(textChanged(const QString&)),    this, SLOT(handler_edit_ProfileName(const QString&)));
        connect(this->m_swProfileIgnoreTx[ii],  SIGNAL(valueChanged(bool)),             this, SLOT(handler_select_IgnoreTx(bool)));
        connect(this->m_swProfilePtt[ii],       SIGNAL(valueChanged(bool)),             this, SLOT(handler_select_ptt(bool)));

        connect(this->m_swFavUseSub[ii],        SIGNAL(valueChanged(bool)),             this, SLOT(handler_select_UseSubCh(bool)));
        connect(this->m_pbDeleteFav[ii],        SIGNAL(pressed()),                      this, SLOT(handler_push_DelFavList()));
        
        connect(this->m_spLevLower[ii],         SIGNAL(valueChanged(int)),              this, SLOT(handler_select_LowLevel(int)));
        connect(this->m_spLevUpper[ii],         SIGNAL(valueChanged(int)),              this, SLOT(handler_select_UpLevel(int)));
        connect(this->m_cbLevUpper[ii],         SIGNAL(stateChanged(int)),              this, SLOT(handler_select_UpLevelActive(int)));

        connect(this->m_spFreqFreq[ii],         SIGNAL(valueChanged(int)),              this, SLOT(handler_select_Freq(int)));
        connect(this->m_pbFreqNext[ii],         SIGNAL(pressed()),                      this, SLOT(handler_pushed_NextFreq()));
        connect(this->m_cbFreqMode[ii],         SIGNAL(activated(int)),                 this, SLOT(handler_select_FreqMode(int)));
        connect(this->m_swFreqPrio[ii],         SIGNAL(valueChanged(bool)),             this, SLOT(handler_select_Priority(bool)));
        connect(this->m_pSwitchMaster[ii],      SIGNAL(valueChanged(bool)),             this, SLOT(handler_select_Master(bool)));
    }
}


/*
*   destructor
*/
wm2000_main_ui_actions::~wm2000_main_ui_actions()
{
    CALL_STACK
}


/*
*   event handler Menu
*   Apply button pressed
*/
void wm2000_main_ui_actions::handler_pbApply_clicked()
{
    CALL_STACK
    bool bNoRestart = this->m_cLocalConfigData.compare_config(*this->m_pcConfigData, true);
    
    //store settings
    *this->m_pcConfigData = this->m_cLocalConfigData;
    this->m_pcConfigData->s_write_param();

    //update ui and go back to selected Tab
    int iActTab = this->m_cUi.tab_group->currentIndex();
    update_language();
    add_tabs_to_group();
    update_expert_mode();
    update_box_size();
    check_LocalSettings();
    this->m_cUi.tab_group->setCurrentIndex( (this->m_cUi.tab_group->count() >= iActTab) ? iActTab : 0 );

    //update all frequencies (serverside)
    this->m_cUi.treeRxList->update_meta_data();
    update_ui();

    // update all windows
    this->m_pFreqUi->update_config();

    //check if changes need restart
    if(!bNoRestart)
    {
        //inform user that he has to resart TS3
        QMessageBox cMsgBox(QMessageBox::Information, QString("Information"), TRANSLATE("mUi_msg_NeedsRestart"), QMessageBox::Ok);
        cMsgBox.exec();
    }
    return;
}

/*
*   event handler Menu
*   SaveAs button pressed
*/
void wm2000_main_ui_actions::handler_SaveAs_clicked()
{
    CALL_STACK

    //store settings
    QString sFileName = QFileDialog::getSaveFileName(this, TRANSLATE("mUi_msg_saveas_title"), QString::fromStdString(m_sConfigPath + std::string("my_config.xml")), TRANSLATE("mUi_msg_saveas_conf"));
    
    if(sFileName.size() > 0)
        this->m_cLocalConfigData.s_write_param(sFileName.toStdString());

    return;
}

/*
*   event handler Menu
*   Load button pressed
*/
void wm2000_main_ui_actions::handler_Load_clicked()
{
    CALL_STACK
    load_base_config(&m_cLocalConfigData);
    return;
}
/*
*   load configuration file from disk
*     Load button pressed or init
*/
void wm2000_main_ui_actions::load_base_config(config_container *pcConfigData, bool bInit)
{
    CALL_STACK
    //create list of all possible config files to read
    QStringList vsItems;
    QDirIterator it(QString::fromStdString(m_sConfigPath), QDir::Filters(QDir::Files | QDir::NoDot | QDir::NoDotDot | QDir::NoSymLinks), QDirIterator::NoIteratorFlags);
    while (it.hasNext())
    {
        it.next();
        QFileInfo cActFile = it.fileInfo();
        if ((cActFile.suffix().compare("xml") == 0) && (cActFile.baseName().compare("config") != 0))
            vsItems << cActFile.baseName();
    }

    if (vsItems.size() > 0)
    {
        //ask user to select a file to read
        bool bOk;
        QString sResult = QInputDialog::getItem(this, TRANSLATE("mUi_msg_load_title"), bInit ? TRANSLATE("mUi_msg_firstload_text") : TRANSLATE("mUi_msg_load_text"), vsItems, 0, false, &bOk);

        if (bOk && !sResult.isEmpty())
        {
            //read file to local container
            QString sLoadFile = QString::fromStdString(m_sConfigPath) + sResult + QString(".xml");
            pcConfigData->s_read_param(sLoadFile.toStdString());
            if (!bInit)
            {
                handler_pbApply_clicked();
            }
            else
            {
                std::string sXmlPath = m_sConfigPath + std::string("config.xml");
                pcConfigData->s_write_param(sLoadFile.toStdString());
            }
        }
    }
    else
    {
        if (!bInit)
        {
            //inform user that he has to resart TS3
            QMessageBox cMsgBox(QMessageBox::Information, QString("Information"), TRANSLATE("mUi_msg_load_nofile"), QMessageBox::Ok);
            cMsgBox.exec();
        }
    }
    return;
}


/*
*   event handler Menu
*     undo all changes
*/
void wm2000_main_ui_actions::handler_Undo_clicked()
{
    CALL_STACK
    //reload settings
    this->m_cLocalConfigData = *this->m_pcConfigData;

    //update ui and go back to selected Tab
    int iActTab = this->m_cUi.tab_group->currentIndex();
    update_language();
    add_tabs_to_group();
    update_expert_mode();
    update_box_size();
    check_LocalSettings();
    this->m_cUi.tab_group->setCurrentIndex((this->m_cUi.tab_group->count() >= iActTab) ? iActTab : 0);

    update_ui();
    return;
}

/*
*   event handler Menu
*   OK button pressed
*/
void wm2000_main_ui_actions::handler_pbOk_clicked()
{
    CALL_STACK
    if(this->m_cLocalConfigData == *this->m_pcConfigData) handler_pbApply_clicked();
    this->close();
    return;
}

/*
*   event handler Menu
*   Cancel button pressed
*/
void wm2000_main_ui_actions::handler_pbCancel_clicked()
{
    CALL_STACK
    
    //restore values with default
    this->m_cLocalConfigData = *this->m_pcConfigData;
    update_language();
    update_expert_mode();
    add_tabs_to_group();

    //close UI
    this->close();
    return;
}

/*
*   event handler Menu
*     switch Expert Mode
*/
void wm2000_main_ui_actions::handler_delete_from_list_clicked(int iProfile, uint64 nChannel)
{
    CALL_STACK
    printf("del: %d / %lld \n", iProfile, nChannel);
    return;
}

/*
*   event handler Menu
*     switch Expert Mode
*/
void wm2000_main_ui_actions::handler_add_to_ignore_clicked(int iProfile, uint64 nChannel)
{
    CALL_STACK
    printf("add: %d / %lld \n", iProfile, nChannel);
    return;
}

/*
*   event handler Menu
*     switch Expert Mode
*/
void wm2000_main_ui_actions::handler_select_Expert()
{
    CALL_STACK
    this->m_cLocalConfigData.s_set_ExpertMode(this->m_pcExpertAction->isChecked());
    check_LocalSettings();
    return;
}


/*
*   event handler
*   Tab Item has changed
*/
void wm2000_main_ui_actions::handler_tabProfile_currentItemChanged(int iIndex)
{
    CALL_STACK
    this->m_cUi.treeRxList->create_tree_entry(iIndex - 1);  // -1 == ignore list, 0 to x-1 == profile array index
    m_iActTabIndex = iIndex;
    return;
}

/*
*   event handler "General" tab
*     number of active profiles changed
*/
void wm2000_main_ui_actions::handler_select_NumProfile(int iNewValue)
{
    CALL_STACK
    //save new value local and update tab group
    this->m_cLocalConfigData.s_set_MaxNumProfiles(iNewValue);
    add_tabs_to_group();
    update_box_size();
    check_LocalSettings();
    return;
}

/*
*   event handler "General" tab
*     Combo box language changed
*/
void wm2000_main_ui_actions::handler_select_Language(const QString &sText)
{
    CALL_STACK
    this->m_cLocalConfigData.s_set_Language(sText.toStdString());
    check_LocalSettings();
    return;
}

/*
*   event handler "General" tab
*     switch use ignore list for RX changed
*/
void wm2000_main_ui_actions::handler_select_IgnoreRx(bool bNewValue)
{
    CALL_STACK
    this->m_cLocalConfigData.s_set_UseIgnoreListRx(bNewValue);
    check_LocalSettings();
    return;
}

/*
*   event handler "General" tab
*     switch save ignore list changed
*/
void wm2000_main_ui_actions::handler_select_IgnoreSave(bool bNewValue)
{
    CALL_STACK
    this->m_cLocalConfigData.s_set_SaveIgnoreList(bNewValue);
    check_LocalSettings();
    return;
}

/*
*   event handler "General" tab
*     Button delete ignore list pushed
*/
void wm2000_main_ui_actions::handler_push_DelIgnore()
{
    CALL_STACK
    //delete local list entries only
    this->m_cLocalConfigData.clear_vector(this->m_cLocalConfigData.s_get_IgnoreList());
    check_LocalSettings();
    return;
}



/*
*   event handler "Profile" tab
*     profile name was edited
*/
void wm2000_main_ui_actions::handler_edit_ProfileName(const QString &sNewText)
{
    CALL_STACK
    int iActProfile = m_iActTabIndex-1;
    const size_t buffer_size = 128;
    char buffer[buffer_size];

    if(iActProfile >= 0 )
    {
        QString sTrimmed = sNewText.trimmed();
        if (sTrimmed.size() == 0)
            this->m_cLocalConfigData.s_set_ProfileName(iActProfile, this->m_cLocalConfigData.get_DefaultProfileName(iActProfile));
        else
            this->m_cLocalConfigData.s_set_ProfileName(iActProfile, sTrimmed.toStdString());

        sprintf_s(buffer, buffer_size, TRANSLATE("mUi_NameProfTab").toUtf8(), iActProfile + 1, this->m_cLocalConfigData.s_get_ProfileName(iActProfile));
        this->m_cUi.tab_group->setTabText(this->m_cUi.tab_group->indexOf(this->m_acTabProfile[iActProfile]), QString(buffer));
        sprintf_s(buffer, buffer_size, TRANSLATE("mUi_ToolProfTab").toUtf8(), this->m_cLocalConfigData.s_get_ProfileName(iActProfile));
        this->m_cUi.tab_group->setTabToolTip(this->m_cUi.tab_group->indexOf(this->m_acTabProfile[iActProfile]), QString(buffer));
        if (sTrimmed.size() != 0)
            this->m_edProfileName[iActProfile]->setText(QString::fromStdString(this->m_cLocalConfigData.s_get_ProfileName(iActProfile)));
        else
            this->m_edProfileName[iActProfile]->setText(QString(""));

        update_box_size();
        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     new profile type was selected
*/
void wm2000_main_ui_actions::handler_select_ProfileType(int iNewValue)
{
    CALL_STACK
    int iActProfile = m_iActTabIndex-1;

    if( (iNewValue < PROFILE_OFF) || (iNewValue > PROFILE_FREQUENCY) )
        iNewValue = PROFILE_OFF;
    
    if (iActProfile >= 0)
    {
        this->m_cLocalConfigData.s_set_ProfileType(iActProfile, (eProfileType)iNewValue);
        this->m_acStackedTyp[iActProfile]->setCurrentIndex(iNewValue);
        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     switch use ignore list for TX changed
*/
void wm2000_main_ui_actions::handler_select_IgnoreTx(bool bNewValue)
{
    CALL_STACK
    int iActProfile = m_iActTabIndex - 1;

    if (iActProfile >= 0)
    {
        this->m_cLocalConfigData.s_set_UseIgnoreListTx(iActProfile, bNewValue);
        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     switch use auto PTT changed
*/
void wm2000_main_ui_actions::handler_select_ptt(bool bNewValue)
{
    CALL_STACK
    int iActProfile = m_iActTabIndex-1;

    if (iActProfile >= 0)
    {
        this->m_cLocalConfigData.s_set_AutoActivate(iActProfile, bNewValue);
        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     Type Favorite: use sub channel changed
*/
void wm2000_main_ui_actions::handler_select_UseSubCh(bool bNewValue)
{
    CALL_STACK
    int iActProfile = m_iActTabIndex - 1;

    if (iActProfile >= 0)
    {
        this->m_cLocalConfigData.s_set_UseSubChOfFav(iActProfile, bNewValue);

        //update list field
        this->m_cUi.treeRxList->create_tree_entry(iActProfile);

        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     Type Level: delete favorite list
*/
void wm2000_main_ui_actions::handler_push_DelFavList()
{
    CALL_STACK
    int iActProfile = m_iActTabIndex - 1;

    if (iActProfile >= 0)
    {
        //delete local list entries only
        this->m_cLocalConfigData.clear_vector(this->m_cLocalConfigData.s_get_FavoriteList(iActProfile));

        //update list field
        this->m_cUi.treeRxList->create_tree_entry(iActProfile);

        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     Type Level: lower level was changed
*/
void wm2000_main_ui_actions::handler_select_LowLevel(int iNewValue)
{
    CALL_STACK
    int iActProfile = m_iActTabIndex - 1;

    if (iActProfile >= 0)
    {
        this->m_cLocalConfigData.s_set_MinChLevel(iActProfile, iNewValue);

        //update list field
        this->m_cUi.treeRxList->create_tree_entry(iActProfile);

        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     Type Level: upper level was changed
*/
void wm2000_main_ui_actions::handler_select_UpLevel(int iNewValue)
{
    CALL_STACK
    int iActProfile = m_iActTabIndex - 1;

    if (iActProfile >= 0)
    {
        this->m_cLocalConfigData.s_set_MaxChLevel(iActProfile, iNewValue);

        //update list field
        this->m_cUi.treeRxList->create_tree_entry(iActProfile);

        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     Type Level: upper level was enabled
*/
void wm2000_main_ui_actions::handler_select_UpLevelActive(int iNewValue)
{
    CALL_STACK
    int iActProfile = m_iActTabIndex - 1;

    if (iActProfile >= 0)
    {
        if (iNewValue == Qt::CheckState::Checked)
        {
            if (this->m_cLocalConfigData.s_get_MaxChLevel(iActProfile) == 0)
            {
                if(this->m_spLevUpper[iActProfile]->value() < this->m_cLocalConfigData.s_get_MinChLevel(iActProfile))
                    this->m_cLocalConfigData.s_set_MaxChLevel(iActProfile, this->m_cLocalConfigData.s_get_MinChLevel(iActProfile));
                else
                    this->m_cLocalConfigData.s_set_MaxChLevel(iActProfile, this->m_spLevUpper[iActProfile]->value());
            }
            this->m_spLevUpper[iActProfile]->setEnabled(true);
            this->m_spLevUpper[iActProfile]->setValue((int)this->m_cLocalConfigData.s_get_MaxChLevel(iActProfile));
        }
        else
        {
            this->m_cLocalConfigData.s_set_MaxChLevel(iActProfile, 0);
            this->m_spLevUpper[iActProfile]->setEnabled(false);
        }

        //update list field
        this->m_cUi.treeRxList->create_tree_entry(iActProfile);

        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     Type Freq: frequency was changed
*/
void wm2000_main_ui_actions::handler_select_Freq(int iNewValue)
{
    CALL_STACK
    int iActProfile = m_iActTabIndex - 1;

    if (iActProfile >= 0)
    {
        //set new value
        this->m_cLocalConfigData.s_set_ActiveFreq(iActProfile, iNewValue);

        //update list field
        this->m_cUi.treeRxList->create_tree_entry(iActProfile);

        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     Type Freq: state of mode ComboBox has changed
*/
void wm2000_main_ui_actions::handler_select_FreqMode(int iNewValue)
{
    CALL_STACK
    int iActProfile = m_iActTabIndex - 1;

    if (iActProfile >= 0)
    {
        bool bState;
        switch (iNewValue)
        {
        case wm2000_freq_ui::State::STATE_OFF:
            //Off
            this->m_cLocalConfigData.s_set_ActiveFreq(iActProfile, 0);
            bState = false;
            break;
        case wm2000_freq_ui::State::STATE_NORMAL:
            //Normal
            this->m_cLocalConfigData.s_set_ActiveFreq(iActProfile, this->m_spFreqFreq[iActProfile]->value());
            this->m_cLocalConfigData.s_set_MuteFreq(iActProfile, false);
            this->m_cLocalConfigData.s_set_SquelchFreq(iActProfile, false);
            bState = true;
            break;
        case wm2000_freq_ui::State::STATE_MUTE:
            //Mute
            this->m_cLocalConfigData.s_set_ActiveFreq(iActProfile, this->m_spFreqFreq[iActProfile]->value());
            this->m_cLocalConfigData.s_set_MuteFreq(iActProfile, true);
            this->m_cLocalConfigData.s_set_SquelchFreq(iActProfile, false);
            bState = true;
            break;
        case wm2000_freq_ui::State::STATE_SQUELCH:
            //Squelch
            this->m_cLocalConfigData.s_set_ActiveFreq(iActProfile, this->m_spFreqFreq[iActProfile]->value());
            this->m_cLocalConfigData.s_set_MuteFreq(iActProfile, false);
            this->m_cLocalConfigData.s_set_SquelchFreq(iActProfile, true);
            bState = true;
            break;
        default:
            return;
            break;
        }

        //enable/disable frequency widgets
        this->m_spFreqFreq[iActProfile]->setEnabled(bState);
        this->m_pbFreqNext[iActProfile]->setEnabled(bState);
        this->m_swFreqPrio[iActProfile]->setEnabled(bState);

        //update ui
        this->m_cUi.treeRxList->create_tree_entry(iActProfile);
        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     Type Freq: select next unused freq
*/
void wm2000_main_ui_actions::handler_pushed_NextFreq()
{
    CALL_STACK
    int iActProfile = m_iActTabIndex - 1;

    if (iActProfile >= 0)
    {
        //check all clients for unused frequencies
        int iFreq = this->m_cUi.treeRxList->get_NextUnusedFreq();

        //set new value (all other parameter where set in dial handler)
        if (iFreq != 0)
            this->m_spFreqFreq[iActProfile]->setValue(iFreq);
        else
            QMessageBox(QMessageBox::Warning, TRANSLATE(L"fUi_fmTitle"), TRANSLATE(L"fUi_FreqFull"), QMessageBox::Ok, this).exec();

        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     Type Freq: select next unused freq
*/
void wm2000_main_ui_actions::handler_select_Priority(bool bNewValue)
{
    CALL_STACK
    int iActProfile = m_iActTabIndex - 1;

    if (iActProfile >= 0)
    {
        this->m_cLocalConfigData.s_set_PrioFreq(iActProfile, bNewValue);

        check_LocalSettings();
    }
    return;
}

/*
*   event handler "Profile" tab
*     Type Freq: select next unused freq
*/
void wm2000_main_ui_actions::handler_select_Master(bool bNewValue)
{
    CALL_STACK
        int iActProfile = m_iActTabIndex - 1;

    if (iActProfile >= 0)
    {
        this->m_cLocalConfigData.s_set_MasterFreq(iActProfile, bNewValue);

        check_LocalSettings();
    }
    return;
}



/*
*   update ui, if server parameter (client/channel) where changed from plugin
*/
void wm2000_main_ui_actions::update_ui()
{
    CALL_STACK
    // update FreqUi
    this->m_pFreqUi->update_ui();

    //update list field
    this->m_cUi.treeRxList->create_tree_entry(m_iActTabIndex - 1);
    return;
}


/*
*   update ui, if parameter where changed from plugin
*/
void wm2000_main_ui_actions::update_config()
{
    CALL_STACK
    // store latest settings from ConfigData
    handler_Undo_clicked();

    //update list field
    this->m_cUi.treeRxList->create_tree_entry(m_iActTabIndex - 1);

    // update all windows
    this->m_pFreqUi->update_config();
    return;
}


/*
*   add entity of client filter
*/
void wm2000_main_ui_actions::add_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter)
{
    CALL_STACK
    // update all clientTreeWidgets
    this->m_pFreqUi->add_pointer(pcClientFilter, pcChannelFilter);
    this->m_cUi.treeRxList->add_pointer(pcClientFilter, pcChannelFilter);

    //update list field
    this->m_cUi.treeRxList->create_tree_entry(m_iActTabIndex - 1);
}


/*
*   delete entity of client_filter
*/
void wm2000_main_ui_actions::delete_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter)
{
    CALL_STACK
    // update all clientTreeWidgets
    this->m_pFreqUi->delete_pointer(pcClientFilter, pcChannelFilter);
    this->m_cUi.treeRxList->delete_pointer(pcClientFilter, pcChannelFilter);

    //update list field
    this->m_cUi.treeRxList->create_tree_entry(m_iActTabIndex - 1);
}


/*
*   compare local with static settings and enable "Save" dialog on demand
*/
bool wm2000_main_ui_actions::check_LocalSettings()
{
    CALL_STACK
    bool bResult = (m_cLocalConfigData == *m_pcConfigData);

    this->m_cUi.pb_apply->setEnabled(!bResult);
    this->m_pcSaveAction->setEnabled(!bResult);
    this->m_pcUndoAction->setEnabled(!bResult);
    return bResult;
}