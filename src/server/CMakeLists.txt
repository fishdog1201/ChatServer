aux_source_directory(. SRC_LIST)

add_executable(chatServer ${SRC_LIST})
target_link_libraries(chatServer muduo_net muduo_base pthread)