add_executable (
    ranked 
    ranked.cpp
    ${INCLUDE_DIRECTORIES}
    )
target_link_libraries(ranked ${RANKED_LIB})