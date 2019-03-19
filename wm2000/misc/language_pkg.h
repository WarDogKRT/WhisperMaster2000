#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <boost/property_tree/ptree.hpp>

class language_pkg
{
public:
    language_pkg();
    ~language_pkg();

    std::string  translate(const char* sStringName);
    void         translate(const char* sStringName, char* cStrBuffer, size_t nBuffSize);

    std::string  translate(const wchar_t* sStringName);
    std::string  wtranslate(std::wstring sStringName);
    std::string  utf8_encode(const std::wstring &wstr);
    std::wstring utf8_decode(const std::string &str);

    void         set_language(const char* sNewLanguage);
    void         set_language(std::string sNewLanguage);
    std::string  get_language() { return utf8_encode(m_wLanguage); };

protected:
    void         init_base();
    void         init_plugin_para_chk();
    void         init_plugin_hotkey();
    void         init_plugin_menu();
    void         init_plugin_info();
    void         init_ui_main();
    void         init_ui_freq();
    void         init_ui_client_tree();
    void         init_ui_about();
    void         init_error_handler();

private:
    std::wstring                 m_wLanguage;
    boost::property_tree::wptree m_cLangPkg;
};

