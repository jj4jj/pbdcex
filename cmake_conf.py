project='pbdcex'
version='0.1.1'
debug = 1    #0/1
defs = []
verbose = 'on'    #on/off
extra_c_flags = '-wno-unused-parameter'
extra_cxx_flags = '--std=c++11 -lpthread -lrt -ldl'
env = {
'protoi':'/usr/local/include',
'dcpotsi':'${project_source_dir}/..',
'protoc':'protoc',
'protoi':'/usr/local/include',
}
units = [
        {
            'name':'pbdcexer',
            'type':'exe',
            'subdir':'src',
            'incs':['{{root}}/..'],
            'lincs':['{{root}}/../dcpots/lib','{{root}}/../cxxtemplates/lib'],
            'libs':['dcutil-drs','dcutil-mysql','dcbase','mysqlclient','protobuf','xctmp','pbjson'],
            'objs':[
                {
                    'name':'pbdcex',
                    'srcs':['pbdcex.cpp','mysql_gen.cpp','cpp_gen.cpp'],
                },
            ],
        },
        {
            'name':'htest',
            'subdir':'test',
            'type':'exe',
            'dsrcs': ['proto'],
            'srcs':['proto/test.pb.cc','proto/test.cex.hpp'],
            'incs':['proto'],
            'lincs':['lib','{{dcpotsi}}/lib'],
            'libs' : [
                'pbdcex',
                'dcutil-drs',
                'dcbase',
                'mysqlclient',
            ],
            'objs': [{
                'out':'{{cdir}}/proto/test.pb.cc',
                'dep':'{{cdir}}/proto/test.proto',
                'cmd':'protoc {{cdir}}/proto/test.proto -I{{cdir}}/proto --cpp_out={{cdir}}/proto/'
                },
                {
                'out':'{{cdir}}/proto/test.cex.hpp',
                'dep':'{{cdir}}/proto/test.proto',
                'cmd':'{{root}}/bin/pbdcexer -mHello -p{{cdir}}/proto/test.proto -I{{cdir}}/../dcpots/utility/drs -I{{cdir}}/proto -I{{protoi}} --cpp_out={{cdir}}/proto/'
                },
                {
                    'name':'sql',
                    'out':'{{cdir}}/proto/DBPlayer.sql',
                    'dep':'{{cdir}}/proto/test.proto',
                    'cmd':'{{protoc}} -p{{cdir}}/proto/test.proto -mDBPlayer -mDBHello --sql_out={{cdir}}/proto -I{{cdir}}/proto -I{{protoi}} -I{{dcpotsi}}/dcpots/utility/drs;'\
                          '{{protoc}} -p{{cdir}}/proto/test.proto -mDBPlayer -mDBHello --sql_out={{cdir}}/proto -I{{cdir}}/proto -I{{protoi}} -I{{dcpotsi}}/dcpots/utility/drs;'\
                          '{{protoc}} -p{{cdir}}/proto/test.proto -mDBPlayer -mDBHello --sql_out={{cdir}}/proto -I{{cdir}}/proto -I{{protoi}} -I{{dcpotsi}}/dcpots/utility/drs;'
                },
            ]
        } 
]
