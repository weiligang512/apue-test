include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

set(AMAROK_COLLECTION_SUPPORT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/support)

add_subdirectory( daap )

add_subdirectory( mediadevicecollection )
add_subdirectory( ipodcollection )
add_subdirectory( mtpcollection )

# needs things from playground/base/nepomuk-kde
#if (SOPRANO_FOUND AND NEPOMUK_FOUND)
#    MESSAGE(STATUS "soprano and nepomuk found, building optional nepomuk collection")
#    add_subdirectory(nepomukcollection)
#endif (SOPRANO_FOUND AND NEPOMUK_FOUND)

add_subdirectory( sqlcollection )
#add_subdirectory( upnp )
