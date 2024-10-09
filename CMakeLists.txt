# CMakeList.txt: проект CMake для RakBot; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

file(GLOB raknetH "raknet/*.h")
file(GLOB raknetCPP "raknet/*.cpp")
file(GLOB sampH "raknet/SAMP/*.h")
file(GLOB sampCPP "raknet/SAMP/*.cpp")


# Добавьте источник в исполняемый файл этого проекта.
add_executable (RakBot "RakBot.cpp" "RakBot.h" "gettimeofday.cpp" "gettimeofday.h" "Log.cpp" "Log.h" ${raknetH} ${raknetCPP} ${sampH} ${sampCPP}  "CNetGame.cpp" "CNetGame.h" "math_stuff.h" "math_stuff.cpp" "CNetRPC.h" "CNetRPC.cpp"            "Game/GLocalPlayer.h" "Game/GLocalPlayer.cpp" "SampFuncs.cpp" "SampFuncs.h" "Math.h" "Math.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET RakBot PROPERTY CXX_STANDARD 14)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
