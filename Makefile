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
	LDLIBS	= -lglfw -lGL -ldl
endif

.cpp.o:
		${CXX} ${CXXFLAGS} -c $< -o ${<:.cpp=.o} -I ${INCS} -I ${GLAD_INC} -I ${GLFW_INC} -I ${KHR_INC} -I ${GLM_INC} -I ${CAM_INC} -I ${AUDIO_INC}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@ -I ${INCS} -I ${GLAD_INC} -I ${GLFW_INC} -I ${KHR_INC} -I ${GLM_INC} -I ${CAM_INC} -I ${AUDIO_INC}

${NAME}: ${OBJS}
	${CXX} ${OBJS} ${STATIC_LIBS} ${CXXFLAGS} ${LDFLAGS} ${LDLIBS} -o ${NAME}

all: ${NAME}

clean:
		${RM} ${OBJS}

fclean: clean
	${RM} ${NAME}

re: fclean all

.PHONY: all clean fclean re
