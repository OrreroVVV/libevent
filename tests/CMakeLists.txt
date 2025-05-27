link_directories(${CMAKE_SOURCE_DIR}/build/lib)

add_executable(test_event test_event.cpp)

set(HZH_LIBS
    event
    # /home/hzh/workspace/libevent/build/lib/libevent.a
    dl
    pthread
    rt
    z
    m
)

target_link_libraries(test_event ${HZH_LIBS})


add_executable(test_client test_client.cpp)
target_link_libraries(test_client ${HZH_LIBS})