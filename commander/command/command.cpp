/* =============================================================================
#     FileName: command.cpp
#         Desc: command function in commander write in cpp
#       Author: LiuYangming
#        Email: sdwhlym@126.com
#     HomePage: http://yummyliu.github.io
#      Version: 0.0.1
#   LastChange: 2018-08-01 15:28:45
#      History:
============================================================================= */

extern "C" {
#include <postgres.h>    //all C headers and macros go inside extern "C"
#include <utils/rel.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(sumofall);
}

#include<vector>           //C++ headers go outside

extern "C" {
    //wrap the C++ function inside the extern "C" block
    Datum sumofall(PG_FUNCTION_ARGS) {
        auto sum_of_all = 0;
        std::vector<int> arr {1,2,3,4,5,6,7};
        for (auto& i : arr )
                 sum_of_all += i;
        return sum_of_all;
    }
}
