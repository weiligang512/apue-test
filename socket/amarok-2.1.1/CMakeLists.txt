project(Amarok)

cmake_minimum_required(VERSION 2.6.2)

set( CMAKE_MODULE_PATH    ${CMAKE_MODULE_PATH}
                          ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules )

option(WITH_UTILITIES "Enable building of utilities" ON)
option(WITH_PLAYER "Enable building of main Amarok player" ON)
option(WITH_MP3Tunes "Enable mp3tunes in the Amarok player, requires multiple extra dependencies" ON)

include(CheckLibraryExists)
check_library_exists(dl dlopen "" LIBDL_FOUND)

set(TAGLIB_MIN_VERSION "1.5")
find_package(Taglib REQUIRED)

find_package(Taglib-Extras 0.1 REQUIRED)
if( TAGLIB-EXTRAS_FOUND )
    add_definitions( -DTAGLIB_EXTRAS_FOUND )
endif( TAGLIB-EXTRAS_FOUND )

include(CheckTagLibFileName)

check_taglib_filename(COMPLEX_TAGLIB_FILENAME)

configure_file(config-amarok.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-amarok.h )

if( WITH_PLAYER )
#We depend on kde4.2 now.. I believe this is the easiest way to do it.
    set(KDE_MIN_VERSION "4.2.0")
    find_package( KDE4 REQUIRED )
    include (KDE4Defaults)
    include (MacroLibrary)

    include(MacroBoolTo01)
    include(MacroLogFeature)

    macro_log_feature( KDE4_FOUND "kdelibs" "The toolkit Amarok uses to build" "http://www.kde.org" TRUE "4.2.0" "" )

    find_package( QtScriptQtBindings REQUIRED )
    macro_log_feature( QTSCRIPTQTBINDINGS_FOUND "qtscript-qt" "QtScript Qt Bindings" "http://code.google.com/p/qtscriptgenerator/" TRUE "" "" )

    add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

    find_package(MySQLAmarok REQUIRED)
    macro_log_feature( MYSQL_EMBEDDED_FOUND "mysqld" "Embedded MySQL" "http://www.mysql.com" TRUE "" "" )

    # zlib is required for mysql embedded
    find_package(ZLIB REQUIRED)
    macro_log_feature( ZLIB_FOUND "zlib" "zlib" "" TRUE "" "" )

    macro_optional_find_package(Strigi)
    macro_log_feature( STRIGI_FOUND "strigi" "Index metadata of files" "http://strigi.sourceforge.net" TRUE "" "" )

    macro_optional_find_package(Ipod)
    if(IPOD_FOUND)
        macro_ensure_version("0.7.0" ${IPOD_VERSION} IPOD_0_7)
    endif(IPOD_FOUND)
    macro_log_feature( IPOD_0_7 "libgpod" "Support Apple iPod audio devices" "http://sourceforge.net/projects/gtkpod/" FALSE "0.7.0" "" )
 
    if( WITH_Ipod )
        macro_optional_find_package(Gdk)
        macro_log_feature( GDK_FOUND "Gdk" "Support for artwork on iPod audio devices via GdkPixbuf" "http://developer.gnome.org/arch/imaging/gdkpixbuf.html" FALSE "2.0.x" "" )
    endif( WITH_Ipod )

    macro_optional_find_package(Mtp)
    macro_log_feature( MTP_FOUND "libmtp" "Enable Support for portable media devices that use the media transfer protocol" "http://libmtp.sourceforge.net/" FALSE "0.3.0" "")

    if( WITH_MP3Tunes )
        find_package(CURL)
        macro_log_feature( CURL_FOUND "curl" "cURL provides the necessary network libraries required by mp3tunes." "http://curl.haxx.se" FALSE "" "" )

        find_package(LibXml2)
        macro_log_feature( LIBXML2_FOUND "libxml2" "LibXML2 is an XML parser required by mp3tunes." "http://www.xmlsoft.org" FALSE "" "" )

        macro_optional_find_package(OpenSSL)
        macro_optional_find_package(Libgcrypt)
        if ( OPENSSL_FOUND OR LIBGCRYPT_FOUND )
            set (_mp3tunes_crypto TRUE )
        else ( OPENSSL_FOUND OR LIBGCRYPT_FOUND )
            message( SEND_ERROR "Building with mp3tunes support REQUIRES either OpenSSL or GNU Libgcrypt" )
        endif ( OPENSSL_FOUND OR LIBGCRYPT_FOUND )
        macro_log_feature( _mp3tunes_crypto "openssl or libgcrypt" "OpenSSL or GNU Libgcrypt provides cryptographic functions required by mp3tunes." "http://www.openssl.org/ or http://www.gnupg.org/download/#libgcrypt" FALSE "" "" )

        find_package(GObject)
        macro_log_feature( GOBJECT_FOUND "gobject" "Required by mp3tunes." "http://www.gtk.org" FALSE "2.x" "" )

        find_package(Loudmouth)
        macro_log_feature( LOUDMOUTH_FOUND "loudmouth" "Loudmouth is the communication backend needed by mp3tunes for syncing." "http://www.loudmouth-project.org" FALSE "" "" )

        include(CheckQtGlib)
        macro_log_feature(QT4_GLIB_SUPPORT "Qt4 Glib support" "Qt4 must be compiled with glib support for mp3tunes" "http://www.trolltech.com" FALSE "" "")
    endif( WITH_MP3Tunes )

    if( WITH_Ipod OR WITH_MP3Tunes )
        find_package(GLIB2)
        macro_log_feature( GLIB2_FOUND "glib2" "Required by libgpod and mp3tunes" "http://www.gtk.org" FALSE "2.x" "")
    endif( WITH_Ipod OR WITH_MP3Tunes )

    #These two are currently unused
    #macro_optional_find_package(Soprano)
    #macro_log_feature( Soprano_FOUND "soprano" "Soprano is a RDF framework required by Nepomuk Collection" "http://soprano.sourceforge.net" FALSE "2.1.0" "")
    #macro_optional_find_package(Nepomuk)
    #macro_log_feature( NEPOMUK_FOUND "nepomuk" "Nepomuk Libraries required by Nepomuk Collection (part of kdelibs)" "http://www.kde.org" FALSE "4.1" "")

    macro_bool_to_01(LIBVISUAL_FOUND HAVE_LIBVISUAL)

    include_directories (${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE4_INCLUDES} ${TAGLIB_INCLUDES})

    add_subdirectory( src )
    add_subdirectory( tests )

    macro_display_feature_log()

    #Do not remove or modify these.  The release script substitutes in for these
    #comments with appropriate doc and translation directories.
    
include(MacroOptionalAddSubdirectory)
macro_optional_add_subdirectory( po )

    #DOC_SUBDIR

    add_executable( bindingstest cmake/modules/QtScriptBindingsTest.cpp )
    target_link_libraries( bindingstest ${QT_QTSCRIPT_LIBRARY} ${KDE4_KDEUI_LIBS} )

else( WITH_PLAYER )

    find_package( Qt4 REQUIRED )
    add_definitions(${QT_DEFINITIONS})
    include_directories (${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${TAGLIB_INCLUDES})

endif( WITH_PLAYER )

if( WITH_UTILITIES )
    set(EXEC_INSTALL_PREFIX  ${CMAKE_INSTALL_PREFIX}       CACHE PATH  "Base directory for executables and libraries" FORCE)
    set(BIN_INSTALL_DIR          "${EXEC_INSTALL_PREFIX}/bin"    CACHE PATH "The subdirectory to the binaries prefix (default prefix/bin)" FORCE)
    add_subdirectory( utilities )
endif( WITH_UTILITIES )

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")

