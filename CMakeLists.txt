###    Esserini, a life simulation program
###    Copyright (C) 2011-...  Mauro Bianco
###
###    This program is free software: you can redistribute it and/or modify
###    it under the terms of the GNU General Public License as published by
###    the Free Software Foundation, either version 3 of the License, or
###    (at your option) any later version.
###
###    This program is distributed in the hope that it will be useful,
###    but WITHOUT ANY WARRANTY; without even the implied warranty of
###    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
###    GNU General Public License for more details.
###
###    You should have received a copy of the GNU General Public License
###    along with this program.  If not, see <http://www.gnu.org/licenses/>.
###
###    Contact information: mauro d o t bianco a t gmail d o t com


Cmake_minimum_required( VERSION 2.8.4 )

project(esserini)

### #Boost
### find_package( Boost )
### if( Boost_FOUND )
###   include_directories( ${Boost_INCLUDE_DIR} )
### endif()

find_package( Boost )
if( Boost_FOUND )
  include_directories( ${Boost_INCLUDE_DIR} )
endif()

include_directories( . ${Boost_INCLUDE_DIR} )
link_directories(${Boost_LIBRARY_DIRS})

### Library
set(EXECUTABLE_OUTPUT_PATH "build")

### Executables

add_executable( esserini main.cc goodness.cc instructions.cc monster.cc mondo.cc)

target_link_libraries (esserini boost_program_options) 
