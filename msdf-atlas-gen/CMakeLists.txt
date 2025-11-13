add_library(msdfgen STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/Contour.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/DistanceMapping.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/EdgeHolder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/MSDFErrorCorrection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/Projection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/Scanline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/Shape.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/contour-combiners.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/convergent-curve-ordering.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/edge-coloring.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/edge-segments.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/edge-selectors.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/equation-solver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/export-svg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/msdf-error-correction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/msdfgen.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/rasterization.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/render-sdf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/save-bmp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/save-fl32.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/save-rgba.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/save-tiff.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/sdf-error-estimation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/core/shape-description.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/ext/import-font.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/ext/resolve-shape-geometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen/ext/save-png.cpp
)

target_include_directories(msdfgen PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen
)

target_compile_definitions(msdfgen PUBLIC
    MSDFGEN_PUBLIC=
    MSDFGEN_USE_CPP11
    MSDFGEN_USE_FREETYPE
    MSDFGEN_USE_LIBPNG
)

set_target_properties(msdfgen PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(msdfgen PUBLIC freetype PNG::PNG)

add_library(msdf-atlas-gen STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/Charset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/FontGeometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/GlyphGeometry.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/GridAtlasPacker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/Padding.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/RectanglePacker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/TightAtlasPacker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/Workload.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/artery-font-export.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/bitmap-blit.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/charset-parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/csv-export.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/glyph-generators.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/image-encode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/json-export.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/shadron-preview-generator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/size-selectors.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdf-atlas-gen/utf8.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/atlas_wrapper.cpp
)

target_include_directories(msdf-atlas-gen PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/msdfgen
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/msdf-atlas-gen/artery-font-format
)

target_compile_definitions(msdf-atlas-gen PUBLIC
    MSDF_ATLAS_PUBLIC=
    MSDF_ATLAS_VERSION="2.3.0"
    MSDF_ATLAS_VERSION_MAJOR=2
    MSDF_ATLAS_VERSION_MINOR=3
    MSDF_ATLAS_VERSION_REVISION=0
    MSDF_ATLAS_COPYRIGHT_YEAR=2024
    MSDFGEN_USE_LIBPNG
)

set_target_properties(msdf-atlas-gen PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(msdf-atlas-gen PUBLIC msdfgen freetype)
