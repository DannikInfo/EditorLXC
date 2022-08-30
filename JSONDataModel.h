#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dataview.h>

#include <logger.h>
#include "wx/hashmap.h"
#include <config.h>
#include <fstream>

WX_DECLARE_HASH_MAP(unsigned, wxString, wxIntegerHash, wxIntegerEqual, IntToStringMap);

class JSONTreeModelNode;
WX_DEFINE_ARRAY_PTR( JSONTreeModelNode*, JSONTreeModelNodePtrArray );

class JSONTreeModelNode{
public:
    JSONTreeModelNode( JSONTreeModelNode* parent, const wxString &key, const wxString &type, const wxString &value ){
        m_parent = parent;

        m_key = key;
        m_type = type;
        m_value = value;

        m_container = false;
    }

    JSONTreeModelNode( JSONTreeModelNode* parent, const wxString &branch, const wxString &type = ""){
        m_parent = parent;

        m_key = branch;
        m_value = "";
        m_type = type;

        m_container = true;
    }

    ~JSONTreeModelNode(){
        // free all our children nodes
        size_t count = m_children.GetCount();
        for (size_t i = 0; i < count; i++){
            JSONTreeModelNode *child = m_children[i];
            delete child;
        }
    }

    bool IsContainer() const{
        return m_container;
    }

    JSONTreeModelNode* GetParent(){
        return m_parent;
    }

    JSONTreeModelNodePtrArray& GetChildren(){
        return m_children;
    }

    JSONTreeModelNode* GetNthChild( unsigned int n ){
        return m_children.Item( n );
    }

    void Insert( JSONTreeModelNode* child, unsigned int n){
        m_children.Insert( child, n);
    }

    void Append( JSONTreeModelNode* child ){
        m_children.Add( child );
    }

    unsigned int GetChildCount() const{
        return m_children.GetCount();
    }

public:     // public to avoid getters/setters
    wxString                m_key;
    wxString                m_type;
    wxString                m_value;

    bool m_container;

private:
    JSONTreeModelNode *m_parent;
    JSONTreeModelNodePtrArray m_children;
};

class JSONTreeModel: public wxDataViewModel{
public:
    JSONTreeModel(nlohmann::json &j);
    ~JSONTreeModel(){
        delete m_root;
    }

    // helper method for wxLog
    wxString GetKey( const wxDataViewItem &item ) const;
    wxString GetType( const wxDataViewItem &item ) const;
    wxString GetValue( const wxDataViewItem &item ) const;

    void SetValue(const wxDataViewItem &item, const wxString &value) const;

    // helper methods to change the model
    void Delete( const wxDataViewItem &item );

    // override sorting to always sort branches ascendingly
    int Compare( const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int column, bool ascending ) const wxOVERRIDE;

    // implementation of base class virtuals to define model
    virtual unsigned int GetColumnCount() const wxOVERRIDE{
        return 3;
    }

    virtual wxString GetColumnType( unsigned int col ) const wxOVERRIDE{
        return wxT("string");
    }

    virtual void GetValue( wxVariant &variant, const wxDataViewItem &item, unsigned int col ) const wxOVERRIDE;
    virtual bool SetValue( const wxVariant &variant, const wxDataViewItem &item, unsigned int col ) wxOVERRIDE;

    virtual wxDataViewItem GetParent( const wxDataViewItem &item ) const wxOVERRIDE;


    virtual bool HasContainerColumns(const wxDataViewItem &item) const wxOVERRIDE{
        return true;
    }
    virtual bool IsContainer( const wxDataViewItem &item ) const wxOVERRIDE;
    void setContainer(const wxDataViewItem &item, bool container) const;

    virtual unsigned int GetChildren( const wxDataViewItem &parent, wxDataViewItemArray &array ) const wxOVERRIDE;

    wxDataViewItem getFirst() const;
    void parseNode(JSONTreeModelNode *pNode, nlohmann::basic_json<> json) const;
    void addItem(const wxDataViewItem &item, const std::string &key, const std::string &type = "string", const std::string &value = "");

    bool save(std::string &path);
    void toJSON(JSONTreeModelNode *node, nlohmann::json &j);
private:
    JSONTreeModelNode*   m_root;
};