set(TOOLS_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE STRING "" FORCE)

# Generates hpp and cpp file in destDirectory
#
# IN
#  - genTarget: new target name (used later for add_dependencies() call)
#  - scxml: path to scxml file
#  - className: class name to use when generating code (default suffix will be added)
#  - destDirectory: path to directory where to save generated files
# OUT
#  - outSrcVariableName: name of the variable where to store path to generated cpp file
function(generateHsm genTarget scxml className destDirectory outSrcVariableName)
    set(templateHpp ${TOOLS_DIR}/template.hpp)
    set(templateCpp ${TOOLS_DIR}/template.cpp)

    find_package(Python3 COMPONENTS Interpreter Development)
    message("Generating HSM from: ${scxml}")
    message(" -- class: ${className}")
    message(" -- output: ${destDirectory}")

    set(GEN_CPP ${destDirectory}/${className}Base.cpp)
    set(${outSrcVariableName} ${GEN_CPP} PARENT_SCOPE)

    add_custom_command(OUTPUT ${GEN_CPP}
                       COMMAND ${Python3_EXECUTABLE} ${TOOLS_DIR}/scxml2gen.py -code -s ${scxml} -c ${className} -thpp ${templateHpp} -tcpp ${templateCpp} -d ${destDirectory}
                       DEPENDS ${scxml} ${templateHpp} ${templateCpp}
                       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    add_custom_target(${genTarget} DEPENDS ${GEN_CPP})
endfunction()

# Extended version of generateHsm which allows to provide custom template and destination files path
#
# IN
#  - genTarget: new target name (used later for add_dependencies() call)
#  - scxml: path to scxml file
#  - className: class name to use when generating code (default suffix will be added)
#  - classSuffix: suffix to append to class name when generating code
#  - destDirectory: path to directory where to save generated files
#  - templateHpp, templateCpp: path to HPP and CPP templates
#  - destHpp, destCpp: destination path for generated HPP and CPP files
function(generateHsmEx genTarget scxml className classSuffix templateHpp templateCpp destHpp destCpp)
    find_package(Python3 COMPONENTS Interpreter Development)

    message("Generating HSM from: ${scxml}")
    message(" -- class: ${className}")
    message(" -- suffix: ${classSuffix}")
    message(" -- template: ${templateHpp} , ${templateCpp}")
    message(" -- output: ${destHpp}, ${destCpp}")

    add_custom_command(OUTPUT ${destCpp}
                       COMMAND ${Python3_EXECUTABLE} ${TOOLS_DIR}/scxml2gen.py -code -s ${scxml} -c ${className} -cs ${classSuffix} -thpp ${templateHpp} -tcpp ${templateCpp} -dhpp ${destHpp} -dcpp ${destCpp}
                       DEPENDS ${scxml}
                       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    add_custom_target(genTarget DEPENDS ${destCpp})
endfunction()

# Generates PlantUML diagram for HSM
#
# IN
#  - genTarget: new target name (used later for add_dependencies() call)
#  - scxml: path to scxml file
#  - destFile: path where to save generated diagram
function(generateHsmDiagram genTarget scxml destFile)
    find_package(Python3 COMPONENTS Interpreter Development)

    message("Generating HSM Diagram from: ${scxml}")
    message(" -- output: ${destFile}")

    add_custom_command(OUTPUT ${destFile}
                       COMMAND ${Python3_EXECUTABLE} ${TOOLS_DIR}/scxml2gen.py -plantuml -s ${scxml} -o ${destFile}
                       DEPENDS ${scxml}
                       WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    add_custom_target(${genTarget} DEPENDS ${destFile})
endfunction()
