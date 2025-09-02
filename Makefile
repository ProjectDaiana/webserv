# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ltreser <ltreser@student.42berlin.de>      +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/08/24 16:25:15 by ltreser           #+#    #+#              #
#    Updated: 2025/09/01 18:54:06 by ltreser          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# **************************************************************************** #
#                                    CONFIG                                    #
# **************************************************************************** #

NAME        := webserv
CXX         := c++
CXXFLAGS    := -Wall -Wextra -Werror -std=c++11

SRC         := main.cpp memory.cpp init.cpp server.cpp client.cpp atoi.cpp server_utils.cpp
OBJ         := $(SRC:.cpp=.o)

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

%.o: %.cpp
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

