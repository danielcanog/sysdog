#ifndef SBUS_H
#define SBUS_H

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#ifndef SBUS_H
#define SBUS_H
void sbuslock(void);
void sbusunlock(void);
void sbuspreempt(void);

void sbus_poke16(unsigned int, unsigned short);
unsigned short sbus_peek16(unsigned int);

void winpoke16(unsigned int, unsigned short);
unsigned short winpeek16(unsigned int);
void winpoke32(unsigned int, unsigned int);
unsigned int winpeek32(unsigned int);
void winpoke8(unsigned int, unsigned char);
unsigned char winpeek8(unsigned int);

void sbus_peekstream16(int adr_reg, int dat_reg, int adr, unsigned char *buf, int n);
void sbus_pokestream16(int adr_reg, int dat_reg, int adr, unsigned char *buf, int n);
#endif


#define MW_ADR    0x18
#define MW_CONF   0x1a
#define MW_DAT1   0x1c
#define MW_DAT2   0x1e
#define _GNU_SOURCE

#ifndef TEMP_FAILURE_RETRY
# define TEMP_FAILURE_RETRY(expression) \
  (__extension__                                                              \
    ({ long int __result;                                                     \
       do __result = (long int) (expression);                                 \
       while (__result == -1L && errno == EINTR);                             \
       __result; }))
#endif



void sbus_poke16(unsigned int, unsigned short);
unsigned short sbus_peek16(unsigned int);

void winpoke16(unsigned int, unsigned short);
unsigned short winpeek16(unsigned int);
void winpoke32(unsigned int, unsigned int);
unsigned int winpeek32(unsigned int);
void winpoke8(unsigned int, unsigned char);
unsigned char winpeek8(unsigned int);

void sbus_peekstream16(int adr_reg, int dat_reg, int adr, unsigned char *buf, int n);
void sbus_pokestream16(int adr_reg, int dat_reg, int adr, unsigned char *buf, int n);

void sbuslock(void);
void sbusunlock(void);
void sbuspreempt(void); 

static volatile unsigned int *cvspiregs, *cvgpioregs;
static int last_gpio_adr = 0;

void sbus_poke16(unsigned int adr, unsigned short dat) {
	unsigned int dummy = 0;
	if (last_gpio_adr != adr >> 5) {
		last_gpio_adr = adr >> 5;
		cvgpioregs[0] = (cvgpioregs[0] & ~(0x3<<15))|((adr>>5)<<15);
	}
	adr &= 0x1f;

	asm volatile (
		"mov %0, %1, lsl #18\n"
		"orr %0, %0, #0x800000\n"
		"orr %0, %0, %2, lsl #3\n"
		"3: ldr r1, [%3, #0x64]\n"
		"cmp r1, #0x0\n"
		"bne 3b\n"
		"2: str %0, [%3, #0x50]\n"
		"1: ldr r1, [%3, #0x64]\n"
		"cmp r1, #0x0\n"
		"beq 1b\n"
		"ldr %0, [%3, #0x58]\n"
		"ands r1, %0, #0x1\n"
		"moveq %0, #0x0\n"
		"beq 3b\n"
		: "+r"(dummy) : "r"(adr), "r"(dat), "r"(cvspiregs) : "r1","cc"
	);
}

unsigned short sbus_peek16(unsigned int adr) {
	unsigned short ret = 0;

	if (last_gpio_adr != adr >> 5) {
		last_gpio_adr = adr >> 5;
		cvgpioregs[0] = ((adr>>5)<<15|1<<3|1<<17);
	}
	adr &= 0x1f;

	asm volatile (
		"mov %0, %1, lsl #18\n"
		"2: str %0, [%2, #0x50]\n"
		"1: ldr r1, [%2, #0x64]\n"
		"cmp r1, #0x0\n"
		"beq 1b\n"
		"ldr %0, [%2, #0x58]\n"
		"ands r1, %0, #0x10000\n"
		"bicne %0, %0, #0xff0000\n"
		"moveq %0, #0x0\n"
		"beq 2b\n" 
		: "+r"(ret) : "r"(adr), "r"(cvspiregs) : "r1", "cc"
	);

	return ret;

}

void winpoke16(unsigned int adr, unsigned short dat) {

        sbus_poke16(MW_ADR, adr >> 11);
        sbus_poke16(MW_CONF, (adr & 0x7ff) | (0x10 << 11));
        sbus_poke16(MW_DAT1, dat);
}

unsigned short winpeek16(unsigned int adr) {

        sbus_poke16(MW_ADR, adr >> 11);
        sbus_poke16(MW_CONF, (adr & 0x7ff) | (0x10 << 11));
        return sbus_peek16(MW_DAT1);
}

void winpoke32(unsigned int adr, unsigned int dat) {

        sbus_poke16(MW_ADR, adr >> 11);
        sbus_poke16(MW_CONF, (adr & 0x7ff) | (0x0d << 11));
        sbus_poke16(MW_DAT2, dat & 0xffff);
        sbus_poke16(MW_DAT2, dat >> 16);
}

unsigned int winpeek32(unsigned int adr) {

        unsigned int ret;

        sbus_poke16(MW_ADR, adr >> 11);
        sbus_poke16(MW_CONF, (adr & 0x7ff) | (0x0d << 11));
        ret = sbus_peek16(MW_DAT2);
        ret |= (sbus_peek16(MW_DAT2) << 16);
        return ret;
}

void winpoke8(unsigned int adr, unsigned char dat) {
        sbus_poke16(MW_ADR, adr >> 11);
        sbus_poke16(MW_CONF, (adr & 0x7ff) | (0x18 << 11));
        sbus_poke16(MW_DAT1, dat);
}

unsigned char winpeek8(unsigned int adr) {
        sbus_poke16(MW_ADR, adr >> 11);
        sbus_poke16(MW_CONF, (adr & 0x7ff) | (0x18 << 11));
        return sbus_peek16(MW_DAT1) & 0xff;
}

void sbus_peekstream16(int adr_reg, int dat_reg, int adr, unsigned char *buf, int n) {
        assert(n != 0);

	sbus_poke16(adr_reg, adr);
	
        while (n >= 2) {
                unsigned int s = sbus_peek16(dat_reg);
                *buf++ = s & 0xff;
                *buf++ = s >> 8;
                n -= 2;
        }

        if (n) *buf = sbus_peek16(dat_reg) & 0xff;
};

void sbus_pokestream16(int adr_reg, int dat_reg, int adr, unsigned char *buf, int n) {
        assert(n != 0);

	sbus_poke16(adr_reg, adr);

        while (n >= 2) {
                unsigned int s;
                s = *buf++;
                s |= *buf++ << 8;
                sbus_poke16(dat_reg, s);
                n -= 2;
        }

        if (n) sbus_poke16(dat_reg, *buf);
}

static void reservemem(void) {
	char dummy[32768];
	int i, pgsize;
	FILE *maps;

	pgsize = getpagesize();
	mlockall(MCL_CURRENT|MCL_FUTURE);
	for (i = 0; i < sizeof(dummy); i += 4096) {
		dummy[i] = 0;
	}

	maps = fopen("/proc/self/maps", "r"); 
	if (maps == NULL) {
		perror("/proc/self/maps");
		exit(1);
	}
	while (!feof(maps)) {
		unsigned int s, e, i, x;
		char m[PATH_MAX + 1];
		char perms[16];
		int r = fscanf(maps, "%x-%x %s %*x %x:%*x %*d",
		  &s, &e, perms, &x);
		if (r == EOF) break;
		assert (r == 4);

		i = 0;
		while ((r = fgetc(maps)) != '\n') {
			if (r == EOF) break;
			m[i++] = r;
		}
		m[i] = '\0';
		assert(s <= e && (s & 0xfff) == 0);
                if (perms[0] == 'r' && perms[3] == 'p' && x != 1)
                  while (s < e) {
                        volatile unsigned char *ptr = (unsigned char *)s;
                        unsigned char d;
                        d = *ptr;
                        if (perms[1] == 'w') *ptr = d;
                        s += pgsize;
                }
        }
        fclose(maps);
}

static int semid = -1;
static int sbuslocked = 0;
void sbuslock(void) {
	int r;
	struct sembuf sop;
	static int inited = 0;
	if (semid == -1) {
		key_t semkey;
		reservemem();
		semkey = 0x75000000;
		semid = semget(semkey, 1, IPC_CREAT|IPC_EXCL|0777);
		if (semid != -1) {
			sop.sem_num = 0;
			sop.sem_op = 1;
			sop.sem_flg = 0;
			r = semop(semid, &sop, 1);
			assert (r != -1);
		} else semid = semget(semkey, 1, 0777);
		assert (semid != -1);
	}
	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = SEM_UNDO;

	r = TEMP_FAILURE_RETRY(semop(semid, &sop, 1));

	assert (r == 0);
	if (inited == 0) {
		int i;
		int devmem;

		inited = 1;
		devmem = open("/dev/mem", O_RDWR|O_SYNC);
		assert(devmem != -1);
		cvspiregs = (unsigned int *) mmap(0, 4096,
		  PROT_READ | PROT_WRITE, MAP_SHARED, devmem, 0x71000000);
		cvgpioregs = (unsigned int *) mmap(0, 4096,
		  PROT_READ | PROT_WRITE, MAP_SHARED, devmem, 0x7c000000);

		cvspiregs[0x64/4] = 0x0;
		cvspiregs[0x40/4] = 0x80000c02;
		cvspiregs[0x60/4] = 0x0;
		cvspiregs[0x6c/4] = 0x0;
		cvspiregs[0x4c/4] = 0x4;
		for (i = 0; i < 8; i++) cvspiregs[0x58 / 4];
		last_gpio_adr = 3;
	}
	cvgpioregs[0] = (cvgpioregs[0] & ~(0x3<<15))|(last_gpio_adr<<15);
	sbuslocked = 1;
}

void sbusunlock(void) {
	struct sembuf sop = { 0, 1, SEM_UNDO};
	int r;
	if (!sbuslocked) return;
	r = semop(semid, &sop, 1);
	assert (r == 0);
	sbuslocked = 0;
}

void sbuspreempt(void) {
	int r;
	r = semctl(semid, 0, GETNCNT);
	assert (r != -1);
	if (r) {
		sbusunlock();
		sched_yield();
		sbuslock();
	}
}

#endif // SBUS_H
