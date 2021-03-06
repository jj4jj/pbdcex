#ifndef __PBDCEX_CORE_EX_HPP_XX__
#define __PBDCEX_CORE_EX_HPP_XX__
#pragma once
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <string>
#include <algorithm>
namespace pbdcex {
namespace util {
    static inline uint64_t FNV_1A_Hash64(const unsigned char * data, size_t zdata){
        uint64_t hashv = 14695981039346656037ULL;
        if (zdata > 0){
            for (size_t i = 0; i < zdata; ++i)
            {	// fold in another byte
                hashv ^= (uint64_t)data[i];
                hashv *= 1099511628211ULL;
            }
        }
        else {
            for (size_t i = 0; data[i] != 0 ; ++i)
            {	// fold in another byte
                hashv ^= (uint64_t)data[i];
                hashv *= 1099511628211ULL;
            }
        }
        hashv ^= hashv >> 32;
        return (hashv);
    }
    static inline size_t FNV_1A_Hash32(const unsigned char * data, size_t zdata){
        uint32_t hashv = 2166136261U;
        if (zdata > 0){
            for (size_t i = 0; i < zdata; ++i)
            {	// fold in another byte
                hashv ^= (uint32_t)data[i];
                hashv *= 16777619U;
            }
        }
        else {
            for (size_t i = 0; data[i] != 0; ++i)
            {	// fold in another byte
                hashv ^= (uint32_t)data[i];
                hashv *= 16777619U;
            }
        }
        return (hashv);
    }
    static inline bool   _is_prime(size_t n){
        for (size_t i = 2; i < n / 2; ++i){
            if (n % i == 0){
                return false;
            }
        }
        return true;
    }
    static inline size_t _next_prime_bigger(size_t n){
        ++n;
        while (true){
            if (util::_is_prime(n)){
                return n;
            }
            ++n;
        }
    }
    static inline size_t _next_prime_smaller(size_t n){
        --n;
        while (true){
            if (util::_is_prime(n)){
                return n;
            }
            --n;
        }
    }

    template<class T>
    struct Hash {
        size_t operator ()(const T & td) const {
            return td.hash();
        }
    };
	template<>
    struct Hash<int8_t> {
        size_t operator ()(const int8_t & td) const {
            return td;
        }
    };
	template<>
    struct Hash<uint8_t> {
        size_t operator ()(const uint8_t & td) const {
            return td;
        }
    };
 	template<>
    struct Hash<int16_t> {
        size_t operator ()(const int16_t & td) const {
            return td;
        }
    };
	template<>
    struct Hash<uint16_t> {
        size_t operator ()(const uint16_t & td) const {
            return td;
        }
    };
    template<>
    struct Hash<int32_t> {
        size_t operator ()(const int32_t & td) const {
            return td;
        }
    };
	template<>
    struct Hash<uint32_t> {
        size_t operator ()(const uint32_t & td) const {
            return td;
        }
    };
	template<>
    struct Hash<int64_t> {
        size_t operator ()(const int64_t & td) const {
            return td;
        }
    };
	template<>
    struct Hash<uint64_t> {
        size_t operator ()(const uint64_t & td) const {
            return td;
        }
    };
    template<>
    struct Hash<float> {
        size_t operator ()(const float & td) const {
            return (size_t)td;
        }
    };  
    template<>
    struct Hash<double> {
        size_t operator ()(const double & td) const {
            return (size_t)td;
        }
    };
}
inline size_t hash_code_merge_multi_value(size_t vs[], size_t n){
    //64bytes optimal todo
    return util::FNV_1A_Hash64((unsigned char*)&vs[0], n*sizeof(size_t));
}
template<class T, class P>
struct serializable_t {
    int dumps(::std::string & str) const {
        P pm;
        reinterpret_cast<const T*>(this)->convto(pm);
        if (!pm.SerializeToString(&str)) {
            return -1;
        }
        return 0;
    }
    int  loads(const ::std::string & str){
        P pm;
        if (!pm.ParseFromString(str)){
            return -1;
        }
        int ret = reinterpret_cast<T*>(this)->check_convfrom(pm);
        if (ret){
            return ret;
        }
        reinterpret_cast<T*>(this)->convfrom(pm);
        return 0;
    }
    int   loads(const char * buff, int ibuff){
        P pm;
        if (!pm.ParseFromArray(buff, ibuff)){
            return -1;
        }
        int ret = reinterpret_cast<T*>(this)->check_convfrom(pm);
        if (ret){
            return ret;
        }
        reinterpret_cast<T*>(this)->convfrom(pm);
        return 0;
    }
    const char * debugs(::std::string & str) const {
        P pm;
        reinterpret_cast<const T*>(this)->convto(pm);
        str = pm.ShortDebugString();
        return str.c_str();
    };
};

template<size_t lmax>
struct string_t {
    char    data[lmax];
    /////////////////////////////////////////////////////////////
    size_t hash() const { //fnv
        return util::FNV_1A_Hash64((unsigned char *)data, 0);
    }
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
        if (!s){ return 0;}
        size_t l = strlen(s);
        if (l > lmax){ l = lmax - 1; }
        memcpy(data, s, l);
        data[l]=0;
        return l;
    }
    int assign(const ::std::string & s){
        return this->assign(s.data());
    }
    string_t & operator = (const char * s) {
        this->assign(s);
        return *this;
    }
    string_t & operator = (const ::std::string & str) {
        this->assign(str);
        return *this;
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
	bool operator > (const string_t & rhs) const {
        return compare(rhs) > 0;
    }
    int     compare(const string_t & rhs) const {
        return strcmp(data, rhs.data);
    }
};

template<size_t lmax, class LengthT = unsigned int>
struct bytes_t {
    unsigned char   data[lmax];
    LengthT         length;
    //////////////////////////////////////////////////////
    size_t hash() const { //fnv
        return util::FNV_1A_Hash64(data, length);
    }
    void  construct(){
        this->data[0] = 0;
        this->length = 0;
    }
    LengthT assign(const char * s, int len){
        LengthT l = len < (int)length ? len : length;
        memcpy(data, s, l);
        return l;
    }
    LengthT assign(const ::std::string & s){
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
	bool operator > (const bytes_t & rhs) const {
        return compare(rhs) > 0;
    }
    int     compare(const bytes_t & rhs) const {
        if (length < rhs.length){
            return memcmp(data, rhs.data, length);
        }
        else {
            return memcmp(data, rhs.data, rhs.length);
        }
    }
};

template<class T, size_t cmax, class LengthT=uint32_t>
struct array_t {
    T           list[cmax];
    LengthT     count;

    /////////////////////////////////
    size_t hash() const {
        //five element 
        if (this->count == 0U){
            return 13131313U;
        }
        size_t hvs[5] = { 0 };
        size_t hvl = this->count;
        size_t gap = 1;
        auto hf = util::Hash<T>();
        if (this->count > 5U){
            hvl = 5;
            gap = this->count / 5U;
        }
        for (LengthT i = 0; i < this->count; i += gap) {
            hvs[i] = hf(this->list[i]);
        }
        return util::FNV_1A_Hash64((unsigned char *)hvs, hvl*sizeof(size_t));
    }
    void construct(){
        count = 0;
    }
    size_t capacity() const {
        return cmax;
    }
    bool   full() const {
        return this->count >= cmax;
    }
    bool   empty() const {
        return this->count == 0;
    }
    void   clear() {
        this->count = 0;
    }
    bool operator == (const array_t & rhs) const {
        return compare(rhs) == 0;
    }
    bool operator < (const array_t & rhs) const {
        return compare(rhs) < 0;
    }
    bool operator > (const array_t & rhs) const {
        return compare(rhs) > 0;
    }
    int  compare(const array_t & rhs) const {
        if (count < rhs.count){
            for (LengthT i = 0; i < count; ++i){
                if (list[i] < rhs.list[i]){
                    return -1;
                }
                else if (!(list[i] == rhs.list[i])){
                    return 1;
                }
            }
        }
        else {
            for (LengthT i = 0; i < rhs.count; ++i){
                if (list[i] < rhs.list[i]){
                    return -1;
                }
                else if (!(list[i] == rhs.list[i])){
                    return 1;
                }
            }
        }
        return (int)(count - rhs.count);
    }
    //linear list///////////////////////////////////////
    int lfind(const T & tk) const {
        for (LengthT i = 0; i < count && i < cmax; ++i){
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
        if (idx < 0 || (LengthT)idx >= count){
            return -1;
        }
        if (swap_remove){
            list[idx] = list[cmax - 1];
            //list[cmax - 1].construct();
        }
        else {
            memmove(list + idx, list + idx + 1, (count - idx - 1)*sizeof(T));
        }
        --count;
        return 0;
    }
    int linsert(int idx, const T & td, bool overflow_shift = false){
        if (count >= cmax && !overflow_shift){
            return -1;
        }
        if ((LengthT)idx >= count){
            idx = count;
        }
        else if (idx < 0){
            idx = 0;
        }
        if ((LengthT)idx == count){
            return lappend(td, overflow_shift);
        }
        //--overlay------idx------>-----idx+1------------------------------
		assert(count <= cmax);
        if (count == cmax){
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
        ::std::sort(out.list, out.list + out.count);
    }
    //////////////////////////////////////////////////////////
    /////////////////////binary-seaching//////////////////////
    int bfind(const T & tk) const {
        LengthT idx1st = lower_bound(tk);
        if (idx1st < count && list[idx1st] == tk){
            return idx1st;
        }
        return -1;
    }
    int binsert(const T & td, bool overflow_shift = false, bool uniq = false) {
        LengthT idx = lower_bound(td);
        if (uniq && idx < count && list[idx] == td) {
            return -1;
        }
        return linsert(idx, td, overflow_shift);
    }
    int bremove(const T & tk){
        int idx = bfind(tk);
        if (idx < 0){ return -1;}
        return lremove(idx, false);
    }
    LengthT lower_bound(const T & tk) const {
        const T * p = ::std::lower_bound(list, list + count, tk);
        return (p - list);
    }
    LengthT upper_bound(const T & tk) const {
        const T * p = ::std::upper_bound(list, list + count, tk);
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
        size_t x,i;
        for (i = 0, x = 0; i < mmpool_bit_block_count; ++i){
            if((x = __builtin_ffsll(~(bmp_[i])))){
                break;
            }
        }
        if(x != 0){
            bmp_[i] |= (1ULL<<(x-1));//set 1
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
           return &(allocator_.list[id-1]);
        }
        else {
            return NULL;
        }
    }
    bool isbusy(size_t id){
        assert(id > 0);
        size_t idx=id-1;
        return bmp_[idx/mmpool_bit_block_bit_sz]&(1ULL<<(idx%mmpool_bit_block_bit_sz));
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
        bool head = true; //(it+1)-1 = idx
        uint32_t pos_bit_ffs = 0;
        for (size_t idx = it / mmpool_bit_block_bit_sz, pos_bit_offset = it % mmpool_bit_block_bit_sz;
            idx < sizeof(bmp_) / sizeof(bmp_[0]); ++idx) {
            if (!head) {
                pos_bit_offset = 0;
            }
            else {
                head = false;
            }
            if (bmp_[idx] != 0ULL) {
                pos_bit_ffs = __builtin_ffsll(bmp_[idx] >> pos_bit_offset);
                if (pos_bit_ffs > 0) {
                    return idx*mmpool_bit_block_bit_sz + pos_bit_offset + pos_bit_ffs;
                }
            }
        }
        return 0;
    }
    void   free(size_t id){
        assert(id > 0);
        size_t idx = id - 1;
        if(isbusy(id)){ //set 0
            bmp_[idx / mmpool_bit_block_bit_sz] &= (~(1ULL << (idx%mmpool_bit_block_bit_sz)));
            --used_;
            allocator_.list[idx].~T();
        }
    }
};
//multi layer hash table implementation
//----------
//------------------
//----------------------------
//collision strategy : 
//1.create a link list in multiple layer
//2.in last max layer linear probe solving

template<class T, size_t cmax, size_t layer = 3, class hcfT = util::Hash<T>>
struct hashtable_t {
    struct hashmap_entry_t {
        size_t  id;
        size_t  next;
        size_t  hco;
    };
    struct {
        size_t  offset;
        size_t  count;
    } hash_layer_segment[layer];
    #define hash_entry_index_size   (layer*cmax*150/100)
    typedef mmpool_t<T, cmax>                                     pool_t;
    typedef array_t<hashmap_entry_t, hash_entry_index_size+8>     index_t;
    index_t     index_;
    pool_t      mmpool_;
    size_t      stat_probe_insert;
    size_t      stat_insert;
    size_t      stat_probe_read;
    size_t      stat_hit_read;
    ////////////////////////////////////////////////////////////
    void        construct(){
        memset(&index_, 0, sizeof(index_));
        index_.count = hash_entry_index_size;
        mmpool_.construct();
        stat_probe_insert = stat_insert = 
            stat_hit_read = stat_probe_read = 1;//for div 0 error
        //bigger and bigger but max is limit
        hash_layer_segment[0].offset = 1;
        size_t hash_layer_max_size = util::_next_prime_bigger(cmax);
        hash_layer_segment[layer - 1].count = hash_layer_max_size;
        for (int i = layer - 2 ; i >= 0; --i){
            hash_layer_segment[i].count = util::_next_prime_smaller(hash_layer_segment[i + 1].count);
        }
        for (size_t i = 1; i < layer; ++i){
            hash_layer_segment[i].offset = hash_layer_segment[i - 1].offset + hash_layer_segment[i - 1].count;
        }
        assert(hash_entry_index_size >= hash_layer_segment[layer - 1].count + hash_layer_segment[layer - 1].offset);
    }
    const pool_t &  mmpool() const {return mmpool_;}
    int         load(int rate = 100) const {
        return mmpool_.used() * rate / index_.capacity();
    }
    int         factor(){
        return cmax * 100 / index_.capacity();
    }
    int         hit(int rate = 100) const {
        return stat_hit_read * rate / stat_probe_read;
    }
    int         collision() const {
        return stat_probe_insert / stat_insert;
    }
    const char * layers(::std::string & str) const {
        for (int i = 0; i < layer; ++i){
            str.append("[" +
                ::std::to_string(this->hash_layer_segment[i].offset) + "," +
                ::std::to_string(this->hash_layer_segment[i].count) + ")");
        }
        return str.c_str();
    }
    const char * stat(::std::string & str){
        str += "mbytes size:" + ::std::to_string(sizeof(*this)) +
            " mused:" + ::std::to_string(this->mmpool().used()) +"/"+::std::to_string(cmax) +
            " musage:" + ::std::to_string(this->mmpool().used() / cmax) +
            " iload:" + ::std::to_string(this->load()) +
            " ihit:" + ::std::to_string(this->hit()) +
            " ifactor:" + ::std::to_string(this->factor()) +
            " icollision:" + ::std::to_string(this->collision()) +
            " ilayers:" + ::std::to_string(layer);
        return this->layers(str);
    }
    bool        empty() const {
        return mmpool_.empty();
    }
    bool        full() const {
        return mmpool_.used() >= cmax;
    }
    size_t      next_slot(size_t hc, size_t ref){
        assert(ref > 0);
        assert(index_.list[ref].next == 0);
        size_t find_slot = 0;
        if (ref < hash_layer_segment[layer - 1].offset){
            for (size_t i = 0; i < layer - 1; ++i){
                ++stat_probe_insert;
                find_slot = hash_layer_segment[i].offset + hc % hash_layer_segment[i].count;
                if (0 == index_.list[find_slot].id){
                    return find_slot;
                }
            }
        }
        //next one idle
        find_slot = ref;
        while (true){
            ++stat_probe_insert;
            find_slot = hash_layer_segment[layer - 1].offset + (find_slot + 1) % hash_layer_segment[layer - 1].count;
            if (0 == index_.list[find_slot].id){
                return find_slot;
            }
        }
        assert(false);
        return 0;
    }
    T *         insert(const T & k){
        if (full()){
            return NULL; //full
        }
        //need to optimalization
        size_t hco = hcfT()(k);
        size_t idx = 1 + hco % hash_layer_segment[0].count;
        T * pv = NULL;
        while(idx && index_.list[idx].id){//find collision queue available
            ++stat_probe_insert;
            pv = mmpool_.ptr(index_.list[idx].id);
            if (index_.list[idx].hco == hco && *pv == k){
                return NULL;
            }
            if(index_.list[idx].next == 0){
                break;
            }
            idx = index_.list[idx].next;
        }
        if(idx > 0){ //valid idx
            if(index_.list[idx].id > 0){ //link list tail
                assert(index_.list[idx].next == 0);
                size_t npos = next_slot(hco, idx);
                assert(npos > 0 && index_.list[npos].id == 0);
                size_t hid = mmpool_.alloc();
                if(hid == 0){
                    return NULL;
                }
                //#list append collision queue
                index_.list[idx].next = npos;
                index_.list[npos].id = hid;
                index_.list[npos].hco = hco;
                index_.list[npos].next = 0;
                ++stat_insert;
                pv = mmpool_.ptr(hid);
                *pv = k;
                return pv;
            }
            else { //head pos or in layer link list node
                size_t hid = mmpool_.alloc();
                if(hid == 0){
                    return NULL;
                }
                hashmap_entry_t & he = index_.list[idx];
                he.id = hid;
                he.hco = hco;
                ++stat_insert;
                pv = mmpool_.ptr(hid);
                *pv = k;
                return pv;
            }
        }
        return NULL;
    }
    int         remove(const T & k){
        //need to optimalization
        size_t hco = hcfT()(k);
        size_t idx = 1 + hco % hash_layer_segment[0].count;
        size_t pidx = 0;
        size_t ltidx = hash_layer_segment[layer - 1].offset + hco % hash_layer_segment[layer - 1].count;
        while(idx){
            if(index_.list[idx].id){
                T * p = mmpool_.ptr(index_.list[idx].id);
                if (index_.list[idx].hco == hco && *p == k){
                    mmpool_.free(index_.list[idx].id);
                    if (idx < hash_layer_segment[layer - 1].offset || idx == ltidx){ //middle layer
                        index_.list[idx].id = 0; //just erase it (no change list)
                        index_.list[idx].hco = 0; //just erase it (no change list)
                        return 0;
                    }
                    assert(pidx > 0);
                    index_.list[idx].id = 0;
                    index_.list[idx].hco = 0;
                    index_.list[pidx].next = index_.list[idx].next;
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
        size_t hco = hcfT()(k);
        size_t idx = 1 + hco % hash_layer_segment[0].count;
        while(idx){
            ++stat_probe_read;
            if (index_.list[idx].id){
                T * pv = mmpool_.ptr(index_.list[idx].id);
                if (index_.list[idx].hco == hco && *pv == k){
                    ++stat_hit_read;
                    return pv;
                }
            }
            idx = index_.list[idx].next;
        }
        return NULL;
    }
};

};





#endif
