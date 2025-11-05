PROG 		= webserv
CXX 		= c++
INCLUDES	:= -Iinclude
CXXFLAGS 	= $(INCLUDES) -Wall -Wextra -Werror -g -std=c++98

SRCS		= ./srcs/main.cpp ./srcs/Server.cpp ./srcs/Webserv.cpp

OBJS		= ${SRCS:.cpp=.o}

all: 		${PROG}

${PROG}:	${OBJS}
		${CXX} ${CXXFLAGS} ${OBJS} -o ${PROG} 

clean:
		rm -f ${OBJS}

fclean:
		rm -f ${PROG} ${OBJS}

re:			fclean all

.PHONY: all clean fclean re