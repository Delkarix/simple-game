# NOTE: THIS ONLY WORKS IF USE_SDL3_SEMANTICS IS DISABLED IN main.h
clang *.c -Ofast -flto -lSDL3 -lOpenCL -o main