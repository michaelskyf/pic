/*
    PIC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PIC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with PIC.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define NUM_PROC 10

unsigned int rectangle_x = 10000;
unsigned int rectangle_y = 10000;
unsigned int area;

unsigned int *shared_mem; /* with length of (uint * NUM_PROC) */

unsigned int check_points(unsigned int start, unsigned int end, unsigned int full)
{
	unsigned int in_circle = 0;
	float side = sqrt(rectangle_x * rectangle_y);

	for(unsigned int x = start; x < end; x++)
		for(unsigned int y = 0; y < full; y++)
			if(sqrt(x*x + y*y) <= side)
				in_circle++;
	return in_circle;
}

int main()
{
	area = rectangle_x * rectangle_y;
	float proc_rec_x = (float)rectangle_x / NUM_PROC;
	float proc_rec_y = (float)rectangle_y / NUM_PROC;
	shared_mem = mmap(NULL, sizeof(unsigned int) * NUM_PROC, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	unsigned int part = 0;
	unsigned int full = 0;
	unsigned int circle = 0;

	if(proc_rec_x > (unsigned int)proc_rec_x)
	{
		if(proc_rec_y > (unsigned int)proc_rec_y)
		{
			part = rectangle_y;
			full = proc_rec_x;
		} else {
			fprintf(stderr, "x or y not dividable by %d\n", NUM_PROC);
			exit(-1);
		}
	} else {
		part = proc_rec_x;
		full = rectangle_y;
	}

	pid_t pids[NUM_PROC];

	for(int i = 0; i < NUM_PROC; i++)
	{
		pids[i] = fork();
		if(pids[i] == 0)
		{
			shared_mem[i] = check_points(i * part, (i+1) * part, full);
			return 0;
		}
	}

	for(int i = 0; i < NUM_PROC; i++)
	{
		waitpid(pids[i], NULL, 0);
		circle += shared_mem[i];
	}

	printf("pi=%f\n", (float)4 * circle/area);

	/* Cleanup */
	munmap(shared_mem, sizeof(unsigned int) * NUM_PROC);

	return 0;
}
