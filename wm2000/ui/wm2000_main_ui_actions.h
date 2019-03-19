#pragma once
#include "wm2000_main_ui.h"


class wm2000_main_ui_actions :
    public wm2000_main_ui
{
    Q_OBJECT

public:
    wm2000_main_ui_actions() { return; };
    wm2000_main_ui_actions(config_container *pcConfigData, void(*fUpdate_server)(), std::string sConfigPath, QWidget *parent = Q_NULLPTR);
    ~wm2000_main_ui_actions();

    //update function for plugin_base
    void update_ui() override;
    void update_config() override;

    //client filter interface
    void add_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter) override;
    void delete_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter) override;

protected:
    bool check_LocalSettings();
    void load_base_config(config_container *pcConfigData, bool bInit = false);
    void init_ui(QWidget *parent) override;

public Q_SLOTS:
    void handler_pbOk_clicked();
    void handler_pbApply_clicked();
    void handler_SaveAs_clicked();
    void handler_Load_clicked();
    void handler_Undo_clicked();
    void handler_pbCancel_clicked();
    void handler_tabProfile_currentItemChanged(int index);

    void handler_delete_from_list_clicked(int iProfile, uint64 nChannel);
    void handler_add_to_ignore_clicked(int iProfile, uint64 nChannel);

    // General tab
    void handler_select_NumProfile(int iNewValue);
    void handler_select_Language(const QString &sText);
    void handler_select_IgnoreRx(bool bNewValue);
    void handler_select_IgnoreSave(bool bNewValue);
    void handler_push_DelIgnore();
    void handler_select_Expert();

    // Profile tab
    void handler_edit_ProfileName(const QString &sNewText);
    void handler_select_ProfileType(int iNewValue);
    void handler_select_IgnoreTx(bool bNewValue);
    void handler_select_ptt(bool bNewValue);
    void handler_select_UseSubCh(bool bNewValue);
    void handler_push_DelFavList();
    void handler_select_LowLevel(int iNewValue);
    void handler_select_UpLevel(int iNewValue);
    void handler_select_UpLevelActive(int iNewValue);
    void handler_select_Freq(int iNewValue);
    void handler_select_FreqMode(int iNewValue);
    void handler_pushed_NextFreq();
    void handler_select_Priority(bool bNewValue);
    void handler_select_Master(bool bNewValue);
};

