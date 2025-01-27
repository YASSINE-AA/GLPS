LIBS = -lGL -lEGL -lm -lwayland-client -lwayland-server -lwayland-cursor -lwayland-egl -lfreetype -lxkbcommon
CFLAGS =  -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -g3 -fsanitize=address,undefined -I/include -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libdrm -I/usr/include/libpng12  -I/usr/include

test :
	gcc src/xdg/xdg-shell.c src/utils/logger/pico_logger.c src/utils/wayland_utils.c  src/xdg/wlr-data-control-unstable-v1.c src/xdg/xdg-decorations.c src/glad/glad.c  src/glps_opengl.c src/glps_window_manager.c test.c ${CFLAGS} -o build/test ${LIBS}

run:
	./build/test
clean:
	rm -f *.o *~
