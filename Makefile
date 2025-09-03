# **************************************************************************** #
#                                    CONFIG                                    #
# **************************************************************************** #

NAME        := webserv
CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -std=c++98

# Source files
SRC         := atoi.cpp main.cpp pollHandler.cpp \
               Client.cpp init.cpp \
               memory.cpp Server.cpp server_utils.cpp

# Object files
OBJ         := $(SRC:.cpp=.o)

# Headers (tracked for dependencies)
HEADERS     := webserv.hpp server.hpp pollHandler.hpp

# Colors
GREEN       := \033[1;32m
BLUE        := \033[1;34m
RESET       := \033[0m

# **************************************************************************** #
#                                   RULES                                      #
# **************************************************************************** #

all: $(NAME)

$(NAME): $(OBJ)
	@echo "$(BLUE)✨ Compiling magic...$(RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)
	@echo "$(GREEN)✅ Build complete: $(NAME)$(RESET)"

# Compile each .cpp to .o, considering headers as dependencies
%.o: %.cpp $(HEADERS)
	@echo "$(BLUE)🔧 Building $< ...$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJ)
	@echo "$(BLUE)🧹 Object files cleaned.$(RESET)"

fclean: clean
	@rm -f $(NAME)
	@echo "$(BLUE)💥 Removed executable: $(NAME)$(RESET)"

re: fclean all

.PHONY: all clean fclean re

