file(GLOB_RECURSE FILES *.cpp)
add_executable(combix_calc
  ${FILES}
  )

target_link_libraries(combix_calc
  coco)
