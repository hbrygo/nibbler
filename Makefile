SRCS_MAIN	=	src/main.cpp \
				src/game.cpp

SRCS_SDL3	=	src/sdl3Game.cpp \
				src/game.cpp

OBJS_MAIN	= ${SRCS_MAIN:.cpp=.o}
OBJS_SDL3	= ${SRCS_SDL3:.cpp=.o}

CFLAGS += -I$(HOME)/Desktop/nibbler/local/include
LDFLAGS += -L$(HOME)/Desktop/nibbler/local/lib
LDLIBS += -lSDL2_image

INCS	= includes
GLAD_INC = glad
KHR_INC = khr
GLFW_INC = glfw-3.4/include
GLM_INC = glm
CAM_INC = Camera.hpp
AUDIO_INC = miniaudio
SDL_DIR = extern/SDL2
SDL_INC = $(SDL_DIR)/include
NAME	= nibbler
LIB_SDL3	= lib_sdl3.so

CC      = cc
CXX     = c++
RM		= rm -rf
CXXFLAGS= -Wall -Wextra -Werror -g -std=c++11 -fPIC

LDFLAGS = -ldl

%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@ -I ${INCS} -I ${GLAD_INC} -I ${KHR_INC} -I ${GLFW_INC} -I ${GLM_INC} -I ${CAM_INC} -I ${AUDIO_INC}

sdl3_build:
	@if [ ! -d sdl3 ]; then \
		echo "Téléchargement de SDL3 3.4.4..."; \
		curl -L -o SDL3.zip https://www.libsdl.org/release/SDL3-3.4.4.zip; \
		unzip -q SDL3.zip; \
		mv SDL3-3.4.4 sdl3; \
		rm SDL3.zip; \
	fi
	@if [ ! -d sdl3/build ]; then \
		mkdir -p sdl3/build; \
		cd sdl3/build && cmake .. -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XTEST=OFF && make -j4; \
		cd ../../..; \
	fi

${NAME}: sdl3_build ${OBJS_MAIN}
	${CXX} ${OBJS_MAIN} ${CXXFLAGS} ${LDFLAGS} -o ${NAME}

${LIB_SDL3}: sdl3_build ${OBJS_SDL3}
	${CXX} ${OBJS_SDL3} ${CXXFLAGS} -shared -fPIC -ldl -L./sdl3/build -L./sdl3_image/build -o ${LIB_SDL3}

.DEFAULT_GOAL := all

all: ${NAME} ${LIB_SDL3}

clean:
	${RM} ${OBJS_MAIN} ${OBJS_SDL3}

fclean: clean
	${RM} ${NAME} ${LIB_SDL3}
	${RM} sdl3

re: fclean all

.PHONY: all clean fclean re sdl3_build