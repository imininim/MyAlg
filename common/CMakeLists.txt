#include路径
include_directories(/usr/local/mioji/include/)
include_directories(${PROJECT_SOURCE_DIR})
include_directories(.)
#链接查找路径
#添加子目录
#add_subdirectory(vlist)
#add_subdirectory(base)
#add_subdirectory(bag)
#打包route
set(LIB_COMMON_SRC
	define.cpp
	)
add_library(common SHARED ${LIB_COMMON_SRC})
#target_link_libraries(common)
install(TARGETS common DESTINATION lib)
#添加可执行程序test
#add_executable(test testView.cpp)
#target_link_libraries(test route)
#install(TARGETS test DESTINATION bin)

