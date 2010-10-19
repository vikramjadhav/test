#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

static inline void usage(char *s)
{
	fprintf(stderr, "Usage: %s <from file> <to file>.\n", s);
}

int main (int argc, char *argv[])
{
	int fdin, fdout;
	char *src, *dst;
	struct stat buf;

	memset(&buf, 0, sizeof(struct stat));

	if (argc != 3) {
		usage(argv[0]);
		return 1;
	}

	if ((fdin = open(argv[1], O_RDONLY)) < 0) {
		perror("src open ");
		return 1;
	}

	if ((fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC)) < 0) {
		perror("dst open ");
		return 1;
	}

	if (fstat(fdin, &buf) < 0) {
		perror("fstat ");
		return 1;
	}

	if (lseek(fdout, buf.st_size - 1, SEEK_SET) < 0) {\
		perror("lseek ");
		return 1;
	}

	if (write(fdout, " ", 1) < 0) {
		perror("write ");
		return 1;
	}

	if ((src = mmap(0, buf.st_size, PROT_READ, MAP_SHARED | MAP_FILE, fdin, 0)) == 
			(void *) -1) {
		perror("mmap ");
		return 1;
	}

	if ((dst = mmap(0, buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FILE, 
					fdout, 0)) == (void *) -1) {
		perror("mmap ");
		return 1;
	}

	memcpy(dst, src, buf.st_size);
	return 0;
}
