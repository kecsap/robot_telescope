
SET(SMTPCLIENT_SRC
    emailaddress.cpp
    mimeattachment.cpp
    mimefile.cpp
    mimehtml.cpp
    mimeinlinefile.cpp
    mimemessage.cpp
    mimepart.cpp
    mimetext.cpp
    smtpclient.cpp
    quotedprintable.cpp
    mimemultipart.cpp
    mimecontentformatter.cpp)

SET(SMTPCLIENT_HEADERS
    emailaddress.h
    mimeattachment.h
    mimefile.h
    mimehtml.h
    mimeinlinefile.h
    mimemessage.h
    mimepart.h
    mimetext.h
    smtpclient.h
    quotedprintable.h
    mimemultipart.h
    mimecontentformatter.h)

QT5_WRAP_CPP(SMTPCLIENT_MOC_SRC ${SMTPCLIENT_HEADERS})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -UQT_NO_KEYWORDS")

ADD_LIBRARY(smtpclient STATIC ${SMTPCLIENT_SRC} ${SMTPCLIENT_MOC_SRC})
TARGET_LINK_LIBRARIES(smtpclient Qt5::Core Qt5::Network)
