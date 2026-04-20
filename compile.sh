# cmake src/core/build && cmake --build src/core/build
# mv -vf src/core/build/libmynic.so dist

# cmake src/adapters/build && cmake --build src/adapters/build
# mv -vf src/adapters/build/libmynic_adapters.so dist

cmake src/cli/build && cmake --build src/cli/build
mv -vf src/cli/build/mynic_cli dist