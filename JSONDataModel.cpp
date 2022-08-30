#include "JSONDataModel.h"
JSONTreeModelNode* m_first;
JSONTreeModelNode* m_node;
int i = 0;

JSONTreeModel::JSONTreeModel(nlohmann::json &j){
    m_root = new JSONTreeModelNode(nullptr, "root" );
    parseNode(m_root, j);
}

wxDataViewItem JSONTreeModel::getFirst() const{
    return wxDataViewItem(m_first);
}

wxString JSONTreeModel::GetKey( const wxDataViewItem &item ) const{
    auto *node = (JSONTreeModelNode*) item.GetID();
    if (!node)
        return wxEmptyString;

    return node->m_key;
}

wxString JSONTreeModel::GetType( const wxDataViewItem &item ) const{
    auto *node = (JSONTreeModelNode*) item.GetID();
    if (!node)
        return wxEmptyString;

    return node->m_type;
}

wxString JSONTreeModel::GetValue( const wxDataViewItem &item ) const{
    auto *node = (JSONTreeModelNode*) item.GetID();
    if (!node)
        return wxEmptyString;

    return node->m_value;
}

void JSONTreeModel::Delete( const wxDataViewItem &item ){
    auto *node = (JSONTreeModelNode*) item.GetID();
    if (!node)
        return;

    wxDataViewItem parent( node->GetParent() );
    if (!parent.IsOk()){
        wxASSERT(node == m_root);
        wxLogWarning( "Cannot remove the root item!" );
        return;
    }

    if(!strcmp(node->GetParent()->m_type.c_str(), "array")) {
        int pos = atoi(node->m_key);
        int i = 0;
        for (const auto &child : node->GetParent()->GetChildren()){
            if(i > pos)
                child->m_key = std::to_string(atoi(child->m_key)-1);
            i++;
        }
    }
    node->GetParent()->GetChildren().Remove( node );

    delete node;

    ItemDeleted( parent, item );
}

int JSONTreeModel::Compare( const wxDataViewItem &item1, const wxDataViewItem &item2, unsigned int column, bool ascending ) const{
    wxASSERT(item1.IsOk() && item2.IsOk());

    if (IsContainer(item1) && IsContainer(item2)){
        wxVariant value1, value2;
        GetValue( value1, item1, 0 );
        GetValue( value2, item2, 0 );

        wxString str1 = value1.GetString();
        wxString str2 = value2.GetString();
        int res = str1.Cmp( str2 );
        if (res) return res;

        // items must be different
        auto lItem1 = (wxUIntPtr) item1.GetID();
        auto lItem2 = (wxUIntPtr) item2.GetID();

        return (int)(lItem1-lItem2);
    }

    return wxDataViewModel::Compare( item1, item2, column, ascending );
}

void JSONTreeModel::GetValue( wxVariant &variant, const wxDataViewItem &item, unsigned int col ) const{
    wxASSERT(item.IsOk());

    auto *node = (JSONTreeModelNode*) item.GetID();
    switch (col){
        case 0:
            variant = node->m_key;
            break;
        case 1:
            variant = node->m_type;
            break;
        case 2:
            variant = node->m_value;
            break;
        default:
            logger::error( "GetValue: wrong column "+ std::to_string(col));
    }
}

bool JSONTreeModel::SetValue( const wxVariant &variant, const wxDataViewItem &item, unsigned int col ){
    wxASSERT(item.IsOk());

    auto *node = (JSONTreeModelNode*) item.GetID();
    switch (col){
        case 0:
            node->m_key = variant.GetString();
            return true;
        case 1:
            node->m_type = variant.GetString();
            return true;
        case 2:
            node->m_value = variant.GetString();
            return true;

        default:
            logger::error( "SetValue: wrong column" );
    }
    return false;
}

wxDataViewItem JSONTreeModel::GetParent( const wxDataViewItem &item ) const{
    if (!item.IsOk())
        return wxDataViewItem(nullptr);

    auto *node = (JSONTreeModelNode*) item.GetID();

    if (node == m_root)
        return wxDataViewItem(nullptr);

    return wxDataViewItem( (void*) node->GetParent() );
}

bool JSONTreeModel::IsContainer( const wxDataViewItem &item ) const{
    if (!item.IsOk())
        return true;

    auto *node = (JSONTreeModelNode*) item.GetID();
    return node->IsContainer();
}

unsigned int JSONTreeModel::GetChildren( const wxDataViewItem &parent, wxDataViewItemArray &array ) const{
    auto *node = (JSONTreeModelNode*) parent.GetID();
    if (!node){
        array.Add( wxDataViewItem( (void*) m_root ) );
        return 1;
    }

    if (node->GetChildCount() == 0)
        return 0;

    unsigned int count = node->GetChildren().GetCount();
    for (unsigned int pos = 0; pos < count; pos++){
        auto *child = node->GetChildren().Item( pos );
        array.Add( wxDataViewItem( (void*) child ) );
    }

    return count;
}

void JSONTreeModel::parseNode(JSONTreeModelNode *pNode, nlohmann::basic_json<> json) const {
    std::string type;
    std::string value;
    bool isNode = false;
    JSONTreeModelNode *child;
    for (const auto &item : json.items()) {
        if (item.value().is_string()) {
            type = "string";
            value = item.value().get<std::string>();
        }
        if (item.value().is_number_integer()) {
            type = "int";
            value = std::to_string(item.value().get<int>());
        }
        if (item.value().is_number_float()) {
            type = "float";
            value = std::to_string(item.value().get<double>());
        }
        if (item.value().is_boolean()) {
            type = "boolean";
            value = std::to_string(item.value().get<bool>());
        }
        if (item.value().is_object()) {
            type = "object";
            isNode = true;
            child = new JSONTreeModelNode(pNode, item.key(), type);
            parseNode(child, item.value());
        }
        if (item.value().is_array()) {
            type = "array";
            isNode = true;
            child = new JSONTreeModelNode(pNode, item.key(), type);
            parseNode(child, item.value());
        }

        if (isNode)
            m_node = child;
        else
            m_node = new JSONTreeModelNode(pNode, item.key(), type, wxString::FromUTF8(value));

        isNode = false;

        if(i == 0)
            m_first = m_node;

        pNode->Append(m_node);
        i++;
    }
}

void JSONTreeModel::addItem(const wxDataViewItem &item, const std::string& key, const std::string& type, const std::string& value){
    auto *node = (JSONTreeModelNode*) item.GetID();

    if (!node)      // happens if item.IsOk()==false
        return;

    auto *newItem = new JSONTreeModelNode(node, key, type, value);
    node->Append(newItem);

    ItemAdded( wxDataViewItem((void*) node), wxDataViewItem((void*) newItem));
}

bool JSONTreeModel::save(std::string &path) {
    nlohmann::json j;
    toJSON(m_root, j);
    std::ofstream o(path.c_str());
    if(o.is_open()) {
        o << std::setw(4) << j << std::endl;
        o.close();
        return true;
    }
    return false;
}

void JSONTreeModel::toJSON(JSONTreeModelNode *node, nlohmann::json &j){
    bool isArray = false;
    if(!strcmp(node->m_type.utf8_string().c_str(), "array"))
        isArray = true;

    for (const auto &item : node->GetChildren()){
        if(item->IsContainer())
            toJSON(item, j[item->m_key.utf8_string()]);
        else {
            if (!strcmp(item->m_type, "string")) {
                if (isArray)
                    j.push_back(item->m_value.utf8_string());
                else
                    j[item->m_key.utf8_string()] = item->m_value.utf8_string();
            }
            if (!strcmp(item->m_type, "int")) {
                if (isArray)
                    j.push_back(atoi(item->m_value));
                else
                    j[item->m_key.utf8_string()] = atoi(item->m_value);
            }
            if (!strcmp(item->m_type, "float")) {
                if (isArray)
                    j.push_back(atof(item->m_value));
                else
                    j[item->m_key.utf8_string()] = atof(item->m_value);
            }
            if (!strcmp(item->m_type, "boolean")) {
                if (isArray)
                    j.push_back(bool(atoi(item->m_value)));
                else
                    j[item->m_key.utf8_string()] = bool(atoi(item->m_value));
            }
        }
    }
}

void JSONTreeModel::SetValue(const wxDataViewItem &item, const wxString &value) const {
    auto *node = (JSONTreeModelNode*) item.GetID();
    if (!node)
        return;

    node->m_value = value;
}

void JSONTreeModel::setContainer(const wxDataViewItem &item, bool container) const{
    auto *node = (JSONTreeModelNode*) item.GetID();
    if (!node)
        return;

    node->m_container = container;
//    ItemDeleted(wxDataViewItem((void *)node->GetParent()), item);
//    ItemAdded(wxDataViewItem((void *)node->GetParent()), item);
}
