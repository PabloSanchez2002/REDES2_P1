NAME1 			= client
NAME2 			= server
NAMEEXTRA 		= wrapper


#SRC_DIR
SRC_DIR		= src/
#OBJ_DIR
OBJ_DIR		= obj/
#COLORS

DEFAULT		=	\033[0;39m
RED			=	\033[0;91m
GREEN		=	\033[1;92m
YELLOW		=	\033[1;93m
CYAN		=	\033[0;96m
PURPLE		=	\033[0;95m

#COMPILATION
CC			= gcc
CFLAGS		= -c

#OTHERS
RM			= rm -f 

#SRCS
SRCCL 		= 	$(addprefix $(SRC_DIR), $(addsuffix .c, $(NAME1)))
SRCSV 		= 	$(addprefix $(SRC_DIR), $(addsuffix .c, $(NAME2)))
SRCEXTRA 	= 	$(addprefix $(SRC_DIR), $(addsuffix .c, $(NAMEEXTRA)))


#OBJS
OBJCL 			= 	$(addprefix $(OBJ_DIR), $(addsuffix .o, $(NAME1)))
OBJSV 			= 	$(addprefix $(OBJ_DIR), $(addsuffix .o, $(NAME2)))
OBJEXTRA 		= 	$(addprefix $(OBJ_DIR), $(addsuffix .o, $(NAMEEXTRA)))

begin:
	@mkdir -p obj
	@make -s all
all: $(NAME1) $(NAME2)

$(NAME1): $(OBJCL) $(OBJEXTRA)
	echo "##############################"
	echo "$(CYAN)       Generating $@ $(DEFAULT)"
	$(CC) $(OBJCL) $(OBJEXTRA) -o $(NAME1)

$(NAME2): $(OBJSV) $(OBJEXTRA)
	echo "##############################"
	echo "$(CYAN)       Generating $@ $(DEFAULT)"
	$(CC) $(OBJSV) $(OBJEXTRA) -o $(NAME2)

$(OBJCL): $(SRCCL)
	echo "##############################"
	echo "$(PURPLE)       Generating $@ $(DEFAULT)"
	$(CC) $(CFLAGS) $< 
	@mv $(NAME1).o obj/

$(OBJSV): $(SRCSV)
	echo "##############################"
	echo "$(PURPLE)       Generating $@ $(DEFAULT)"
	$(CC) $(CFLAGS) $< 
	@mv $(NAME2).o obj/

$(OBJEXTRA): $(SRCEXTRA)
	echo "##############################"
	echo "$(PURPLE)       Generating $@ $(DEFAULT)"
	$(CC) $(CFLAGS) $< 
	$(eval EXTRA := $(<:.c=.o))
	@mv $(EXTRA:src/%=%) obj/


clean:
	@$(RM) $(OBJBC) $(OBJBS) $(OBJEXTRA) $(OBJCL) $(OBJSV)
	@echo "$(RED) DELETED $(OBJEXTRA) $(OBJCL) $(OBJSV) $(DEFAULT)"

fclean: clean
	@$(RM) $(NAME1) $(NAME2) 
	@echo "$(RED) DELETED $(OBJEXTRA) $(NAME1) $(NAME2) $(DEFAULT)"

re: fclean begin

.PHONY: bonus begin all re fclean clean
