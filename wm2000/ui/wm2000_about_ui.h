#pragma once

#include "ui_wm2000_about_ui.h"
#include "misc/language_pkg.h"
#include "misc/error_handler.h"

class wm2000_about_ui : public QDialog
{
    Q_OBJECT

public:
    wm2000_about_ui(std::string sConfigPath, QIcon *pIconWindow, QWidget *parent = Q_NULLPTR);
    ~wm2000_about_ui();

    void        set_language(std::string sNewLanguage);

private:
    Ui::fmAboutWindow        m_cUi;              // ui
    language_pkg             m_cTranslate;       // language converter
    error_handler            m_cErrHandler;      // link to error handler
};
