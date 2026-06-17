SRCS_MAIN	=	src/main.cpp \
				src/game.cpp

SRCS_SDL3	=	src/sdl3Game.cpp \
				src/game.cpp

SRCS_GL		=	src/glGame.cpp \
				src/game.cpp

SRCS_SFML	=	src/sfmlGame.cpp \
				src/game.cpp

OBJS_MAIN	= ${SRCS_MAIN:.cpp=.o}
OBJS_SDL3	= ${SRCS_SDL3:.cpp=.o}
OBJS_GL		= ${SRCS_GL:.cpp=.o}
OBJS_SFML	= ${SRCS_SFML:.cpp=.o}

CFLAGS += -I$(HOME)/Desktop/nibbler/local/include
LDFLAGS += -L$(HOME)/Desktop/nibbler/local/lib
LDLIBS += -lSDL2_image

INCS	= includes
GLFW_INC = glfw-3.4/include
GLM_INC = glm
CAM_INC = Camera.hpp
AUDIO_INC = miniaudio
SFML_INC = sfml-2.5.1/include
SFML_LIB = sfml-2.5.1/build/lib
SDL_DIR = extern/SDL2
SDL_INC = $(SDL_DIR)/include
NAME	= nibbler
LIB_SDL3	= lib_sdl3.so
LIB_GL		= lib_gl.so
SFML_OUTPUT_NAME ?= lib_sfml.so
SFML_INSTALL_DIR ?= .
LIB_SFML	= ${SFML_OUTPUT_NAME}

CC      = cc
CXX     = c++
RM		= rm -rf
CXXFLAGS= -Wall -Wextra -Werror -g -std=c++11 -fPIC

LDFLAGS = -ldl

%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@ -I ${INCS} -I ${GLFW_INC} -I ${GLM_INC} -I ${CAM_INC} -I ${AUDIO_INC} -I ${SFML_INC}

miniaudio_build:
	@if [ ! -d miniaudio ]; then \
		echo "Téléchargement de miniaudio..."; \
		git clone https://github.com/mackron/miniaudio.git; \
	fi
	@set -e; \
	cmake -S miniaudio -B miniaudio/build -DMINIAUDIO_BUILD_EXAMPLES=OFF -DMINIAUDIO_BUILD_TESTS=OFF -DMINIAUDIO_INSTALL=OFF; \
	cmake --build miniaudio/build --target miniaudio -j4; \
	cp -f miniaudio/build/libminiaudio.a miniaudio/libminiaudio_all.a

sdl3_build:
	@if [ ! -d sdl3 ]; then \
		echo "Téléchargement de SDL3 3.4.4..."; \
		curl -L -o SDL3.zip https://www.libsdl.org/release/SDL3-3.4.4.zip; \
		unzip -q SDL3.zip; \
		mv SDL3-3.4.4 sdl3; \
		rm SDL3.zip; \
	fi
	@set -e; \
	mkdir -p sdl3/build; \
	cd sdl3/build && cmake .. -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XTEST=OFF && make -j4 && cd ../..;

glfw_build:
	@if [ ! -d glfw-3.4 ]; then \
		echo "Téléchargement de GLFW 3.4..."; \
		curl -L -o glfw-3.4.zip https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip; \
		unzip -q glfw-3.4.zip; \
		rm glfw-3.4.zip; \
	fi
	@set -e; \
	mkdir -p glfw-3.4/build; \
	cd glfw-3.4/build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF && make && cd ../..;

sfml_build:
	@if [ ! -d sfml-2.5.1 ]; then \
		echo "Téléchargement de SFML 2.5.1..."; \
		curl -L -o sfml-2.5.1.tar.gz https://github.com/SFML/SFML/archive/2.5.1.tar.gz; \
		tar -xzf sfml-2.5.1.tar.gz; \
		mv SFML-2.5.1 sfml-2.5.1; \
		rm sfml-2.5.1.tar.gz; \
	fi
	@if ! pkg-config --exists libudev; then \
		echo "Erreur: libudev (dev) est requis pour compiler SFML sous Linux."; \
		echo "Installez-le puis relancez la compilation."; \
		echo "Debian/Ubuntu: sudo apt install libudev-dev"; \
		exit 1; \
	fi
	@if ! pkg-config --exists freetype2; then \
		echo "Erreur: Freetype (dev) est requis pour compiler SFML Graphics."; \
		echo "Installez-le puis relancez la compilation."; \
		echo "Debian/Ubuntu: sudo apt install libfreetype6-dev"; \
		exit 1; \
	fi
	@set -e; \
	mkdir -p sfml-2.5.1/build; \
	cd sfml-2.5.1/build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DSFML_BUILD_GRAPHICS=ON -DSFML_BUILD_WINDOW=ON -DSFML_BUILD_NETWORK=OFF -DSFML_BUILD_AUDIO=OFF -DSFML_BUILD_EXAMPLES=OFF && make -j4 && cd ../..;

${NAME}: miniaudio_build sdl3_build glfw_build sfml_build ${OBJS_MAIN} miniaudio/libminiaudio_all.a
	${CXX} ${OBJS_MAIN} miniaudio/libminiaudio_all.a ${CXXFLAGS} ${LDFLAGS} -rdynamic -no-pie -o ${NAME}

${LIB_SDL3}: sdl3_build ${OBJS_SDL3}
	${CXX} ${OBJS_SDL3} ${CXXFLAGS} -shared -fPIC -ldl -Wl,--allow-shlib-undefined -L./sdl3/build -L./sdl3_image/build -o ${LIB_SDL3}

${LIB_GL}: glfw_build ${OBJS_GL}
	${CXX} ${OBJS_GL} ${CXXFLAGS} -shared -fPIC -ldl -Wl,--allow-shlib-undefined -L./glfw-3.4/build -o ${LIB_GL}

${LIB_SFML}: sfml_build ${OBJS_SFML}
	${CXX} ${OBJS_SFML} ${CXXFLAGS} -shared -fPIC -ldl -Wl,--allow-shlib-undefined -L./${SFML_LIB} -Wl,-z,origin -Wl,-rpath,\$$ORIGIN/${SFML_LIB} -lsfml-graphics -lsfml-window -lsfml-system -o ${LIB_SFML}
	@mkdir -p ${SFML_INSTALL_DIR}
	@dst="${SFML_INSTALL_DIR}/$(notdir ${LIB_SFML})"; \
	if [ "$$(realpath -m "${LIB_SFML}")" != "$$(realpath -m "$$dst")" ]; then \
		cp -f "${LIB_SFML}" "$$dst"; \
	fi

install_sfml: ${LIB_SFML}
	@mkdir -p ${SFML_INSTALL_DIR}
	@dst="${SFML_INSTALL_DIR}/$(notdir ${LIB_SFML})"; \
	if [ "$$(realpath -m "${LIB_SFML}")" != "$$(realpath -m "$$dst")" ]; then \
		cp -f "${LIB_SFML}" "$$dst"; \
	fi

.DEFAULT_GOAL := all

all: ${NAME} ${LIB_SDL3} ${LIB_GL} ${LIB_SFML}

clean:
	${RM} ${OBJS_MAIN} ${OBJS_SDL3} ${OBJS_GL} ${OBJS_SFML}

fclean: clean
	${RM} ${NAME} ${LIB_SDL3} ${LIB_GL} ${LIB_SFML}
	${RM} sdl3
	${RM} glfw-3.4
	${RM} sfml-2.5.1
	${RM} miniaudio

re: fclean all

.PHONY: all clean fclean re sdl3_build glfw_build sfml_build install_sfml miniaudio_build
