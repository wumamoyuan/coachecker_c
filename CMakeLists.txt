cmake_minimum_required(VERSION 3.20)
project(hello)

include_directories(include)

aux_source_directory(src COACHECKER_SRC)
list(REMOVE_ITEM COACHECKER_SRC "src/coachecker.c" "src/exp1.c")

link_directories(lib)

add_executable(coachecker src/coachecker.c ${COACHECKER_SRC})

add_executable(exp1 src/exp1.c ${COACHECKER_SRC})

target_link_libraries(coachecker PRIVATE ccl)

target_link_libraries(exp1 PRIVATE ccl)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)