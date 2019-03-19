#pragma once
#include "error_handler.h"
#include <boost/property_tree/ptree.hpp>
#include <QtTextToSpeech\qtexttospeech.h>


class speech_engine
{
public:
    speech_engine();
    ~speech_engine();

    int  get_possible_language(std::wstring sNewLanguage);
    void set_language(const char* sNewLanguage);
    void set_volume(double dVolume);

    void say(const char* sStringName);

protected:
    bool         check_engine();
    std::wstring utf8_decode(const std::string &str);

private:
    static QTextToSpeech        *m_pcSpeechEngine;
    static std::wstring          m_wLanguage;

    boost::property_tree::wptree m_cLangPkg;
    error_handler                m_cErrHandler;      // link to error handler
};

