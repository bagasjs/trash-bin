CC := clang
CFLAGS := -Wall -Wextra -pedantic -Isrc -Isrc/vendors/glfw/include -D_CRT_SECURE_NO_WARNINGS -g -fsanitize=address
LFLAGS := -luser32 -lgdi32 -lshell32

main.exe: ./build/stb_image.o ./build/glfw_unity.o ./src/vendors/glad.c ./src/cutils.c ./src/graphic.c ./src/main.c 
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

./build/glfw_unity.o: ./src/glfw_unity.c
	$(CC) $(CFLAGS) -c -o $@ $^

./build/stb_image.o: ./src/vendors/stb_image.h
	$(CC) $(CFLAGS) -DSTB_IMAGE_IMPLEMENTATION -x c -c -o $@ $^
