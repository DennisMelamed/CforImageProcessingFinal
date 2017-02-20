# C++ For Image Processing: Coursework

## Prerequisites

* cmake (>= 3.0)
* wxGTK3-devel (for building)
* wxGTK3 (for runtime)

## Building

The codebase uses cmake build system. Its recommended to do an
out-of-source build, i.e.

```
cd dir_of_the_cloned_repo
mkdir build
cd build
cmake ../
make
```

If debug version is required please append ```-DCMAKE_BUILD_TYPE=Debug```
to the cmake invocation.

Please note that on CentOS 7 you will have to use ```cmake3``` command.
On Ubuntu ```cmake``` works fine.

## Features Present Currently
* Loading Images
* Pixel Value Rescaling
* Pixel Value Shifting

* Convolution of various types including:
 * Image Smoothing
 * Edge Detection

* Various Order Statistics Filters
* Point Processing Functions
* Region of Interest Selection
* A Menu System with undo functinality

## To Be Implemented:
* Thresholding
* Histogram Equilization
