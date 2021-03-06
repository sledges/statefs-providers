pkg_check_modules(COR_UDEV cor-udev REQUIRED)
pkg_check_modules(COR cor REQUIRED)
pkg_check_modules(STATEFS_UTIL statefs-util REQUIRED)

add_definitions(-DBOOST_ASIO_DISABLE_EPOLL)
find_package(Boost COMPONENTS system REQUIRED)
set(Boost_USE_MULTITHREADED ON)
set(Boost_COMPILER -gcc)

include_directories(
  ${COR_INCLUDE_DIRS}
  ${COR_UDEV_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIR}
  ${STATEFS_UTIL_INCLUDE_DIRS}
)

link_directories(
  ${COR_LIBRARY_DIRS}
  ${COR_UDEV_LIBRARY_DIRS}
  ${BOOST_LIBRARY_DIRS}
  ${STATEFS_UTIL_LIBRARY_DIRS}
)

add_library(provider-udev SHARED
  provider_udev.cpp
  )

target_link_libraries(provider-udev
  ${COR_LIBRARIES}
  ${COR_UDEV_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${STATEFS_LIBRARIES}
  ${STATEFS_UTIL_LIBRARIES}
  ${Boost_SYSTEM_LIBRARY}
  )

install(TARGETS provider-udev DESTINATION ${DST_LIB}/statefs)
