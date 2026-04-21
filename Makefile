SRCS	=	src/main.cpp \
			src/glad.c \
			src/game.cpp \

OBJS	= ${SRCS:.cpp=.o}
OBJS	:= ${OBJS:.c=.o}
INCS	= includes
GLAD_INC = glad
KHR_INC = khr
GLFW_INC = glfw-3.4/include
GLM_INC = glm
CAM_INC = Camera.hpp
AUDIO_INC = miniaudio
SDL_DIR = extern/SDL2
SDL_INC = $(SDL_DIR)/include
SDL_LIB_DIRS = -L$(SDL_DIR)/lib -L$(SDL_DIR)/build/install/lib
NAME	= nibbler
CC      = cc
CXX     = c++
RM		= rm -rf
CFLAGS  = -Wall -Wextra -Werror -g -std=c11 -DGL_SILENCE_DEPRECATION
CXXFLAGS= -Wall -Wextra -Werror -g -std=c++11 -DGL_SILENCE_DEPRECATION

STATIC_LIBS = miniaudio/libminiaudio_all.a

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	HOMEBREW_LIB64 := /usr/local/lib
	HOMEBREW_LIBARM := /opt/homebrew/lib
	LDLIBS = -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	ifneq (,$(wildcard $(HOMEBREW_LIB64)/libglfw*))
		LDLIBS := -L$(HOMEBREW_LIB64) $(LDLIBS)
	endif
	ifneq (,$(wildcard $(HOMEBREW_LIBARM)/libglfw*))
		LDLIBS := -L$(HOMEBREW_LIBARM) $(LDLIBS)
	endif
else
	LDFLAGS = -no-pie
	LDLIBS	= -lglfw -lGL -ldl
endif

LDLIBS += $(SDL_LIB_DIRS) -lSDL2

.cpp.o:
	${CXX} ${CXXFLAGS} -c $< -o ${<:.cpp=.o} -I ${INCS} -I ${GLAD_INC} -I ${GLFW_INC} -I ${KHR_INC} -I ${GLM_INC} -I ${CAM_INC} -I ${AUDIO_INC} -I ${SDL_INC}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@ -I ${INCS} -I ${GLAD_INC} -I ${GLFW_INC} -I ${KHR_INC} -I ${GLM_INC} -I ${CAM_INC} -I ${AUDIO_INC} -I ${SDL_INC}

.PHONY: submodules
submodules:
	git submodule update --init --recursive
	if [ -f "$(SDL_DIR)/CMakeLists.txt" ]; then \
		mkdir -p $(SDL_DIR)/build && cd $(SDL_DIR)/build && cmake .. -DSDL_STATIC=ON -DCMAKE_INSTALL_PREFIX=$$(pwd)/install && make -j$$(nproc) && make install; \
	fi
	if [ -f "$(SDL_DIR)/autogen.sh" ]; then \
		cd $(SDL_DIR) && ./autogen.sh && ./configure --prefix=$$(pwd)/build/install && make -j$$(nproc) && make install; \
	fi

${NAME}: submodules ${OBJS}
	${CXX} ${OBJS} ${STATIC_LIBS} ${CXXFLAGS} ${LDFLAGS} ${LDLIBS} -o ${NAME}

all: ${NAME}

clean:
		${RM} ${OBJS}

fclean: clean
	${RM} ${NAME}

re: fclean all

.PHONY: all clean fclean re