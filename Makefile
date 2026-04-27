SRCS_MAIN	=	src/main.cpp \
				src/game.cpp

SRCS_SDL2	=	src/sdl2Game.cpp \
				src/game.cpp

OBJS_MAIN	= ${SRCS_MAIN:.cpp=.o}
OBJS_SDL2	= ${SRCS_SDL2:.cpp=.o}

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
LIB_SDL2	= lib_sdl2.so

CC      = cc
CXX     = c++
RM		= rm -rf
CXXFLAGS= -Wall -Wextra -Werror -g -std=c++11 -fPIC

# SDL2 flags
SDL2_CFLAGS = -I$(HOME)/Desktop/nibbler/local/include/SDL2
SDL2_LIBS = -L$(HOME)/Desktop/nibbler/local/lib -lSDL2
SDL2_IMAGE_CFLAGS = -I$(HOME)/Desktop/nibbler/local/include
SDL2_IMAGE_LIBS = -L$(HOME)/Desktop/nibbler/local/lib -lSDL2_image
SDL2_TTF_CFLAGS = -I$(HOME)/Desktop/nibbler/local/include
SDL2_TTF_LIBS = -L$(HOME)/Desktop/nibbler/local/lib -lSDL2_ttf

# Main link flags
LDFLAGS = -ldl

# Compilation rules
%.o: %.cpp
	${CXX} ${CXXFLAGS} -c $< -o $@ -I ${INCS} -I ${GLAD_INC} -I ${KHR_INC} -I ${GLFW_INC} -I ${GLM_INC} -I ${CAM_INC} -I ${AUDIO_INC} -I ${SDL_INC} -I$(HOME)/Desktop/nibbler/local/include ${SDL2_IMAGE_CFLAGS}

# Main executable
${NAME}: ${OBJS_MAIN}
	${CXX} ${OBJS_MAIN} ${CXXFLAGS} ${LDFLAGS} -o ${NAME}

# SDL2 library
${LIB_SDL2}: ${OBJS_SDL2}
	${CXX} ${OBJS_SDL2} ${CXXFLAGS} -shared -fPIC ${SDL2_CFLAGS} ${SDL2_LIBS} ${SDL2_IMAGE_LIBS} ${SDL2_TTF_LIBS} -o ${LIB_SDL2}

.DEFAULT_GOAL := all

all: ${NAME} ${LIB_SDL2}

clean:
	${RM} ${OBJS_MAIN} ${OBJS_SDL2}

fclean: clean
	${RM} ${NAME} ${LIB_SDL2}

re: fclean all

.PHONY: all clean fclean re