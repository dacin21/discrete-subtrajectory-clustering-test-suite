cmake_minimum_required(VERSION 3.11)
project(movetk_examples)

# add dependencies
if (NOT TARGET c2d::movetk)
    find_package(movetk CONFIG REQUIRED)
endif()

if (MOVETK_WITH_CGAL_BACKEND)
  set(CGAL_BACKEND_ENABLED 1)
else ()
  set(CGAL_BACKEND_ENABLED 0)
endif ()

# Enable folders
if(CMAKE_GENERATOR MATCHES ".*Visual Studio.*")
    set_property( GLOBAL PROPERTY USE_FOLDERS ON)
endif()

# Optional second argument is the source name. Otherwise, same as the example name is assume
function(CreateExample name)
        set(SRC_NAME ${name}.cpp)
        if(${ARGC} GREATER 1)
                set(SRC_NAME ${ARGV1})
        endif()
        MOVETK_LOG(VERBOSE "Created example ${name}")
        add_executable(${name} ${SRC_NAME})
        target_compile_definitions(${name} PRIVATE -DCGAL_BACKEND_ENABLED=${CGAL_BACKEND_ENABLED})
        target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)
        target_link_libraries(${name}
                PRIVATE
                c2d::movetk
                )
        if(CMAKE_GENERATOR MATCHES ".*Visual Studio.*")
                set_target_properties(${name} PROPERTIES FOLDER "Examples")
        endif()
endfunction()

#IO examples
# CreateExample(rawprobe2trajectories)
# CreateExample(probe2trajectories)
# CreateExample(geolife_count_probes)
# CreateExample(geolife_distance_heading)
# CreateExample(read_trajectories)
# CreateExample(write_geojson)
#
# # Segmentation examples
# CreateExample(TrajectorySegmentationLocation segment_by_meb.cpp)
# CreateExample(TrajectorySegmentationSpeed segment_by_speed.cpp)
# CreateExample(TrajectorySegmentationHeading segment_by_heading.cpp)
# CreateExample(TrajectorySegmentationVelocity segment_by_velocity.cpp)
# CreateExample(MonotoneSegmentation monotone_segmentation.cpp)
#
# # Creation examples
# CreateExample(make_point)
# CreateExample(make_min_sphere)
# CreateExample(make_segment)
# CreateExample(make_line)
# CreateExample(make_polygon)
# CreateExample(make_sphere)
#
# # Computations
# CreateExample(compute_squared_distance)
#
# # Simplification
# CreateExample(douglas_peucker_simplification)
# CreateExample(simplification)
#
# CreateExample(directed_graph)
# CreateExample(model_based_segmentation)
# CreateExample(outlier_detection)
# CreateExample(trajectory_sampler)
#
# # Interpolation examples
# CreateExample(kinematic_interpolation)
# CreateExample(linear_interpolation)
# CreateExample(interpolation)
#
# # Statistics example
# CreateExample(compute_statistics)

CreateExample(clustering)


if (MOVETK_WITH_CGAL_BACKEND)
    # CreateExample(TrajectoriesPolygonIntersection trajectories_polygon_intersection.cpp)
    # CreateExample(curves_intersections)
endif()
