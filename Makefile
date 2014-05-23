#Penelope Over psover, Stefan Utamaru sualexander
all: runCommand shell shell2
runCommand: runCommand.c
	gcc -g runCommand.c -o runCommand

shell: shell.c
	gcc -g shell.c -o shell

shell2: shell2.c
	gcc -g shell2.c -o shell2

clean:
	rm runCommand shell shell2
