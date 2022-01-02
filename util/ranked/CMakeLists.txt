set(SOURCES
    ranked.cpp
    ${INCLUDE_DIRECTORIES}
    )
add_executable (ranked ${SOURCES})
target_link_libraries(ranked)