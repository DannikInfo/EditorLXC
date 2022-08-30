#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "logger.h"
#include "config.h"

#include "mainFrame.h"
#include "resourceLoader.h"
#include "configPathes.h"

class main : public wxApp{
public:
    virtual bool OnInit();
};

bool main::OnInit() {
    resourceLoader::load();

    logger::init("xcleditor");

    config::init("config");
    config::set(LAST_OPENED, "");
    config::set(DEBUG, false);
    config::loadConfig();

    auto* mf = new mainFrame;
    mf->Show(true);

    return true;
}

wxIMPLEMENT_APP(class main);
