#include "wm2000_freq_ui.h"
#include <QtWidgets\QMessageBox>
#include <QtWidgets\QInputDialog>

#define TRANSLATE(a) QString(this->m_cTranslate.translate(a).c_str())
#if USE_CALL_STACK
#define CALL_STACK if(USE_CALL_STACK) this->m_cErrHandler.message_callstack(__FUNCSIG__);
#else
#define CALL_STACK
#endif

/*
*   Constructor of UI
*/
wm2000_freq_ui::wm2000_freq_ui(config_container *pcConfigData, QIcon *pIconWindow, QIcon *pcIgnoreIcon, QIcon *pcFavIcon, QWidget *parent)
    : QDialog(parent)
{
    CALL_STACK
    // set interface up
    this->m_pcConfigData     = pcConfigData;
    this->m_cLocalConfigData = *this->m_pcConfigData;
    this->m_cTranslate.set_language(this->m_cLocalConfigData.s_get_Language());
 
    m_cUi.setupUi(this);
    this->m_cUi.treeRxList->init(&m_cLocalConfigData, pcIgnoreIcon, pcFavIcon);

    // basic setup of ui elements
    this->setWindowIcon(*pIconWindow);
    this->setWindowTitle(TRANSLATE(L"fUi_fmTitle"));
    this->setFixedSize(size());
    this->m_cUi.pbOk->setText(TRANSLATE(L"gUi_pbOk"));
    this->m_cUi.pbApply->setText(TRANSLATE(L"gUi_pbApply"));
    this->m_cUi.pbCancel->setText(TRANSLATE(L"gUi_pbCancel"));

    this->m_iFreqProfileCnt = 0;
    init_profile_tab();
    for (int ii = 0; ii < this->m_cLocalConfigData.s_get_MaxNumProfiles(); ii++)
    {
        if (this->m_cLocalConfigData.s_get_ProfileType(ii) == PROFILE_FREQUENCY)
        {
            create_profile_tab(this->m_iFreqProfileCnt, ii);
            this->m_iFreqProfileIdx[this->m_iFreqProfileCnt] = ii;
            this->m_iFreqProfileCnt++;
        }
    }

    if (this->m_iFreqProfileCnt > 0)
    {
        //at least one profile of type FREQUENCY found
        this->m_cUi.tabProfile->removeTab(this->m_cUi.tabProfile->indexOf(this->m_cUi.tabProfile_dummy));

        //set index to first element
        this->m_iActualTabIndex = 0;
    }
    else
    {
        //no profile found => set index to invalid
        this->m_iActualTabIndex = -1;
    }

    //initialize all elements
    if(this->m_iActualTabIndex >= 0) this->m_cUi.treeRxList->create_tree_entry(this->m_iFreqProfileIdx[this->m_iActualTabIndex]);
    this->m_cUi.pbApply->setEnabled(false);

    // link events to handler
    connect(this->m_cUi.pbOk,       &QPushButton::clicked,          this, &wm2000_freq_ui::handler_pbOk_clicked);
    connect(this->m_cUi.pbApply,    &QPushButton::clicked,          this, &wm2000_freq_ui::handler_pbApply_clicked);
    connect(this->m_cUi.pbCancel,   &QPushButton::clicked,          this, &wm2000_freq_ui::handler_pbCancel_clicked);
    connect(this->m_cUi.tabProfile, &QTabWidget::currentChanged,    this, &wm2000_freq_ui::handler_tabProfile_currentItemChanged);
    return;
}

/*
*   update ui, if parameter where changed from other parts of the plugin
*/
void wm2000_freq_ui::update_config()
{
    CALL_STACK
    //copy actual state
    this->m_cLocalConfigData = *this->m_pcConfigData;

    // update parameter shown in ui
    for (int ii = 0; ii < this->m_iFreqProfileCnt; ii++)
    {
        set_profile_tab(ii, this->m_iFreqProfileIdx[ii]);
    }

    //update list field
    if(this->m_iActualTabIndex >= 0) this->m_cUi.treeRxList->create_tree_entry(this->m_iFreqProfileIdx[this->m_iActualTabIndex]);

    //all changes are cleared, disable Apply again
    this->m_cUi.pbApply->setEnabled(false);
    return;
}

/*
*   add entity of client filter
*/
void wm2000_freq_ui::add_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter)
{
    CALL_STACK
    //try to find requested element
    this->m_cUi.treeRxList->add_pointer(pcClientFilter, pcChannelFilter);

    //update list field
    if(this->m_iActualTabIndex >= 0) this->m_cUi.treeRxList->create_tree_entry(this->m_iFreqProfileIdx[this->m_iActualTabIndex]);
}

/*
*   delete entity of client_filter
*/
void wm2000_freq_ui::delete_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter)
{
    CALL_STACK
    //try to find requested element and delete element
    this->m_cUi.treeRxList->delete_pointer(pcClientFilter, pcChannelFilter);

    //update list field
    if(this->m_iActualTabIndex >= 0) this->m_cUi.treeRxList->create_tree_entry(this->m_iFreqProfileIdx[this->m_iActualTabIndex]);
}

/*
*   event handler
*   Apply button pressed
*/
void wm2000_freq_ui::handler_pbApply_clicked()
{
    CALL_STACK
    //save data
    this->m_cLocalConfigData.s_write_param();
    *this->m_pcConfigData = this->m_cLocalConfigData;

    //update all frequencies (serverside)
    this->m_cUi.treeRxList->update_meta_data();

    //update base UI
    emit config_updated();

    //all changes are saved, disable Apply again
    this->m_cUi.pbApply->setEnabled(false);
    return;
}

/*
*   event handler
*   OK button pressed
*/
void wm2000_freq_ui::handler_pbOk_clicked()
{
    CALL_STACK
    handler_pbApply_clicked();
    this->close();
    return;
}

/*
*   event handler
*   Cancel button pressed
*/
void wm2000_freq_ui::handler_pbCancel_clicked()
{
    CALL_STACK
    this->close();
    return;
}

/*
*   event handler
*   Value of frequency dial has changed
*/
void wm2000_freq_ui::handler_freqdial_value_changed( int iValue )
{
    CALL_STACK
    int iIndex = this->m_cUi.tabProfile->currentIndex();
    // if widget index is invalid, bail out early
    if ((iIndex < 0) || (iIndex >= this->m_iFreqProfileCnt))
        return;

    //set new value
    this->m_cLocalConfigData.s_set_ActiveFreq(this->m_iFreqProfileIdx[iIndex], iValue);
    set_lcd_display(iIndex, iValue);

    //update client list
    if(this->m_iActualTabIndex >= 0) this->m_cUi.treeRxList->create_tree_entry(this->m_iFreqProfileIdx[this->m_iActualTabIndex]);

    this->m_cUi.pbApply->setEnabled(this->m_cLocalConfigData != (*this->m_pcConfigData));
    return;
}

/*
*   event handler
*   Value of state dial has changed
*/
void wm2000_freq_ui::handler_statedial_value_changed(int iValue)
{
    CALL_STACK
    int iIndex = this->m_cUi.tabProfile->currentIndex();
    // if widget index is invalid, bail out early
    if ((iIndex < 0) || (iIndex >= this->m_iFreqProfileCnt))
        return;
    
    bool bState;
    switch (iValue)
    {
        case STATE_OFF:
        case STATE_OFF2:
            //Off
            bState = false;
            break;
        case STATE_NORMAL:
            //Normal
            this->m_cLocalConfigData.s_set_MuteFreq(this->m_iFreqProfileIdx[iIndex],    false);
            this->m_cLocalConfigData.s_set_SquelchFreq(this->m_iFreqProfileIdx[iIndex], false);
            bState = true;
            break;
        case STATE_MUTE:
            //Mute
            this->m_cLocalConfigData.s_set_MuteFreq(this->m_iFreqProfileIdx[iIndex],    true);
            this->m_cLocalConfigData.s_set_SquelchFreq(this->m_iFreqProfileIdx[iIndex], false);
            bState = true;
            break;
        case STATE_SQUELCH:
            //Squelch
            this->m_cLocalConfigData.s_set_MuteFreq(this->m_iFreqProfileIdx[iIndex],    false);
            this->m_cLocalConfigData.s_set_SquelchFreq(this->m_iFreqProfileIdx[iIndex], true);
            bState = true;
            break;
        default:
            return;
            break;
    }

    //set frequency depending on state
    if (bState)
    {
        //enable frequency
        handler_freqdial_value_changed(this->m_pDialFreq[iIndex]->value());
    }
    else
    {
        //disable frequency
        this->m_cLocalConfigData.s_set_ActiveFreq(this->m_iFreqProfileIdx[iIndex], 0);

        set_lcd_display(iIndex, 0);
        if(this->m_iActualTabIndex >= 0) this->m_cUi.treeRxList->create_tree_entry(this->m_iFreqProfileIdx[this->m_iActualTabIndex]);
        this->m_cUi.pbApply->setEnabled(this->m_cLocalConfigData != (*this->m_pcConfigData));
    }

    // Enable/Disable all elements based on actual state
    this->m_pLcdFreq[iIndex]->setEnabled(bState);
    this->m_pDialFreq[iIndex]->setEnabled(bState);
    this->m_pPbSetFreq[iIndex]->setEnabled(bState);
    this->m_pPbNextFreq[iIndex]->setEnabled(bState);
    this->m_pSwitchPriority[iIndex]->setEnabled(bState);

    this->m_pSwitchPriority[iIndex]->setValue(this->m_cLocalConfigData.s_get_PrioFreq(this->m_iFreqProfileIdx[iIndex]));
    return;
}

void wm2000_freq_ui::handler_pbSetFreq_clicked()
{
    CALL_STACK
    int iIndex = this->m_cUi.tabProfile->currentIndex();
    // if widget index is invalid, bail out early
    if ((iIndex < 0) || (iIndex >= this->m_iFreqProfileCnt))
        return;

    // ask user for a value
    bool ok;
    int iValue = QInputDialog::getInt(this, TRANSLATE(L"fUi_fmTitle"), TRANSLATE(L"fUi_gbFreq"), this->m_cLocalConfigData.s_get_ActiveFreq(this->m_iFreqProfileIdx[iIndex]), 1, this->m_cLocalConfigData.s_get_MaxNumFreq(), 1, &ok);
    
    //if cancled, bail out
    if (!ok)
        return;

    //if nothing has changed, bail out
    if (iValue == this->m_cLocalConfigData.s_get_ActiveFreq(this->m_iFreqProfileIdx[iIndex]))
        return;

    //check if freq is already in use
    for (int ii = 0; ii < this->m_cLocalConfigData.s_get_MaxNumProfiles(); ii++)
    {
        if ((this->m_cLocalConfigData.s_get_ProfileType(ii) == PROFILE_FREQUENCY) && (this->m_cLocalConfigData.s_get_ActiveFreq(ii) == iValue))
        {
            QMessageBox(QMessageBox::Information, TRANSLATE(L"fUi_fmTitle"), TRANSLATE(L"fUi_FreqUsed"), QMessageBox::Ok, this).exec();
            break;
        }
    }

    //set new value (all other parameter where set in dial handler)
    this->m_pDialFreq[iIndex]->setValue(iValue);
    return;
}

void wm2000_freq_ui::handler_pbNextFreq_clicked()
{
    CALL_STACK
    int iIndex = this->m_cUi.tabProfile->currentIndex();
    // if widget index is invalid, bail out early
    if ((iIndex < 0) || (iIndex >= this->m_iFreqProfileCnt))
        return;

    //check all clients for unused frequencies
    int iFreq = this->m_cUi.treeRxList->get_NextUnusedFreq();

    //set new value (all other parameter where set in dial handler)
    if (iFreq != 0)
        this->m_pDialFreq[iIndex]->setValue(iFreq);
    else
        QMessageBox(QMessageBox::Warning, TRANSLATE(L"fUi_fmTitle"), TRANSLATE(L"fUi_FreqFull"), QMessageBox::Ok, this).exec();

    return;
}

void wm2000_freq_ui::handler_swPriority_valueChanged(bool bValue)
{
    CALL_STACK
    int iIndex = this->m_cUi.tabProfile->currentIndex();
    // if widget index is invalid, bail out early
    if ((iIndex < 0) || (iIndex >= this->m_iFreqProfileCnt))
        return;

    this->m_cLocalConfigData.s_set_PrioFreq(this->m_iFreqProfileIdx[iIndex], bValue);

    this->m_cUi.pbApply->setEnabled(this->m_cLocalConfigData != (*this->m_pcConfigData));
    return;
}

void wm2000_freq_ui::handler_tabProfile_currentItemChanged(int iIndex)
{
    CALL_STACK
    //make sure, that create_tree_entry is not using m_iActualTabIndex while we change it
    this->m_iActualTabIndex = iIndex;

    //update client list
    if(this->m_iActualTabIndex >= 0) this->m_cUi.treeRxList->create_tree_entry(this->m_iFreqProfileIdx[this->m_iActualTabIndex]);
}


void wm2000_freq_ui::set_lcd_display(int iIndex, int iValue)
{
    CALL_STACK
    const int iOff = 0x10FF;

    if (iValue == 0)
    {
        // set to "OFF"
        this->m_pLcdFreq[iIndex]->setMode(QLCDNumber::Hex);
        this->m_pLcdFreq[iIndex]->display(iOff);
        this->m_pLcdFreq[iIndex]->setDigitCount(3);
    }
    else
    {
        // set to selected value
        this->m_pLcdFreq[iIndex]->setMode(QLCDNumber::Dec);
        this->m_pLcdFreq[iIndex]->setDigitCount(5);
        this->m_pLcdFreq[iIndex]->display(iValue);
    }
    return;
}

/*
*   init Tab pointer
*/
void wm2000_freq_ui::init_profile_tab()
{
    CALL_STACK
    for( int ii = 0; ii < REAL_MAXNUMPROFILES; ii++)
    {
        this->m_pTabProfile[ii]     = nullptr;
        this->m_pGbFreq[ii]         = nullptr;
        this->m_pLcdFreq[ii]        = nullptr;
        this->m_pDialFreq[ii]       = nullptr;
        this->m_pPbSetFreq[ii]      = nullptr;
        this->m_pPbNextFreq[ii]     = nullptr;
        this->m_pGbGeneral[ii]      = nullptr;
        this->m_pDialState[ii]      = nullptr;
        this->m_pLabelOff[ii]       = nullptr;
        this->m_pLabelNormal[ii]    = nullptr;
        this->m_pLabelMute[ii]      = nullptr;
        this->m_pLabelSquelch[ii]   = nullptr;
        this->m_pLabelSquelch[ii]   = nullptr;
        this->m_pLabelPriority[ii]  = nullptr;
        this->m_pSwitchPriority[ii] = nullptr;
    }

    // init dummy Tab
    this->m_cUi.tabProfile->setTabText(this->m_cUi.tabProfile->indexOf(this->m_cUi.tabProfile_dummy), TRANSLATE(L"fUi_tabProfileDummy"));
    this->m_cUi.tabProfile->setTabEnabled(this->m_cUi.tabProfile->indexOf(this->m_cUi.tabProfile_dummy), false);
    this->m_cUi.tabProfile->setWhatsThis(TRANSLATE(L"fUi_WhatIsTabProfile"));
    this->m_cUi.gbFreq->setTitle(TRANSLATE(L"fUi_gbFreq"));
    this->m_cUi.pbSetFreq->setText(TRANSLATE(L"fUi_pbSetFreq"));
    this->m_cUi.pbNextFreq->setText(TRANSLATE(L"fUi_pbNextFreq"));
    this->m_cUi.gbGeneral->setTitle(TRANSLATE(L"fUi_gbGeneral"));
}


/*
*   set up a Tab for a new Profile
*/
void wm2000_freq_ui::create_profile_tab(int iTabIdx, int iProfileIdx)
{
    CALL_STACK
    QFont cFontCheckBox;
    cFontCheckBox.setPointSize(10);
    cFontCheckBox.setBold(true);
    cFontCheckBox.setWeight(75);

    QFont cFontLabel;
    cFontLabel.setPointSize(11);
    cFontLabel.setBold(true);
    cFontLabel.setWeight(75);

    // Profile "Tab"
    if(this->m_pTabProfile[iTabIdx] == nullptr) this->m_pTabProfile[iTabIdx] = new QWidget();
    this->m_pTabProfile[iTabIdx]->setObjectName(QStringLiteral("tabProfile_dummy"));
    this->m_pTabProfile[iTabIdx]->setEnabled(true);

    // Frequency group
    if(this->m_pGbFreq[iTabIdx] == nullptr) this->m_pGbFreq[iTabIdx] = new QGroupBox(this->m_pTabProfile[iTabIdx]);
    this->m_pGbFreq[iTabIdx]->setObjectName(QStringLiteral("gbFreq"));
    this->m_pGbFreq[iTabIdx]->setGeometry(QRect(10, 170, 341, 131));
    this->m_pGbFreq[iTabIdx]->setTitle(TRANSLATE(L"fUi_gbFreq"));
    this->m_pGbFreq[iTabIdx]->setCheckable(false);

    if (this->m_pLcdFreq[iTabIdx] == nullptr) this->m_pLcdFreq[iTabIdx] = new QLCDNumber(this->m_pGbFreq[iTabIdx]);
    this->m_pLcdFreq[iTabIdx]->setObjectName(QStringLiteral("lcdFreq"));
    this->m_pLcdFreq[iTabIdx]->setGeometry(QRect(40, 30, 131, 51));
    this->m_pLcdFreq[iTabIdx]->setFrameShape(QFrame::Panel);
    this->m_pLcdFreq[iTabIdx]->setFrameShadow(QFrame::Sunken);
    this->m_pLcdFreq[iTabIdx]->setSmallDecimalPoint(false);
    this->m_pLcdFreq[iTabIdx]->setSegmentStyle(QLCDNumber::Flat);
    this->m_pLcdFreq[iTabIdx]->setWhatsThis(TRANSLATE(L"fUi_WhatIsLcdFreq"));

    if (this->m_pDialFreq[iTabIdx] == nullptr) this->m_pDialFreq[iTabIdx] = new QDial(this->m_pGbFreq[iTabIdx]);
    this->m_pDialFreq[iTabIdx]->setObjectName(QStringLiteral("dialFreq"));
    this->m_pDialFreq[iTabIdx]->setGeometry(QRect(210, 30, 91, 81));
    this->m_pDialFreq[iTabIdx]->setFocusPolicy(Qt::TabFocus);
    this->m_pDialFreq[iTabIdx]->setMinimum(1);
    this->m_pDialFreq[iTabIdx]->setInvertedAppearance(false);
    this->m_pDialFreq[iTabIdx]->setInvertedControls(false);
    this->m_pDialFreq[iTabIdx]->setWrapping(false);
    this->m_pDialFreq[iTabIdx]->setNotchesVisible(true);
    this->m_pDialFreq[iTabIdx]->setWhatsThis(TRANSLATE(L"fUi_WhatIsDialFreq"));

    if (this->m_pPbSetFreq[iTabIdx] == nullptr) this->m_pPbSetFreq[iTabIdx] = new QPushButton(this->m_pGbFreq[iTabIdx]);
    this->m_pPbSetFreq[iTabIdx]->setObjectName(QStringLiteral("pbSetFreq"));
    this->m_pPbSetFreq[iTabIdx]->setGeometry(QRect(40, 90, 61, 21));
    this->m_pPbSetFreq[iTabIdx]->setText(TRANSLATE(L"fUi_pbSetFreq"));
    this->m_pPbSetFreq[iTabIdx]->setWhatsThis(TRANSLATE(L"fUi_WhatIsSetFreq"));

    if (this->m_pPbNextFreq[iTabIdx] == nullptr) this->m_pPbNextFreq[iTabIdx] = new QPushButton(this->m_pGbFreq[iTabIdx]);
    this->m_pPbNextFreq[iTabIdx]->setObjectName(QStringLiteral("pbNextFreq"));
    this->m_pPbNextFreq[iTabIdx]->setGeometry(QRect(110, 90, 61, 21));
    this->m_pPbNextFreq[iTabIdx]->setText(TRANSLATE(L"fUi_pbNextFreq"));
    this->m_pPbNextFreq[iTabIdx]->setWhatsThis(TRANSLATE(L"fUi_WhatIsNextFreq"));

    // General group
    if (this->m_pGbGeneral[iTabIdx] == nullptr) this->m_pGbGeneral[iTabIdx] = new QGroupBox(this->m_pTabProfile[iTabIdx]);
    this->m_pGbGeneral[iTabIdx]->setObjectName(QStringLiteral("gbGeneral"));
    this->m_pGbGeneral[iTabIdx]->setGeometry(QRect(10, 10, 341, 151));
    this->m_pGbGeneral[iTabIdx]->setTitle(TRANSLATE(L"fUi_gbGeneral"));
    this->m_pGbGeneral[iTabIdx]->setCheckable(false);

    if (this->m_pDialState[iTabIdx] == nullptr) this->m_pDialState[iTabIdx] = new QDial(this->m_pGbGeneral[iTabIdx]);
    this->m_pDialState[iTabIdx]->setObjectName(QStringLiteral("dial"));
    this->m_pDialState[iTabIdx]->setGeometry(QRect(80, 47, 50, 64));
    this->m_pDialState[iTabIdx]->setMaximum(4);
    this->m_pDialState[iTabIdx]->setWrapping(true);
    this->m_pDialState[iTabIdx]->setNotchesVisible(true);
    this->m_pDialState[iTabIdx]->setWhatsThis(TRANSLATE(L"fUi_WhatIsDialState"));

    if (this->m_pLabelOff[iTabIdx] == nullptr) this->m_pLabelOff[iTabIdx] = new QLabel(this->m_pGbGeneral[iTabIdx]);
    this->m_pLabelOff[iTabIdx]->setObjectName(QStringLiteral("lbOff"));
    this->m_pLabelOff[iTabIdx]->setGeometry(QRect(80, 107, 50, 21));
    this->m_pLabelOff[iTabIdx]->setFont(cFontLabel);
    this->m_pLabelOff[iTabIdx]->setStyleSheet(QStringLiteral("background-color: rgb(255, 0, 0)"));
    this->m_pLabelOff[iTabIdx]->setText(QStringLiteral("OFF"));
    this->m_pLabelOff[iTabIdx]->setAlignment(Qt::AlignCenter);

    if (this->m_pLabelNormal[iTabIdx] == nullptr) this->m_pLabelNormal[iTabIdx] = new QLabel(this->m_pGbGeneral[iTabIdx]);
    this->m_pLabelNormal[iTabIdx]->setObjectName(QStringLiteral("lbNormal"));
    this->m_pLabelNormal[iTabIdx]->setGeometry(QRect(0, 70, 71, 20));
    this->m_pLabelNormal[iTabIdx]->setFont(cFontLabel);
    this->m_pLabelNormal[iTabIdx]->setText(QStringLiteral("Normal"));
    this->m_pLabelNormal[iTabIdx]->setAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);

    if (this->m_pLabelSquelch[iTabIdx] == nullptr) this->m_pLabelSquelch[iTabIdx] = new QLabel(this->m_pGbGeneral[iTabIdx]);
    this->m_pLabelSquelch[iTabIdx]->setObjectName(QStringLiteral("lbSquelch"));
    this->m_pLabelSquelch[iTabIdx]->setGeometry(QRect(140, 70, 71, 20));
    this->m_pLabelSquelch[iTabIdx]->setFont(cFontLabel);
    this->m_pLabelSquelch[iTabIdx]->setText(QStringLiteral("Squelch"));
    this->m_pLabelSquelch[iTabIdx]->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);

    if (this->m_pLabelMute[iTabIdx] == nullptr) this->m_pLabelMute[iTabIdx] = new QLabel(this->m_pGbGeneral[iTabIdx]);
    this->m_pLabelMute[iTabIdx]->setObjectName(QStringLiteral("lbMute"));
    this->m_pLabelMute[iTabIdx]->setGeometry(QRect(80, 30, 50, 20));
    this->m_pLabelMute[iTabIdx]->setFont(cFontLabel);
    this->m_pLabelMute[iTabIdx]->setText(QStringLiteral("Mute"));
    this->m_pLabelMute[iTabIdx]->setAlignment(Qt::AlignCenter);

    if (this->m_pSwitchPriority[iTabIdx] == nullptr) this->m_pSwitchPriority[iTabIdx] = new SwitchButton(this->m_pGbGeneral[iTabIdx]); // Default style is Style::ONOFF
    this->m_pSwitchPriority[iTabIdx]->setObjectName(QStringLiteral("widPriority"));
    this->m_pSwitchPriority[iTabIdx]->setGeometry(QRect(240, 70, 71, 21));
    this->m_pSwitchPriority[iTabIdx]->setWhatsThis(TRANSLATE(L"fUi_WhatIsPriority"));

    if (this->m_pLabelPriority[iTabIdx] == nullptr) this->m_pLabelPriority[iTabIdx] = new QLabel(this->m_pGbGeneral[iTabIdx]);
    this->m_pLabelPriority[iTabIdx]->setObjectName(QStringLiteral("lbPriority"));
    this->m_pLabelPriority[iTabIdx]->setGeometry(QRect(225, 100, 90, 41));
    this->m_pLabelPriority[iTabIdx]->setFont(cFontLabel);
    this->m_pLabelPriority[iTabIdx]->setText(TRANSLATE(L"fUi_lbPriority"));
    this->m_pLabelPriority[iTabIdx]->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    this->m_pLabelPriority[iTabIdx]->setWordWrap(true);
    this->m_pLabelPriority[iTabIdx]->setWhatsThis(TRANSLATE(L"fUi_WhatIsPriority"));

    //add "Tab" to tabProfile
    this->m_cUi.tabProfile->addTab(this->m_pTabProfile[iTabIdx], QString());

    // set tab name
    QString sTemp = QString::number(iProfileIdx + 1);
    sTemp.append(". ");
    sTemp.append(this->m_cLocalConfigData.s_get_ProfileName(iProfileIdx).c_str());
    this->m_cUi.tabProfile->setTabText(this->m_cUi.tabProfile->indexOf(this->m_pTabProfile[iTabIdx]), sTemp);

    //set values from config container
    set_profile_tab(iTabIdx, iProfileIdx);

    // link events to handler
    connect(this->m_pDialFreq[iTabIdx],         &QDial::valueChanged,       this,   &wm2000_freq_ui::handler_freqdial_value_changed);
    connect(this->m_pPbSetFreq[iTabIdx],        &QPushButton::clicked,      this,   &wm2000_freq_ui::handler_pbSetFreq_clicked);
    connect(this->m_pPbNextFreq[iTabIdx],       &QPushButton::clicked,      this,   &wm2000_freq_ui::handler_pbNextFreq_clicked);
    connect(this->m_pDialState[iTabIdx],        &QDial::valueChanged,       this,   &wm2000_freq_ui::handler_statedial_value_changed);
    connect(this->m_pSwitchPriority[iTabIdx],   &SwitchButton::valueChanged,this,   &wm2000_freq_ui::handler_swPriority_valueChanged);
    return;
}

/*
*   set values in actual tab
*/
void wm2000_freq_ui::set_profile_tab(int iTabIdx, int iProfileIdx)
{
    CALL_STACK
    // Frequency group
    this->m_pLcdFreq[iTabIdx]->setEnabled(this->m_cLocalConfigData.s_get_ActiveFreq(iProfileIdx) != 0);
    set_lcd_display(iTabIdx, this->m_cLocalConfigData.s_get_ActiveFreq(iProfileIdx));

    this->m_pDialFreq[iTabIdx]->setMaximum(this->m_cLocalConfigData.s_get_MaxNumFreq());
    this->m_pDialFreq[iTabIdx]->setValue(this->m_cLocalConfigData.s_get_ActiveFreq(iProfileIdx));
    this->m_pDialFreq[iTabIdx]->setEnabled(this->m_cLocalConfigData.s_get_ActiveFreq(iProfileIdx) != 0);

    this->m_pPbSetFreq[iTabIdx]->setEnabled(this->m_cLocalConfigData.s_get_ActiveFreq(iProfileIdx) != 0);

    this->m_pPbNextFreq[iTabIdx]->setEnabled(this->m_cLocalConfigData.s_get_ActiveFreq(iProfileIdx) != 0);

    // General group
    if (this->m_cLocalConfigData.s_get_ActiveFreq(iProfileIdx) == 0)
        this->m_pDialState[iTabIdx]->setValue(STATE_OFF);
    else if (this->m_cLocalConfigData.s_get_MuteFreq(iProfileIdx))
        this->m_pDialState[iTabIdx]->setValue(STATE_MUTE);
    else if (this->m_cLocalConfigData.s_get_SquelchFreq(iProfileIdx))
        this->m_pDialState[iTabIdx]->setValue(STATE_SQUELCH);
    else
        this->m_pDialState[iTabIdx]->setValue(STATE_NORMAL);

    this->m_pSwitchPriority[iTabIdx]->setValue(this->m_cLocalConfigData.s_get_PrioFreq(iProfileIdx));
    this->m_pSwitchPriority[iTabIdx]->setEnabled(this->m_cLocalConfigData.s_get_ActiveFreq(iProfileIdx) != 0);
    return;
}