include_directories()

SET(TEST_SRC main.cpp)

ADD_EXECUTABLE(test ${TEST_SRC})
TARGET_LINK_LIBRARIES(test libcommon)
TARGET_LINK_LIBRARIES(test libgraph)
TARGET_LINK_LIBRARIES(test libstr)
