# Library
add_library(image_processing STATIC)

target_sources(image_processing PRIVATE
    image_processing.cpp
    image_processing.h
)

target_include_directories(image_processing PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_compile_features(image_processing PUBLIC cxx_std_20)

# OpenCV - FESTE VERBINDUNG
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs highgui)  # highgui für imshow/waitKey

if(TARGET OpenCV::OpenCV)
    target_link_libraries(image_processing PUBLIC OpenCV::OpenCV)
elseif(TARGET OpenCV::opencv_core)
    target_link_libraries(image_processing PUBLIC
        OpenCV::opencv_core
        OpenCV::opencv_imgproc
        OpenCV::opencv_imgcodecs
        OpenCV::opencv_highgui  # Für imshow und waitKey
    )
else()
    target_include_directories(image_processing PUBLIC ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(image_processing PUBLIC ${OpenCV_LIBS})
endif()

# # LibRaw und LCMS2
# target_include_directories(image_processing PUBLIC
#     /opt/homebrew/Cellar/libraw/0.21.4/include/libraw
#     /opt/homebrew/Cellar/libraw/0.21.4/include
#     /opt/homebrew/opt/little-cms2/include
# )

# target_link_libraries(image_processing PUBLIC
#     /opt/homebrew/Cellar/libraw/0.21.4/lib/libraw.dylib
#     /opt/homebrew/lib/liblcms2.dylib
# )

# Warnungen
target_compile_options(image_processing PRIVATE 
    $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic -g>
)

# Test - WICHTIG: Auch OpenCV verlinken!
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_main.cpp")
    add_executable(image_processing_test test_main.cpp)
    target_link_libraries(image_processing_test PRIVATE 
        image_processing
        ${OpenCV_LIBS}  # OpenCV auch hier verlinken!
    )
endif()