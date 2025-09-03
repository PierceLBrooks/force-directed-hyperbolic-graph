#!/bin/sh
rm -rf build
mkdir build
cd build
cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_POLICY_DEFAULT_CMP0074=NEW -DCMAKE_FIND_DEBUG_MODE=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_DEBUG_POSTFIX="" ..
cmake --build . --target install --config Release
cd ..

