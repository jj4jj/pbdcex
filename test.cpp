
#include "test.pb.h"
#include "test.cex.hpp"
#include <iostream>

int main(){
    Hello_ST hs;
    hs.construct();
    hs.d.b.a.f1 = 23;
    Hello   ho;
    hs.convto(ho);
    printf("%s\n", ho.ShortDebugString().c_str());    
    ho.set_ev(MAX_DB_SPLIT_NUM);
    ho.set_id(195);
    hs.convfrom(ho);

    typedef string_t<32> str32_t;
    str32_t as;
    
    int ret = hs.myworld.binsert(str32_t::assign(as, "bbbbb"));
    printf("%d,%d\n", ret, hs.myworld.count);
    ret = hs.myworld.binsert(str32_t::assign(as, "aaaaa"));
    printf("%d,%d\n", ret, hs.myworld.count);
    ret = hs.myworld.lower_bound(str32_t::assign(as, "ababababab"));
    printf("%d,%d\n", ret, hs.myworld.count);
    ret = hs.myworld.binsert(str32_t::assign(as, "ddxxa"));
    printf("%d,%d\n", ret, hs.myworld.count);
    ret = hs.myworld.binsert(str32_t::assign(as, "exf24a"));
    printf("%d,%d\n", ret, hs.myworld.count);
    ret = hs.myworld.binsert(str32_t::assign(as, "asf24a"));
    printf("%d,%d\n", ret, hs.myworld.count);
    for (int i = 0; i < hs.myworld.count; ++i){
        printf("%s ", hs.myworld[i].data);
    }
    printf("\n");

    Hello ho2;
    hs.convto(ho2);
    printf("%s\n", ho2.ShortDebugString().c_str());

    using namespace pbdcex;
    static mmpool_t<Hello_ST, 120> pool;
    pool.construct();
    printf("pool size:%zu, cap:%zu, unit:%zu allocator:%zu\n",
        sizeof(pool), pool.capacity(), sizeof(Hello_ST),
        pool.allocator().capacity());
    for (int i = 0; i < pool.capacity()*3; ++i){
        ret = pool.alloc();
        //printf("alloc %d\n", ret);
        if (ret > 0 && rand() % 100 < 50){
            pool.free(ret);
            //printf("free %d\n", ret);
        }
    }
    printf("mmpool:%zu/%zu\n", pool.used(), pool.capacity());
    int it = 0;
    std::string  str;
    int count = 0;
    while ((it = pool.next(it))){
        ++count;
    }
    printf("traverse count:%d\n", count);    

    struct hc {
        size_t operator() (const Hello_ST & h){
            return h.id;
        };
    };
    static pbdcex::hashmap_t<Hello_ST, 100, hc>  hash;
    hash.construct();
    printf("hash size:%zu load:%d hit:%d factor:%d\n", 
        sizeof(hash), hash.load(), hash.hit(), hash.factor());
    for (int i = 0; i < 100; ++i){
        Hello_ST h1;
        h1.id = rand();
        auto p = hash.insert(h1);
        if (p && rand() % 100 < 50){
            hash.remove(h1);
        }
    }
    printf("after random insert/remove hash size:%zu load:%d hit:%d\n",
        sizeof(hash), hash.load(10000), hash.hit(10000));
    printf("hash mmpool:%zu/%zu\n", hash.mmpool().used(), hash.mmpool().capacity());

    return 0;
}