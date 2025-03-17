# !/bin/bash
set -ev
mkdir -p build
cd build
cmake -S .. -B . -DMOVETK_BUILD_TESTS=OFF -DMOVETK_BUILD_EXAMPLES=ON -DMOVETK_WITH_CGAL_BACKEND=ON -DMOVETK_WITH_BOOST_BACKEND=OFF
cmake --build .
