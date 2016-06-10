#ifndef __PBDCEX_CORE_EX_HPP_XX__
#define __PBDCEX_CORE_EX_HPP_XX__
#include <algorithm>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <string>
namespace pbdcex {
template<class T, class P>
struct serializable_t {
    int dumps(std::string & str) const {
        P pm;
        int ret = reinterpret_cast<const T*>(this)->convto(pm);
        if (ret){
            return ret;
        }
        if (!pm.SerializeToString(str)){
            return -1;
        }
        return 0;
    }
    int  loads(const std::string & str){
        P pm;
        if (!pm.ParseFromString(str)){
            return -1;
        }
        int ret = reinterpret_cast<T*>(this)->convfrom(pm);
        if (ret){
            return ret;
        }
        return 0;
    }
    int   loads(const char * buff, int ibuff){
        P pm;
        if (!pm.ParseFromArray(buff, ibuff)){
            return -1;
        }
        int ret = reinterpret_cast<T*>(this)->convfrom(pm);
        if (ret){
            return ret;
        }
        return 0;
    }
    const char * debugs(std::string & str) const {
        P pm;
        reinterpret_cast<const T*>(this)->convto(pm);
        str = pm.ShortDebugString();
        return str.c_str();
    };
};

template<size_t lmax>
struct string_t {
    char data[lmax];
    /////////////////////////////////////////////////////////////
    static string_t & assign(string_t & str, const char * cs){
        str.assign(cs);
        return str;
    }
    void  construct(){
        this->data[0] = 0;
    }
    size_t format(const char * fmt, ...){
        va_list ap;
        va_start(ap, fmt);
        size_t n=vsnprintf(data, lmax-1, fmt, ap);
        data[lmax-1]=0;
        va_end(ap);
        return n;
    }
    int assign(const char * s){
        if (!s){ return 0; }
        int l = strlen(s);
        if (l > lmax){ l = lmax - 1; }
        memcpy(data, s, l);
        data[l]=0;
        return l;
    }
    int assign(const std::string & s){
        return this->assign(s.data());
    }
    int clone(char * s, int len) const {
        if (!s){ return 0; }
        int xlen = this->length();
        if (xlen > len){ xlen = len - 1; }
        memcpy(s, data, xlen);
        data[xlen] = 0;
        return xlen;
    }
    operator char * (){
        return data;
    }
    size_t length() const {
        return strlen(data);
    }
    bool operator == (const string_t & rhs) const {
        return compare(rhs) == 0;
    }
    bool operator < (const string_t & rhs) const {
        return compare(rhs) < 0;
    }
    int     compare(const string_t & rhs) const {
        return strcmp(data, rhs.data);
    }
    size_t hash() const { //sdbm_hash? std::hash?
        std::hash<const char *> hcs;
        return hcs((char*)data);
    }
};

template<size_t lmax, class LengthT=unsigned int>
struct bytes_t {
    unsigned char data[lmax];
    LengthT  length;
    //////////////////////////////////////////////////////
    void  construct(){
        this->data[0] = 0;
        this->length = 0;
    }
    LengthT assign(const char * s, int len){
        LengthT l = len < (int)length ? len : length;
        memcpy(data, s, l);
        return l;
    }
    LengthT assign(const std::string & s){
        return this->assign(s.data(), s.length());
    }
    LengthT clone(char * s, int len) const {
        LengthT l = len < (int)length ? len : length;
        memcpy(s, data, l);
        return l;
    }
    bool operator == (const bytes_t & rhs) const {
        return compare(rhs) == 0;
    }
    bool operator < (const bytes_t & rhs) const {
        return compare(rhs) < 0;
    }
    int     compare(const bytes_t & rhs) const {
        if (length < rhs.length){
            return memcmp(data, rhs.data, length);
        }
        else {
            return memcmp(data, rhs.data, rhs.length);
        }
    }
    size_t hash() const { //sdbm_hash? std::hash?
        std::hash<std::string> hcs;
        std::string str;
        for (int i = 0; i < length; ++i){
            str.push_back(data[i]);
        }
        return hcs(str);
    }
};

template<class T, size_t cmax, class LengthT=unsigned int>
struct array_t {
    T list[cmax];
    LengthT count;

    /////////////////////////////////
    void construct(){
        count = 0;
    }
    size_t capacity() const {
        return cmax;
    }
    bool operator == (const array_t & rhs) const {
        return compare(rhs) == 0;
    }
    bool operator < (const array_t & rhs) const {
        return compare(rhs) < 0;
    }
    int  compare(const array_t & rhs) const {
        int ret = 0;
        if (count < rhs.count){
            for (int i = 0; i < count; ++i){
                ret = list[i].compare(rhs.list[i]);
                if (ret){
                    return ret;
                }
            }
        }
        else {
            for (int i = 0; i < rhs.count; ++i){
                ret = list[i].compare(rhs.list[i]);
                if (ret){
                    return ret;
                }
            }
        }
        return (int)(count - rhs.count);
    }
    ////////////////////////////////////////////////////
    //linear list///////////////////////////////////////
    int lfind(const T & tk) const {
        for (size_t i = 0; i < count && i < cmax; ++i){
            if (list[i] == tk){
                return i;
            }
        }
        return -1;
    }
    int lappend(const T & td, bool shift_overflow = false){
        if (count >= cmax && !shift_overflow){
            return -1;
        }
        if (count < cmax){
            list[count] = td;
            ++count;
            return 0;
        }
        else {
            if (cmax > 0){
                memmove(list, list + 1, (cmax - 1)*sizeof(T));
                list[cmax - 1] = td;
            }
        }
        return 0;
    }
    int lremove(int idx, bool swap_remove = false){
        if (idx < 0 || idx >= count){
            return -1;
        }
        if (swap_remove){
            list[idx] = list[cmax - 1];
            list[cmax - 1].construct();
        }
        else {
            memmove(list + idx, list + idx + 1, count - idx - 1);
        }
        --count;
        return 0;
    }
    int linsert(int idx, const T & td, bool overflow_shift = false){
        if (count >= cmax && !overflow_shift){
            return -1;
        }
        if (idx >= count){
            idx = count;
        }
        else if (idx < 0){
            idx = 0;
        }
        if (idx == count){
            return lappend(td, overflow_shift);
        }
        //--overlay------idx------>-----idx+1------------------------------
        if (count >= cmax){
            memmove(list + idx + 1, list + idx, (cmax - 1 - idx)*sizeof(T));
            list[idx] = td;
        }
        else {
            memmove(list + idx + 1, list + idx, (count - idx)*sizeof(T));
            list[idx] = td;
            ++count;
        }
        return 0;
    }
    void lsort(array_t & out) const {
        memcpy(&out, this, sizeof(*this));
        std::sort(out.list, out.list + out.count);
    }
    //////////////////////////////////////////////////////////
    /////////////////////binary-seaching//////////////////////
    int bfind(const T & tk) const {
        int idx1st = lower_bound(tk);
        int idx2nd = upper_bound(tk);
        if (idx1st == idx2nd){
            return -1;
        }
        while (idx1st < idx2nd){
            if (tk == list[idx1st]){
                return idx1st;
            }
            ++idx1st;
        }
        return -1;
    }
    int binsert(const T & td){
        int idx = lower_bound(td);
        return linsert(idx, td, false);
    }
    int bremove(const T & tk){
        int idx = bfind(tk);
        if (idx < 0){ return -1;}
        return lremove(idx, false);
    }
    int lower_bound(const T & tk) const {
        const T * p = std::lower_bound((T*)list, (T*)list + count, (T&)tk);
        return (p - list);
    }
    int upper_bound(const T & tk) const {
        const T * p = std::upper_bound((T*)list, (T*)list + count, (T&)tk);
        return (p - list);
    }
    T & operator [](size_t idx){
        assert(idx < count);
        return list[idx];
    }
    const T & operator [](size_t idx) const {
        assert(idx < count);
        return list[idx];
    }
};

template<class T, size_t cmax>
struct mmpool_t {
    typedef uint64_t mmpool_bit_block_t; //typedef unsigned long long mmpool_bit_block_t;    
    #define mmpool_bit_block_byte_sz  (sizeof(mmpool_bit_block_t))
    #define mmpool_bit_block_bit_sz  (8*mmpool_bit_block_byte_sz)
    #define mmpool_bit_block_count ((cmax+mmpool_bit_block_bit_sz-1)/mmpool_bit_block_bit_sz)
    typedef array_t<T, mmpool_bit_block_count*mmpool_bit_block_bit_sz>    allocator_t;
    allocator_t                 allocator_;
    mmpool_bit_block_t          bmp_[mmpool_bit_block_count];
    size_t                      used_;
    /////////////////////////////////////////////////////////////////////////////////////////
    void   construct(){
        memset(this, 0, sizeof(*this));
        allocator_.count = cmax;
        for (int i = 0; i < mmpool_bit_block_count; ++i){
            bmp_[i] = 0xFFFFFFFFFFFFFFFFULL;//all set 1
        }
        used_ = 0;
    }
    const allocator_t & allocator() {
        return allocator_;
    }
    size_t alloc(){
        if (used() >= capacity()){
            return 0; //full
        }
        //1.find first 0 set 1
        size_t x = 0;
        size_t i = 0;
        for (; i < mmpool_bit_block_count; ++i){
            if((x = __builtin_ffsll(bmp_[i]))){
                break;
            }
        }
        if(x != 0){
            bmp_[i] &= (~(1ULL<<(x-1)));//set 0
            size_t id=i*mmpool_bit_block_bit_sz+x;
            ++used_;
            new (&(allocator_.list[id - 1]))T();
            return id;
        }
        else {
            return 0;
        }

    }
    size_t id(const T * p) const {
        assert(p >= allocator_.list && p < allocator_.list + cmax);
        return 1 + (p - allocator_.list);
    }
    T * ptr(size_t id) {
        if(id > 0 && id <= capacity() && isbusy(id)){
           return &allocator_.list[id-1];
        }
        else {
            return NULL;
        }
    }
    bool isbusy(size_t id){
        assert(id > 0);
        size_t idx=id-1;
        return !(bmp_[idx/mmpool_bit_block_bit_sz]&(1ULL<<(idx%mmpool_bit_block_bit_sz)));
    }
    size_t capacity() const {
        return cmax;
    }
    bool   empty() const {
        return used() == 0;
    }
    size_t used() const {
        return used_;
    }
    size_t next(size_t it) const {
        if (used_ == 0){
            return 0;
        }
        it = it + 1;
        for (size_t i = (it - 1) / mmpool_bit_block_bit_sz; i < sizeof(bmp_)/sizeof(bmp_[0]); ++i){
            if(bmp_[i] != 0xFFFFFFFFFFFFFFFFULL){
                for (int j = (it - 1) % mmpool_bit_block_bit_sz; j < mmpool_bit_block_bit_sz; ++j){
                    if ((bmp_[i] & (1ULL << j)) == 0){//find first 0
                       return i*mmpool_bit_block_bit_sz + j + 1;
                   }
               }
            }
        }
        return 0;
    }
    void   free(size_t id){
        assert(id > 0);
        size_t idx = id - 1;
        if(isbusy(id)){ //set 1
            bmp_[idx / mmpool_bit_block_bit_sz] |= (1ULL << (idx%mmpool_bit_block_bit_sz));
            allocator_.list[idx].~T();
            --used_;
        }
    }
};


template<size_t n>
struct next_prime {
    enum { value = is_prime<n + 1>::value ? n + 1 : next_prime<n + 1>};
};

template<size_t n, size_t i>
struct multiple {
    enum { value = n % i ? 0: 1};
};

template<size_t n>
struct is_prime_loop_i<n, 2> {
    enum {
        value = n % 2;
    };
};

template<size_t n, size_t i>
struct is_prime_loop_i {
    enum {
        value = multiple<n, i>::value ? 0 : is_prime<n, i - 1>::value
    };
};

template<size_t n>
struct is_prime {
    enum {
        value = is_prime_loop_i<n>::value
    };
};

template<size_t n, size_t m>
struct n_next_prime_sum {
    enum {
        value = next_prime<n>::value + n_next_prime_sum<next_prime<n>::value, m - 1>::value;
    };
};
template<size_t n>
struct n_next_prime_sum<n, 0> {
    enum { value = 0 };
};


template<class T, size_t cmax, class hcf, size_t layer = 3>
struct hashmap_t {
    struct hashmap_entry_t {
        size_t  id;
        size_t  next;
    };
    size_t  hash_layer_size[layer];
    //(cmax*100/load_factor)
    #define hash_entry_index_size   (n_next_prime_sum<cmax, layer>::value) 
    typedef mmpool_t<T, cmax>                                     pool_t;
    typedef array_t<hashmap_entry_t, hash_entry_index_size+8>     index_t;
    index_t     index_;
    pool_t      mmpool_;
    size_t      stat_nread;
    size_t      stat_nhit;
    ////////////////////////////////////////////////////////////
    void        construct(){
        memset(&index_, 0, sizeof(index_));
        index_.count = hash_entry_index_size;
        mmpool_.construct();
        stat_nread = stat_nhit = 1;//for div 0 error
    }
    const pool_t &  mmpool() const {return mmpool_;}
    int         load(int rate = 100) const {
        return mmpool_.used() * rate / index_.capacity();
    }
    int         factor(){
        return load_factor;
    }
    int         hit(int rate = 100) const {
        return stat_nhit * rate / stat_nread;
    }
    bool        empty() const {
        return mmpool_.empty();
    }
    bool        full() const {
        return mmpool_.used() >= cmax;
    }
    T *         insert(const T & k){
        if (full()){
            return NULL; //full
        }
        //need to optimalization
        size_t hco = hcf()(k);
        size_t idx = hco % hash_entry_index_size + 1;
        while(index_.list[idx].id){//find collision queue tail
            ++stat_nread;
            if (*mmpool_.ptr(index_.list[idx].id) == k){
                return NULL;
            }
            if(index_.list[idx].next == 0){
                break;
            }
            idx = index_.list[idx].next;
        }
        if(idx > 0){ //valid idx
            size_t max_retry = hash_entry_index_size;
            if(index_.list[idx].id > 0){//next , find next pos
                size_t npos = (idx+1) % hash_entry_index_size + 1;
                while(max_retry--){ //find a empty node
                    ++stat_nread;
                   if(index_.list[npos].id == 0){
                       break;
                   }
                   npos = (npos+1) % hash_entry_index_size + 1;
                }
                size_t hid = mmpool_.alloc();
                if(hid == 0){
                    return NULL;
                }
                //#list append collision queue
                index_.list[idx].next = npos;
                index_.list[npos].id = hid;
                index_.list[npos].next = 0;
                ++stat_nhit;
                return mmpool_.ptr(hid);
            }
            else { //head new collison queue
                size_t hid = mmpool_.alloc();
                if(hid == 0){
                    return NULL;
                }
                hashmap_entry_t & he = index_.list[idx];
                he.id = hid;
                he.next = 0;
                ++stat_nhit;
                return mmpool_.ptr(hid);
            }
        }
        return NULL;
    }
    int         remove(const T & k){
        //need to optimalization
        size_t hco = hcf()(k);
        size_t idx = hco % hash_entry_index_size + 1;
        size_t pidx = 0;
        while(idx){
            if(index_.list[idx].id){
                ++stat_nread;
                T * p = mmpool_.ptr(index_.list[idx].id);
                if (*p == k){
                    ++stat_nhit;
                    //remove ith
                    mmpool_.free(index_.list[idx].id);
                    index_.list[idx].id = 0;
                    if(pidx){
                        index_.list[pidx].next = index_.list[idx].next;
                    }
                    index_.list[idx].next = 0;
                    return 0;
                }
            }
            pidx = idx;
            idx = index_.list[idx].next;
        }
        return -1;
    }
    T *         find(const T & k){
        //need to optimalization
        size_t hco = hcf()(k);
        size_t idx = hco % hash_entry_index_size;
        size_t max_retry = hash_entry_index_size;
        while(index_.list[idx].id){
            ++stat_nread;
            T * p = mmpool_.ptr(index_.list[idx].id);
            if (*p == k){
                ++stat_nhit;
                return p;
            }
            idx = index_.list[idx].next;
        }
        return NULL;
    }

};


};





#endif
