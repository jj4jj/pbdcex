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

    //
    void construct(){
        count = 0;
    }

    //linear
    int lfind(const T & tk)const;
    int lappend(const T & td);
    int lremove(const T & tk);
    int linsert(const T & td);
    int lsort(array_t & out) const;

    //binary-seaching
    int bfind(const T & tk)const;
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
    typedef array_t<T, cmax>    pool_t;
    pool_t                      pool_;
    mmpool_bit_block_t          bmp_[mmpool_bit_block_count];
    size_t                      used_;

    void   construct(){
        memset(&pool_, 0, sizeof(pool_));
        for(int i = 0;i < mmpool_bit_block_count; ++i){
            bmp_[i] = 0xFFFFFFFFFFFFFFFFUL;//all set 1
        }
    }
    const pool_t & pool(){
        return pool_;
    }
    size_t alloc(){
        //1.find first 1,set 0
        size_t x = 0;
        size_t i = 0;
        for(; i < mmpool_bit_block_count; ++i){
            if((x = __builtin_ffsll(bmp_[i]))){
                break;
            }
        }
        if(x != 0){
            bmp_[i] &= (~(1ULL<<(x-1)));//set 0
            size_t id=i*mmpool_bit_block_bit_sz+x;
            if(id > pool_.count){
                pool_.count = id;
            }
            ++used_;
            return id;
        }
        else {
            return 0;
        }

    }
    T * get(size_t id){
        if(id > 0 && id <= pool_.count && isbusy(id)){
           return pool_.list[id-1];
        }
        else {
            return nullptr;
        }
    }
    bool isbusy(size_t id){
        assert(id > 0);
        size_t idx=id-1;
        return !(bmp_[idx/mmpool_bit_block_bit_sz]&(1ULL<<(idx&(mmpool_bit_block_bit_sz-1))));
    }
    size_t used(){
        return used_;
    }
    size_t next(size_t it){
        it = it + 1;
        for(size_t i=(it-1)/mmpool_bit_block_bit_sz; i < mmpool_bit_block_count; ++i){
            if(pool_[i] != 0xFFFFFFFFFFFFFFFFUL){
               for(int j = 0;j < mmpool_bit_block_bit_sz; ++j){
                   if(pool_[i]&(1ULL << j) == 0){//find first 0
                        return i*mmpool_bit_block_bit_sz+j+1;
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
            bmp_[idx/mmpool_bit_block_bit_sz] |= (1ULL << (idx&(mmpool_bit_block_bit_sz-1)));
            --used_;
            pool_[idx].construct();
        }
    }
};

template<class T, size_t cmax>
struct hashmap_t {

};


};







#endif
