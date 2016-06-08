struct {{meta|cs.msg.name}} {
    using pbdcex::string_t;
    using pbdcex::bytes_t;
    using pbdcex::array_t;
    ////////////////////////////////////////////////////////////////////////
    {{!for vf in fields}}{{vf.meta|cs.field.type}}       {{vf.meta|cs.field.name}};
    {{}}////////////////////////////////////////////////////////////////////////
    void    construct(){
        memset(this, 0, sizeof(*this));
    }
    int     convto({{meta|msg.name}} & tomsg_) const {
        int ret = 0;
        {{!for vf in fields}}{{!if vf.meta|field.is_array}}for (int i = 0;i < {{vf.meta|field.name}}.count; ++i){
            {{!if vf.meta|field.is_msg}}ret = this->{{vf.meta|cs.field.name}}.list[i].convto(*tomsg_.add_{{vf.meta|field.name}}());
            if (ret) return __LINE__+ret;
            {{!elif vf.meta|field.is_bytes }}tomsg_.add_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.list[i].data, this->{{vf.meta|cs.field.name}}.list[i].length);
            {{!elif vf.meta|field.is_string }}tomsg_.add_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.list[i].data);
            {{!else}}tomsg_.add_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.list[i]);{{}}
        }
        {{!else}}{{!if vf.meta|field.is_msg}}ret = this->{{vf.meta|cs.field.name}}.convto(*tomsg_.mutable_{{vf.meta|field.name}});
        if (ret) return __LINE__+ret;
        {{!elif vf.meta|field.is_bytes}}tomsg_.set_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.data, this->{{vf.meta|cs.field.name}}.length);
        {{!elif vf.meta|field.is_string}}tomsg_.set_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}}.data);
        {{!else}}tomsg_.set_{{vf.meta|field.name}}(this->{{vf.meta|cs.field.name}});
        {{}}{{}}{{}}
        return ret;
    }
    int     convfrom(const {{meta|msg.name}} & frommsg_) {
		int ret = 0;
        {{!for vf in fields}}{{!if vf.meta|field.is_array}}
        this->{{vf.meta|cs.field.name}}.count=0;
        for (int i = 0;i < frommsg_.{{vf.meta|field.name}}_size() && i < {{vf.meta|field.count}}; ++i,++(this->{{vf.meta|cs.field.name}}.count)){ {{!if vf.meta|field.is_msg}}
            ret = this->{{vf.meta|cs.field.name}}.list[i].convfrom(frommsg_.{{vf.meta|field.name}}());
            if (ret) return __LINE__+ret;
            {{!elif vf.meta|field.is_bytes}}
            assert({{vf.meta|field.name}}(i).length() <= {{vf.meta|field.length}});
            this->{{vf.meta|cs.field.name}}.list[i].length = {{vf.meta|field.name}}(i).length();
            memcpy(this->{{vf.meta|cs.field.name}}.list[i].data, frommsg_.{{vf.meta|field.name}}(i).data(),
                  {{vf.meta|field.name}}(i).length());{{!elif vf.meta|field.is_string}}
            assert({{vf.meta|field.name}}(i).length() < {{vf.meta|field.length}});
            memcpy(this->{{vf.meta|cs.field.name}}.list[i].data, frommsg_.{{vf.meta|field.name}}(i).data(),
                   this->{{vf.meta|cs.field.name}}.list[i].length);
            this->{{vf.meta|cs.field.name}}.list[i].data[this->{{vf.meta|cs.field.name}}.list[i].length]=0;
            {{!else}}this->{{vf.meta|cs.field.name}}.list[i] = frommsg_.{{vf.meta|field.name}};
            {{}}
        }
        {{!else}}{{!if vf.meta|field.is_msg}}
        ret = this->{{vf.meta|cs.field.name}}.convfrom(*frommsg_.{{vf.meta|field.name}}());
        if (ret) return __LINE__+ret;
        {{!elif vf.meta|field.is_bytes}}assert({{vf.meta|field.name}}.length() <= {{vf.meta|field.length}});
        this->{{vf.meta|cs.field.name}}.length = {{vf.meta|field.name}}().length();
        memcpy(this->{{vf.meta|cs.field.name}}.data, frommsg_.{{vf.meta|field.name}}().data(),
               {{vf.meta|field.name}}().length());
        {{!elif vf.meta|field.is_string}}assert({{vf.meta|field.name}}().length() < {{vf.meta|field.length}});
        memcpy(this->{{vf.meta|cs.field.name}}.data, frommsg_.{{vf.meta|field.name}}().data(),
               this->{{vf.meta|cs.field.name}}.length);
        this->{{vf.meta|cs.field.name}}.data[this->{{vf.meta|cs.field.name}}.length]=0;
        {{!else}}this->{{vf.meta|cs.field.name}} = frommsg_.{{vf.meta|field.name}}();
        {{}}{{}}{{}}
        return ret;
    }
    int     compare(const msg.name | cs.msg_type_name & rhs_) const {
        int cmp = 0;{{!for vf in pkfields}}{{!if vf.meta|field.is_array}}
        cmp = this->{{vf.meta|cs.field.name}}.compare(rhs_.{{vf.meta|cs.field.name}});
        if(cmp){return cmp;}
        {{!elif vf.meta|field.is_msg}}cmp = this->{{vf.meta|cs.field.name}}.compare(rhs_.{{vf.meta|cs.field.name}});
        if(cmp){return cmp;}{{!else}}
        if (this->{{vf.meta|cs.field.name}} < rhs_.{{vf.meta|cs.field.name}}){
            return -1;
        }
        else if(this->{{vf.meta|cs.field.name}} > rhs_.{{vf.meta|cs.field.name}}){
            return 1;
        }{{}}{{}}
        return 0;
    }
    bool    operator == (const msg.name | cs.msg_type_name & rhs_) const {
        return this->compare(rhs_) == 0;
    }
    bool    operator < (const msg.name | cs.msg_type_name & rhs_) const {
        return this->compare(rhs_) < 0;
    }

}
