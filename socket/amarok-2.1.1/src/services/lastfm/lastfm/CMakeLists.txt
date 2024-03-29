# This is our own CMakeLists to compile liblastfm internally
# not the ideal solutio, but we can't wait for last.fm to release
# this, so we're bundling it for now

include_directories( .
                     ..
                     ../../ # for amarok_export
                     core
                     core/mac
                     core/win
                     fingerprint
                     radio
                     scrobble
                     types
                     ws
                    )

add_definitions(${KDE4_ENABLE_EXCEPTIONS})

# core

if( Q_WS_MAC )
    set( amarok_service_liblastfm_core_EXTRASRCS
        core/mac/AppleScript.cpp
        core/mac/Growl.cpp
        core/mac/CFStringToQString.cpp
        )
endif( Q_WS_MAC )

if( WIN32 )
    set( amarok_service_liblastfm_core_EXTRASRCS
        core/win/Utils.cpp
        ws/NdisEvents_win.cpp
#        ws/WmiSink_win.cpp
       )
endif( WIN32 )

set( amarok_service_liblastfm_SRCS
        core/CoreDir.cpp
        core/CoreDomElement.cpp
        core/CoreLocale.cpp
        core/CoreProcess.cpp
        core/CoreSysInfo.cpp
        core/CoreUrl.cpp
        core/QMessageBoxBuilder.cpp
        core/UniqueApplication.cpp
        radio/Playlist.cpp
        radio/Tuner.cpp
        scrobble/NowPlaying.cpp
        scrobble/ScrobbleCache.cpp
        scrobble/Scrobble.cpp
        scrobble/Scrobbler.cpp
        scrobble/ScrobblerHandshake.cpp
        scrobble/ScrobblerHttp.cpp
        scrobble/ScrobblerSubmission.cpp
		scrobble/knetworkreply.cpp
        types/Album.cpp
        types/Artist.cpp
        types/Mbid.cpp
        types/Tag.cpp
        types/Track.cpp
        types/User.cpp
        ws/WsAccessManager.cpp
        ws/WsAutoProxy.cpp
        ws/WsNetEvent.cpp
        ws/WsProxy.cpp
        ws/WsReply.cpp
        ws/WsRequestBuilder.cpp
        ws/WsRequestParameters.cpp
   )


kde4_add_library(amarok_service_liblastfm SHARED ${amarok_service_liblastfm_SRCS} ${amarok_service_liblastfm_core_EXTRASRCS} )

set(amarok_service_liblastfm_LIBS
        ${QT_QTCORE_LIBRARY}
        ${QT_QTXML_LIBRARY}
        ${QT_QTGUI_LIBRARY}
        ${QT_QTNETWORK_LIBRARY}
        ${KDE4_PHONON_LIBRARY}
        ${KDE4_KIO_LIBS}
   )

if ( WIN32 )
    set(amarok_service_liblastfm_LIBS ${amarok_service_liblastfm_LIBS} winhttp)
endif ( WIN32 )

target_link_libraries(amarok_service_liblastfm ${amarok_service_liblastfm_LIBS} )

if(APPLE)
    SET_TARGET_PROPERTIES(amarok_service_liblastfm PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
endif(APPLE)

install(TARGETS amarok_service_liblastfm ${INSTALL_TARGETS_DEFAULT_ARGS} )
