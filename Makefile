CC= c++
CFLAGS= -Wall -Wextra -Werror -std=c++98
NAME= ircserv
SRCS= main.cpp \
	  Server.cpp \
	  ServerUtils.cpp \
	  Reply.cpp \
	  Client.cpp \
	  Channel.cpp \
	  Commands.cpp \
	  CommandsUtils.cpp



OBJDIR= obj

OBJS=  $(addprefix $(OBJDIR)/, $(SRCS:.cpp=.o))

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

debug: 
	$(CC) $(CFLAGS) -DDEBUG=1 $(SRCS) -o $(NAME)

.PHONY: all clean fclean re debug
