FLAGS = -Wall -Wextra -pedantic -lm
#FLAGS = -lm

EXEC_TEST = test_exec
EXEC_ATTACK = attack_exec

all : $(EXEC_TEST)

test : clean
test : $(EXEC_TEST)
test : ;./$(EXEC_TEST)

attack : $(EXEC_ATTACK)
attack : ;./$(EXEC_ATTACK)

$(EXEC_ATTACK) :
	gcc $(FLAGS) -o $(EXEC_ATTACK) src/attack.c src/second_preim_48_fillme.c

$(EXEC_TEST) :
	gcc $(FLAGS) -o $(EXEC_TEST) src/test.c src/second_preim_48_fillme.c


clean_test :
	rm -f $(EXEC_TEST)

clean_attack :
	rm -f $(EXEC_ATTACK)

.PHONY: clean
clean :
	rm -f $(EXEC_TEST) $(EXEC_ATTACK)
