#
# Gerson Carvalho
# Particle Life v0.1
#

# Compiler settings
CXX := g++
CXXFLAGS := -Wall -Werror -Wextra -g -O3 -march=native -ffast-math \
            -ftree-vectorize -fopenmp -std=c++20

# Project settings
NAME := particlelife
BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj

# Debugger
DEBUG := valgrind --leak-check=full --show-leak-kinds=all \
         --track-origins=yes -s

# Directories
SRC_DIR := src
LIB_DIR := lib
CLASS_DIR := classes
INCLUDE_DIR := include

# Project sources
CLASS_SRC := $(CLASS_DIR)/Object.cpp \
			 $(CLASS_DIR)/physics_server/PhysicsServer.cpp \
			 $(CLASS_DIR)/render_server/RenderServer.cpp \
			 $(CLASS_DIR)/shape2d/Shape2D.cpp \
			 $(CLASS_DIR)/shape2d/circle_shape2d/CircleShape2D.cpp \
			 $(CLASS_DIR)/shape2d/rectangle_shape2d/RectangleShape2D.cpp \
			 $(CLASS_DIR)/node/Node.cpp \
			 $(CLASS_DIR)/node/canvas_item/CanvasItem.cpp \
			 $(CLASS_DIR)/node/canvas_item/control/Control.cpp \
			 $(CLASS_DIR)/node/canvas_item/control/button/Button.cpp \
			 $(CLASS_DIR)/node/canvas_item/node2d/Node2D.cpp \
			 $(CLASS_DIR)/node/canvas_item/node2d/particle/Particle.cpp

SRC := $(SRC_DIR)/main.cpp

# Project headers
HEADERS := $(INCLUDE_DIR)/json.hpp \
		   $(INCLUDE_DIR)/raylib-cpp.hpp \
		   $(INCLUDE_DIR)/raylib.h \
		   $(INCLUDE_DIR)/raymath.h \
		   $(INCLUDE_DIR)/rlgl.h \
		   $(CLASS_DIR)/Object.hpp \
		   $(CLASS_DIR)/physics_server/PhysicsServer.hpp \
		   $(CLASS_DIR)/render_server/RenderServer.hpp \
		   $(CLASS_DIR)/shape2d/Shape2D.hpp \
		   $(CLASS_DIR)/shape2d/circle_shape2d/CircleShape2D.hpp \
		   $(CLASS_DIR)/shape2d/rectangle_shape2d/RectangleShape2D.hpp \
		   $(CLASS_DIR)/node/Node.hpp \
		   $(CLASS_DIR)/node/canvas_item/CanvasItem.hpp \
		   $(CLASS_DIR)/node/canvas_item/control/Control.hpp \
		   $(CLASS_DIR)/node/canvas_item/control/button/Button.hpp \
		   $(CLASS_DIR)/node/canvas_item/node2d/Node2D.hpp \
		   $(CLASS_DIR)/node/canvas_item/node2d/particle/Particle.hpp

# Include paths
INC_FLAGS := -I$(CLASS_DIR) \
             -I$(INCLUDE_DIR)

# Libraries
LIBS := $(LIB_DIR)/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11 -latomic

# Object files with build directory
CLASS_OBJ := $(CLASS_SRC:%.cpp=$(OBJ_DIR)/%.o)
OBJS := $(SRC:%.cpp=$(OBJ_DIR)/%.o)
ALL_OBJS := $(CLASS_OBJ) $(OBJS)

# Colors for output
RED := \033[0;31m
GREEN := \033[0;32m
YELLOW := \033[0;33m
NC := \033[0m

# Default target
.DEFAULT_GOAL := all

# Main targets
all: $(NAME)
	@echo "$(GREEN)✓ Build complete: $(NAME)$(NC)"

$(NAME): $(ALL_OBJS)
	@echo "$(YELLOW)Linking $(NAME)...$(NC)"
	@$(CXX) $(CXXFLAGS) $(ALL_OBJS) -o $@ $(LIBS)

# Compile project sources
$(OBJ_DIR)/%.o: %.cpp $(HEADERS)
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@mkdir -p $(dir $@)
	@$(CXX) $(CXXFLAGS) $(INC_FLAGS) -c $< -o $@

# Clean targets
clean:
	@echo "$(RED)Cleaning object files...$(NC)"
	@rm -rf $(BUILD_DIR)

fclean: clean
	@echo "$(RED)Cleaning executable...$(NC)"
	@rm -f $(NAME)

re: fclean all

# Utility targets
run: all
	@echo "$(GREEN)Running $(NAME)...$(NC)"
	@./$(NAME)

debug: all
	@echo "$(GREEN)Running $(NAME) with valgrind...$(NC)"
	@$(DEBUG) ./$(NAME)

# Show compilation commands (for debugging makefile)
verbose: CXXFLAGS += -v
verbose: re

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the project (default)"
	@echo "  clean   - Remove object files"
	@echo "  fclean  - Remove object files and executable"
	@echo "  re      - Rebuild everything"
	@echo "  run     - Build and run the project"
	@echo "  debug   - Run with valgrind"
	@echo "  verbose - Build with verbose compiler output"
	@echo "  help    - Show this help message"

.PHONY: all clean fclean re run debug verbose
