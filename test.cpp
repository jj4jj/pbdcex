
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
    static pbdcex::hashtable_t<Hello_ST, 10000>  hash;
    hash.construct();
    std::string dstr;
    printf("%s\n", hash.stat(dstr));
    for (int i = 0; i < 10000; ++i){
        Hello_ST h1;
        h1.id = rand()%1000;
        auto p = hash.insert(h1);
        if (p && rand() % 100 < 90){
            auto pf = hash.find(h1);
            assert(pf == p);
            pf = hash.insert(h1);
            assert(pf == NULL);
            hash.remove(h1);
            pf = hash.find(h1);
            assert(pf == NULL);
        }
    }

    dstr.clear();
    printf("after insert %s\n", hash.stat(dstr));
    return 0;
}
