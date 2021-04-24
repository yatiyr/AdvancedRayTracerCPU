# HOW TO RUN THIS RAY TRACER -- !IMPORTANT!
This project uses CMake. In project root we use these commands

```console
mkdir build
cd build
cmake ..
./AdvancedRayTracer path/to/xml/file
```

One important note is that the paths we provide to the program sees
project root as base. I coded it like this. In assets/scenes folder I
have the xml files that I've created.

so In order to render bunny.xml for example, we say;

./AdvancedRayTracer assets/scenes/bunny.xml

output file will be written in build/outputs/bunny.png

Special thanks to our friend akif uslu for sharing his scenes.
