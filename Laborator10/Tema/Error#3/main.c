#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>

int main()
{
	char w=0, *text;
	int p, q[2];
	
	pipe(q); // pipe(q) in loc de pipe(q[2])
	p = fork(); // p = fork in loc de p = fork(2)
	
	if(p == -1) exit(2);
	if(!p) {
		dup2(q[0], 0);
		//close(q[1]);
		while( read(0, &w, 1) != 0)
			print("%c",w);
		wait(NULL);
	}
	else // else in loc de elif
	{
		text="Salutari!\n";
		write(q[1], text, strlen(text));
		// EROAREA SEMANTICA : write(q[1]) in loc de write(q[0])
	}
	return 0;
}
