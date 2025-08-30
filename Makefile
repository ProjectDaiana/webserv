NAME = webserv
CXX = c++
# CXXFLAGS = -Wall -Wextra -Werror -std=c++98
CFLAGS = g++ -Wall -Wextra -Werror
SRC = main.cpp server.cpp client.cpp

OBJ = $(SRC:.cpp=.o)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

all: $(NAME)

clean: 
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re++