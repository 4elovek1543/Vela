cmake_minimum_required(VERSION 3.20)
project(Vela LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -O2 -fsanitize=address,undefined")

find_package(PkgConfig REQUIRED)


set(REQUIRED_PACKAGES yaml-cpp)
find_program(PACMAN pacman)
if(PACMAN)
    # Проверка каждого пакета
    foreach(PKG ${REQUIRED_PACKAGES})
        execute_process(
            COMMAND pacman -Q ${PKG}
            RESULT_VARIABLE PKG_INSTALLED
            OUTPUT_QUIET ERROR_QUIET
        )
        
        if(PKG_INSTALLED EQUAL 0)
            message(STATUS "Пакет ${PKG} уже установлен")
        else()
            message(WARNING "Пакет ${PKG} не найден. Устанавливаем...")
            execute_process(
                COMMAND sudo pacman -S --noconfirm ${PKG}
                RESULT_VARIABLE INSTALL_RESULT
            )
            if(NOT INSTALL_RESULT EQUAL 0)
                message(FATAL_ERROR "Не удалось установить ${PKG}!")
            endif()
        endif()
    endforeach()
else()
    set(PKG_LIST "")
    foreach(PKG ${REQUIRED_PACKAGES})
        set(PKG_LIST "${PKG_LIST} ${PKG}")
    endforeach()
    message(WARNING "pacman не найден. Установите зависимости вручную:${PKG_LIST}")
endif()



pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)
pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)




file(GLOB SOURCES "src/*.cpp" "src/**/*.cpp")
file(GLOB HEADERS "src/*.hpp" "src/**/*.hpp")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})


target_include_directories(${PROJECT_NAME} PRIVATE ${GTKMM_INCLUDE_DIRS} ${YAML_CPP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${GTKMM_LIBRARIES} ${YAML_CPP_LIBRARIES})
