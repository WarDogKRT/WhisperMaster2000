#include "clientTreeWidget.h"

#define TRANSLATE(a) QString(this->m_cTranslate.translate(a).c_str())
#if USE_CALL_STACK
#define CALL_STACK if(USE_CALL_STACK) this->m_cErrHandler.message_callstack(__FUNCSIG__);
#else
#define CALL_STACK
#endif

/*
*   Constructor of widget
*/
clientTreeWidget::clientTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    CALL_STACK
    m_pcConfigData      = nullptr;
    m_qTreeWidgetItem   = nullptr;
    m_iActProfile       = -1;
    m_pcActionIgnore    = nullptr;
    m_pcActionDelete    = nullptr;

    // prepare fonts
    m_cServerFont.setBold(true);
    m_cServerFont.setUnderline(true);
    m_cServerFont.setWeight(75);

    m_cChFont.setBold(true);
    m_cChFont.setWeight(75);
}


void clientTreeWidget::init(config_container *pcConfigData, QIcon *pcIgnoreIcon, QIcon *pcFavIcon)
{
    CALL_STACK
    m_pcConfigData = pcConfigData;

    //init header
    this->clear();
    m_qTreeWidgetItem = new QTreeWidgetItem();
    this->setHeaderItem(m_qTreeWidgetItem);
    this->setColumnWidth(0, 200);

    //add context menu
    m_pcActionIgnore = new QAction(this);
    m_pcActionIgnore->setIcon(*pcIgnoreIcon);
    m_pcActionDelete = new QAction(this);
    m_pcActionDelete->setIcon(*pcFavIcon);

    m_pcContextMenu = new QMenu(this);
    m_pcContextMenu->addAction(m_pcActionIgnore);
    m_pcContextMenu->addAction(m_pcActionDelete);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeWidget::customContextMenuRequested, this, &clientTreeWidget::prepare_context_menu);

    //add text
    this->set_language(this->m_pcConfigData->s_get_Language());

}


/*
*   update language where needed
*/
void clientTreeWidget::set_language(std::string sNewLanguage)
{
    CALL_STACK
    //update translator
    this->m_cTranslate.set_language(sNewLanguage);

    //update all "user"
    m_qTreeWidgetItem->setText(0, TRANSLATE(L"ctUi_treeHeaderRx"));
    m_qTreeWidgetItem->setText(1, TRANSLATE(L"ctUi_treeHeaderState"));
    this->setWhatsThis(TRANSLATE(L"ctUi_WhatIsTreeItem"));

    this->m_pcActionIgnore->setText(TRANSLATE(L"ctUi_contextIgnore"));
    this->m_pcActionIgnore->setIconText(TRANSLATE(L"ctUi_contextIgnore"));
    this->m_pcActionIgnore->setToolTip(TRANSLATE(L"ctUi_contextIgnoreTool"));

    this->m_pcActionDelete->setText(TRANSLATE(L"ctUi_contextFav"));
    this->m_pcActionDelete->setIconText(TRANSLATE(L"ctUi_contextFav"));
    this->m_pcActionDelete->setToolTip(TRANSLATE(L"ctUi_contextFavTool"));

    create_tree_entry(m_iActProfile);
}


/*
*   destructor of widget
*/
clientTreeWidget::~clientTreeWidget()
{
    CALL_STACK
}

/*
*   prepare context menu
*/
void clientTreeWidget::prepare_context_menu(const QPoint & pos)
{
    CALL_STACK
    //get info about the selected item and bail out early, if no element is selected
    QTreeWidgetItem *nd = this->itemAt(pos);
    
    //if the Profile index or clientList is invalid, bail out early
    if ((nd != nullptr) && (this->m_vcClientFilter.size() != 0))
    {
        eItemType eType = (eItemType)nd->data(0, ROLE_TYPE).toInt();
        uint64    nItemId = nd->data(0, ROLE_ID).toULongLong();

        if ((eType >= ITEM_MIN) && (eType <= ITEM_MAX))
        {
            bool isFav = false;
            bool isIgnore = false;
            if (m_iActProfile < 0)
                isIgnore = true;
            else
                isFav = this->m_pcConfigData->s_get_ProfileType(m_iActProfile) == PROFILE_FAVORITE;

            // create menu
            m_pcActionIgnore->setEnabled((eType == eItemType::ITEM_CHANNEL) && !isIgnore);
            disconnect(m_cConnIgnore);
            m_cConnIgnore = connect(m_pcActionIgnore, &QAction::triggered, this, [=]() { emit channel_ignore_triggered(m_iActProfile, nItemId); });
            m_pcActionDelete->setEnabled((eType == eItemType::ITEM_CHANNEL) && (isFav || isIgnore));
            disconnect(m_cConnDelete);
            m_cConnDelete = connect(m_pcActionDelete, &QAction::triggered, this, [=]() { emit channel_remove_triggered(m_iActProfile, nItemId); });

            //show menu
            QPoint pt(pos);
            m_pcContextMenu->exec(this->mapToGlobal(pos));
        }
    }
    return;
}


/*
*   add entity of client filter
*/
void clientTreeWidget::add_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter)
{
    CALL_STACK
    //lock other threads
    this->m_cClientTreeMutex.lock();

    //try to find requested elements
    bool bItemFound = false;
    for (std::vector<client_filter*>::iterator it = this->m_vcClientFilter.begin(); it != this->m_vcClientFilter.end(); it++)
    {
        if (*it == pcClientFilter)
        {
            bItemFound = true;
            break;
        }
    }
    //add entry
    if(!bItemFound)
        this->m_vcClientFilter.push_back(pcClientFilter);

    bItemFound = false;
    for (std::vector<channel_filter*>::iterator it = this->m_vcChannelFilter.begin(); it != this->m_vcChannelFilter.end(); it++)
    {
        if (*it == pcChannelFilter)
        {
            bItemFound = true;
            break;
        }
    }
    //add entry
    if (!bItemFound)
        this->m_vcChannelFilter.push_back(pcChannelFilter);

    //request unlock
    this->m_cClientTreeMutex.unlock();

    //update list field
    create_tree_entry(m_iActProfile);
}

/*
*   delete entity of client_filter
*/
void clientTreeWidget::delete_pointer(client_filter *pcClientFilter, channel_filter* pcChannelFilter)
{
    CALL_STACK
    //lock other threads
    this->m_cClientTreeMutex.lock();

    //try to find requested element and delete element
    for (std::vector<client_filter*>::iterator it = this->m_vcClientFilter.begin(); it != this->m_vcClientFilter.end(); it++)
    {
        if (*it == pcClientFilter)
        {
            this->m_vcClientFilter.erase(it);
            break;
        }
    }
    for (std::vector<channel_filter*>::iterator it = this->m_vcChannelFilter.begin(); it != this->m_vcChannelFilter.end(); it++)
    {
        if (*it == pcChannelFilter)
        {
            this->m_vcChannelFilter.erase(it);
            break;
        }
    }

    //request unlock
    this->m_cClientTreeMutex.unlock();

    //update list field
    create_tree_entry(m_iActProfile);
}


/*
*   update meta data on Server
*/
void clientTreeWidget::update_meta_data()
{
    //update frequency at all server
    this->m_cClientTreeMutex.lock();
    for (std::vector<client_filter*>::iterator it = this->m_vcClientFilter.begin(); it != this->m_vcClientFilter.end(); it++)
    {
        (*it)->m_cClientListMutex.lock();
        (*it)->set_meta_data();
        (*it)->m_cClientListMutex.unlock();
    }
    this->m_cClientTreeMutex.unlock();
}


/*
*   return next frequency, that is not used bei any active client
*/
int clientTreeWidget::get_NextUnusedFreq()
{
    CALL_STACK
    int iFreq = -1;
    //lock client tree
    this->m_cClientTreeMutex.lock();

    //if no clients connected
    if (this->m_vcClientFilter.size() != 0)
    {
        //find frequency that is free in all instances
        for (std::vector<client_filter*>::iterator it = this->m_vcClientFilter.begin(); it != this->m_vcClientFilter.end(); it++)
        {
            //use iFreq-1 to make sure, that even if first instance sets one freq as free, that it is checked with all following instances
            iFreq = (*it)->get_next_free_freq(iFreq - 1);
            if (iFreq == 0)
                break;
        }
    }

    //request unlock
    this->m_cClientTreeMutex.unlock();

    return iFreq;
}


/*
*   update tree client/channel list
*/
void clientTreeWidget::create_tree_entry(int iActProfile)
{
    CALL_STACK
    //make sure, that we are alone from now on
    this->m_cClientTreeMutex.lock();

    //clear all old entries
    this->clear();

    m_iActProfile = iActProfile;

    //if the Profile index or clientList is invalid, bail out early
    if ((this->m_vcClientFilter.size() != 0) && (this->m_vcChannelFilter.size() != 0) && (iActProfile >= 0))
    {
        //call
        switch (this->m_pcConfigData->s_get_ProfileType(iActProfile))
        {
        case PROFILE_OFF:
            break;
        case PROFILE_LEVEL:
            create_tree_entry_level(iActProfile);
            break;
        case PROFILE_FAVORITE:
            create_tree_entry_fav(iActProfile);
            break;
        case PROFILE_FREQUENCY:
            create_tree_entry_freq(iActProfile);
            break;
        case PROFILE_AUDIO:
            break;
        default:
            break;
        }
    }
    else
    {
        create_tree_entry_ignore();
    }

    //set default width
    this->setColumnWidth(0, (this->columnWidth(0) > 200) ? this->columnWidth(0) : 200);

    //work done with client tree, release mutex
    this->m_cClientTreeMutex.unlock();
    return;
}


/*
*   update tree client/channel list, if profile is of type FREQUENCY
*/
void clientTreeWidget::create_tree_entry_freq(int iActProfile)
{
    CALL_STACK
    for (std::vector<client_filter*>::iterator it = this->m_vcClientFilter.begin(); it != this->m_vcClientFilter.end(); it++)
    {
        //make sure, that no one writes to ClientList variable while we are reading
        (*it)->m_cClientListMutex.lock();

        int iActFreq = this->m_pcConfigData->s_get_ActiveFreq(iActProfile);
        std::vector<int> vClientIdx = (*it)->get_client_list_idx(iActFreq, false, false);


        QStringList cEntry;

        //create Server Item for first client that is in list
        cEntry.clear();
        cEntry << QString::fromStdString((*it)->get_server_name()) << TRANSLATE(L"ctUi_treeItemStateNone");
        QTreeWidgetItem *ServerParent = new QTreeWidgetItem(this, cEntry);
        ServerParent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        ServerParent->setFont(0, m_cServerFont);
        ServerParent->setData(0, ROLE_TYPE, eItemType::ITEM_SERVER);
        ServerParent->setData(0, ROLE_ID, (*it)->get_server_id());

        size_t nClientIndex = 0;
        while (!vClientIdx.empty())
        {
            //create Channel Item for first client that is in list
            uint64 nActualChannelID = (*it)->m_cClientList[vClientIdx[0]].nActualChannelID;
            cEntry.clear();
            cEntry << QString::fromStdString((*it)->get_channel_name(nActualChannelID)) << TRANSLATE(L"ctUi_treeItemStateNone");
            QTreeWidgetItem *ChannelParent = new QTreeWidgetItem(ServerParent, cEntry);
            ChannelParent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            ChannelParent->setFont(0, m_cChFont);
            ChannelParent->setData(0, ROLE_TYPE, eItemType::ITEM_CHANNEL);
            ChannelParent->setData(0, ROLE_ID, nActualChannelID);

            nClientIndex = 0;
            while ((!vClientIdx.empty()) && (nClientIndex < vClientIdx.size()))
            {
                //create Client Items for all clients with the actual channel ID and delete them from the list
                if (nActualChannelID == (*it)->m_cClientList[vClientIdx[nClientIndex]].nActualChannelID)
                {
                    cEntry.clear();
                    QString sState;
                    //get state of user and create new item
                    int iActFreqSet = (*it)->find_active_freq(vClientIdx[nClientIndex], iActFreq, false, false);
                    if (iActFreqSet < 0)
                        sState = TRANSLATE(L"ctUi_treeItemStateError");
                    else if ((*it)->m_cClientList[vClientIdx[nClientIndex]].acFreqList[iActFreqSet].nBit.nMute == 1)
                        sState = TRANSLATE(L"ctUi_treeItemStateMute");
                    else if ((*it)->m_cClientList[vClientIdx[nClientIndex]].acFreqList[iActFreqSet].nBit.nSquelch == 1)
                        sState = TRANSLATE(L"ctUi_treeItemStateSquelch");
                    else if ((*it)->m_cClientList[vClientIdx[nClientIndex]].acFreqList[iActFreqSet].nBit.nPriority == 1)
                        sState = TRANSLATE(L"ctUi_treeItemStatePriority");
                    else
                        sState = TRANSLATE(L"ctUi_treeItemStateActive");
                    cEntry << QString::fromStdString((*it)->m_cClientList[vClientIdx[nClientIndex]].sClientName) << sState;

                    //add item to list
                    QTreeWidgetItem *ClientParent = new QTreeWidgetItem(ChannelParent, cEntry);
                    ClientParent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
                    ClientParent->setData(0, ROLE_TYPE, eItemType::ITEM_CLIENT);
                    ClientParent->setData(0, ROLE_ID, (uint64)((*it)->m_cClientList[vClientIdx[nClientIndex]].nClientID));
                    vClientIdx.erase(vClientIdx.begin() + nClientIndex);
                }
                nClientIndex++;
            }
            this->expandItem(ChannelParent);
        }
        this->expandItem(ServerParent);

        //work done with this client, release mutex
        (*it)->m_cClientListMutex.unlock();
    }
    return;
}


/*
*   update tree client/channel list, if profile is of type LEVEL
*/
void clientTreeWidget::create_tree_entry_level(int iActProfile)
{
    CALL_STACK
    for (std::vector<channel_filter*>::iterator it = this->m_vcChannelFilter.begin(); it != this->m_vcChannelFilter.end(); it++)
    {
        QStringList cEntry;

        //create Server Item for first client that is in list
        cEntry.clear();
        cEntry << QString::fromStdString((*it)->get_server_name()) << TRANSLATE(L"ctUi_treeItemStateNone");
        QTreeWidgetItem *ServerParent = new QTreeWidgetItem(this, cEntry);
        ServerParent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        ServerParent->setFont(0, m_cServerFont);
        ServerParent->setData(0, ROLE_TYPE, eItemType::ITEM_SERVER);
        ServerParent->setData(0, ROLE_ID, (*it)->get_server_id());

        // print channel list based on same filter like hotkey
        QTreeWidgetItem *ChannelParent;
        uint64 *pnFilteredList = (*it)->filter_channel_from_level(this->m_pcConfigData->s_get_MinChLevel(iActProfile), this->m_pcConfigData->s_get_MaxChLevel(iActProfile), false);
        if (pnFilteredList != nullptr)
        {
            for (int jj = 0; pnFilteredList[jj] != 0; jj++)
            {
                bool bIgnored = false;
                if (((*it)->find_channel_in_list(this->m_pcConfigData->s_get_IgnoreList(), pnFilteredList[jj]) >= 0) && this->m_pcConfigData->s_get_UseIgnoreListTx(iActProfile))
                    bIgnored = true;
                
                cEntry.clear();
                cEntry << QString::fromStdString((*it)->get_channel_name(pnFilteredList[jj])) << (bIgnored ? TRANSLATE(L"ctUi_Ignore") : (TRANSLATE(L"ctUi_Level") + QString::number((*it)->get_channel_level(pnFilteredList[jj]))));
                ChannelParent = new QTreeWidgetItem(ServerParent, cEntry);
                ChannelParent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
                ChannelParent->setFont(0, m_cChFont);
                ChannelParent->setData(0, ROLE_TYPE, eItemType::ITEM_CHANNEL);
                ChannelParent->setData(0, ROLE_ID, pnFilteredList[jj]);
            }
            // free filtered list after function is done
            (*it)->free_channel_list(pnFilteredList);
        }
        else
        {
            cEntry.clear();
            cEntry << TRANSLATE(L"ctUi_NoChannel") << QString("");
            ChannelParent = new QTreeWidgetItem(ServerParent, cEntry);
            ChannelParent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            ChannelParent->setFont(0, m_cChFont);
            ChannelParent->setData(0, ROLE_TYPE, eItemType::ITEM_CHANNEL);
            ChannelParent->setData(0, ROLE_ID, 0);
        }
        this->expandItem(ChannelParent);
        this->expandItem(ServerParent);
    }
    return;
}


/*
*   update tree client/channel list, if profile is of type FAVORITE
*/
void clientTreeWidget::create_tree_entry_fav(int iActProfile)
{
    CALL_STACK
    for (std::vector<channel_filter*>::iterator it = this->m_vcChannelFilter.begin(); it != this->m_vcChannelFilter.end(); it++)
    {
        QStringList cEntry;

        //create Server Item for first client that is in list
        cEntry.clear();
        cEntry << QString::fromStdString((*it)->get_server_name()) << TRANSLATE(L"ctUi_treeItemStateNone");
        QTreeWidgetItem *ServerParent = new QTreeWidgetItem(this, cEntry);
        ServerParent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        ServerParent->setFont(0, m_cServerFont);
        ServerParent->setData(0, ROLE_TYPE, eItemType::ITEM_SERVER);
        ServerParent->setData(0, ROLE_ID, (*it)->get_server_id());

        // print channel list based on same filter like hotkey
        QTreeWidgetItem *ChannelParent;
        uint64 *pnFilteredList = (*it)->filter_channel_from_list(this->m_pcConfigData->s_get_FavoriteList(iActProfile), this->m_pcConfigData->s_get_UseSubChOfFav(iActProfile), false);
        if (pnFilteredList != nullptr)
        {
            for (int jj = 0; pnFilteredList[jj] != 0; jj++)
            {
                bool bIsSubChannel = false;
                bool bIgnored = false;

                //if subchannel are used, try to find it in the master list
                if (this->m_pcConfigData->s_get_UseSubChOfFav(iActProfile))
                    if (this->m_pcConfigData->s_find_entry(this->m_pcConfigData->s_get_FavoriteList(iActProfile), (*it)->get_channel_info(pnFilteredList[jj])) < 0)
                        bIsSubChannel = true;;

                if (((*it)->find_channel_in_list(this->m_pcConfigData->s_get_IgnoreList(), pnFilteredList[jj]) >= 0) && this->m_pcConfigData->s_get_UseIgnoreListTx(iActProfile))
                    bIgnored = true;

                cEntry.clear();
                cEntry << QString::fromStdString((*it)->get_channel_name(pnFilteredList[jj])) << (bIgnored ? TRANSLATE(L"ctUi_Ignore") : (bIsSubChannel ? TRANSLATE(L"ctUi_SubChannel") : TRANSLATE(L"ctUi_Favorite")));
                ChannelParent = new QTreeWidgetItem(ServerParent, cEntry);
                ChannelParent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
                ChannelParent->setFont(0, m_cChFont);
                ChannelParent->setData(0, ROLE_TYPE, eItemType::ITEM_CHANNEL);
                ChannelParent->setData(0, ROLE_ID, pnFilteredList[jj]);
            }
            // free filtered list after function is done
            (*it)->free_channel_list(pnFilteredList);
        }
        else
        {
            cEntry.clear();
            cEntry << TRANSLATE(L"ctUi_NoChannel") << QString("");
            ChannelParent = new QTreeWidgetItem(ServerParent, cEntry);
            ChannelParent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            ChannelParent->setFont(0, m_cChFont);
            ChannelParent->setData(0, ROLE_TYPE, eItemType::ITEM_CHANNEL);
            ChannelParent->setData(0, ROLE_ID, 0);
        }
        this->expandItem(ChannelParent);
        this->expandItem(ServerParent);
    }
    return;
}

/*
*   update tree client/channel list, if ignore list is selected
*/
void clientTreeWidget::create_tree_entry_ignore()
{
    CALL_STACK
    for (std::vector<channel_filter*>::iterator it = this->m_vcChannelFilter.begin(); it != this->m_vcChannelFilter.end(); it++)
    {
        QStringList cEntry;

        //create Server Item for first client that is in list
        cEntry.clear();
        cEntry << QString::fromStdString((*it)->get_server_name()) << TRANSLATE(L"ctUi_treeItemStateNone");
        QTreeWidgetItem *ServerParent = new QTreeWidgetItem(this, cEntry);
        ServerParent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        ServerParent->setFont(0, m_cServerFont);
        ServerParent->setData(0, ROLE_TYPE, eItemType::ITEM_SERVER);
        ServerParent->setData(0, ROLE_ID, (*it)->get_server_id());

        // print channel list based on same filter like hotkey
        QTreeWidgetItem *ChannelParent;
        uint64 *pnFilteredList = (*it)->filter_channel_from_list(this->m_pcConfigData->s_get_IgnoreList(), false, false);
        if (pnFilteredList != nullptr)
        {
            for (int jj = 0; pnFilteredList[jj] != 0; jj++)
            {
                cEntry.clear();
                cEntry << QString::fromStdString((*it)->get_channel_name(pnFilteredList[jj])) << TRANSLATE(L"ctUi_Ignore");
                ChannelParent = new QTreeWidgetItem(ServerParent, cEntry);
                ChannelParent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
                ChannelParent->setFont(0, m_cChFont);
                ChannelParent->setData(0, ROLE_TYPE, eItemType::ITEM_CHANNEL);
                ChannelParent->setData(0, ROLE_ID, pnFilteredList[jj]);
            }
            // free filtered list after function is done
            (*it)->free_channel_list(pnFilteredList);
        }
        else
        {
            cEntry.clear();
            cEntry << TRANSLATE(L"ctUi_NoChannel") << QString("");
            ChannelParent = new QTreeWidgetItem(ServerParent, cEntry);
            ChannelParent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            ChannelParent->setFont(0, m_cChFont);
            ChannelParent->setData(0, ROLE_TYPE, eItemType::ITEM_CHANNEL);
            ChannelParent->setData(0, ROLE_ID, 0);
        }
        this->expandItem(ChannelParent);
        this->expandItem(ServerParent);
    }
    return;
}