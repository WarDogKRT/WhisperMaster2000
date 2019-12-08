#include "wm2000_about_ui.h"
#include <QtCore/qfile.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qtextstream.h>

#define TRANSLATE(a) QString(this->m_cTranslate.translate(a).c_str())
#if USE_CALL_STACK
#define CALL_STACK if(USE_CALL_STACK) this->m_cErrHandler.message_callstack(__FUNCSIG__);
#else
#define CALL_STACK
#endif

/*
*   Constructor of UI
*/
wm2000_about_ui::wm2000_about_ui(std::string sConfigPath, QIcon *pIconWindow, QWidget *parent)
    : QDialog(parent)
{
    CALL_STACK
    // create ui with basic settings
    m_cUi.setupUi(this);

    //set default strings
    set_language(m_cTranslate.get_language());

    // set GPL text
    QString sFileName = QString::fromStdString(sConfigPath) + QString("license_gpl_v3.txt");
    if (QFileInfo(sFileName).exists())
    {
        QFile cFile(sFileName);
        cFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream cReadFile(&cFile);
        m_cUi.txt_gnu->setText(cReadFile.readAll());
    }
    else
        m_cUi.txt_gnu->setText(TRANSLATE("aUi_GnuError"));

    //add icon and set fixed window size
    this->setWindowIcon(*pIconWindow);
    this->setFixedSize(size());

    return;
}

/*
*   destructor of UI
*/
wm2000_about_ui::~wm2000_about_ui()
{
    CALL_STACK
}

/*
*   set language based Ui
*/
void wm2000_about_ui::set_language(std::string sNewLanguage)
{
    CALL_STACK
    //update Translate
    m_cTranslate.set_language(sNewLanguage);

    //remove all tabs from tab-group
    m_cUi.tab_license->clear();

    if (sNewLanguage.compare("de") == 0)
    {
        // add german license info tab
        m_cUi.tab_license->addTab(this->m_cUi.tab_general_de, QString());
        m_cUi.tab_license->setTabText(this->m_cUi.tab_license->indexOf(this->m_cUi.tab_general_de), QStringLiteral("Allgemein"));
        //m_cUi.tab_license->setTabToolTip(this->m_cUi.tab_license->indexOf(this->m_cUi.tab_general_de), TRANSLATE("mUi_TabGeneralTip"));
        //m_cUi.tab_license->setTabWhatsThis(this->m_cUi.tab_license->indexOf(this->m_cUi.tab_general_de), TRANSLATE("mUi_TabGeneralWhat"));
    }
    else
    {
        // add english license info tab
        m_cUi.tab_license->addTab(this->m_cUi.tab_general_en, QString());
        m_cUi.tab_license->setTabText(this->m_cUi.tab_license->indexOf(this->m_cUi.tab_general_en), QStringLiteral("General"));
        //m_cUi.tab_license->setTabToolTip(this->m_cUi.tab_license->indexOf(this->m_cUi.tab_general_en), TRANSLATE("mUi_TabGeneralTip"));
        //m_cUi.tab_license->setTabWhatsThis(this->m_cUi.tab_license->indexOf(this->m_cUi.tab_general_en), TRANSLATE("mUi_TabGeneralWhat"));
    }

    // add general license tab again
    m_cUi.tab_license->addTab(this->m_cUi.tab_gnu, QString());
    m_cUi.tab_license->setTabText(this->m_cUi.tab_license->indexOf(this->m_cUi.tab_gnu), QStringLiteral("GNU GPLv3 License"));
    //m_cUi.tab_license->setTabToolTip(this->m_cUi.tab_license->indexOf(this->m_cUi.tab_gnu), TRANSLATE("mUi_TabGeneralTip"));
    //m_cUi.tab_license->setTabWhatsThis(this->m_cUi.tab_license->indexOf(this->m_cUi.tab_gnu), TRANSLATE("mUi_TabGeneralWhat"));


    // set all other text objects
    this->setWindowTitle(TRANSLATE(L"aUi_fmTitle"));
    QString sFullVersion = TRANSLATE(L"base_version") + QString(" (build: ") + QString(__DATE__) + QString(" ") + QString(__TIME__) + QString(")");
    m_cUi.lb_title->setText(TRANSLATE(L"base_name"));
    m_cUi.lb_version->setText(TRANSLATE(L"aUi_Version"));
    m_cUi.lb_version_2->setText(sFullVersion);
    m_cUi.lb_author->setText(TRANSLATE(L"aUi_Author"));
    m_cUi.lb_author_2->setText(TRANSLATE(L"base_user"));
}