#include "google/protobuf/compiler/importer.h"
#include <iostream>
#include <fstream>
#include "cxxtemplates/include/xctmp.h"
//#include "cxxtemplates/xctmp_utils.hpp"
#include "ext_meta.h"
#include "cpp_gen.h"

extern ::std::stringstream error_stream;
using namespace std;
using namespace google::protobuf;
static const char * pbdcex_core_hxx = "#ifndef __PBDCEX_H_DEF_{{file_name|unif}}__\n#define __PBDCEX_H_DEF_{{file_name|unif}}__\n#pragma once\n//This file is auto generated by pbdcex [http://github.com/jj4jj/pbdcex.git],\n//Please do NOT edit it ! Any bug please let jj4jj known .\n//File generated time: {{time}}\n\n{{!for vd in depencies}}\n#include \"{{vd}}\"\n{{}}\n#include \"pbdcex/pbdcex.core.hpp\"\n{{ns_begin}}\nusing pbdcex::string_t;\nusing pbdcex::bytes_t;\nusing pbdcex::array_t;\nusing pbdcex::serializable_t;\n{{!for vmsg in msgs}}\nstruct {{vmsg.meta|cs.msg.name}} : public serializable_t<{{vmsg.meta|cs.msg.name}}, {{vmsg.meta|msg.name}}> {\n    {{!for vf in vmsg.fields}}{{vf.meta|cs.field.wtype}}       {{vf.meta|cs.field.name}};\n    {{}}////////////////////////////////////////////////////////////////////////\n    void    construct(){\n        if(sizeof(*this) < 1024){\n            memset(this, 0, sizeof(*this));\n        }\n        else {\n            {{!for vf in vmsg.fields}}\n            {{!if vf.meta|field.is_array}}\n            {{vf.meta|cs.field.name}}.construct();\n            {{!elif vf.meta|field.is_num}}\n            {{vf.meta|cs.field.name}}=static_cast<{{vf.meta|cs.field.type}}>(0);\n            {{!elif vf.meta|field.is_bool}}\n            {{vf.meta|cs.field.name}}=false;\n            {{!else}}\n            {{vf.meta|cs.field.name}}.construct();\n            {{}}\n            {{}}\n        }\n    }\n    int     convto({{vmsg.meta|msg.name}} & tomsg_) const {\n        int ret = 0;\n        tomsg_.Clear();\n        {{!for vf in vmsg.fields}}\n        {{!if vf.meta|field.is_array}}\n        for (size_t i = 0;i < {{vf.meta|field.name}}.count; ++i){\n            {{!if vf.meta|field.is_msg}}\n            ret = this->{{vf.meta|cs.field.name}}.list[i].convto(*tomsg_.add_{{vf.meta|field.name}}());\n            if (ret){return __LINE__+ret;}\n            {{!elif vf.meta|field.is_bytes }}\n            tomsg_.add_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.list[i].data, this->{{vf.meta|cs.field.name}}.list[i].length);\n            {{!elif vf.meta|field.is_string }}\n            tomsg_.add_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.list[i].data);\n            {{!else}}\n            tomsg_.add_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.list[i]);\n            {{}}}\n        {{!else}}\n        {{!if vf.meta|field.is_msg}}\n        ret = this->{{vf.meta|cs.field.name}}.convto(*tomsg_.mutable_{{vf.meta|field.name}}());\n        if (ret) {return __LINE__+ret;}\n        {{!elif vf.meta|field.is_bytes}}\n        tomsg_.set_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.data, this->{{vf.meta|cs.field.name}}.length);\n        {{!elif vf.meta|field.is_string}}\n        tomsg_.set_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.data);\n        {{!else}}tomsg_.set_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}});\n        {{}}\n        {{}}\n        {{}}\n        return ret;\n    }\n    int     convfrom(const {{vmsg.meta|msg.name}} & frommsg_) {\n        int ret = 0;\n        {{!for vf in vmsg.fields}}\n        {{!if vf.meta|field.is_array}}\n        this->{{vf.meta|cs.field.name}}.count=0;\n        for (size_t i = 0; i < (size_t)frommsg_.{{ vf.meta | field.name }}_size() && i < {{ vf.meta | field.count }}; ++i, ++(this->{{ vf.meta | cs.field.name }}.count)){\n            {{!if vf.meta|field.is_msg}}\n            ret = this->{{vf.meta|cs.field.name}}.list[i].convfrom(frommsg_.{{vf.meta|field.name}}(i));\n            if (ret) { return __LINE__+ret; }\n            {{!elif vf.meta|field.is_bytes}}\n            assert(frommsg_.{{vf.meta|field.name}}(i).length() <= {{vf.meta|field.length}});\n            this->{{ vf.meta | cs.field.name }}.list[i].assign(frommsg_.{{ vf.meta | field.name }}(i));\n            {{ !elif vf.meta | field.is_string }}\n            assert(frommsg_.{{ vf.meta | field.name }}(i).length() < {{ vf.meta | field.length }});\n            this->{{ vf.meta | cs.field.name }}.list[i].assign(frommsg_.{{ vf.meta | field.name }}(i).data());\n            {{!else}}\n            this->{{vf.meta|cs.field.name}}.list[i] = frommsg_.{{vf.meta|field.name}}(i);\n            {{}}\n        }\n        {{!else}}\n        {{!if vf.meta|field.is_msg}}\n        ret = this->{{vf.meta|cs.field.name}}.convfrom(frommsg_.{{vf.meta|field.name}}());\n        if (ret) { return __LINE__+ret; }\n        {{ !elif vf.meta | field.is_bytes }}\n        assert(frommsg_.{{ vf.meta | field.name }}().length() <= {{ vf.meta | field.length }});\n        this->{{vf.meta|cs.field.name}}.assign(frommsg_.{{vf.meta|field.name}}());\n        {{ !elif vf.meta | field.is_string }}\n        assert(frommsg_.{{ vf.meta | field.name }}().length() < {{ vf.meta | field.length }});\n        this->{{vf.meta|cs.field.name}}.assign(frommsg_.{{vf.meta|field.name}}().data());\n        {{!else}}\n        this->{{vf.meta|cs.field.name}} = frommsg_.{{vf.meta|field.name}}();\n        {{}}\n        {{}}\n        {{}}\n        return ret;\n    }\n    int     check_convfrom(const {{vmsg.meta|msg.name}} & frommsg_) const {\n        int ret = 0;\n        {{!for vf in vmsg.fields}}\n        {{!if vf.meta|field.is_array}}\n        if ((size_t)frommsg_.{{ vf.meta | field.name }}_size() > {{ vf.meta | field.count }}){ return __LINE__; }\n        for (size_t i = 0; i < (size_t)frommsg_.{{ vf.meta | field.name }}_size() && i < {{ vf.meta | field.count }}; ++i){\n            {{!if vf.meta|field.is_msg}}\n            ret = this->{{vf.meta|cs.field.name}}.list[i]._check_convfrom(frommsg_.{{vf.meta|field.name}}(i));\n            if (ret) {return ret;}\n            {{!elif vf.meta|field.is_bytes}}\n            if (frommsg_.{{vf.meta|field.name}}(i).length() > {{vf.meta|field.length}}){ return __LINE__; }\n            {{ !elif vf.meta | field.is_string }}\n            if (frommsg_.{{ vf.meta | field.name }}(i).length() >= {{ vf.meta | field.length }}){ return __LINE__; }\n            {{!else}}\n            //\n            {{}}\n        }\n        {{!else}}\n        {{!if vf.meta|field.is_msg}}\n        ret = this->{{vf.meta|cs.field.name}}.check_convfrom(frommsg_.{{vf.meta|field.name}}());\n        if (ret) { return __LINE__; }\n        {{ !elif vf.meta | field.is_bytes }}\n        if (frommsg_.{{ vf.meta | field.name }}().length() > {{ vf.meta | field.length }}){ return __LINE__; }\n        {{ !elif vf.meta | field.is_string }}\n        if (frommsg_.{{ vf.meta | field.name }}().length() >= {{ vf.meta | field.length }}){ return __LINE__; }\n        {{!else}}\n        //\n        {{}}\n        {{}}\n        {{}}\n        return ret;\n    }\n    void     diff(const {{ vmsg.meta | cs.msg.name }} & orgv , {{ vmsg.meta | msg.name }} & updates) const {\n        updates.Clear();\n        {{!for vf in vmsg.fields}}\n        {{!if vf.meta | field.is_array }}\n        {//block of checking array differ\n            decltype(this->{{vf.meta | cs.field.name}}.count) i = 0;\n            for (i = 0;i < this->{{vf.meta | cs.field.name}}.count; ++i) {\n                auto upd_ = updates.mutable_{{vf.meta | field.name}}()->Add();\n                {{!if vf.meta | field.is_msg }}\n                if (i < orgv.{{vf.meta | cs.field.name}}.count) {\n                    this->{{vf.meta | cs.field.name}}.list[i].diff(orgv.{{vf.meta | cs.field.name}}.list[i], *upd_);\n                }\n                else {\n                    this->{{vf.meta | cs.field.name}}.list[i].convto(*upd_);\n                }\n                {{!elif vf.meta | field.is_string }}\n                upd_->assign(this->{{vf.meta | cs.field.name}}.list[i].data);\n                {{!elif vf.meta | field.is_bytes }}\n                upd_->assign((const char*)this->{{vf.meta | cs.field.name}}.list[i].data, this->{{vf.meta | cs.field.name}}.list[i].length);\n                {{!else}}\n                *upd_ = this->{{vf.meta | cs.field.name}}.list[i];\n                {{}}\n            }            \n        }//end with array diff block \n        {{!elif vf.meta | field.is_msg}}\n        this->{{vf.meta | cs.field.name}}.diff(orgv.{{vf.meta | cs.field.name}}, *updates.mutable_{{ vf.meta | field.name }}());\n        {{!else}}\n        {{!if vf.meta | field.is_required}}\n        {{!if vf.meta | field.is_string }}\n        updates.set_{{ vf.meta | field.name }}(this->{{ vf.meta | cs.field.name }}.data);\n        {{!elif vf.meta | field.is_bytes}}\n        updates.set_{{ vf.meta | field.name }}((char*)this->{{ vf.meta | cs.field.name }}.data, this->{{ vf.meta | cs.field.name }}.length);\n        {{!else}}\n        updates.set_{{ vf.meta | field.name }}(this->{{ vf.meta | cs.field.name }});\n        {{}}\n        {{!else}}\n        if (!(this->{{vf.meta | cs.field.name}} == orgv.{{vf.meta | cs.field.name}})) {\n            {{!if vf.meta | field.is_string }}\n            updates.set_{{ vf.meta | field.name }}(this->{{ vf.meta | cs.field.name }}.data);\n            {{!elif vf.meta | field.is_bytes}}\n            updates.set_{{ vf.meta | field.name }}((char*)this->{{ vf.meta | cs.field.name }}.data, this->{{ vf.meta | cs.field.name }}.length);\n            {{!else}}\n            updates.set_{{ vf.meta | field.name }}(this->{{ vf.meta | cs.field.name }});\n            {{}}\n        }\n        {{}}\n        {{}}\n        {{}}\n    }\n    void     patch(const {{ vmsg.meta | msg.name }} & updates) {\n        {{!for vf in vmsg.fields}}\n        {{!if vf.meta | field.is_array }}\n        {//block of checking array patch\n            static {{vf.meta | cs.field.scalar_type}}    item_temp;\n            for (int i = 0; i < updates.{{vf.meta | field.name}}_size(); ++i) {\n                if (i < (int)this->{{vf.meta | cs.field.name}}.count) {\n                    {{!if vf.meta | field.is_msg }}\n                    this->{{vf.meta | cs.field.name}}.list[i].patch(updates.{{vf.meta | field.name}}(i));    \n                    {{!elif vf.meta | field.is_string }}\n                    this->{{vf.meta | cs.field.name}}.list[i].assign(updates.{{vf.meta | field.name}}(i));\n                    {{!elif vf.meta | field.is_bytes }}\n                    this->{{vf.meta | cs.field.name}}.list[i].assign(updates.{{vf.meta | field.name}}(i));\n                    {{!else}}\n                    this->{{vf.meta | cs.field.name}}.list[i] = updates.{{vf.meta | field.name}}(i);\n                    {{}}\n                }\n                else {\n                    {{!if vf.meta | field.is_msg }}\n                    item_temp.convfrom(updates.{{vf.meta | field.name}}(i));\n                    {{!elif vf.meta | field.is_string }}\n                    item_temp.assign(updates.{{vf.meta | field.name}}(i));\n                    {{!elif vf.meta | field.is_bytes }}\n                    item_temp.assign(updates.{{vf.meta | field.name}}(i));\n                    {{!else}}\n                    item_temp = updates.{{vf.meta | field.name}}(i);\n                    {{}}\n                    this->{{vf.meta | cs.field.name}}.lappend(item_temp);\n                }\n            }//end for append or update\n        }//end with array diff block \n        {{!elif vf.meta | field.is_msg}}\n        this->{{vf.meta | cs.field.name}}.patch(updates.{{ vf.meta | field.name }}());\n        {{!else}}\n        {{!if vf.meta | field.is_required}}\n        {{!if vf.meta | field.is_string }}\n        this->{{ vf.meta | cs.field.name }}.assign(updates.{{ vf.meta | field.name }}());\n        {{!elif vf.meta | field.is_bytes}}\n        this->{{ vf.meta | cs.field.name }}.assign(updates.{{ vf.meta | field.name }}());\n        {{!else}}\n        this->{{ vf.meta | cs.field.name }} = updates.{{ vf.meta | field.name }}();\n        {{}}\n        {{!else}}\n        if (updates.has_{{vf.meta | field.name}}()) {\n            {{!if vf.meta | field.is_string }}\n            this->{{ vf.meta | cs.field.name }}.assign(updates.{{ vf.meta | field.name }}());\n            {{!elif vf.meta | field.is_bytes}}\n            this->{{ vf.meta | cs.field.name }}.assign(updates.{{ vf.meta | field.name }}());\n            {{!else}}\n            this->{{ vf.meta | cs.field.name }} = updates.{{ vf.meta | field.name }}();\n            {{}}\n        }\n        {{}}\n        {{}}\n        {{}}\n    }\n    int     compare(const {{vmsg.meta|cs.msg.name}} & rhs_) const {\n        int cmp = 0;\n        {{!for vf in vmsg.pkfields}}\n        {{!if vf.meta|field.is_array}}\n        cmp = this->{{vf.meta|cs.field.name}}.compare(rhs_.{{vf.meta|cs.field.name}});\n        if(cmp){return cmp;}\n        {{!elif vf.meta|field.is_msg}}\n        cmp = this->{{vf.meta|cs.field.name}}.compare(rhs_.{{vf.meta|cs.field.name}});\n        if(cmp){return cmp;}\n        {{!else}}\n        if (this->{{vf.meta|cs.field.name}} < rhs_.{{vf.meta|cs.field.name}}){\n            return -1;\n        }\n        else if(this->{{vf.meta|cs.field.name}} > rhs_.{{vf.meta|cs.field.name}}){\n            return 1;\n        }\n        {{}}\n        {{}}\n        return cmp;\n    }\n    bool    operator == (const {{vmsg.meta|cs.msg.name}} & rhs_) const {\n        return this->compare(rhs_) == 0;\n    }\n    bool    operator < (const {{vmsg.meta|cs.msg.name}} & rhs_) const {\n        return this->compare(rhs_) < 0;\n    }\n    size_t hash() const {\n        {{!if vmsg.pkfields_num < 2}}\n        {{!for vf in vmsg.pkfields}}\n        {{!if vf.meta|field.is_array}}\n        return this->{{vf.meta|cs.field.name}}.hash();\n        {{!elif vf.meta|field.is_num}}\n        return (size_t)(this->{{vf.meta|cs.field.name}});\n        {{!else}}\n        return this->{{vf.meta|cs.field.name}}.hash();\n        {{}}\n        {{}}\n        {{!else}}\n        size_t avhs[] = {\n        {{!for vf in vmsg.pkfields}}\n        {{!if vf.meta|field.is_array}}\n            this->{{vf.meta|cs.field.name}}.hash(),\n        {{!elif vf.meta|field.is_num}}\n            (size_t)(this->{{vf.meta|cs.field.name}}),\n        {{!else}}\n            this->{{vf.meta|cs.field.name}}.hash(),\n        {{}}\n        {{}}\n        };\n        return pbdcex::hash_code_merge_multi_value(avhs, {{vmsg.pkfields_num}});\n        {{}}\n    }\n};\n{{}}\n{{ns_end}}\n#endif\n";
class CXXFlatMsgGenerater {
	typedef ::std::unordered_map<const Descriptor *, int>
		MsgDegree;
    typedef	::std::unordered_map<const Descriptor *,
        ::std::unordered_set<const Descriptor *> >	ReverseRef;
    typedef ReverseRef::iterator	ReverseRefItr;

    ReverseRef	reverse_refer;
    MsgDegree	msg_degrees;
    const Descriptor * root;
    ::std::vector<const Descriptor*>	ordered_desc;
    xctmp_t         * xctmp{ nullptr };
public:
    CXXFlatMsgGenerater(const Descriptor * desc);
    int     Init(const char * pszTmpl);
    int	    GetCodeText(::std::string & code, const char * tf);
    void	TopologySort(::std::vector<const Descriptor * > & msgs);
};

/////////////////////////////////////////////////////////////////////////////////////
static inline  void
_strreplace(::std::string & ts, const ::std::string & p, const ::std::string & v){
	auto beg = 0;
	auto fnd = ts.find(p, beg);
	while (fnd != ::std::string::npos){
		ts.replace(fnd, p.length(), v);
		beg = fnd + v.length();
		fnd = ts.find(p, beg);
	}
}
extern ::std::stringstream error_stream;
CXXFlatMsgGenerater::CXXFlatMsgGenerater(const Descriptor * desc){
    root = desc;
}
int     CXXFlatMsgGenerater::Init(const char * pszTmpl){
    if(pszTmpl){
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
    }
    else {
        xctmp = xctmp_parse(pbdcex_core_hxx);
    }
    if (!xctmp){
        ::std::cerr << "xctmp parse errror !" << ::std::endl;
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
    xctmp_push_filter(xctmp, "cs.field.wtype", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        string r = m->GetTypeName();
        if (r.length() < 32){
            r.append(32 - r.length(), ' ');
        }
        return r;
    });
    xctmp_push_filter(xctmp, "cs.field.type", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->GetTypeName();
    });

	xctmp_push_filter(xctmp, "cs.field.scalar_type", [](const string & p)->string {
		EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
		return m->GetScalarTypeName();
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

	xctmp_push_filter(xctmp, "field.is_required", [](const string & p)->string {
		EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
		return m->field_desc->is_required() ? "1" : "0";
	});

	xctmp_push_filter(xctmp, "field.is_optional", [](const string & p)->string {
		EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
		return m->field_desc->is_required() ? "0" : "1";
	});

    xctmp_push_filter(xctmp, "field.is_array", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->is_repeated() ? "1" : "0";
    });
    xctmp_push_filter(xctmp, "field.is_num", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        switch (m->field_desc->cpp_type()){
            case google::protobuf::FieldDescriptor::CPPTYPE_INT32 :
            case google::protobuf::FieldDescriptor::CPPTYPE_INT64 :
            case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
            case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
            case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
            case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
            case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
                return "1";
        }
        return "0";
    });

    xctmp_push_filter(xctmp, "field.is_bool", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        switch (m->field_desc->cpp_type()){
        case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
            return "1";
        }
        return "0";
    });

    xctmp_push_filter(xctmp, "field.is_enum", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        switch (m->field_desc->cpp_type()){
        case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
            return "1";
        }
        return "0";
    });
    xctmp_push_filter(xctmp, "field.enum_type", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return m->field_desc->default_value_enum()->name();
    });

    xctmp_push_filter(xctmp, "field.enum_default", [](const string & p)->string {
        EXTFieldMeta * m = (EXTFieldMeta*)(strtoull(p.c_str(), NULL, 16));
        return ::std::to_string(m->field_desc->default_value_enum()->number());
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
static inline void _str_trim_empty_line(::std::string & output){
    ::std::string::size_type beg = 0, fnd = 0;
    while (true){
        fnd = output.find('\n', beg);
        if (fnd == ::std::string::npos){
            break;
        }
        bool eptl = true;
        for (int i = beg; i <= fnd; ++i){
            if (output[i] != ' ' &&
                output[i] != '\t' &&
                output[i] != '\r' &&
                output[i] != '\n'){
                eptl = false;
                break;
            }
        }
        if (eptl && fnd >= beg){
            output.erase(beg, fnd - beg + 1);
        }
        else {
            beg = fnd + 1;
        }
    }
}
int	CXXFlatMsgGenerater::GetCodeText(::std::string & output,const char * tf){
    if (!xctmp && Init(tf)){
        ::std::cerr << "init pbdcex.core.hxx for templates error !" << ::std::endl;
        return -1;
    }
    TopologySort(ordered_desc);
    ::std::stringstream oss;

    time_t t_m = time(NULL);
    xctmp_push_s(xctmp, "time", asctime(localtime(&t_m)));

    const string proto_name_post_fix = ".proto";
    string this_proto_header_file_name = root->file()->name();
    this_proto_header_file_name.replace(this_proto_header_file_name.find(proto_name_post_fix), proto_name_post_fix.length(), ".pb.h");
    //depencies
    ::std::vector<::std::string>    vs;
    //import
    for (int i = 0; i < root->file()->dependency_count(); ++i){
        auto f = root->file()->dependency(i);
        string ns_name = f->name();
        if (ns_name == "extensions.proto"){
            continue;
        }
        ns_name.replace(ns_name.find(proto_name_post_fix), proto_name_post_fix.length(), ".cex.hpp");
        vs.push_back(ns_name);
    }
    vs.push_back(this_proto_header_file_name);
    xctmp_push_vs(xctmp, "depencies", vs);
    if (!root->file()->package().empty()){
        xctmp_push_s(xctmp, "ns_begin", "namespace " + root->file()->package() + " {");
        xctmp_push_s(xctmp, "ns_end", "}");
    }
    else {
        xctmp_push_s(xctmp, "ns_begin", "");
        xctmp_push_s(xctmp, "ns_end", "");
    }

    string this_file_name = root->file()->name();
    this_file_name.replace(this_file_name.find(proto_name_post_fix),
        proto_name_post_fix.length(), ".cex.hpp");
    xctmp_push_s(xctmp, "file_name", this_file_name);

    xctmp_push_filter(xctmp, "unif", [](const string & fn)->::std::string{
        ::std::string ts = fn;
        _strreplace(ts, ".", "_");
        _strreplace(ts, "/", "_");
        ::std::transform(ts.begin(), ts.end(),
                ts.begin(), ::toupper);
        return ts;
    });
    ////////////////////////////////////////////////////////////////////////////////////////////
    //convert
    ::std::vector<EXTMessageMeta> vems;
    vems.resize(ordered_desc.size());
    for (int i = 0; i < (int)ordered_desc.size(); ++i){
        if (vems[i].AttachDesc(ordered_desc[i])){
            fprintf(stderr, "parsing message type:%s error ! (check contraints)", ordered_desc[i]->full_name().c_str());
            return -2;
        }
    }
    //msg, meta , fields    
    char cvbuff[64] = { 0 };
    string jenv = "{\"msgs\":[";
    bool mhead = true;
    for (int i = 0; i < (int)vems.size(); ++i){
        if (!mhead){
            jenv += ",";
        }
        mhead = false;
        jenv += "{\"meta\":";
        snprintf(cvbuff, sizeof(cvbuff), "\"%p\"", &vems[i]);
        jenv += cvbuff;
        jenv += ",\"fields\":[";
        bool head = true;
        for (auto & sfm : vems[i].sub_fields){
            if (!head){
                jenv += ",";
            }
            head = false;
            snprintf(cvbuff, sizeof(cvbuff), "\"%p\"", &sfm);
            jenv += "{\"meta\":";
            jenv += cvbuff;
            jenv += "}";
        }
        jenv += "],\"pkfields\":[";
        head = true;
        for (auto & sfm : vems[i].pks_fields){
            if (!head){
                jenv += ",";
            }
            head = false;
            snprintf(cvbuff, sizeof(cvbuff), "\"%p\"", sfm);
            jenv += "{\"meta\":";
            jenv += cvbuff;
            jenv += "}";
        }
        jenv += "],\"pkfields_num\":" + ::std::to_string(vems[i].pks_fields.size());
        jenv += "}";
    }
    jenv += "]}";
    //::std::cout << jenv << ::std::endl;
    int ret = xctmp_render(xctmp, output, jenv);
    if (ret){
        fprintf(stderr, "templates render error :%d !", ret);
        return -4;
    }
    //clear empty line
    _str_trim_empty_line(output);
    return 0;
}
void	CXXFlatMsgGenerater::TopologySort(::std::vector<const Descriptor * > & msgs){
    msgs.clear();
    ::std::queue<const Descriptor *>  desc_queue;
    desc_queue.push(root);
    msg_degrees[root] = 0;
    while (!desc_queue.empty()){
        auto msg = desc_queue.front();
        desc_queue.pop();
        for (int i = 0; i < msg->field_count(); ++i){
            auto field_def = msg->field(i);
            if (field_def->type() == FieldDescriptor::TYPE_MESSAGE){
                const Descriptor * field_msg_type = field_def->message_type();
				if (field_msg_type->file()->name() != root->file()->name()) {
					continue; //just same one file
				}
                if (reverse_refer[field_msg_type].empty()){
                    desc_queue.push(field_msg_type);
                    msg_degrees[field_msg_type] = 0;
                }
                int md = reverse_refer[field_msg_type].size();
                reverse_refer[field_msg_type].insert(msg);
                if (md != (int)reverse_refer[field_msg_type].size()){
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
int cpph_generate(::std::string & code, const google::protobuf::Descriptor * msg, const char * tf){
    CXXFlatMsgGenerater cgen(msg);
    return cgen.GetCodeText(code, tf);
}
