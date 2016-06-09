
#include "test.pb.h"
#include "test.cex.hpp"

int main(){
    Hello_ST hs;
    Hello   ho;

    hs.convto(ho);

    hs.convfrom(ho);
    return 0;
}