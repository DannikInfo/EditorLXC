#include "mainFrame.h"

wxBEGIN_EVENT_TABLE(mainFrame, wxFrame)
    EVT_BUTTON( BTN_ADD_ID, mainFrame::onAddItem )
    EVT_BUTTON( BTN_DEL_ID, mainFrame::onDeleteItem)
    EVT_DATAVIEW_ITEM_VALUE_CHANGED(DWCTRL_ID, mainFrame::onEdit)
    EVT_DATAVIEW_ITEM_START_EDITING(DWCTRL_ID, mainFrame::onEditStart)
    EVT_DATAVIEW_ITEM_EDITING_DONE(DWCTRL_ID, mainFrame::onEditDone)
    EVT_SIZE(mainFrame::onSize)
    EVT_CLOSE(mainFrame::onClose)
wxEND_EVENT_TABLE()

mainFrame::mainFrame() : wxFrame(nullptr, MAIN_F_ID, "EditorLXC"){
    isSaved = true;
    isInitialized = false;
    this->SetSize(800, 600);
    this->SetIcon(wxIcon("assets/logo.png"));

    auto *menuFile = new wxMenu;
    menuFile->Append(1, "&Create");
    menuFile->Append(wxID_OPEN);
    menuFile->Append(wxID_SAVE);
    menuFile->Append(wxID_SAVEAS);
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    if(config::get<bool>(DEBUG)) logger::info("Loaded file menu bar");

    auto *menuHelp = new wxMenu;
    menuHelp->Append(BTN_DEBUG_ID, "&Debug");
    menuHelp->Append(wxID_ABOUT);

    if(config::get<bool>(DEBUG)) logger::info("Loaded help menu bar");

    auto *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "File");
    menuBar->Append(menuHelp, "Help");

    SetMenuBar(menuBar);

    if(config::get<bool>(DEBUG)) logger::info("Added menu bar");

    Bind(wxEVT_MENU, &mainFrame::onAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &mainFrame::onExit, this, wxID_EXIT);
    Bind(wxEVT_MENU, &mainFrame::onOpen, this, wxID_OPEN);
    Bind(wxEVT_MENU, &mainFrame::onSave, this, wxID_SAVE);
    Bind(wxEVT_MENU, &mainFrame::onSaveAs, this, wxID_SAVEAS);
    Bind(wxEVT_MENU, &mainFrame::onDebug, this, BTN_DEBUG_ID);

    if(config::get<bool>(DEBUG)) logger::info("Menu buttons bind events");

    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* bSizer5;
    bSizer5 = new wxBoxSizer( wxVERTICAL );


    wSizer2 = new wxWrapSizer( wxHORIZONTAL, wxWRAPSIZER_DEFAULT_FLAGS );

    addBtn = new wxButton( this, BTN_ADD_ID, wxT("+"), wxDefaultPosition, wxSize( 30,30 ), wxBORDER_NONE );
    addBtn->SetFont( wxFont( 15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
    wSizer2->Add( addBtn, 0, wxALL, 5 );

    if(config::get<bool>(DEBUG)) logger::info("Loaded add button");

    delBtn = new wxButton( this, BTN_DEL_ID, wxT("-"), wxDefaultPosition, wxSize( 30,30 ), wxBORDER_NONE );
    delBtn->SetFont( wxFont( 15, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
    wSizer2->Add( delBtn, 0, wxALL, 5 );

    wSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

    fileLabel = new wxStaticText( this, wxID_ANY, wxT("File"), wxDefaultPosition, wxSize(-1,30), 0 );
    fileLabel->Wrap( -1 );
    fileLabel->SetFont( wxFont( 10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD, false, wxEmptyString ) );
    wSizer2->Add( fileLabel, 0, wxALL, 5 );

    bSizer5->Add( wSizer2, 0, wxEXPAND, 0 );

    if(config::get<bool>(DEBUG)) logger::info("Loaded remove button");

    dataViewCtrl = new wxDataViewCtrl(this, DWCTRL_ID, wxPoint(0,0), wxDefaultSize, wxDV_ROW_LINES | wxDV_HORIZ_RULES | wxDV_ROW_LINES | wxDV_VERT_RULES);
    dataViewCtrl->Connect(wxEVT_CHAR,wxKeyEventHandler(mainFrame::onDataViewChar),nullptr, this);

    if(config::get<bool>(DEBUG)) logger::info("DataViewCtrl initialized");

    // column 0 of the view control:
    auto *tr = new wxDataViewTextRenderer( "string", wxDATAVIEW_CELL_EDITABLE );
    auto *column0 = new wxDataViewColumn( "Key", tr, 0, 300, wxALIGN_LEFT,wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_RESIZABLE );
    dataViewCtrl->AppendColumn( column0 );

    if(config::get<bool>(DEBUG)) logger::info("DataViewCtrl - column 0 loaded");

    // column 1 of the view control:
    wxArrayString choices;
    choices.Add( "string" );
    choices.Add( "boolean" );
    choices.Add( "array" );
    choices.Add( "object" );
    choices.Add( "int" );
    choices.Add( "float" );
    auto *c = new wxDataViewChoiceRenderer( choices, wxDATAVIEW_CELL_EDITABLE,wxALIGN_CENTER | wxALIGN_CENTRE_VERTICAL);
    auto *column1 = new wxDataViewColumn( "Type", c, 1, 80, wxALIGN_LEFT,wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE | wxDATAVIEW_COL_RESIZABLE );
    dataViewCtrl->AppendColumn( column1 );

    if(config::get<bool>(DEBUG)) logger::info("DataViewCtrl - column 1 loaded");

    // column 2 of the view control:
    tr = new wxDataViewTextRenderer( "string", wxDATAVIEW_CELL_EDITABLE);
    auto *column2 = new wxDataViewColumn( "Value", tr, 2, 350, wxALIGN_LEFT,wxDATAVIEW_COL_SORTABLE | wxDATAVIEW_COL_REORDERABLE );
    dataViewCtrl->AppendColumn( column2 );
    dataViewCtrl->SetMinSize(wxSize(800, 520));

    if(config::get<bool>(DEBUG)) logger::info("DataViewCtrl - column 2 loaded");

    nlohmann::json j;
    std::ifstream is(config::get<std::string>(LAST_OPENED));
    if (is.is_open()) {
        path = config::get<std::string>(LAST_OPENED);
        is >> j;
        std::vector<std::string> vPath;
        split(config::get<std::string>(LAST_OPENED), "/", vPath);
        std::string fName = vPath.back();
        jModel = new JSONTreeModel(j);
        is.close();
        if(config::get<bool>(DEBUG)) logger::info("DataViewCtrl - Loaded last opened file");
        fileLabel->SetLabel(fName);
    }else {
        jModel = new JSONTreeModel(j);
        if(config::get<bool>(DEBUG)) logger::info("DataViewCtrl - Loaded clear data");
        fileLabel->SetLabel(wxT("Unnamed"));
    }
    dataViewCtrl->AssociateModel(jModel.get());
    dataViewCtrl->Select(jModel->getFirst());

    if(config::get<bool>(DEBUG)) logger::info("DataViewCtrl - associated data model");

    wxWrapSizer* wSizer3;
    wSizer3 = new wxWrapSizer( wxHORIZONTAL, wxWRAPSIZER_DEFAULT_FLAGS );

    wSizer3->Add( dataViewCtrl, 0, wxEXPAND|wxALL|wxTOP, 0 );

    bSizer5->Add( wSizer3, 0, wxEXPAND, 0);
    if(config::get<bool>(DEBUG)) logger::info("DataViewCtrl - Loaded fully");

    this->SetSizerAndFit( bSizer5 );
    this->Layout();

    this->Centre( wxBOTH );

    if(config::get<bool>(DEBUG)) logger::success("Loading ended");
    isInitialized = true;
}

void mainFrame::onClose(wxCloseEvent &event) {
    exit();
}

void mainFrame::onExit(wxCommandEvent& event){
    exit();
}

void mainFrame::exit(){
    if(config::get<bool>(DEBUG)) logger::info("Pressed 'Exit' or 'X' button or command CTRL-Q ");
    if (!isSaved){
        if(config::get<bool>(DEBUG)) logger::warn("Content has not been saved!");
        if (wxMessageBox(_("Current content has not been saved! Proceed?"), _("Please confirm"),wxICON_QUESTION | wxYES_NO, this) == wxNO )
            return;
    }
    if(config::get<bool>(DEBUG)) logger::info("Program closing: GoodBye!");
    Close(true);

}

void mainFrame::onAbout(wxCommandEvent& event){
    wxAboutDialogInfo aboutInfo;
    aboutInfo.SetName("EditorLXC");
    aboutInfo.SetIcon(wxIcon("assets/logo.png"));
    aboutInfo.SetVersion("0.0.1");
    aboutInfo.SetDescription(_("EditorLXC for JSON and another data structured formats"));
    aboutInfo.SetCopyright("(C) 2022 Kozlovskiy Danil ");
    aboutInfo.SetWebSite("https://github.com/DannikInfo/EditorLXC");

    wxAboutBox(aboutInfo);
    if(config::get<bool>(DEBUG)) logger::info("Opened About dialog");
}

void mainFrame::onDataViewChar(wxKeyEvent& event){
    if ( event.GetKeyCode() == WXK_DELETE ) {
        if(config::get<bool>(DEBUG)) logger::info("Pressed del key");
        deleteSelectedItems();
    }else
        event.Skip();
}

void mainFrame::deleteSelectedItems() {
    wxDataViewItemArray items;
    int len = dataViewCtrl->GetSelections(items);
    for (int i = 0; i < len; i++)
        if (items[i].IsOk()) {
            if(config::get<bool>(DEBUG)) logger::info(jModel->GetKey(items[i]) + " delete");
            jModel->Delete(items[i]);
        }
}

void mainFrame::onOpen(wxCommandEvent& event){
    if(config::get<bool>(DEBUG)) logger::info("Pressed 'open' menu button or CTRL-O");
    if (!isSaved){
       if(config::get<bool>(DEBUG)) logger::warn("Content has not been saved!");
       if (wxMessageBox(_("Current content has not been saved! Proceed?"), _("Please confirm"),wxICON_QUESTION | wxYES_NO, this) == wxNO )
           return;
    }

    if(config::get<bool>(DEBUG)) logger::info("Opening 'open file dialog'");
    wxFileDialog openFileDialog(this, _("Open JSON file"), "", "","JSON files (*.json)|*.json", wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    std::ifstream is(openFileDialog.GetPath());
    if (!is.is_open()){

        if(config::get<bool>(DEBUG)) logger::error("Cannot open file '"+openFileDialog.GetPath()+"'");
        wxLogError("Cannot open file '%s'.", openFileDialog.GetPath());
        return;
    }

    isSaved = false;

    path = openFileDialog.GetPath();
    config::set(LAST_OPENED, path);
    config::saveConfig();

    nlohmann::json j;
    is >> j;
    is.close();

    if(config::get<bool>(DEBUG)) logger::info("Data loaded to JSON scheme");

    jModel = new JSONTreeModel(j);
    dataViewCtrl->AssociateModel(jModel.get());
    dataViewCtrl->Select(jModel->getFirst());

    if(config::get<bool>(DEBUG)) logger::info("Opened file data displayed on screen");
    fileLabel->SetLabel(openFileDialog.GetFilename());
}

void mainFrame::onAddItem(wxCommandEvent& event){
    if(config::get<bool>(DEBUG)) logger::info("Pressed '+' button ");
    auto* ctrl = (wxDataViewTreeCtrl*) mainFrame::dataViewCtrl;
    wxDataViewItem selected = ctrl->GetSelection();
    if (!ctrl->IsContainer(selected))
        return;

    std::string value = "item";
    wxDataViewItemArray child;
    jModel->GetChildren(selected, child);

    if(!strcmp(jModel->GetType(selected), "array")) {
        value = std::to_string(child.size());
        if(config::get<bool>(DEBUG)) logger::info("Adding new item to array");
    } else
        if(config::get<bool>(DEBUG)) logger::info("Adding new item to object");

    jModel->addItem(selected, value);

    if(config::get<bool>(DEBUG)) logger::info("New item has been added");
}

void mainFrame::onDeleteItem(wxCommandEvent& event) {
    if(config::get<bool>(DEBUG)) logger::info("Pressed '-' button");
    deleteSelectedItems();
}

void mainFrame::onSave(wxCommandEvent& event){
    if(config::get<bool>(DEBUG)) logger::info("Pressed 'save' menu button or CTRL-S");
    if(path.empty()){
        onSaveAs(event);
        return;
    }
    if(jModel->save(path)) {
        if(config::get<bool>(DEBUG)) logger::success("File has been saved! - " + path);
        wxLogInfo("Saved");
    }else {
        if(config::get<bool>(DEBUG)) logger::error("Error when saving file!! - " + path);
        wxLogError("Error when saving file!!");
    }
    isSaved = true;
}

void mainFrame::onSaveAs(wxCommandEvent& event){
    if(config::get<bool>(DEBUG)) logger::info("Pressed 'save as' menu button");
    wxFileDialog saveAsDialog(this, _("Save JSON file"), "", "","JSON files (*.json)|*.json", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (saveAsDialog.ShowModal() == wxID_CANCEL)
        return;
    path = saveAsDialog.GetPath().utf8_string();
    if(jModel->save(path)) {
        if(config::get<bool>(DEBUG)) logger::success("File has been saved! - " + path);
        wxLogInfo("Saved");
    }else {
        if(config::get<bool>(DEBUG)) logger::error("Error when saving file!! - " + path);
        wxLogError("Error when saving file!!");
    }
    isSaved = true;
}

void mainFrame::onEditStart(wxDataViewEvent &event) {
    if(config::get<bool>(DEBUG)) logger::info("Edit started event");
    wxDataViewItem parent = jModel->GetParent(event.GetItem());
    wxString type = jModel->GetType(parent);
    if(event.GetColumn() == 0 && !strcmp(type.c_str(), "array") or
    (jModel->IsContainer(event.GetItem()) && event.GetColumn() == 2)
    )
        event.Veto();
    savedValue = jModel->GetValue(event.GetItem());
}
void mainFrame::onEditDone(wxDataViewEvent &event) {
    if(event.GetColumn() == 1 && jModel->GetParent(event.GetItem()) == nullptr) event.Veto();
}

void mainFrame::onEdit(wxDataViewEvent &event) {
    if(config::get<bool>(DEBUG)) logger::info("Value changed event");

    isSaved = false;
    wxString type = jModel->GetType(event.GetItem());
    if(event.GetColumn() == 2){
        if(((!strcmp(type.c_str(), "int") or !strcmp(type.c_str(), "float")) && !jModel->GetValue(event.GetItem()).IsNumber()) ||
            (!strcmp(type.c_str(), "boolean")  && strcmp(jModel->GetValue(event.GetItem()).c_str(), "0") && strcmp(jModel->GetValue(event.GetItem()).c_str(), "1"))
        ){
            if(config::get<bool>(DEBUG)) logger::warn("Incorrect value in " + jModel->GetKey(event.GetItem()) + " for type " + jModel->GetType(event.GetItem()));
            wxLogWarning("Incorrect value!!");
            jModel->SetValue(event.GetItem(), savedValue);
        }
    }

    if(event.GetColumn() == 1){
        if(config::get<bool>(DEBUG)) logger::info("Change type for "+ jModel->GetKey(event.GetItem()) +" to " + jModel->GetType(event.GetItem()));
        if(!strcmp(type.c_str(), "array") or !strcmp(type.c_str(), "object")) {
            jModel->setContainer(event.GetItem(), true);
            if(config::get<bool>(DEBUG)) logger::info(jModel->GetKey(event.GetItem()) +" is container now ");
        }else
            if(jModel->IsContainer(event.GetItem())){
                jModel->setContainer(event.GetItem(), false);
                if(config::get<bool>(DEBUG)) logger::info(jModel->GetKey(event.GetItem()) +" is no container anymore");
            }
    }
}

void mainFrame::onDebug(wxCommandEvent &event){
    config::set(DEBUG, !config::get<bool>(DEBUG));
    if(config::get<bool>(DEBUG))
        wxLogInfo("Debug mode is enabled, now all activity will be logged");
    else
        wxLogInfo("Debug mode is disabled");
    config::saveConfig();
}


//
void mainFrame::split(std::string const &str, const char* delim, std::vector<std::string> &out){
    char *token = strtok(const_cast<char*>(str.c_str()), delim);
    while (token != nullptr){
        out.emplace_back(token);
        token = strtok(nullptr, delim);
    }
}

void mainFrame::onSize(wxSizeEvent &event) {
    if (isInitialized) {
        this->Layout();
        dataViewCtrl->SetSize(event.GetSize().GetWidth(), event.GetSize().GetHeight());
    }
}
