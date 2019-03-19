#pragma once

#include <QtWidgets/qmainwindow.h>
#include <QStackedLayout>

#include "ui_wm2000_main_ui.h"
#include "misc/language_pkg.h"
#include "misc/error_handler.h"
#include "misc/config_container.h"
#include "misc/client_filter.h"
#include "misc/channel_filter.h"
#include "wm2000_about_ui.h"
#include "wm2000_freq_ui.h"

#include "ui/StyleSheets.h"
#include "SwitchButton.h"

class wm2000_main_ui : public QDialog
{
    Q_OBJECT

public:
    wm2000_main_ui();
    wm2000_main_ui(config_container *pcConfigData, void(*fUpdate_server)(), std::string sConfigPath, QWidget *parent = Q_NULLPTR);
    ~wm2000_main_ui();

    //update function for plugin_base
    virtual void update_ui() = 0;
    virtual void update_config() = 0;

    //client filter interface
    virtual void add_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter) = 0;
    virtual void delete_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter) = 0;
    void update_box_size();

public Q_SLOTS:
    //event handler
    void open_about_ui();
    void open_freq_ui();

protected:
    // init function for Tabs
    void create_menu(bool bTransOnly = false);
    void create_general_tab(bool bTransOnly = false);
    void create_profile_tab(int nIndex, bool bTransOnly=false);
    void create_led_group(QWidget* pTab, QGroupBox* &pGroupBox, QPushButton* &pPbLedColor, QPushButton* &pPbLedTest, QComboBox* &pCbKey, QComboBox* &pCbType, QLabel* &pLbColor, bool bTransOnly);
    void set_led_color_preview(QLabel* pLbColor, QColor cColor);
    void add_tabs_to_group();
    void update_language();
    void update_expert_mode();
    void set_box_size(QGroupBox* &pGroupBox, int dNewWidth);
    virtual void init_ui(QWidget *parent);

protected:
    Ui::fmMainWindow        m_cUi;              // ui
    wm2000_about_ui        *m_pAboutUi;         // about UI
    wm2000_freq_ui         *m_pFreqUi;          // freq UI

    config_container       *m_pcConfigData;     // pointer to config data
    config_container        m_cLocalConfigData; // local copy of config data
    std::string             m_sConfigPath;      // path to plugin configurations
    language_pkg            m_cTranslate;       // language converter
    error_handler           m_cErrHandler;      // link to error handler

    int                     m_iActTabIndex;

    void(*m_fUpdate_server)();

    QIcon                   m_cIgnoreIcon;
    QIcon                   m_cFavIcon;

    QIcon                   m_cWindowIcon;
    CustomTabStyle          m_cTabStyle;
    QIcon                   m_cLoadIcon;
    QAction*                m_pcLoadAction;
    QIcon                   m_cSaveIcon;
    QAction*                m_pcSaveAction;
    QAction*                m_pcSaveAsAction;
    QAction*                m_pcCloseAction;
    QIcon                   m_cUndoIcon;
    QAction*                m_pcUndoAction;
    QAction*                m_pcExpertAction;
    QAction*                m_pcFreqAction;
    QIcon                   m_cAboutIcon;
    QAction*                m_pcAboutAction;

    // UI references ("General" tab)
    QWidget*                m_pcTabGeneral;
    QGroupBox*              m_pcGroupGeneral;
    QSpinBox*               m_pcSpinNumProfile;
    QLabel*                 m_pcLabNumProfile;
    QComboBox*              m_pcCombLanguage;
    QLabel*                 m_pcLabLanguage;
    QLineEdit*              m_pcEdtHotkey_reset;
    QLabel*                 m_pcLabHotkey_reset;
    QLineEdit*              m_pcEdtHotkey_mute;
    QLabel*                 m_pcLabHotkey_mute;

    QGroupBox*              m_pcGroupIgnore;
    QPushButton*            m_pcPbDelIgnore;
    SwitchButton*           m_pcSwIgnoreUseRx;
    QLabel*                 m_pcLabIgnoreUseRx;
    SwitchButton*           m_pcSwAutoSave;
    QLabel*                 m_pcLabAutoSave;

    QGroupBox*              m_pcGroupLED;
    QPushButton*            m_pcPbLedColor;
    QPushButton*            m_pcPbLedTest;
    QComboBox*              m_pcCbKey;
    QComboBox*              m_pcCbType;
    QLabel*                 m_pcLbColor;

    // UI references ("Profile" tab)
    QWidget*                m_acTabProfile[REAL_MAXNUMPROFILES];

    QGroupBox*              m_acGroupGeneral[REAL_MAXNUMPROFILES];
    QLineEdit*              m_edProfileName[REAL_MAXNUMPROFILES];
    QLabel*                 m_lbProfileType[REAL_MAXNUMPROFILES];
    QComboBox*              m_cbProfileType[REAL_MAXNUMPROFILES];
    QLabel*                 m_lbProfileIgnoreTx[REAL_MAXNUMPROFILES];
    SwitchButton*           m_swProfileIgnoreTx[REAL_MAXNUMPROFILES];
    QLabel*                 m_lbProfilePtt[REAL_MAXNUMPROFILES];
    SwitchButton*           m_swProfilePtt[REAL_MAXNUMPROFILES];
    QLabel*                 m_lbProfileKeyDw[REAL_MAXNUMPROFILES];
    QLineEdit*              m_edProfileKeyDw[REAL_MAXNUMPROFILES];
    QLabel*                 m_lbProfileKeyUp[REAL_MAXNUMPROFILES];
    QLineEdit*              m_edProfileKeyUp[REAL_MAXNUMPROFILES];

    QStackedWidget*         m_acStackedTyp[REAL_MAXNUMPROFILES];
    QWidget*                m_widgetNoProfile[REAL_MAXNUMPROFILES];
    QGroupBox*              m_acGroupNoProfile[REAL_MAXNUMPROFILES];

    QWidget*                m_widgetFavorite[REAL_MAXNUMPROFILES];
    QGroupBox*              m_acGroupFavorite[REAL_MAXNUMPROFILES];
    QPushButton*            m_pbDeleteFav[REAL_MAXNUMPROFILES];
    QLabel*                 m_lbFavUseSub[REAL_MAXNUMPROFILES];
    SwitchButton*           m_swFavUseSub[REAL_MAXNUMPROFILES];

    QWidget*                m_widgetLevel[REAL_MAXNUMPROFILES];
    QGroupBox*              m_acGroupLevel[REAL_MAXNUMPROFILES];
    QLabel*                 m_lbLevLower[REAL_MAXNUMPROFILES];
    QSpinBox*               m_spLevLower[REAL_MAXNUMPROFILES];
    QLabel*                 m_lbLevUpper[REAL_MAXNUMPROFILES];
    QSpinBox*               m_spLevUpper[REAL_MAXNUMPROFILES];
    QCheckBox*              m_cbLevUpper[REAL_MAXNUMPROFILES];

    QWidget*                m_widgetFreq[REAL_MAXNUMPROFILES];
    QGroupBox*              m_acGroupFreq[REAL_MAXNUMPROFILES];
    QLabel*                 m_lbFreqFreq[REAL_MAXNUMPROFILES];
    QSpinBox*               m_spFreqFreq[REAL_MAXNUMPROFILES];
    QPushButton*            m_pbFreqNext[REAL_MAXNUMPROFILES];
    QLabel*                 m_lbFreqMode[REAL_MAXNUMPROFILES];
    QComboBox*              m_cbFreqMode[REAL_MAXNUMPROFILES];
    QLabel*                 m_lbFreqPrio[REAL_MAXNUMPROFILES];
    SwitchButton*           m_swFreqPrio[REAL_MAXNUMPROFILES];
    QLabel                 *m_pLabelMaster[REAL_MAXNUMPROFILES];
    SwitchButton           *m_pSwitchMaster[REAL_MAXNUMPROFILES];

    QGroupBox*              m_acGroupLED[REAL_MAXNUMPROFILES];
    QPushButton*            m_acPbLedColor[REAL_MAXNUMPROFILES];
    QPushButton*            m_acPbLedTest[REAL_MAXNUMPROFILES];
    QComboBox*              m_acCbKey[REAL_MAXNUMPROFILES];
    QComboBox*              m_acCbType[REAL_MAXNUMPROFILES];
    QLabel*                 m_acLbColor[REAL_MAXNUMPROFILES];
};
