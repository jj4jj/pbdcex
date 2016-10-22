
#include "test.pb.h"
#include "test.cex.hpp"
#include <iostream>
#include "time.h"
#include "sys/time.h"

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
	
	printf("sizeof(Hello_ST):%zu\n", sizeof(Hello_ST));

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
        static Hello_ST h1;
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


	static Hello_ST	hst1,hst2, hst3;
	hst1.construct();
	static HelloD_ST hds;
	hds.b.a.f1 = 10;
	hst1.testadss.lappend(hds);
	hds.b.a.f1 = 24;
	hst1.testadss.lappend(hds);
	hds.b.a.f1 = 25;
	hst1.testadss.lappend(hds);
	hds.b.a.f1 = 30;
	hst1.testadss.lappend(hds);

	hst2 = hst1;
	hst2.xx9.b.a.f1 = 2200;
	hst2.xxx1 = 1200;
	hst2.d.f1 = 1990;
	hst2.id = 2020;
	hst2.testadss.lremove(0);
	hst2.testadss.lremove(3);
	hds.b.a.f1 = 99;
	hst1.testadss.lappend(hds);

	//remove 10,30
	//add 99
	Hello upd;	
	timeval tv;
	gettimeofday(&tv, 0);
	for (int i = 0;i < 10000; ++i) {
		hst2.diff(hst1, upd);
		hst3 = hst1;
		hst3.patch(upd);
	}
	timeval tv2;
	gettimeofday(&tv2, 0);
	hst2.diff(hst1, upd);
	hst3=hst1;
	hst3.patch(upd);
	std::string ds1,ds2,ds3;
	printf("hst2(\n[%s]\n).diff(hst1)\n[%s]\n 100000 times:%ld.%ld \nupdates(add or replace):\n[%s]\nhst3:\n[%s]\n hst2=hst3?:%d\n",
		hst2.debugs(ds1),hst1.debugs(ds2),
		tv2.tv_sec - tv.tv_sec, tv2.tv_usec - tv.tv_usec,
	upd.ShortDebugString().c_str(), hst3.debugs(ds3), hst3==hst2);


    return 0;
}
