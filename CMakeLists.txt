# 요구 CMake 최소 버전
CMAKE_MINIMUM_REQUIRED ( VERSION 3.8 )
 
# 프로젝트 이름 및 버전
PROJECT ( "ftp-proxy" )
SET ( PROJECT_VERSION_MAJOR 1 )
SET ( PROJECT_VERSION_MINOR 01 )
 
# 빌드 형상(Configuration) 및 주절주절 Makefile 생성 여부
SET ( CMAKE_BUILD_TYPE Release )
SET ( CMAKE_VERBOSE_MAKEFILE false )
 
# 빌드 대상 바이너리 파일명 및 소스파일 목록
SET ( OUTPUT_ELF
        "${CMAKE_PROJECT_NAME}-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.out"
        )
SET ( SRC_FILES
        ftp-proxy/src/main.c
        ftp-proxy/src/option.c
        ftp-proxy/src/server.c
        ftp-proxy/src/log.c
        ftp-proxy/src/socket.c
        ftp-proxy/src/client.c
        ftp-proxy/src/session.c
        ftp-proxy/src/thread.c
        ftp-proxy/src/packet.c
        ftp-proxy/src/utils.c
        )
 
# 공통 컴파일러
SET ( CMAKE_C_COMPILER "gcc" )
 
# 공통 헤더 파일 Include 디렉토리 (-I)
INCLUDE_DIRECTORIES ( ftp-proxy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ftp-proxy/include)
 
# 공통 컴파일 옵션
ADD_COMPILE_OPTIONS ( -std=c90 -Wall )
 
# 공통 링크 라이브러리 (-l)
LINK_LIBRARIES( pthread )

# 출력 디렉토리
SET ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE} )
SET ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/lib )
SET ( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BUILD_TYPE}/lib )
 
# 빌드 대상 바이너리 추가
ADD_EXECUTABLE( ${OUTPUT_ELF} ${SRC_FILES} )