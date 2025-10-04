# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/piotr/esp/esp-idf/components/bootloader/subproject"
  "/home/piotr/Documents/Studia/praca_inzynierska/smart-medication-box/firmware/build/bootloader"
  "/home/piotr/Documents/Studia/praca_inzynierska/smart-medication-box/firmware/build/bootloader-prefix"
  "/home/piotr/Documents/Studia/praca_inzynierska/smart-medication-box/firmware/build/bootloader-prefix/tmp"
  "/home/piotr/Documents/Studia/praca_inzynierska/smart-medication-box/firmware/build/bootloader-prefix/src/bootloader-stamp"
  "/home/piotr/Documents/Studia/praca_inzynierska/smart-medication-box/firmware/build/bootloader-prefix/src"
  "/home/piotr/Documents/Studia/praca_inzynierska/smart-medication-box/firmware/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/piotr/Documents/Studia/praca_inzynierska/smart-medication-box/firmware/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/piotr/Documents/Studia/praca_inzynierska/smart-medication-box/firmware/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
