#include "google/protobuf/compiler/importer.h"
#include <iostream>
#include <fstream>
#include "cxxtemplates/xctmp.h"
#include "ext_meta.h"
#include "cpp_gen.h"
extern std::stringstream error_stream;
using namespace std;
using namespace google::protobuf;

class CXXFlatMsgGenerater {
    typedef std::unordered_map<const Descriptor *, int>
    MsgDegree;
    typedef	std::unordered_map<const Descriptor *,
        std::unordered_set<const Descriptor *> >	ReverseRef;
    typedef ReverseRef::iterator	ReverseRefItr;

    ReverseRef	reverse_refer;
    MsgDegree	msg_degrees;
    const Descriptor * root;
    std::vector<const Descriptor*>	ordered_desc;
    xctmp_t         * xctmp{ nullptr };
public:
    CXXFlatMsgGenerater(const Descriptor * desc);
    void	DumpFile(const char * file);
    int     Init(const char * pszTmpl);
    string	GetCodeText();
    string	ConvertMsg(const Descriptor * desc);
    void	TopologySort(std::vector<const Descriptor * > & msgs);
};

/////////////////////////////////////////////////////////////////////////////////////
extern std::stringstream error_stream;
CXXFlatMsgGenerater::CXXFlatMsgGenerater(const Descriptor * desc){
    root = desc;
}
void	CXXFlatMsgGenerater::DumpFile(const char * file){
    ofstream ofs(file, ios::out);
    ofs << GetCodeText() << endl;
}
int     CXXFlatMsgGenerater::Init(const char * pszTmpl){
    FILE* pf = fopen(pszTmpl, "r");
    if (!pf){
        fprintf(stderr, "fopen error :%s !\n", pszTmpl);
        return -1;
    }
    fseek(pf, 0, SEEK_END);
    int sz = ftell(pf);
    fseek(pf, 0, SEEK_SET);
    char * tmpbuff = new char[sz + 1];
    int rsz = fread(tmpbuff, 1, sz, pf);
    if (rsz != sz){
        fprintf(stderr, "read file bytes not eq error rsz:%d sz:%d  errno:%d error:%s!\n",
            rsz, sz, errno, strerror(errno));
        return -2;
    }
    tmpbuff[sz] = 0;
    fclose(pf);
    xctmp = xctmp_parse(tmpbuff);
    delete[] tmpbuff;
    if (!xctmp){
        std::cerr << "xctmp parse errror !" << std::endl;
        return -2;
    }
    //////////////////////////////////////////////////////////
    //meta
    //cs.msg.name <- meta
    //cs.field.name <- meta
    xctmp_push_filter(xctmp, "cs.msg.name", [](const string & p)->string {
        EXTMessageMeta * m = (EXTMessageMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->msg_desc->name() + "_ST";
    });
    xctmp_push_filter(xctmp, "cs.field.type", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        string r = m->GetTypeName();
        if (r.length() < 40){
            r.append(40 - r.length(), ' ');
        }
        return r;
    });
    xctmp_push_filter(xctmp, "cs.field.name", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->GetVarName();
    });
    xctmp_push_filter(xctmp, "msg.name", [](const string & p)->string {
        EXTMessageMeta * m = (EXTMessageMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->msg_desc->name();
    });
    xctmp_push_filter(xctmp, "field.count", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->f_count;
    });
    xctmp_push_filter(xctmp, "field.length", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->f_length;
    });

    xctmp_push_filter(xctmp, "field.is_array", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->is_repeated() ? "1" : "0";
    });
    xctmp_push_filter(xctmp, "field.name", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->name();
    });
    xctmp_push_filter(xctmp, "field.is_msg", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->message_type() ? "1" : "0";
    });
    xctmp_push_filter(xctmp, "field.is_string", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->type() == google::protobuf::FieldDescriptor::TYPE_STRING ? "1" : "0";
    });
    xctmp_push_filter(xctmp, "field.is_bytes", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->type() == google::protobuf::FieldDescriptor::TYPE_BYTES ? "1" : "0";
    });
    /////////
    xctmp_push_filter(xctmp, "field.is_ui32", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT32 ? "1" : "0";
    });
    xctmp_push_filter(xctmp, "field.is_ui64", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_UINT64 ? "1" : "0";
    });
    xctmp_push_filter(xctmp, "field.is_i64", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT64 ? "1" : "0";
    });
    xctmp_push_filter(xctmp, "field.is_i32", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_INT32 ? "1" : "0";
    });
    xctmp_push_filter(xctmp, "field.is_fixed64", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->type() == google::protobuf::FieldDescriptor::TYPE_FIXED64 ? "1" : "0";
    });
    xctmp_push_filter(xctmp, "field.is_foat", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_FLOAT ? "1" : "0";
    });
    xctmp_push_filter(xctmp, "field.is_double", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->cpp_type() == google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE ? "1" : "0";
    });

    //field.is_msg
    //field.is_string
    //field.is_bytes


    //field.is_ui32
    //field.is_ui64
    //field.is_i32
    //field.is_i64
    //field.is_fixed64
    //field.is_float
    //field.is_double

    //cs.msg.name <- meta
    //cs.field.type <- meta
    //cs.field.name <- meta
    //msg.type
    //msg.name

    //field.count
    //field.length
    //field.msg
    return 0;
}
string	CXXFlatMsgGenerater::GetCodeText(){
    if (!xctmp && Init("pbdcex.core.hxx")){
        std::cerr << "init pbdcex.core.hxx for templates error !" << std::endl;
        return "";
    }
    TopologySort(ordered_desc);
    std::stringstream oss;

    time_t t_m = time(NULL);
    xctmp_push_s(xctmp, "time", asctime(localtime(&t_m)));

    const string proto_name_post_fix = ".proto";
    string this_proto_heder_file_name = root->file()->name();
    this_proto_heder_file_name.replace(this_proto_heder_file_name.find(proto_name_post_fix), proto_name_post_fix.length(), ".pb.h");
    //depencies
    std::vector<std::string>    vs;
    //import
    for (int i = 0; i < root->file()->dependency_count(); ++i){
        auto f = root->file()->dependency(i);
        string ns_name = f->name();
        if (ns_name == "extensions.proto"){
            continue;
        }
        ns_name.replace(ns_name.find(proto_name_post_fix), proto_name_post_fix.length(), ".cex.h");
        vs.push_back(ns_name);
    }
    vs.push_back(this_proto_heder_file_name);
    xctmp_push_vs(xctmp, "depencies", vs);
    if (!root->file()->package().empty()){
        xctmp_push_s(xctmp, "ns_begin", "namespace " + root->file()->package() + " {");
        xctmp_push_s(xctmp, "ns_end", "}");
    }
    else {
        xctmp_push_s(xctmp, "ns_begin", "");
        xctmp_push_s(xctmp, "ns_end", "");
    }
    ////////////////////////////////////////////////////////////////////////////////////////////

    oss << "#pragma once" << endl << endl;
    oss << "//this file is auto generated by (hbpex)[https://github.com/jj4jj/hpbex.git] , don't edit this file !" << endl;
    oss << "//file generate datetime: " << asctime(localtime(&t_m)) << endl;

    //import default
    oss << "#include \"pbdcex.core.hpp\"" << endl;
    //import
    for (int i = 0; i < root->file()->dependency_count(); ++i){
        auto f = root->file()->dependency(i);
        string ns_name = f->name();
        if (ns_name == "extensions.proto"){
            continue;
        }
        ns_name.replace(ns_name.find(proto_name_post_fix), proto_name_post_fix.length(), ".hpb.h");
        oss << "#include \"" << ns_name << "\"" << endl;
    }

    //self proto .pb.h
    oss << "#include \"" << this_proto_heder_file_name << "\"" << endl;
    oss << endl;

    //package begin
    if (!root->file()->package().empty()){
        oss << "namespace " << root->file()->package() << " { " << endl;
    }
    oss << endl;
    //declaration using
    for (int i = 0; i < (int)ordered_desc.size(); ++i){
        string ns_name = ordered_desc[i]->file()->package();
        if (!ns_name.empty() && ns_name != root->file()->package()){
            //oss << "using " << ns_name << "::" << ordered_desc[i]->name() << endl;
            oss << "using " << ns_name << "::" << EXTMetaUtil::GetStructName(ordered_desc[i]) << ";" << endl;
        }
    }
    oss << endl;

    //convert
    for (int i = 0; i < (int)ordered_desc.size(); ++i){
        oss << ConvertMsg(ordered_desc[i]);
    }
    oss << endl;

    //package end
    if (!root->file()->package().empty()){
        oss << "} // end of namespace: " << root->file()->package() << endl;
    }
    return oss.str();
}
string	CXXFlatMsgGenerater::ConvertMsg(const Descriptor * desc){
    EXTMessageMeta	msg_meta;
    if (msg_meta.AttachDesc(desc)){
        throw logic_error("parse error !");
    }
    char cvbuff[64];
    string output;
    string jenv = "{\
                    \"meta\":";
    snprintf(cvbuff, sizeof(cvbuff), "\"%p\"", &msg_meta);
    jenv += cvbuff;
    jenv += ",\"fields\":[";
    bool head = true;
    for (auto & sfm : msg_meta.sub_fields){
        if (!head){
            jenv += ",";
        }
        snprintf(cvbuff, sizeof(cvbuff), "\"%p\"", &sfm);
        jenv += "{\"meta\":";
        jenv += cvbuff;
        jenv += "}";
        head = false;
    }
    jenv += "],\"pkfields\":[";
    head = true;
    for (auto & sfm : msg_meta.pks_fields){
        if (!head){
            jenv += ",";
        }
        snprintf(cvbuff, sizeof(cvbuff), "\"%p\"", sfm);
        jenv += "{\"meta\":";
        jenv += cvbuff;
        jenv += "}";
        head = false;
    }
    jenv += "]}";
    //cout << jenv << endl;       
    xctmp_render(xctmp, output, jenv);
    return output;
}
void	CXXFlatMsgGenerater::TopologySort(std::vector<const Descriptor * > & msgs){
    msgs.clear();
    std::queue<const Descriptor *>  desc_queue;
    desc_queue.push(root);
    msg_degrees[root] = 0;
    while (!desc_queue.empty()){
        auto msg = desc_queue.front();
        desc_queue.pop();
        for (int i = 0; i < msg->field_count(); ++i){
            auto field_def = msg->field(i);
            if (field_def->type() == FieldDescriptor::TYPE_MESSAGE){
                auto field = field_def->message_type();
                if (reverse_refer[field].empty()){
                    desc_queue.push(field);
                    msg_degrees[field] = 0;
                }
                int md = reverse_refer[field].size();
                reverse_refer[field].insert(msg);
                if (md != (int)reverse_refer[field].size()){
                    ++msg_degrees[msg];//out degree
                }
            }
        }
    }
    /////////////////////////////////////////////////////////
    while (!msg_degrees.empty()){
        auto it = msg_degrees.begin();
        while (it != msg_degrees.end()){
            if (it->second == 0){
                break;
            }
            ++it;
        }
        if (it == msg_degrees.end()){
            error_stream << "msg degree not found zero !" << endl;
            throw logic_error("msg degree error !");
        }
        msgs.push_back(it->first);
        for (auto desc : reverse_refer[it->first]){
            if (msg_degrees[desc] > 0){
                --msg_degrees[desc];
            }
            else {
                error_stream << "msg type:" << desc->name() << " refer count is 0 !" << endl;
            }
        }
        msg_degrees.erase(it);
    }
}
int cpph_generate(std::string & code, const google::protobuf::Descriptor * msg){
    CXXFlatMsgGenerater cgen(msg);
    code = cgen.GetCodeText();
    return 0;
}
