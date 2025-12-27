PROG 		= webserv
CXX 		= c++
INCLUDES	:= -Iinclude
CXXFLAGS 	= $(INCLUDES) -g -std=c++98 #-Wall -Wextra -Werror 

SRCS		= ./main.cpp ./srcs/Pollable.cpp ./srcs/Server.cpp ./srcs/Webserv.cpp ./srcs/Connection.cpp ./srcs/Request.cpp ./srcs/Response.cpp ./srcs/Utils.cpp ./srcs/Cgi.cpp ./srcs/Handle.cpp
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