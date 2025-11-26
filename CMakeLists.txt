cmake_minimum_required(VERSION 3.10)
project(gaudyk LANGUAGES CXX)

# Nustato C++ standartą į C++14 (kaip nurodyta originalioje užklausoje)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Randa SFML bibliotekas. SFML 3.x reikalauja komponentų nurodymo.
# Naudojami komponentai: Graphics, Window ir System.
find_package(SFML COMPONENTS Graphics Window System REQUIRED)

# Nurodo įtraukiamųjų failų (headerių) aplankus
include_directories(
        src
        resources
)

# Šaltinio failai (.cpp), kuriuos reikia kompiliuoti
set(SOURCE_FILES
        src/main.cpp
        src/utils.cpp
)

# Sukuria vykdomąjį failą
add_executable(gaudyk ${SOURCE_FILES})

# Susieja SFML bibliotekas su vykdomuoju failu.
# Naudojami modernūs SFML 3.x tiksliniai pavadinimai (SFML::Graphics ir t.t.),
# nes jie yra teisingas būdas nuorodoms, kai nurodyti komponentai find_package komandoje.
target_link_libraries(gaudyk PRIVATE SFML::Graphics SFML::Window SFML::System)

# ---- Resursų kopijavimo funkcija ----
# Ši dalis nukopijuoja visus resursus (nuotraukas, šriftus) ir DLL failą (Windows atveju)
# į vykdomojo failo aplanką, kad programa galėtų juos rasti.

# Nurodo visus resursų failus, kurie turi būti nukopijuoti
set(ASSET_FILES
        resources/arialbd.ttf
        resources/background.png
        resources/bolt.png
        resources/dezikas.png
        resources/him.png
        resources/him2.png
        resources/wolt.png
)

# Kopijavimo vieta (šalia vykdomojo failo)
set(DEST_DIR "$<TARGET_FILE_DIR:gaudyk>")

# Kopijuoja resursus į 'resources' aplanką šalia vykdomojo failo
foreach(file ${ASSET_FILES})
    # Gauna failo pavadinimą (pvz., arialbd.ttf)
    get_filename_component(FILENAME ${file} NAME)
    add_custom_command(
            TARGET gaudyk POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${file}"
            "${DEST_DIR}/resources/${FILENAME}"
            COMMENT "Kopijuoja resursą: ${file} į ${DEST_DIR}/resources/"
    )
endforeach()

# Kopijuoja DLL failą į vykdomojo failo aplanką (reikalinga Windows sistemose)
add_custom_command(
        TARGET gaudyk POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/openal32.dll"
        "${DEST_DIR}/openal32.dll"
        COMMENT "Kopijuoja openal32.dll"
)