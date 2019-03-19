#pragma once
#include <QtWidgets/QTreeWidget>
#include <qaction.h>
#include <qmenu.h>
#include <boost\thread.hpp>

#include "misc/client_filter.h"
#include "misc/channel_filter.h"
#include "misc/error_handler.h"
#include "misc/language_pkg.h"
#include "misc/config_container.h"
#include "misc/language_pkg.h"

class clientTreeWidget :
    public QTreeWidget
{
    Q_OBJECT

public:
    clientTreeWidget(QWidget *parent = Q_NULLPTR);
    ~clientTreeWidget();

    //init pointer
    void init(config_container *pcConfigData, QIcon *pcIgnoreIcon, QIcon *pcFavIcon);

    //client filter interface
    void add_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter);
    void delete_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter);

    void set_language(std::string sNewLanguage);
    void create_tree_entry(int iActProfile);
    void update_meta_data();

    int  get_NextUnusedFreq();

protected:
    void prepare_context_menu(const QPoint & pos);

    void create_tree_entry_freq(int iActProfile);
    void create_tree_entry_level(int iActProfile);
    void create_tree_entry_fav(int iActProfile);
    void create_tree_entry_ignore();

    enum eItemRole
    {
        ROLE_TYPE = Qt::UserRole,
        ROLE_ID
    };

    enum eItemType
    {
        ITEM_MIN,
        ITEM_SERVER = ITEM_MIN,
        ITEM_CHANNEL,
        ITEM_CLIENT,
        ITEM_MAX = ITEM_CLIENT
    };

signals:
    void channel_ignore_triggered(int iProfile, uint64 nChannel);
    void channel_remove_triggered(int iProfile, uint64 nChannel);

protected:
    boost::mutex                m_cClientTreeMutex; // mutex to get possibility to write to client list TreeWidget from different thread
    int                         m_iActProfile;      // latest used profile
    config_container           *m_pcConfigData;     // pointer to config data
    language_pkg                m_cTranslate;       // language converter
    error_handler               m_cErrHandler;      // link to error handler

    QMenu                      *m_pcContextMenu;    // context menu
    QAction                    *m_pcActionIgnore;   // context menu action to add channel to Ignore-list
    QMetaObject::Connection     m_cConnIgnore;      // helper to disconnect
    QAction                    *m_pcActionDelete;   // context menu action to remove channel from Favorite-list
    QMetaObject::Connection     m_cConnDelete;      // helper to disconnect

    std::vector<client_filter*>  m_vcClientFilter;   // vector of client filter for all connected ServerTabs
    std::vector<channel_filter*> m_vcChannelFilter;  // vector of channel filter for all connected ServerTabs

    QFont                       m_cServerFont;
    QFont                       m_cChFont;
    QTreeWidgetItem            *m_qTreeWidgetItem;
};

