PROG 	= webserv
SRCS 	= ./srcs/main.cpp ./srcs/Server.cpp ./srcs/Webserv.cpp
OBJS 	= ${SRCS:.cpp=.o}
CXX 		= c++ 
CXXFLAGS 	= -Wall -Wextra -Werror -g -std=c++98


all: 		${PROG}

${PROG}:	${OBJS}
		${CXX} ${CXXFLAGS} ${OBJS} -o ${PROG} 

%.o: %.c
		$(CXX) ${CXXFLAGS} -cpp $< -o $@

clean:
		rm -f ${OBJS}

fclean:
		rm -f ${PROG} ${OBJS}

re:			fclean all

.PHONY: all clean fclean re