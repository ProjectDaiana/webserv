# **************************************************************************** #
#                                    CONFIG                                    #
# **************************************************************************** #

NAME        := webserv
CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -std=c++98 -I. -g3 -Ihelper

# Source files
SRC         := helper/ft_atoi.cpp main.cpp pollHandler.cpp \
               Client.cpp init.cpp Request.cpp CGI.cpp \
               helper/memory.cpp Server.cpp helper/server_utils.cpp \
			   methods_post.cpp response.cpp response_utils.cpp \
			   methods_delete.cpp http_errors.cpp methods_get.cpp \
	           config.cpp config_lexer.cpp config_parser_init.cpp \
	           config_parser_utils.cpp config_parser.cpp \
               helper/multipart_utils.cpp helper/ft_memcpy.cpp \
			   helper/ft_isspace.cpp helper/ft_strcmp.cpp helper/ft_strchr.cpp \
			   helper/ft_strlen.cpp server_errors.cpp
		


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

