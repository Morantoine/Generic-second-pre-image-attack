FLAGS = -Wall -Wextra -pedantic -lm
#FLAGS = -lm

EXEC_TEST = test
EXEC_ATTACK = attack

all : $(EXEC_TEST)

test_run : $(EXEC_TEST)
test_run : ;./test

attack_run : $(EXEC_ATTACK)
attack_run : ;./attack

$(EXEC_ATTACK) :
	gcc $(FLAGS) -o $(EXEC_ATTACK) src/attack.c src/second_preim_48_fillme.c

$(EXEC_TEST) :
	gcc $(FLAGS) -o $(EXEC_TEST) src/test.c src/second_preim_48_fillme.c


.PHONY: clean
clean :
	rm -rf $(EXEC_TEST) $(EXEC_ATTACK)
