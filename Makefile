SRCS	=	src/main.cpp \
			src/Three.cpp \
			src/game.cpp

OBJS	= ${SRCS:.cpp=.o}
OBJS	:= ${OBJS:.c=.o}
INCS	= includes
GLFW_INC = glfw-3.4/include
GLM_INC = glm
CAM_INC = Camera.hpp
AUDIO_INC = miniaudio
NAME	= nibbler
CC      = cc
CXX     = c++
RM		= rm -rf
CFLAGS  = -Wall -Wextra -Werror -g -std=c11 -DGL_SILENCE_DEPRECATION
CXXFLAGS= -Wall -Wextra -Werror -g -std=c++11 -DGL_SILENCE_DEPRECATION

STATIC_LIBS = miniaudio/libminiaudio_all.a

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	# macOS: use frameworks for OpenGL and Cocoa, GLFW should be installed via Homebrew
	# prefer Homebrew locations for libglfw (Intel and Apple Silicon)
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
	LDLIBS	= -ldl
endif

.cpp.o:
		${CXX} ${CXXFLAGS} -c $< -o ${<:.cpp=.o} -I ${INCS} -I ${GLFW_INC} -I ${GLM_INC} -I ${CAM_INC} -I ${AUDIO_INC}

${NAME}: glfw_build ${OBJS}
	${CXX} ${OBJS} ${STATIC_LIBS} ${CXXFLAGS} ${LDFLAGS} ${LDLIBS} -o ${NAME}

glfw_build:
	@if [ ! -d glfw-3.4 ]; then \
		echo "Téléchargement de GLFW 3.4..."; \
		curl -L -o glfw-3.4.zip https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip; \
		unzip -q glfw-3.4.zip; \
		rm glfw-3.4.zip; \
	fi
	@if [ ! -d glfw-3.4/build ]; then \
		mkdir -p glfw-3.4/build; \
		cd glfw-3.4/build && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF && make; \
	fi

all: ${NAME}

clean:
		${RM} ${OBJS}

fclean: clean
	${RM} ${NAME}
	${RM} glfw-3.4

re: fclean all

.PHONY: all clean fclean re glfw_build
