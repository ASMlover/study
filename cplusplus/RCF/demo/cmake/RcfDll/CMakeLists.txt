
ADD_DEFINITIONS(
    -DRCF_BUILD_DLL 
    -DRCF_NO_AUTO_INIT_DEINIT)
    
ADD_DEFINITIONS( ${RCF_DEFINES} )

# Disable some warnings
# warning C4275: non dll-interface class '' used as base for dll-interface class ''
# warning C4251: '' : class '' needs to have dll-interface to be used by clients of class ''
IF(MSVC)
ADD_DEFINITIONS(/wd4275 /wd4251)
ENDIF(MSVC)

INCLUDE_DIRECTORIES( ${RCF_INCLUDES} )

ADD_LIBRARY(
    RcfDll
    SHARED 
    ${RCF_ROOT}/src/RCF/RCF.cpp)

TARGET_LINK_LIBRARIES(RcfDll ${RCF_LIBS})
