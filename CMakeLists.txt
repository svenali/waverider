cmake_minimum_required(VERSION 3.5.0)
project(waverider VERSION 0.1.0)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
    add_definitions(-DHAVE_ALSA=1)
endif()

# Wt
find_library( WT_LIBRARY wt )
message("WT_LIBRARY = ${WT_LIBRARY} ")

include_directories(
    "/usr/local/include"
    "/usr/local/opt/fftw/include"
    "/usr/local/opt/mpg123/include"
    "/usr/local/opt/faad2/include"
    "/usr/local/opt/rtl-sdr/include"
    "/usr/local/opt/lame/include")
include_directories(
    "${CMAKE_SOURCE_DIR}/lib"
    "${CMAKE_SOURCE_DIR}/lib/dab"
    "${CMAKE_SOURCE_DIR}/lib/various"
    "${CMAKE_SOURCE_DIR}/lib/libs"
    "${CMAKE_SOURCE_DIR}/lib/libs/fec"
    "${CMAKE_SOURCE_DIR}/lib/input"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
if (APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lwthttp -lwt -lwtdbo -lwtdbosqlite3 -lboost_system -lboost_thread-mt -lboost_filesystem -lboost_program_options -lrtlsdr -lmpg123 -lfaad -lfftw3 -lfftw3f ${CMAKE_SOURCE_DIR}/lib/libs/fec/libfec.a")
endif(APPLE)

#FFMPEG
find_path( AVCODEC_INCLUDE_DIR libavcodec/avcodec.h )
find_library( AVCODEC_LIBRARY avcodec )
find_path( AVFORMAT_INCLUDE_DIR libavformat/avformat.h )
find_library( AVFORMAT_LIBRARY avformat )
message("AVCODEC_INCLUDE_DIRS = ${AVCODEC_INCLUDE_DIR} ")
message("AVCODEC_LIBRARY = ${AVCODEC_LIBRARY} ")
message("AVFORMAT_INCLUDE_DIRS = ${AVFORMAT_INCLUDE_DIR} ")
message("AVFORMAT_LIBRARY = ${AVFORMAT_LIBRARY} ")
if (AVCODEC_LIBRARY)
    message("AVCODEC FOUND TURNING HAVE_FFMPEG ON")
    set(HAVE_FFMPEG TRUE)
    add_definitions(-DHAVE_FFMPEG)
    include_directories(${AVCODEC_INCLUDE})
    include_directories(${AVFORMAT_INCLUDE})
    if (APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lwthttp -lwt -lwtdbo -lwtdbosqlite3 -lboost_system -lboost_thread-mt -lboost_filesystem -lboost_program_options -lrtlsdr -lmpg123 -lfaad -lfftw3 -lfftw3f -lavcodec -lavformat -lavutil -lswresample ${CMAKE_SOURCE_DIR}/lib/libs/fec/libfec.a")
    endif(APPLE)
endif()

set(HEADERS
    lib/dab/charsets.h 
    lib/dab/dab_decoder.h
    lib/dab/dab-audio.h
    lib/dab/dab-constants.h
    lib/dab/dab-processor.h
    lib/dab/dab-virtual.h
    lib/dab/dabplus_decoder.h
    lib/dab/decoder_adapter.h
    lib/dab/eep-protection.h
    lib/dab/energy_dispersal.h
    lib/dab/fib-processor.h
    lib/dab/fic-handler.h
    lib/dab/freq-interleaver.h
    lib/dab/mot_manager.h
    lib/dab/msc-handler.h
    lib/dab/ofdm-decoder.h
    lib/dab/ofdm-processor.h
    lib/dab/pad_decoder.h
    lib/dab/phasereference.h
    lib/dab/phasetable.h
    lib/dab/protection.h
    lib/dab/protTables.h
    lib/dab/radio-controller.h
    lib/dab/radio-receiver-options.h
    lib/dab/radio-receiver.h
    lib/dab/subchannel_sink.h
    lib/dab/tii-decoder.h
    lib/dab/tools.h
    lib/dab/uep-protection.h
    lib/dab/viterbi.h
    lib/various/channels.h
    lib/various/fft.h
    lib/various/MathHelper.h
    lib/various/profiling.h
    lib/various/ringbuffer.h
    lib/various/Socket.h
    lib/various/wavfile.h
    lib/various/Xtan2.h
    lib/input/input_factory.h
    lib/input/null_device.h
    lib/input/raw_file.h
    lib/input/rtl_sdr.h
    lib/input/rtl_tcp.h
    lib/input/virtual_input.h
    src/include/tool.h
    src/include/calsaoutput.h
    src/include/cstreamingserver.h
    src/include/cinternetdevice.h
    src/include/ccoverloader.h
    src/include/cistreammetadata.h
    src/include/caudiocompression.h
    src/include/cmp3audiocompression.h
    src/include/cflacaudiocompression.h
    src/include/calacaudiocompression.h
    src/include/caacaudiocompression.h
    src/include/cmp2audiocompression.h
    src/include/cnoaudiocompression.h
    src/include/coriginalaudiocompression.h
    src/include/cvorbisaudiocompression.h
    src/include/cexportaudioresource.h
    src/include/cinternet-audio.h
    src/include/cdeletepushbutton.h
    src/include/crecords.h
    src/include/cradiobrowser.h
    src/include/csettings.h
    src/include/cdbocountry.h
    src/include/cdbointernetchannel.h
    src/include/cdbobouquet.h
    src/include/cdborecord.h
    src/include/csettingsform.h
    src/include/csettingsformview.h
    src/include/cchannelitem.h
    src/include/cbouqueteditorform.h
    src/include/cbouqueteditorview.h
    src/include/service.h
    src/include/radio-server.h
    src/include/wwav-streamer-resource.h
    src/include/jplayer-streamer-resource.h
    src/include/station-item.h
    src/include/simple-timer.h
    src/include/wradio-controller.h
    src/include/waverider.h
    src/include/waverider-gui.h)
set(SOURCES
    lib/dab/charsets.cpp 
    lib/dab/dab_decoder.cpp
    lib/dab/dab-audio.cpp
    lib/dab/dab-constants.cpp
    lib/dab/dabplus_decoder.cpp
    lib/dab/decoder_adapter.cpp
    lib/dab/eep-protection.cpp
    lib/dab/fib-processor.cpp
    lib/dab/fic-handler.cpp
    lib/dab/freq-interleaver.cpp
    lib/dab/mot_manager.cpp
    lib/dab/msc-handler.cpp
    lib/dab/ofdm-decoder.cpp
    lib/dab/ofdm-processor.cpp
    lib/dab/pad_decoder.cpp
    lib/dab/phasereference.cpp
    lib/dab/phasetable.cpp
    lib/dab/protTables.cpp
    lib/dab/radio-receiver.cpp
    lib/dab/tii-decoder.cpp
    lib/dab/tools.cpp
    lib/dab/uep-protection.cpp
    lib/dab/viterbi.cpp
    lib/various/channels.cpp
    lib/various/fft.cpp
    lib/various/profiling.cpp
    lib/various/Socket.cpp
    lib/various/Xtan2.cpp
    lib/various/wavfile.c
    lib/input/input_factory.cpp
    lib/input/null_device.cpp
    lib/input/raw_file.cpp
    lib/input/rtl_sdr.cpp
    lib/input/rtl_tcp.cpp
    src/sources/tool.cpp
    src/sources/calsaoutput.cpp
    src/sources/cstreamingserver.cpp
    src/sources/cinternetdevice.cpp
    src/sources/ccoverloader.cpp
    src/sources/cistreammetadata.cpp
    src/sources/caudiocompression.cpp
    src/sources/cmp3audiocompression.cpp
    src/sources/cflacaudiocompression.cpp
    src/sources/calacaudiocompression.cpp
    src/sources/caacaudiocompression.cpp
    src/sources/cmp2audiocompression.cpp
    src/sources/cnoaudiocompression.cpp
    src/sources/coriginalaudiocompression.cpp
    src/sources/cvorbisaudiocompression.cpp
    src/sources/cexportaudioresource.cpp
    src/sources/cinternet-audio.cpp
    src/sources/cdeletepushbutton.cpp
    src/sources/crecords.cpp
    src/sources/cradiobrowser.cpp
    src/sources/csettings.cpp
    src/sources/cdbocountry.cpp
    src/sources/cdbointernetchannel.cpp
    src/sources/cdbobouquet.cpp
    src/sources/cdborecord.cpp
    src/sources/csettingsform.cpp
    src/sources/csettingsformview.cpp
    src/sources/cchannelitem.cpp
    src/sources/cbouqueteditorform.cpp
    src/sources/cbouqueteditorview.cpp
    src/sources/service.cpp
    src/sources/radio-server.cpp
    src/sources/wwav-streamer-resource.cpp
    src/sources/jplayer-streamer-resource.cpp
    src/sources/station-item.cpp
    src/sources/wradio-controller.cpp
    src/sources/waverider.cpp
    src/sources/waverider-gui.cpp)
add_executable(waverider main.cpp ${HEADERS} ${SOURCES})
target_include_directories(waverider PRIVATE src src/include src/sources)
if (LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
    set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -latomic")
    if (AVCODEC_LIBRARY)
        TARGET_LINK_LIBRARIES(waverider wt wtdbo pthread wthttp wtdbosqlite3 boost_system boost_thread boost_filesystem boost_program_options rtlsdr mpg123 faad fftw3 asound fftw3f avcodec avformat avutil swresample ${CMAKE_SOURCE_DIR}/lib/libs/fec/libfec.a)
    else (AVCODEC_LIBRARY)
        TARGET_LINK_LIBRARIES(waverider wt wtdbo pthread wthttp wtdbosqlite3 boost_system boost_thread boost_filesystem boost_program_options rtlsdr mpg123 faad fftw3 asound fftw3f ${CMAKE_SOURCE_DIR}/lib/libs/fec/libfec.a)
    endif (AVCODEC_LIBRARY)
    INCLUDE_DIRECTORIES(/usr/local/include/wt)
endif(LINUX)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)