# glslc ScanlineRender.comp -o ScanlineRender.spv -O
# clang *.c -lm -lSDL3 -o main -fuse-ld=lld -DDEBUG=false -DWIDTH=640 -DHEIGHT=480 -Ofast -flto -march=native -mtune=native -O3 -ffast-math
clang -g *.c -lSDL3 -lm -o main -DDEBUG=false -DWIDTH=640 -DHEIGHT=480
