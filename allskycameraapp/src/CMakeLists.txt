
INCLUDE_DIRECTORIES(${TENSORFLOWCPP_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(. ../libs/libsunrise/src)
INCLUDE_DIRECTORIES(. ../libs/smtpclient/src)
INCLUDE_DIRECTORIES(/usr/include/libmindcommon /usr/include/libmindaibo /usr/include/libmindeye)
INCLUDE_DIRECTORIES(${DEPS_INCLUDE_DIRS} /usr/include/libgeoclue-2.0/)

ADD_EXECUTABLE(allskycameraapp inference.cpp main.cpp)
TARGET_LINK_LIBRARIES(allskycameraapp Qt5::Core sunrise smtpclient -lmindcommon -lmindaibo_core -lmindeye -lgeoclue-2 ${TENSORFLOWCPP_LIBRARIES})
