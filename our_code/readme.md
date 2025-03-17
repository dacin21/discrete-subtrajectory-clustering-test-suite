# Build
There is a container with the required packages.
```
cd podman
./podman.sh
```
Then you can build with meson.
```
mkdir build
cd build
meson setup
meson compile
meson test
```

