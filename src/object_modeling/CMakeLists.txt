cmake_minimum_required (VERSION 2.6)
project(OBJECT_MODELING)

# G2O on Linux at least now seems to require this more than it used to...
# C++11 support
if (UNIX)
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()
endif()


find_package(OpenNI2)
if(OPENNI2_FOUND)
    include_directories(${OPENNI2_INCLUDE_DIRS})
    link_directories(${OPENNI2_REDIST_DIR})
    link_libraries(${OPENNI2_LIBRARY})
endif()

# build type (new)
#if( NOT CMAKE_BUILD_TYPE )
#  set( CMAKE_BUILD_TYPE Debug CACHE STRING
#       "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
#       FORCE )
#endif()
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release)
endif()

# look here for cmake modules as well
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake_modules)

# overall build settings
SET(BUILD_LIBRARIES true)
SET(FRAME_PROVIDER_OPENNI2 true)
SET(FRAME_PROVIDER_PCL false)
SET(VOLUME_MODELER_GLFW true)
SET(DBOW_PLACE_RECOGNITION true) # see this

SET(BUILD_OLD_OBJECT_MODELER false) # SHOULD BE FALSE (and no longer compiles)

# move these later to avoid recompiles of everything?
IF (FRAME_PROVIDER_OPENNI2)
add_definitions(-DFRAME_PROVIDER_OPENNI2)
ENDIF()
IF (FRAME_PROVIDER_PCL)
add_definitions(-DFRAME_PROVIDER_PCL)
ENDIF()
IF (VOLUME_MODELER_GLFW)
add_definitions(-DVOLUME_MODELER_GLFW)
ENDIF()

# assume we only want shared also on linux (for now)
if (MSVC)
	set(BUILD_SHARED_ALSO false)
else()  
# no-one needs shared libraries right now
        #set(BUILD_SHARED_ALSO true)
set(BUILD_SHARED_ALSO false)
endif()
message (STATUS "BUILD_SHARED_ALSO: ${BUILD_SHARED_ALSO}")


# openni2
# only inlude if FRAME_PROVIDER_OPENNI2
IF (FRAME_PROVIDER_OPENNI2)
IF (MSVC)
# WINDOWS
	IF ( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		include_directories("$ENV{OPENNI2_INCLUDE64}")
		link_directories("$ENV{OPENNI2_LIB64}")
	ELSE()
		include_directories("$ENV{OPENNI2_INCLUDE}")
		link_directories("$ENV{OPENNI2_LIB}")
	ENDIF()
#ELSE()
# LINUX
#	include_directories("$ENV{OPENNI2_INCLUDE}")
#	link_directories("./")
#	set(CMAKE_EXE_LINKER_FLAGS "-Wl,-rpath ./")
ENDIF()
ENDIF()


# PCL
# brings in eigen
IF(FRAME_PROVIDER_PCL)
find_package(PCL 1.6 REQUIRED)
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})
message(STATUS "PCL_INCLUDE_DIRS: ${PCL_INCLUDE_DIRS}")
ELSE()
# eigen (alone!)
find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})
ENDIF()


# opencv
# switching off default path because my ubuntu has two versions of OpenCV (one to make ROS happy, one with nonfree that I compiled)
#find_package( OpenCV REQUIRED )
find_package( OpenCV REQUIRED )


# opencl
find_package( OpenCL REQUIRED )
include_directories(${OPENCL_INCLUDE_DIR})
# link_directories?
# add_definitions?


# openmp
find_package( OpenMP )
if (OPENMP_FOUND)
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	add_definitions(-DHAS_OPENMP)
endif()


# DLoopDetector
# http://webdiis.unizar.es/~dorian
# do this after OpenCV
IF (DBOW_PLACE_RECOGNITION)
add_definitions(-DDBOW_PLACE_RECOGNITION)
find_package(DLib REQUIRED)
find_package(DBoW2 REQUIRED)
find_package(DLoopDetector REQUIRED)
include_directories(${DLoopDetector_INCLUDE_DIRS} ${DLib_INCLUDE_DIRS} ${DBoW2_INCLUDE_DIRS})
message(STATUS "DLoopDetector_INCLUDE_DIRS: ${DLoopDetector_INCLUDE_DIRS}")
message(STATUS "DLib_INCLUDE_DIRS: ${DLib_INCLUDE_DIRS}")
message(STATUS "DBoW2_INCLUDE_DIRS: ${DBoW2_INCLUDE_DIRS}")
ENDIF()

# g2o
# THIS IS AN UGLY HACK...FIX IT SOMETIME
if (MSVC)
set(G2O_ROOT "C:/devlibs/github/g2o/install")
endif()
find_package(G2O REQUIRED)
message(STATUS "G2O_INCLUDE_DIRS: ${G2O_INCLUDE_DIRS}")
message(STATUS "G2O_LIBRARIES: ${G2O_LIBRARIES}")
message(STATUS "G2O_LIBRARIES_DEBUG: ${G2O_LIBRARIES_DEBUG}")
include_directories(${G2O_INCLUDE_DIRS})
# THIS IS ALSO AN UGLY HACK...SHOULD BE IN FINDG2O.CMAKE
FOREACH(L ${G2O_LIBRARIES})
  SET(WITH_KEYWORD optimized ${L})
  SET(G2O_LIBRARIES_TO_LINK ${G2O_LIBRARIES_TO_LINK} ${WITH_KEYWORD})
ENDFOREACH()
MESSAGE(STATUS "G2O_LIBRARIES_TO_LINK: ${G2O_LIBRARIES_TO_LINK}")
FOREACH(L ${G2O_LIBRARIES_DEBUG})
  SET(WITH_KEYWORD debug ${L})
  SET(G2O_LIBRARIES_DEBUG_TO_LINK ${G2O_LIBRARIES_DEBUG_TO_LINK} ${WITH_KEYWORD})
ENDFOREACH()
MESSAGE(STATUS "G2O_LIBRARIES_DEBUG_TO_LINK: ${G2O_LIBRARIES_DEBUG_TO_LINK}")

# boost
# hmmm...(only) static works on my PCL-based boost in windows, but fails when building shared on ubuntu
if (MSVC)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME OFF) # new idea for prebuilt version from boost.org?
else()
set(Boost_USE_STATIC_LIBS OFF)
endif()
find_package (Boost COMPONENTS thread system filesystem program_options serialization REQUIRED)
IF (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR})
	link_directories(${Boost_LIBRARY_DIRS})
	add_definitions( "-DHAS_BOOST" ) # sure...
ENDIF()

# csparse (unix only??)
if (NOT MSVC)
find_package (CSparse REQUIRED)
include_directories(${CSPARSE_INCLUDE_DIR})
endif()

# ME:

############################################################################
# LIBRARIES
if (BUILD_LIBRARIES) # HUGE IF BLOCK


############
# a ply library not in PCL (!!)
add_library( ply_lib STATIC
ply_lib/rply.h
ply_lib/rply.c
)
IF(BUILD_SHARED_ALSO)
add_library( ply_lib_shared SHARED
ply_lib/rply.h
ply_lib/rply.c
)
ENDIF()
include_directories(ply_lib)


#########################
# eigen utilities lib
SET(EIGEN_UTILITIES_SOURCE_FILES
EigenUtilities/EigenUtilities.cpp
EigenUtilities/ros_timestamp.cpp
)
SET(EIGEN_UTILITIES_HEADER_FILES
EigenUtilities/EigenUtilities.h
EigenUtilities/EigenUtilities.hpp
EigenUtilities/ros_timestamp.h
EigenUtilities/params_camera.h
)
add_library( eigen_utilities STATIC
${EIGEN_UTILITIES_SOURCE_FILES}
${EIGEN_UTILITIES_HEADER_FILES}
)
target_link_libraries(eigen_utilities ${Boost_LIBRARIES})
IF(BUILD_SHARED_ALSO)
add_library( eigen_utilities_shared SHARED
${EIGEN_UTILITIES_SOURCE_FILES}
${EIGEN_UTILITIES_HEADER_FILES}
)
target_link_libraries(eigen_utilities_shared ${Boost_LIBRARIES})
ENDIF()
include_directories(EigenUtilities)


#########################
# opencv utilities lib
SET(OPENCV_UTILITIES_SOURCE_FILES
opencv_utilities/opencv_utilities.cpp
opencv_utilities/pick_pixel.cpp
opencv_utilities/trackbar_window.cpp
opencv_utilities/cvplot.cpp
)
SET(OPENCV_UTILITIES_HEADER_FILES
opencv_utilities/opencv_utilities.h
opencv_utilities/pick_pixel.h
opencv_utilities/trackbar_window.h
opencv_utilities/cvplot.h
)
add_library( opencv_utilities STATIC
${OPENCV_UTILITIES_SOURCE_FILES}
${OPENCV_UTILITIES_HEADER_FILES}
)
target_link_libraries(opencv_utilities ${OpenCV_LIBS})
IF(BUILD_SHARED_ALSO)
add_library( opencv_utilities_shared SHARED
${OPENCV_UTILITIES_SOURCE_FILES}
${OPENCV_UTILITIES_HEADER_FILES}
)
target_link_libraries(opencv_utilities_shared ${OpenCV_LIBS})
ENDIF()
include_directories(opencv_utilities)


#########################
# new general utilities lib (could replace and include opencv utilities, etc. if you want)
SET(UTILITIES_SOURCE_FILES
utilities/ordering_container.cpp
utilities/ordering_container.h
)
add_library( utilities STATIC
${UTILITIES_SOURCE_FILES}
)
target_link_libraries(utilities opencv_utilities)
target_link_libraries(utilities eigen_utilities)
IF(BUILD_SHARED_ALSO)
add_library( utilities_shared SHARED
${UTILITIES_SOURCE_FILES}
)
target_link_libraries(utilities_shared opencv_utilities_shared)
target_link_libraries(utilities_shared eigen_utilities_shared)
ENDIF()
include_directories(utilities)


#########################
# opencl static lib
SET(OPENCL_STATIC_LIB_SOURCE_FILES
#OpenCLStaticLib/CIsoSurface.cpp
#OpenCLStaticLib/Vectors.cpp
OpenCLStaticLib/ImageBuffer.cpp
OpenCLStaticLib/BufferWrapper.cpp
OpenCLStaticLib/Noise.cpp
OpenCLStaticLib/OpenCLImages.cpp
OpenCLStaticLib/OpenCLKernelsBuilder.cpp
OpenCLStaticLib/OpenCLNormals.cpp
OpenCLStaticLib/OpenCLOptimize.cpp
OpenCLStaticLib/OpenCLOptimizeNew.cpp
OpenCLStaticLib/OpenCLTSDF.cpp
OpenCLStaticLib/RenderBuffers.cpp
OpenCLStaticLib/stdafx.cpp
OpenCLStaticLib/cl_util.cpp
OpenCLStaticLib/cll.cpp
OpenCLStaticLib/util.cpp
OpenCLStaticLib/convertOpenCLEigen.cpp
OpenCLStaticLib/OpenCLAllKernels.cpp
OpenCLStaticLib/MarchingCubes.cpp
OpenCLStaticLib/MarchingCubesManyVolumes.cpp
OpenCLStaticLib/VolumeBuffer.cpp
OpenCLStaticLib/MeshUtilities.cpp


OpenCLStaticLib/KernelSetFloat.cpp
OpenCLStaticLib/KernelSetUChar.cpp
OpenCLStaticLib/KernelSetInt.cpp
OpenCLStaticLib/KernelSetFloat4.cpp

OpenCLStaticLib/KernelDivideFloats.cpp
OpenCLStaticLib/KernelAddFloats.cpp
OpenCLStaticLib/KernelAddFloatsWithWeights.cpp
OpenCLStaticLib/KernelAddFloatsWithWeightsExternalWeight.cpp
OpenCLStaticLib/KernelMinAbsFloatsWithWeights.cpp
OpenCLStaticLib/KernelMinAbsFloatsWithWeightsRecordIndex.cpp
OpenCLStaticLib/KernelMinAbsFloatsWithWeightsAndMinimumWeightFraction.cpp
OpenCLStaticLib/KernelGaussianPDF.cpp
OpenCLStaticLib/KernelGaussianPDFConstantX.cpp
OpenCLStaticLib/KernelAddFrameToHistogram.cpp
OpenCLStaticLib/KernelHistogramSum.cpp
OpenCLStaticLib/KernelHistogramSumCheckIndex.cpp
OpenCLStaticLib/KernelHistogramMax.cpp
OpenCLStaticLib/KernelHistogramMaxCheckIndex.cpp
OpenCLStaticLib/KernelHistogramVariance.cpp
OpenCLStaticLib/KernelExtractVolumeSlice.cpp
OpenCLStaticLib/KernelExtractVolumeSliceFloat4.cpp
OpenCLStaticLib/KernelExtractVolumeSliceFloat4Length.cpp
OpenCLStaticLib/KernelExtractVolumeFloat.cpp
OpenCLStaticLib/KernelAddFrame.cpp
OpenCLStaticLib/KernelAddFrameTo2Means.cpp
OpenCLStaticLib/KernelAddFrameTo2MeansUsingNormals.cpp
OpenCLStaticLib/KernelAddFrameTo2MeansUsingStoredNormals.cpp
OpenCLStaticLib/KernelAddFrameIfCompatible.cpp

OpenCLStaticLib/KernelRenderPointsAndNormals.cpp
OpenCLStaticLib/KernelRenderPoints.cpp
OpenCLStaticLib/KernelRenderNormalForPoints.cpp
OpenCLStaticLib/KernelRenderColorForPoints.cpp
OpenCLStaticLib/KernelRenderMax.cpp
OpenCLStaticLib/KernelRender2MeansAbs.cpp

OpenCLStaticLib/KernelNormalsToShadedImage.cpp
OpenCLStaticLib/KernelNormalsToColorImage.cpp
OpenCLStaticLib/KernelPointsToDepthImage.cpp
OpenCLStaticLib/KernelDotVolumeNormal.cpp
OpenCLStaticLib/KernelMinAbsVolume.cpp
OpenCLStaticLib/KernelAddVolumes.cpp
OpenCLStaticLib/KernelBetterNormal.cpp
OpenCLStaticLib/KernelExtractFloat4ForPointImage.cpp
OpenCLStaticLib/KernelExtractIntForPointImage.cpp
OpenCLStaticLib/KernelApplyPoseToPoints.cpp
OpenCLStaticLib/KernelApplyPoseToNormals.cpp

OpenCLStaticLib/KernelComputeNormalVolume.cpp
OpenCLStaticLib/KernelComputeNormalVolumeWithWeights.cpp
OpenCLStaticLib/KernelComputeNormalVolumeWithWeightsUnnormalized.cpp

OpenCLStaticLib/KernelMaxFloats.cpp
OpenCLStaticLib/KernelMinFloats.cpp
OpenCLStaticLib/KernelPickIfIndexFloats.cpp
OpenCLStaticLib/KernelPickIfIndexFloat4.cpp

OpenCLStaticLib/KernelSetVolumeSDFBox.cpp
OpenCLStaticLib/KernelRaytraceBox.cpp
OpenCLStaticLib/KernelRaytraceSpecial.cpp

OpenCLStaticLib/KernelMarkPointsViolateEmpty.cpp

OpenCLStaticLib/KernelDepthImageToPoints.cpp
OpenCLStaticLib/KernelTransformPoints.cpp
OpenCLStaticLib/KernelSetInvalidPointsTrue.cpp

OpenCLStaticLib/KernelOptimizeErrorAndJacobianICP.cpp
OpenCLStaticLib/KernelOptimizeErrorAndJacobianImage.cpp
OpenCLStaticLib/KernelOptimizeNormalEquationTerms.cpp

OpenCLStaticLib/KernelVignetteApplyModelPolynomial3Uchar4.cpp
OpenCLStaticLib/KernelVignetteApplyModelPolynomial3Float.cpp


OpenCLStaticLib/Images.cl
OpenCLStaticLib/Normals.cl
OpenCLStaticLib/Optimize.cl
OpenCLStaticLib/TSDF.cl
)
SET(OPENCL_STATIC_LIB_HEADER_FILES
#OpenCLStaticLib/CIsoSurface.h
#OpenCLStaticLib/Vectors.h
OpenCLStaticLib/convertOpenCLEigen.h
OpenCLStaticLib/ImageBuffer.h
OpenCLStaticLib/BufferWrapper.h
OpenCLStaticLib/Noise.h
OpenCLStaticLib/OpenCLImages.h
OpenCLStaticLib/OpenCLKernelsBuilder.h
OpenCLStaticLib/OpenCLNormals.h
OpenCLStaticLib/OpenCLOptimize.h
OpenCLStaticLib/OpenCLOptimizeNew.h
OpenCLStaticLib/OpenCLTSDF.h
OpenCLStaticLib/RenderBuffers.h
OpenCLStaticLib/RunLengthEncoding.hpp
OpenCLStaticLib/stdafx.h
OpenCLStaticLib/targetver.h
OpenCLStaticLib/cl_util.h
OpenCLStaticLib/cll.h
OpenCLStaticLib/util.h
OpenCLStaticLib/OpenCLAllKernels.h
OpenCLStaticLib/MarchingCubes.h
OpenCLStaticLib/MarchingCubesManyVolumes.h
OpenCLStaticLib/VolumeBuffer.h
OpenCLStaticLib/MeshTypes.h
OpenCLStaticLib/MeshUtilities.h

OpenCLStaticLib/KernelSetFloat.h
OpenCLStaticLib/KernelSetUChar.h
OpenCLStaticLib/KernelSetInt.h
OpenCLStaticLib/KernelSetFloat4.h

OpenCLStaticLib/KernelDivideFloats.h
OpenCLStaticLib/KernelAddFloats.h
OpenCLStaticLib/KernelAddFloatsWithWeights.h
OpenCLStaticLib/KernelAddFloatsWithWeightsExternalWeight.h
OpenCLStaticLib/KernelMinAbsFloatsWithWeights.h
OpenCLStaticLib/KernelMinAbsFloatsWithWeightsRecordIndex.h
OpenCLStaticLib/KernelMinAbsFloatsWithWeightsAndMinimumWeightFraction.h
OpenCLStaticLib/KernelGaussianPDF.h
OpenCLStaticLib/KernelGaussianPDFConstantX.h
OpenCLStaticLib/KernelAddFrameToHistogram.h
OpenCLStaticLib/KernelHistogramSum.h
OpenCLStaticLib/KernelHistogramSumCheckIndex.h
OpenCLStaticLib/KernelHistogramMax.h
OpenCLStaticLib/KernelHistogramMaxCheckIndex.h
OpenCLStaticLib/KernelHistogramVariance.h
OpenCLStaticLib/KernelExtractVolumeSlice.h
OpenCLStaticLib/KernelExtractVolumeSliceFloat4.h
OpenCLStaticLib/KernelExtractVolumeSliceFloat4Length.h
OpenCLStaticLib/KernelExtractVolumeFloat.h
OpenCLStaticLib/KernelAddFrame.h
OpenCLStaticLib/KernelAddFrameTo2Means.h
OpenCLStaticLib/KernelAddFrameTo2MeansUsingNormals.h
OpenCLStaticLib/KernelAddFrameTo2MeansUsingStoredNormals.h
OpenCLStaticLib/KernelAddFrameIfCompatible.h

OpenCLStaticLib/KernelRenderPointsAndNormals.h
OpenCLStaticLib/KernelRenderPoints.h
OpenCLStaticLib/KernelRenderNormalForPoints.h
OpenCLStaticLib/KernelRenderColorForPoints.h
OpenCLStaticLib/KernelRenderMax.h
OpenCLStaticLib/KernelRender2MeansAbs.h


OpenCLStaticLib/KernelNormalsToShadedImage.h
OpenCLStaticLib/KernelNormalsToColorImage.h
OpenCLStaticLib/KernelPointsToDepthImage.h
OpenCLStaticLib/KernelDotVolumeNormal.h
OpenCLStaticLib/KernelMinAbsVolume.h
OpenCLStaticLib/KernelAddVolumes.h
OpenCLStaticLib/KernelBetterNormal.h
OpenCLStaticLib/KernelExtractFloat4ForPointImage.h
OpenCLStaticLib/KernelExtractIntForPointImage.h
OpenCLStaticLib/KernelApplyPoseToPoints.h
OpenCLStaticLib/KernelApplyPoseToNormals.h

OpenCLStaticLib/KernelComputeNormalVolume.h
OpenCLStaticLib/KernelComputeNormalVolumeWithWeights.h
OpenCLStaticLib/KernelComputeNormalVolumeWithWeightsUnnormalized.h

OpenCLStaticLib/KernelMaxFloats.h
OpenCLStaticLib/KernelMinFloats.h
OpenCLStaticLib/KernelPickIfIndexFloats.h
OpenCLStaticLib/KernelPickIfIndexFloat4.h

OpenCLStaticLib/KernelSetVolumeSDFBox.h
OpenCLStaticLib/KernelRaytraceBox.h
OpenCLStaticLib/KernelRaytraceSpecial.h

OpenCLStaticLib/KernelMarkPointsViolateEmpty.h

OpenCLStaticLib/KernelDepthImageToPoints.h
OpenCLStaticLib/KernelTransformPoints.h
OpenCLStaticLib/KernelSetInvalidPointsTrue.h

OpenCLStaticLib/KernelOptimizeErrorAndJacobianICP.h
OpenCLStaticLib/KernelOptimizeErrorAndJacobianImage.h
OpenCLStaticLib/KernelOptimizeNormalEquationTerms.h

OpenCLStaticLib/KernelVignetteApplyModelPolynomial3Uchar4.h
OpenCLStaticLib/KernelVignetteApplyModelPolynomial3Float.h

)
add_library( opencl_static_lib STATIC
${OPENCL_STATIC_LIB_SOURCE_FILES}
${OPENCL_STATIC_LIB_HEADER_FILES}
)
target_link_libraries(opencl_static_lib ${Boost_LIBRARIES})
target_link_libraries(opencl_static_lib ${OpenCV_LIBS})
target_link_libraries(opencl_static_lib ${OPENCL_LIBRARIES})
target_link_libraries(opencl_static_lib eigen_utilities)
target_link_libraries(opencl_static_lib opencv_utilities)
target_link_libraries(opencl_static_lib utilities)
target_link_libraries(opencl_static_lib ply_lib)
IF(BUILD_SHARED_ALSO)
add_library( opencl_static_lib_shared SHARED
${OPENCL_STATIC_LIB_SOURCE_FILES}
${OPENCL_STATIC_LIB_HEADER_FILES}
)
target_link_libraries(opencl_static_lib_shared ${Boost_LIBRARIES})
target_link_libraries(opencl_static_lib_shared ${OpenCV_LIBS})
target_link_libraries(opencl_static_lib_shared ${OPENCL_LIBRARIES})
target_link_libraries(opencl_static_lib_shared eigen_utilities_shared)
target_link_libraries(opencl_static_lib_shared opencv_utilities_shared)
target_link_libraries(opencl_static_lib_shared utilities_shared)
target_link_libraries(opencl_static_lib_shared ply_lib_shared)
ENDIF()
include_directories(OpenCLStaticLib)


#########################
# alignment library
SET(ALIGNMENT_LIB_SOURCE_FILES
alignment/alignment.cpp
)
SET(ALIGNMENT_LIB_HEADER_FILES
alignment/alignment.h
alignment/params_alignment.h
)
add_library( alignment_lib STATIC
${ALIGNMENT_LIB_SOURCE_FILES}
${ALIGNMENT_LIB_HEADER_FILES}
)
target_link_libraries(alignment_lib ${Boost_LIBRARIES})
target_link_libraries(alignment_lib ${OpenCV_LIBS})
target_link_libraries(alignment_lib ${OPENCL_LIBRARIES})
target_link_libraries(alignment_lib opencv_utilities)
target_link_libraries(alignment_lib opencl_static_lib)
IF(BUILD_SHARED_ALSO)
add_library( alignment_lib_shared SHARED
${ALIGNMENT_LIB_SOURCE_FILES}
${ALIGNMENT_LIB_HEADER_FILES}
)
target_link_libraries(alignment_lib_shared ${Boost_LIBRARIES})
target_link_libraries(alignment_lib_shared ${OpenCV_LIBS})
target_link_libraries(alignment_lib_shared ${OPENCL_LIBRARIES})
target_link_libraries(alignment_lib_shared opencv_utilities_shared)
target_link_libraries(alignment_lib_shared opencl_static_lib_shared)
ENDIF()
include_directories(alignment)


#########################
# g2o static lib
SET(G2O_LIB_SOURCE_FILES
"G2OStaticLib/G2OPoseGraph.cpp"
"G2OStaticLib/G2OPoseGraphImpl.cpp"
)
SET(G2O_LIB_HEADER_FILES
"G2OStaticLib/G2OPoseGraph.h"
"G2OStaticLib/G2OPoseGraphImpl.h"
"G2OStaticLib/G2OStereoProjector.hpp"
"G2OStaticLib/sac_model_registration_reprojection.h"
"G2OStaticLib/sac_model_registration_reprojection.hpp"
)	
add_library( g2o_static_lib STATIC
	${G2O_LIB_SOURCE_FILES}
	${G2O_LIB_HEADER_FILES}
	)
target_link_libraries(g2o_static_lib ${G2O_LIBRARIES_TO_LINK})
target_link_libraries(g2o_static_lib ${G2O_LIBRARIES_DEBUG_TO_LINK})
target_link_libraries(g2o_static_lib ${CSPARSE_LIBRARY} -lg2o_ext_csparse)

IF(BUILD_SHARED_ALSO)
add_library( g2o_static_lib_shared SHARED
	${G2O_LIB_SOURCE_FILES}
	${G2O_LIB_HEADER_FILES}
	)
target_link_libraries(g2o_static_lib_shared ${G2O_LIBRARIES_TO_LINK})
target_link_libraries(g2o_static_lib_shared ${G2O_LIBRARIES_DEBUG_TO_LINK})
target_link_libraries(g2o_static_lib_shared ${CSPARSE_LIBRARY})

ENDIF()
include_directories(G2OStaticLib)


#########
# FRAME_PROVIDER_LIB
SET(FRAME_PROVIDER_LIB_SOURCE_FILES
frame_provider/frame_provider.h
frame_provider/frame_provider_base.h
frame_provider/frame_provider_base.cpp
frame_provider/frame_provider_file_wrapper.h
frame_provider/frame_provider_file_wrapper.cpp
frame_provider/frame_provider_png.h
frame_provider/frame_provider_png.cpp
frame_provider/frame_provider_handa.h
frame_provider/frame_provider_handa.cpp
frame_provider/frame_provider_empty.h
frame_provider/frame_provider_empty.cpp
frame_provider/frame_provider_yuyin.h
frame_provider/frame_provider_yuyin.cpp
frame_provider/frame_provider_freiburg.h
frame_provider/frame_provider_freiburg.cpp
frame_provider/frame_provider_luis.h
frame_provider/frame_provider_luis.cpp
frame_provider/frame_provider_png_depth.h
frame_provider/frame_provider_png_depth.cpp
frame_provider/frame_provider_arun.h
frame_provider/frame_provider_arun.cpp
frame_provider/pose_provider_base.h
frame_provider/pose_provider_base.cpp
frame_provider/pose_provider_standard.h
frame_provider/pose_provider_standard.cpp
)
IF (FRAME_PROVIDER_OPENNI2)
SET(FRAME_PROVIDER_LIB_SOURCE_FILES
${FRAME_PROVIDER_LIB_SOURCE_FILES}
frame_provider/frame_provider_openni2.h
frame_provider/frame_provider_openni2.cpp
)
ENDIF()

IF (FRAME_PROVIDER_PCL)
SET(FRAME_PROVIDER_LIB_SOURCE_FILES
${FRAME_PROVIDER_LIB_SOURCE_FILES}
frame_provider/frame_provider_pcd.h
frame_provider/frame_provider_pcd.cpp
)
ENDIF()

add_library(frame_provider_lib STATIC
${FRAME_PROVIDER_LIB_SOURCE_FILES}
)
target_link_libraries(frame_provider_lib ${Boost_LIBRARIES})
target_link_libraries(frame_provider_lib opencv_utilities)
target_link_libraries(frame_provider_lib eigen_utilities)
IF (FRAME_PROVIDER_OPENNI2)
target_link_libraries(frame_provider_lib OpenNI2 ${OPENNI2_LIBRARY} )
ENDIF()
IF (FRAME_PROVIDER_PCL)
target_link_libraries(frame_provider_lib ${PCL_LIBRARIES} )
ENDIF()

IF(BUILD_SHARED_ALSO)
add_library(frame_provider_lib_shared SHARED
${FRAME_PROVIDER_LIB_SOURCE_FILES}
)
target_link_libraries(frame_provider_lib_shared ${Boost_LIBRARIES})
target_link_libraries(frame_provider_lib_shared opencv_utilities_shared)
target_link_libraries(frame_provider_lib_shared eigen_utilities_shared)
IF (FRAME_PROVIDER_OPENNI2)
target_link_libraries(frame_provider_lib_shared OpenNI2 )
ENDIF()
IF (FRAME_PROVIDER_PCL)
target_link_libraries(frame_provider_lib_shared ${PCL_LIBRARIES} )
ENDIF()
ENDIF()
include_directories(frame_provider)



#########################
# object model
IF (BUILD_OLD_OBJECT_MODELER)
SET(OBJECT_MODEL_SOURCE_FILES
"object_model/DisjointSet.cpp"
"object_model/Frustum.cpp"
"object_model/histogramUtil.cpp"
"object_model/LearnHistogram.cpp"
"object_model/main.cpp"
"object_model/MaskObject.cpp"
"object_model/ObjectModeler.cpp"
"object_model/opencvUtil.cpp"
"object_model/PCLGrabberInput.cpp"
#"object_model/OpenNIInput.cpp"
)

SET(OBJECT_MODEL_HEADER_FILES
"object_model/cloudToImage.hpp"
"object_model/DisjointSet.h"
"object_model/Frame.hpp"
"object_model/Frustum.h"
"object_model/histogramUtil.h"
"object_model/Keypoints.hpp"
"object_model/LearnHistogram.h"
"object_model/MaskObject.h"
"object_model/ObjectModeler.h"
"object_model/opencvUtil.h"
"object_model/opencv_eigen.hpp"
#"object_model/OpenNIInput.h"
"object_model/parameters.h"
"object_model/PCLGrabberInput.h"
"object_model/pclUtil.hpp"
"object_model/runningStatistics.h"
"object_model/stdafx.h"
"object_model/targetver.h"
"object_model/ToggleBase.hpp"
"object_model/ToggleCloud.hpp"
"object_model/ToggleCloudNormals.hpp"
"object_model/ToggleLineSet.hpp"
"object_model/ToggleMesh.hpp"
"object_model/ToggleNormals.hpp"
"object_model/typedefs.h"
)

SET(INCLUDE_OBJECT_MODEL TRUE)
if (INCLUDE_OBJECT_MODEL)
add_executable( object_model
	${OBJECT_MODEL_SOURCE_FILES}
	${OBJECT_MODEL_HEADER_FILES}
	)
target_link_libraries(object_model ${Boost_LIBRARIES})
target_link_libraries(object_model ${OpenCV_LIBS})
target_link_libraries(object_model ${PCL_LIBRARIES})
target_link_libraries(object_model opencl_static_lib)
target_link_libraries(object_model eigen_utilities)
target_link_libraries(object_model g2o_static_lib)
endif()
endif() # BUILD_OLD_OBJECT_MODELER

 
#########################
# volume modeler
SET(VOLUME_MODELER_SOURCE_FILES
volume_modeler/volume_modeler.cpp
volume_modeler/model_base.cpp
volume_modeler/model_single_volume.cpp
volume_modeler/model_grid.cpp
volume_modeler/model_patch.cpp
volume_modeler/model_moving_volume_grid.cpp
volume_modeler/model_histogram.cpp
volume_modeler/model_k_means.cpp
volume_modeler/frustum.cpp
volume_modeler/image_to_cloud.cpp
volume_modeler/feature_matching.cpp
volume_modeler/frame.cpp
volume_modeler/keypoints.cpp
volume_modeler/disjoint_set.cpp
volume_modeler/histogram_util.cpp
volume_modeler/learn_histogram.cpp
volume_modeler/mask_object.cpp
volume_modeler/vignette_calibration.cpp
volume_modeler/keyframe_struct.cpp
)
SET(VOLUME_MODELER_HEADER_FILES
volume_modeler/basic.h
volume_modeler/frame.h
volume_modeler/keypoints.h
volume_modeler/running_statistics.h
volume_modeler/model_base.h
volume_modeler/model_single_volume.h
volume_modeler/model_grid.h
volume_modeler/model_patch.h
volume_modeler/model_moving_volume_grid.h
volume_modeler/model_histogram.h
volume_modeler/model_k_means.h
volume_modeler/params_normals.h
volume_modeler/params_volume.h
volume_modeler/params_grid.h
volume_modeler/params_features.h
volume_modeler/params_loop_closure.h
volume_modeler/params_patch.h
volume_modeler/params_moving_volume_grid.h
volume_modeler/params_model_histogram.h
volume_modeler/params_model_k_means.h
volume_modeler/params_glfw_keys.h
volume_modeler/params_dbow_place_recognition.h
volume_modeler/volume_modeler_all_params.h
volume_modeler/running_statistics.h
volume_modeler/volume_modeler.h
volume_modeler/frustum.h
volume_modeler/image_to_cloud.h
volume_modeler/feature_matching.h
volume_modeler/params_volume_modeler.h
volume_modeler/update_interface.h
volume_modeler/disjoint_set.h
volume_modeler/histogram_util.h
volume_modeler/learn_histogram.h
volume_modeler/mask_object.h
volume_modeler/params_mask_object.h
volume_modeler/camera_struct.h
volume_modeler/keyframe_struct.h
volume_modeler/edge_struct.h
volume_modeler/vignette_calibration.h
)

IF(DBOW_PLACE_RECOGNITION)
SET(VOLUME_MODELER_SOURCE_FILES
${VOLUME_MODELER_SOURCE_FILES}
volume_modeler/dbow_place_recognition.cpp
volume_modeler/dbow_brief_extractor.cpp
)
SET(VOLUME_MODELER_HEADER_FILES
${VOLUME_MODELER_HEADER_FILES}
volume_modeler/dbow_place_recognition.h
volume_modeler/dbow_brief_extractor.h
)
ENDIF()


add_library( volume_modeler_lib STATIC
${VOLUME_MODELER_SOURCE_FILES}
${VOLUME_MODELER_HEADER_FILES}
)
target_link_libraries(volume_modeler_lib ${Boost_LIBRARIES})
target_link_libraries(volume_modeler_lib ${OpenCV_LIBS})
target_link_libraries(volume_modeler_lib opencl_static_lib)
target_link_libraries(volume_modeler_lib eigen_utilities)
target_link_libraries(volume_modeler_lib opencv_utilities)
target_link_libraries(volume_modeler_lib utilities)
target_link_libraries(volume_modeler_lib g2o_static_lib)
target_link_libraries(volume_modeler_lib ply_lib)
target_link_libraries(volume_modeler_lib alignment_lib)
target_link_libraries(volume_modeler_lib frame_provider_lib)
IF(DBOW_PLACE_RECOGNITION)
target_link_libraries(volume_modeler_lib ${DLib_LIBS} ${DBoW2_LIBS})
ENDIF()
IF(BUILD_SHARED_ALSO)
add_library( volume_modeler_lib_shared SHARED
${VOLUME_MODELER_SOURCE_FILES}
${VOLUME_MODELER_HEADER_FILES}
)
target_link_libraries(volume_modeler_lib_shared ${Boost_LIBRARIES})
target_link_libraries(volume_modeler_lib_shared ${OpenCV_LIBS})
target_link_libraries(volume_modeler_lib_shared opencl_static_lib_shared)
target_link_libraries(volume_modeler_lib_shared eigen_utilities_shared)
target_link_libraries(volume_modeler_lib_shared opencv_utilities_shared)
target_link_libraries(volume_modeler_lib_shared utilities_shared)
target_link_libraries(volume_modeler_lib_shared g2o_static_lib_shared)
target_link_libraries(volume_modeler_lib_shared ply_lib_shared)
target_link_libraries(volume_modeler_lib_shared alignment_lib_shared)
target_link_libraries(volume_modeler_lib_shared frame_provider_lib_shared)
ENDIF()
include_directories(volume_modeler)



# this links to volume_modeler_lib
IF (VOLUME_MODELER_GLFW)
# some initial crap (ignore):
#add_subdirectory(path/to/glfw)
#include_directories(path/to/glfw/include)
#target_link_libraries(myapp glfw ${GLFW_LIBRARIES})
#target_link_libraries(myapp glfw ${OPENGL_glu_LIBRARY} ${GLFW_LIBRARIES})

IF (MSVC)
find_package(GLFW REQUIRED)
set(CUSTOM_GLFW_LIBRARIES ${GLFW_LIBRARIES})
ELSE()
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
set(CUSTOM_GLFW_LIBRARIES ${GLFW_STATIC_LIBRARIES})
ENDIF()
MESSAGE(STATUS "GLFW_INCLUDE_DIRS: ${GLFW_INCLUDE_DIRS}")
MESSAGE(STATUS "CUSTOM_GLFW_LIBRARIES: ${CUSTOM_GLFW_LIBRARIES}")
include_directories(${GLFW_INCLUDE_DIRS})

# needed in addition to GLFW for linking...on windows at least
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIR}) # was DIRS, doesn't matter
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
	message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

add_library ( volume_modeler_glfw_lib
volume_modeler_glfw/volume_modeler_glfw.h
volume_modeler_glfw/volume_modeler_glfw.cpp
)
target_link_libraries(volume_modeler_glfw_lib ${CUSTOM_GLFW_LIBRARIES} ${OPENGL_LIBRARIES})
target_link_libraries(volume_modeler_glfw_lib ${Boost_LIBRARIES}) # at least threads
target_link_libraries(volume_modeler_glfw_lib volume_modeler_lib)
include_directories(volume_modeler_glfw)

ENDIF()



ENDIF() # BUILD_LIBRARIES
############################################################################


# MAIN executable for refactored volume modeler lib

# This hack allows me to use the same CMakeLists for with and without library version
IF (BUILD_LIBRARIES)
set(MAIN_FOLDER "volume_modeler_main/")
ELSE()
set(MAIN_FOLDER "")
ENDIF()

SET(VOLUME_MODELER_MAIN_SOURCE_FILES
${MAIN_FOLDER}volume_modeler_main.cpp
)
SET(VOLUME_MODELER_MAIN_HEADER_FILES
)
add_executable( volume_modeler_main
${VOLUME_MODELER_MAIN_SOURCE_FILES}
${VOLUME_MODELER_MAIN_HEADER_FILES}
)


IF (BUILD_LIBRARIES)
target_link_libraries(volume_modeler_main volume_modeler_lib)
IF (VOLUME_MODELER_GLFW)
target_link_libraries(volume_modeler_main volume_modeler_glfw_lib)
ENDIF (VOLUME_MODELER_GLFW)
ELSE()
# NOTE: ORDER MATTERS (earlier things need to link against later things)
target_link_libraries(volume_modeler_main ${CMAKE_SOURCE_DIR}/libvolume_modeler_lib.a)
target_link_libraries(volume_modeler_main ${CMAKE_SOURCE_DIR}/libalignment_lib.a)
target_link_libraries(volume_modeler_main ${CMAKE_SOURCE_DIR}/libopencl_static_lib.a)
target_link_libraries(volume_modeler_main ${CMAKE_SOURCE_DIR}/libframe_provider_lib.a)
target_link_libraries(volume_modeler_main ${CMAKE_SOURCE_DIR}/libg2o_static_lib.a)
target_link_libraries(volume_modeler_main ${CMAKE_SOURCE_DIR}/libopencv_utilities.a)
target_link_libraries(volume_modeler_main ${CMAKE_SOURCE_DIR}/libply_lib.a)
target_link_libraries(volume_modeler_main ${CMAKE_SOURCE_DIR}/libeigen_utilities.a)

target_link_libraries(volume_modeler_main ${Boost_LIBRARIES})
target_link_libraries(volume_modeler_main ${OpenCV_LIBS})
target_link_libraries(volume_modeler_main ${OPENCL_LIBRARIES})
target_link_libraries(volume_modeler_main ${G2O_LIBRARIES_TO_LINK})
target_link_libraries(volume_modeler_main ${G2O_LIBRARIES_DEBUG_TO_LINK})
IF (NOT MSVC)
target_link_libraries(volume_modeler_main ${CSPARSE_LIBRARY} )
ENDIF()
ENDIF()


#########################
# alignment main (alignment by itself test)
# I'm lazy and not making this work in the compiled version yet...
IF (FALSE) # not bothering with this...
IF (BUILD_LIBRARIES)
SET(ALIGNMENT_MAIN_SOURCE_FILES
alignment_main/main.cpp
)
SET(ALIGNMENT_MAIN_HEADER_FILES
)
add_executable( alignment_main
${ALIGNMENT_MAIN_SOURCE_FILES}
${ALIGNMENT_MAIN_HEADER_FILES}
)
target_link_libraries(alignment_main alignment_lib)
target_link_libraries(alignment_main frame_provider_lib)
ENDIF()
ENDIF()


#########################
# conversion utilities
IF (BUILD_LIBRARIES AND FRAME_PROVIDER_PCL)
add_executable( pcd_to_png
conversion_utilities/pcd_to_png.cpp
)
target_link_libraries(pcd_to_png frame_provider_lib)
ENDIF()


####################
# oni record (moved from openni_utilities)
# also probably not working...
#IF (BUILD_LIBRARIES)
#add_executable( oni_recorder
#oni_recorder/oni_recorder.cpp
#)
#target_link_libraries(oni_recorder frame_provider_lib)
#ENDIF()

####################
# tools (new...starting with generating synthetic data)
add_executable( generate_frames
tools/generate_frames.cpp
)
target_link_libraries(generate_frames volume_modeler_lib)
target_link_libraries(generate_frames volume_modeler_glfw_lib)

#add_executable( vignette_calibration
#tools/vignette_calibration_main.cpp
#)
#target_link_libraries(vignette_calibration volume_modeler_lib)
#target_link_libraries(vignette_calibration volume_modeler_glfw_lib)

#add_executable( test_g2o_crash
#tools/test_g2o_crash.cpp
#)
#target_link_libraries(test_g2o_crash volume_modeler_lib)
