set(SOURCES
    test.cpp
    ${INCLUDE_DIRECTORIES}
    )

add_executable(rankedtest ${SOURCES})
target_link_libraries(rankedtest)
add_test(ranked_test rankedtest)