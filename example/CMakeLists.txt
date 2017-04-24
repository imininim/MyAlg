#include路径
include_directories(/usr/local/mioji/include/)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(.)
#链接查找路径
link_directories(.)
link_directories(../lib)
LINK_DIRECTORIES(bin)
#添加可执行程序test
add_executable(test test.cpp)
target_link_libraries(test common graph)
install(TARGETS test DESTINATION bin)

