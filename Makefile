CC= c++
CFLAGS= -Wall -Wextra -Werror -std=c++98
NAME= ircserv
SRCS= main.cpp \
	  Server.cpp \

OBJS= $(SRCS:.cpp=.o)
all: $(NAME)
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
clean:
	rm -f $(OBJS)
fclean: clean
	rm -f $(NAME)
re: fclean all
.PHONY: all clean fclean re
