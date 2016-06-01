#ifndef _PBDC_CORE_EX_HPP_XX__
#define _PBDC_CORE_EX_HPP_XX__

namespace pbdcex {
template<size_t lmax>
struct string_t {
    char data[lmax];

    //////////////////////////////////////////////////////
    size_t format(const char * fmt, ...){
        va_list ap;
        va_start(ap, fmt);
        size_t n=vsnprintf(data, lmax-1, fmt, ap);
        data[lmax-1]=0;
        va_end(ap);
        return n;
    }
    const char * assign(const char * s){
        strncpy(data,s,lmax-1);
        data[lmax-1]=0;
    }
    const char * copyto(char * s, int len){
        strncpy(s, data, len-1);
        s[len-1]=0;
        return s;
    }
    operator char * (){
        return data;
    }
    size_t length() const {
        return strlen(data);
    }

};

template<size_t lmax, class LengthT=unsigned int>
struct bytes_t {
    unsigned char data[lmax];
    LengthT  n;

    //////////////////////////////////////////////////////
};

template<class T, size_t cmax, class LengthT=unsigned int>
struct array_t {
    T list[cmax];
    LengthT count;

    /////////////////////////////////
    void construct(){
        count = 0;
    }
    //linear
    int lfind(const T & tk) const;
    int lappend(const T & td, bool shift_overflow = false);
    int lremove(const T & tk, bool swap_remove = false);
    int linsert(int pos, const T & td);
    int lsort(array_t & out) const;

    //binary-seaching
    int bfind(const T & tk) const;
    int binsert(const T & td);
    int bremove(const T & tk);
    int lower_bound(const T & tk) const;
    int upper_bound(const T & tk) const;

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
    typedef unsigned long long mmpool_bit_block_t;
    #define mmpool_bit_block_byte_sz  (sizeof(mmpool_bit_block_t))
    #define mmpool_bit_block_bit_sz  (8*mmpool_bit_block_byte_sz)
    #define mmpool_bit_block_count ((cmax+mmpool_bit_block_bit_sz-1)/mmpool_bit_block_bit_sz)
    typedef array_t<T, cmax>    allocator_t;
    allocator_t                 allocator_;
    mmallocator_bit_block_t          bmp_[mmallocator_bit_block_count];
    size_t                      used_;

    void   construct(){
        memset(&allocator_, 0, sizeof(allocator_));
        for(int i = 0;i < mmallocator_bit_block_count; ++i){
            bmp_[i] = 0xFFFFFFFFFFFFFFFFUL;//all set 1
        }
    }
    const allocator_t & allocator(){
        return allocator_;
    }
    size_t alloc(){
        //1.find first 1,set 0
        size_t x = 0;
        size_t i = 0;
        for(; i < mmallocator_bit_block_count; ++i){
            if((x = __builtin_ffsll(bmp_[i]))){
                break;
            }
        }
        if(x != 0){
            bmp_[i] &= (~(1ULL<<(x-1)));//set 0
            size_t id=i*mmallocator_bit_block_bit_sz+x;
            if(id > allocator_.count){
                allocator_.count = id;
            }
            ++used_;
            return id;
        }
        else {
            return 0;
        }

    }
    size_t id(const T * p) const {
        assert(p >= allocator_.list && p < allocator_.list + cmax);
        return 1 + (p - allocator_.list)/sizeof(T);
    }
    T * ptr(size_t id)const {
        if(id > 0 && id <= allocator_.count && isbusy(id)){
           return &allocator_.list[id-1];
        }
        else {
            return nullptr;
        }
    }
    bool isbusy(size_t id){
        assert(id > 0);
        size_t idx=id-1;
        return !(bmp_[idx/mmallocator_bit_block_bit_sz]&(1ULL<<(idx&(mmallocator_bit_block_bit_sz-1))));
    }
    size_t used(){
        return used_;
    }
    size_t next(size_t it){
        it = it + 1;
        for(size_t i=(it-1)/mmallocator_bit_block_bit_sz; i < mmallocator_bit_block_count; ++i){
            if(allocator_[i] != 0xFFFFFFFFFFFFFFFFUL){
               for(int j = 0;j < mmallocator_bit_block_bit_sz; ++j){
                   if(allocator_[i]&(1ULL << j) == 0){//find first 0
                        return i*mmallocator_bit_block_bit_sz+j+1;
                   }
               }
            }
        }
        return 0;
    }
    void   free(size_t id){
        assert(id > 0);
        size_t idx = id - 1;
        //set 0
        if(isbusy(id)){
            bmp_[idx/mmallocator_bit_block_bit_sz] |= (1ULL << (idx&(mmallocator_bit_block_bit_sz-1)));
            --used_;
            allocator_[idx].construct();
        }
    }
};

template<class T, size_t cmax, class hc, size_t factor=75>
struct hashmap_t {
    struct hashmap_entry_t {
        size_t  id;
        size_t  hc;
        size_t  next;
    };
    #define hash_entry_index_size   (cmax*100/factor)
    typedef mmpool_t<T, cmax>                                     pool_t;
    typedef array_t<hashmap_entry_t, hash_entry_index_size+8>     index_t;
    index_t     index_;
    pool_t      mmpool_;

    ////////////////////////////////////////////////////////////
    void        construct(){
        memset(&index_, 0, sizeof(index_);
        mmpool_.construct();
        index_.count = hash_entry_index_size;
    }
    pool_t &    mmpool(){return mmpool_;}
    T *         insert(const T & k){
        //need to optimalization
        size_t hco = hc(k);
        size_t idx = hco % hash_entry_index_size + 1;
        size_t max_retry = hash_entry_index_size;
        while(index_.list[idx].id){
            if (*pool_.ptr(index_.list[idx].id) == k){
                return nullptr;
            }
            if(index_.list[idx].next == 0){
                break;
            }
            idx = index_.list[idx].next;
        }
        if(idx > 0){
            if(index_.list[idx].id > 0){//next , find next pos
                size_t npos = (idx+1) % hash_entry_index_size + 1;
                while(max_retry--){
                   if(index_.list[npos].id == 0){
                       break;
                   }
                   npos = (npos+1)% hash_entry_index_size + 1;
                }
                size_t hid = pool_.alloc();
                if(hid == 0){
                    return nullptr;
                }
                //#list append
                index_.list[idx].next = npos;

                index_.list[npos].id = hco;
                index_.list[npos].hc = hid;
                index_.list[npos].next = 0;
                return pool_.ptr(hid);
            }
            else { //head
                size_t hid = pool_.alloc();
                if(hid == 0){
                    return nullptr;
                }
                hashmap_entry_t & he = index_.list[idx];
                he.hc = hco;
                he.id = hid;
                he.next = 0;
                return pool_.ptr(hid);
            }
        }
        return nullptr;
    }
    int         remove(const T & k){
        //need to optimalization
        size_t hco = hc(k);
        size_t idx = hco % hash_entry_index_size + 1;
        size_t pidx = 0;
        while(idx){
            if(index_.list[idx].id){
                T * p = pool_.ptr(index_.list[idx].id);
                if (*p == k){
                    //remove it
                    pool_.free(index_.list[idx].id);
                    index_.list[idx].id = 0;
                    index_.list[idx].hc = 0;
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
        size_t hco = hc(k);
        size_t idx = hco % hash_entry_index_size;
        size_t max_retry = hash_entry_index_size;
        while(index_.list[idx].id){
            T * p = pool_.ptr(index_.list[idx].id);
            if (*p == k){
                return p;
            }
            idx = index_.list[idx].next;
        }
        return nullptr;
    }

};


};







#endif
