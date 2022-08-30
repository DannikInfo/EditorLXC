#ifndef XCODELIKEEDITOR_MAINFRAME_H
#define XCODELIKEEDITOR_MAINFRAME_H
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/frame.h>
#include <wx/aboutdlg.h>
#include <wx/dataview.h>
#include <wx/wrapsizer.h>
#include <wx/wfstream.h>

#include "config.h"
#include "logger.h"

#include "idEnum.h"
#include "JSONDataModel.h"
#include "configPathes.h"

class mainFrame : public wxFrame{
public:
    mainFrame();
private:
    void onExit(wxCommandEvent &event);
    void onAbout(wxCommandEvent &event);
    void onDataViewChar(wxKeyEvent &event);
    void deleteSelectedItems();
    void onOpen(wxCommandEvent &event);
    void onAddItem(wxCommandEvent &event);
    void onDeleteItem(wxCommandEvent &event);

    wxDECLARE_EVENT_TABLE();
protected:
    wxButton* addBtn;
    wxButton* delBtn;
    wxDataViewCtrl* dataViewCtrl;
    wxObjectDataPtr<JSONTreeModel> jModel;
    bool isSaved;
    bool isInitialized;
    std::string path;
    wxString savedValue;
    wxStaticText *fileLabel;
    wxWrapSizer* wSizer2;

    void onSave(wxCommandEvent &event);
    void onEdit(wxDataViewEvent &event);

    void onEditStart(wxDataViewEvent &event);

    void onSaveAs(wxCommandEvent &event);

    void split(const std::string &str, const char *delim, std::vector<std::string> &out);

    void onDebug(wxCommandEvent &event);

    void onSize(wxSizeEvent &event);

    void onEditDone(wxDataViewEvent &event);

    void onClose(wxCloseEvent &event);

    void exit();
};

#endif //XCODELIKEEDITOR_MAINFRAME_H
