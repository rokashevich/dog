#include <stdio.h>
int main() {
	for (int i = 1; i <= 1000; ++i) {
		fprintf(stdout, "abcdefghijklmnopqrstuwxyz%d\n",i);
		//fprintf(stdout, "TEXT TEXT TEXT TEXT TEXT TEXT TEXT STDOUT %d\n",i);
		//fprintf(stderr, "TEXT TEXT TEXT TEXT TEXT TEXT TEXT STDERR %d\n",i);
		fflush(stdout);
	}
	return 1/0;
}
