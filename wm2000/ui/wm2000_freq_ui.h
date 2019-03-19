#pragma once

#include <QtWidgets/QDialog>
#include <boost\thread.hpp>
#include "ui_wm2000_freq_ui.h"
#include "misc/client_filter.h"
#include "misc/channel_filter.h"
#include "misc/error_handler.h"
#include "misc/language_pkg.h"
#include "misc/config_container.h"

#include "SwitchButton.h"

class wm2000_freq_ui : public QDialog
{
    Q_OBJECT
public:
    enum State
    {
        STATE_OFF       = 0,
        STATE_NORMAL    = 1,
        STATE_MUTE      = 2,
        STATE_SQUELCH   = 3,
        STATE_OFF2      = 4
    };

public:
    wm2000_freq_ui(config_container *pcConfigData, QIcon *pIconWindow, QIcon *pcIgnoreIcon, QIcon *pcFavIcon, QWidget *parent = Q_NULLPTR);

    //update function for plugin_base
    void update_ui() { this->m_cUi.treeRxList->create_tree_entry(this->m_iFreqProfileIdx[this->m_iActualTabIndex]); };
    void update_config();

    //client filter interface
    void add_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter);
    void delete_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter);

    //event handler
    void handler_pbOk_clicked();
    void handler_pbApply_clicked();
    void handler_pbCancel_clicked();
    void handler_freqdial_value_changed(int iValue);
    void handler_statedial_value_changed(int iValue);
    void handler_pbSetFreq_clicked();
    void handler_pbNextFreq_clicked();
    void handler_swPriority_valueChanged(bool bValue);
    void handler_tabProfile_currentItemChanged(int index);

private:
    void init_profile_tab();
    void create_profile_tab(int iTabIdx, int iProfileIdx);
    void set_profile_tab(int iTabIdx, int iProfileIdx);
    void set_lcd_display(int iIndex, int iValue);

signals:
    void config_updated();

private:
    Ui::fmFreq              m_cUi;              // ui
    config_container       *m_pcConfigData;     // pointer to config data
    config_container        m_cLocalConfigData; // local copy of config data
    language_pkg            m_cTranslate;       // language converter
    error_handler           m_cErrHandler;      // link to error handler

    int                     m_iFreqProfileCnt;
    int                     m_iFreqProfileIdx[REAL_MAXNUMPROFILES];
    int                     m_iActualTabIndex;

    //array of generic tabs
    QWidget                *m_pTabProfile[REAL_MAXNUMPROFILES];
    QGroupBox              *m_pGbFreq[REAL_MAXNUMPROFILES];
    QLCDNumber             *m_pLcdFreq[REAL_MAXNUMPROFILES];
    QDial                  *m_pDialFreq[REAL_MAXNUMPROFILES];
    QPushButton            *m_pPbSetFreq[REAL_MAXNUMPROFILES];
    QPushButton            *m_pPbNextFreq[REAL_MAXNUMPROFILES];
    QGroupBox              *m_pGbGeneral[REAL_MAXNUMPROFILES];

    QDial                  *m_pDialState[REAL_MAXNUMPROFILES];
    QLabel                 *m_pLabelOff[REAL_MAXNUMPROFILES];
    QLabel                 *m_pLabelNormal[REAL_MAXNUMPROFILES];
    QLabel                 *m_pLabelMute[REAL_MAXNUMPROFILES];
    QLabel                 *m_pLabelSquelch[REAL_MAXNUMPROFILES];
    QLabel                 *m_pLabelPriority[REAL_MAXNUMPROFILES];
    SwitchButton           *m_pSwitchPriority[REAL_MAXNUMPROFILES];
};
