/* Copyright(C) 2018 Hex Five Security, Inc. - All Rights Reserved */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h> // round()

#include <platform.h>
#include <libhexfive.h>

#define CMD_LINE_SIZE 32
#define MSG_SIZE 4
#define ZONE 3
#define STD_OUT 0
#define STD_IN 0

void trap_0x0_handler(void)__attribute__((interrupt("user")));
void trap_0x0_handler(void){

	int msg[MSG_SIZE]={0,0,0,0};
	ECALL_RECV(ZONE, msg);
	printf("Instruction address misaligned : 0x%08x 0x%08x 0x%08x \n", msg[0], msg[1], msg[2]);

	printf("\nPress any key to restart");
	char c='\0'; while ( read(STD_IN, &c, 1) <=0 ) ECALL_YIELD(); asm ("j _start");

}

void trap_0x1_handler(void)__attribute__((interrupt("user")));
void trap_0x1_handler(void){

	int msg[MSG_SIZE]={0,0,0,0};
	ECALL_RECV(ZONE, msg);
	printf("Instruction access fault : 0x%08x 0x%08x 0x%08x \n", msg[0], msg[1], msg[2]);
	
	printf("\nPress any key to restart");
	char c='\0'; while ( read(STD_IN, &c, 1) <=0 ) ECALL_YIELD(); asm ("j _start");

}

void trap_0x2_handler(void)__attribute__((interrupt("user")));
void trap_0x2_handler(void){

	int msg[MSG_SIZE]={0,0,0,0};
	ECALL_RECV(ZONE, msg);
	printf("Illegal instruction : 0x%08x 0x%08x 0x%08x \n", msg[0], msg[1], msg[2]);

}

void trap_0x3_handler(void)__attribute__((interrupt("user")));
void trap_0x3_handler(void){

	const uint64_t T = ECALL_CSRR_MTIME();

	printf("\e7"); 		// save curs pos
	printf("\e[r"); 	// scroll all screen
	printf("\e[2M"); 	// scroll up up
	printf("\e8");   	// restore curs pos
	printf("\e[2A"); 	// curs up 2 lines
	printf("\e[2L\r"); 	// insert 2 lines

	printf("Z%d > ",ZONE);
	printf("timer expired : %lu", (unsigned long)(T*1000/RTC_FREQ));

	printf("\e8");   	// restore curs pos

}

void trap_0x4_handler(void)__attribute__((interrupt("user")));
void trap_0x4_handler(void){

	int msg[MSG_SIZE]={0,0,0,0};
	ECALL_RECV(ZONE, msg);
	printf("Load address misaligned : 0x%08x 0x%08x 0x%08x \n", msg[0], msg[1], msg[2]);

}

void trap_0x5_handler(void)__attribute__((interrupt("user")));
void trap_0x5_handler(void){

	int msg[MSG_SIZE]={0,0,0,0};
	ECALL_RECV(ZONE, msg);
	printf("Load access fault : 0x%08x 0x%08x 0x%08x \n", msg[0], msg[1], msg[2]);

}

void trap_0x6_handler(void)__attribute__((interrupt("user")));
void trap_0x6_handler(void){

	int msg[MSG_SIZE]={0,0,0,0};
	ECALL_RECV(ZONE, msg);
	printf("Store/AMO address misaligned : 0x%08x 0x%08x 0x%08x \n", msg[0], msg[1], msg[2]);

}

void trap_0x7_handler(void)__attribute__((interrupt("user")));
void trap_0x7_handler(void){

	int msg[MSG_SIZE]={0,0,0,0};
	ECALL_RECV(ZONE, msg);
	printf("Store access fault : 0x%08x 0x%08x 0x%08x \n", msg[0], msg[1], msg[2]);

}

// ------------------------------------------------------------------------
void print_cpu_info(void) {
// ------------------------------------------------------------------------

	// misa
	uint64_t misa = 0x0; asm volatile("csrr %0, misa" : "=r"(misa)); // trap & emulate example
	//const uint64_t misa = ECALL_CSRR_MISA();

	const int xlen = ((misa >> __riscv_xlen-2)&0b11)==1 ?  32 :
					 ((misa >> __riscv_xlen-2)&0b11)==2 ?  64 :
					 ((misa >> __riscv_xlen-2)&0b11)==1 ? 128 : 0;

	char misa_str[26+1]="";
	for (int i=0, j=0; i<26; i++)
		if ( (misa & (1ul << i)) !=0){
			misa_str[j++]=(char)('A'+i); misa_str[j]='\0';
		}

	printf("Machine ISA   : 0x%08x RV%d %s \n", (int)misa, xlen, misa_str);

	// mvendorid
	const uint64_t mvendorid = ECALL_CSRR_MVENDID();
	const char *mvendorid_str = (mvendorid==0x10e31913 ? "SiFive, Inc.\0" :
						         mvendorid==0x489      ? "SiFive, Inc.\0" :
								 mvendorid==0x57c      ? "Hex Five, Inc.\0" :
												         "\0");
	printf("Vendor        : 0x%08x %s \n", (int)mvendorid, mvendorid_str);

	// marchid
	const uint64_t marchid = ECALL_CSRR_MARCHID();
	const char *marchid_str = (mvendorid==0x489 && (int)misa==0x40101105    && marchid==0x80000002 ? "E21\0"  :
							   mvendorid==0x489 && (int)misa==0x40101105    && marchid==0x00000001 ? "E31\0"  :
						       mvendorid==0x489 && misa==0x8000000000101105 && marchid==0x00000001 ? "S51\0"  :
						       mvendorid==0x57c && (int)misa==0x40101105    && marchid==0x00000001 ? "X300\0" :
						       "\0");
	printf("Architecture  : 0x%08x %s \n", (int)marchid, marchid_str);

	// mimpid
	const uint64_t mimpid = ECALL_CSRR_MIMPID();
	printf("Implementation: 0x%08x \n", (int)mimpid );

	// mhartid
	const uint64_t mhartid = ECALL_CSRR_MHARTID();
	printf("Hart ID       : 0x%08x \n", (int)mhartid );

	// CPU Clock
	const int cpu_clk = round(CPU_FREQ/1E+6);
	printf("CPU clock     : %d MHz \n", cpu_clk );

}

// ------------------------------------------------------------------------
int cmpfunc(const void* a , const void* b){
// ------------------------------------------------------------------------
    const int ai = *(const int* )a;
    const int bi = *(const int* )b;
    return ai < bi ? -1 : ai > bi ? 1 : 0;
}

// ------------------------------------------------------------------------
void print_stats(void){
// ------------------------------------------------------------------------

	#define COUNT (10+1) // odd values for median
	#define MHZ (CPU_FREQ/1000000)

	int cycles[COUNT];

	for (int i=0, first=1; i<COUNT; i++){

		volatile unsigned long C1 = ECALL_CSRR_MCYCLE();
		ECALL_YIELD();
		volatile unsigned long C2 = ECALL_CSRR_MCYCLE();

		cycles[i] = C2-C1;

	}

	int max_cycle = 0; for (int i=0; i<COUNT; i++)
		max_cycle = cycles[i] > max_cycle ? cycles[i] : max_cycle;
	int max_col = 0;
	while(max_cycle>0){
		max_col++; max_cycle /= 10;
	}
	for (int i=0; i<COUNT; i++)
		printf("%*d cycles in %*d us \n", max_col, cycles[i], max_col-2, (int)(cycles[i]/MHZ));

	qsort(cycles, COUNT, sizeof(int), cmpfunc);

	printf("------------------------------------------------\n");
	int min = cycles[0], med = cycles[COUNT/2], max = cycles[COUNT-1];
	printf("cycles  min/med/max = %d/%d/%d \n", min, med, max);
	printf("time    min/med/max = %d/%d/%d us \n", (int)min/MHZ, (int)med/MHZ, (int)max/MHZ);

	volatile unsigned ctxsw_cycle = ECALL_CSRR_MHPMC3();
	volatile unsigned ctxsw_instr = ECALL_CSRR_MHPMC4();
	if (ctxsw_instr>0 && cycles>0){ // mhpmcounters might not be implemented
		printf("\n");
		printf("ctx sw instr  = %lu \n", ctxsw_instr);
		printf("ctx sw cycles = %lu \n", ctxsw_cycle);
		printf("ctx sw time   = %d us \n", (int)(ctxsw_cycle/MHZ));
	}

}

// ------------------------------------------------------------------------
void print_pmp(void){
// ------------------------------------------------------------------------

	#define TOR   0b00001000
	#define NA4   0b00010000
	#define NAPOT 0b00011000

	#define PMP_R 1<<0
	#define PMP_W 1<<1
	#define PMP_X 1<<2

	volatile uint64_t pmpcfg=0x0;
#if __riscv_xlen==32
	volatile uint32_t pmpcfg0; asm ( "csrr %0, pmpcfg0" : "=r"(pmpcfg0) );
	volatile uint32_t pmpcfg1; asm ( "csrr %0, pmpcfg1" : "=r"(pmpcfg1) );
	pmpcfg = pmpcfg1;
	pmpcfg <<= 32;
	pmpcfg |= pmpcfg0;
#else
	asm ( "csrr %0, pmpcfg0" : "=r"(pmpcfg) );
#endif

#if __riscv_xlen==32
	uint32_t pmpaddr[8];
#else
	uint64_t pmpaddr[8];
#endif
	asm ( "csrr %0, pmpaddr0" : "=r"(pmpaddr[0]) );
	asm ( "csrr %0, pmpaddr1" : "=r"(pmpaddr[1]) );
	asm ( "csrr %0, pmpaddr2" : "=r"(pmpaddr[2]) );
	asm ( "csrr %0, pmpaddr3" : "=r"(pmpaddr[3]) );
	asm ( "csrr %0, pmpaddr4" : "=r"(pmpaddr[4]) );
	asm ( "csrr %0, pmpaddr5" : "=r"(pmpaddr[5]) );
	asm ( "csrr %0, pmpaddr6" : "=r"(pmpaddr[6]) );
	asm ( "csrr %0, pmpaddr7" : "=r"(pmpaddr[7]) );

	for (int i=0; i<8; i++){

		const uint8_t cfg = (pmpcfg >> 8*i); if (cfg==0x0) continue;

		char rwx[3+1] = {cfg & PMP_R ? 'r':'-', cfg & PMP_W ? 'w':'-', cfg & PMP_X ? 'x':'-', '\0'};

		uint64_t start=0, end=0;

		char type[5+1]="";

		if ( (cfg & (TOR | NA4 | NAPOT)) == TOR){
			start = pmpaddr[i-1]<<2;
			end =  (pmpaddr[i]<<2) -1;
			strcpy(type, "TOR");

		} else if ( (cfg & (TOR | NA4 | NAPOT)) == NA4){
			start = pmpaddr[i]<<2;
			end =  start+4 -1;
			strcpy(type, "NA4");

		} else if ( (cfg & (TOR | NA4 | NAPOT)) == NAPOT){
			for (int j=0; j<__riscv_xlen; j++){
				if ( ((pmpaddr[i] >> j) & 0x1) == 0){
					const uint64_t size = 1 << (3+j);
					start = (pmpaddr[i] >>j) <<(j+2);
					end = start + size -1;
					strcpy(type, "NAPOT");
					break;
				}
			}

		} else break;

#if __riscv_xlen==32
		printf("0x%08x 0x%08x %s %s \n", (unsigned int)start, (unsigned int)end, rwx, type);
#else
		printf("0x%08" PRIX64 " 0x%08" PRIX64 " %s %s \n", start, end, rwx, type);
#endif

	}

}

// ------------------------------------------------------------------------
 int readline(char *cmd_line) {
// ------------------------------------------------------------------------
	int p=0;
	char c='\0';
	int esc=0;
	cmd_line[0] = '\0';
	static char history[CMD_LINE_SIZE+1]="";

	while(c!='\r' && c!='\n'){

		if ( read(STD_IN, &c, 1) > 0 ) {

			if (c=='\e'){
				esc=1;

			} else if (esc==1 && c=='['){
				esc=2;

			} else if (esc==2 && c=='3'){
				esc=3;

			} else if (esc==3 && c=='~'){ // del key
				for (int i=p; i<strlen(cmd_line); i++) cmd_line[i]=cmd_line[i+1];
				printf("\e7"); // save curs pos
				printf("\e[K"); // clear line from curs pos
				printf("%s",&cmd_line[p]);
				printf("\e8"); // restore curs pos
				esc=0;

			} else if (esc==2 && c=='C'){ // right arrow
				esc=0;
				if (p < strlen(cmd_line)){
					p++;
					printf("\e[C");
				}

			} else if (esc==2 && c=='D'){ // left arrow
				esc=0;
				if (p>0){
					p--;
					printf("\e[D");
				}

			} else if (esc==2 && c=='A'){ // up arrow
				esc=0;
				if (strlen(history)>0){
					p=strlen(history);
					strcpy(cmd_line, history);
					printf("\e[2K\r"); // 2K clear entire line - cur pos dosn't change
					printf("Z%d > ",ZONE);
					printf("%s",&cmd_line[0]);
				}

			} else if (esc==2 && c=='B'){ // down arrow
				esc=0;

			} else if ((c=='\b' || c=='\x7f') && p>0 && esc==0){ // backspace
				p--;
				for (int i=p; i<strlen(cmd_line); i++) cmd_line[i]=cmd_line[i+1];
				printf("\e[D");
				printf("\e7");
				printf("\e[K");
				printf("%s",&cmd_line[p]);
				printf("\e8");

			} else if (c>=' ' && c<='~' && p < CMD_LINE_SIZE && esc==0){
				for (int i = CMD_LINE_SIZE-1; i > p; i--) cmd_line[i]=cmd_line[i-1]; // make room for 1 ch
				cmd_line[p]=c;
				printf("\e7"); // save curs pos
				printf("\e[K"); // clear line from curs pos
				printf("%s",&cmd_line[p]); p++;
				printf("\e8"); // restore curs pos
				printf("\e[C"); // move curs right 1 pos

			} else
				esc=0;
		}

		// poll & print incoming messages
		int msg[MSG_SIZE]={0,0,0,0};


		if (ECALL_RECV(1, msg)){

			printf("\e7"); // save curs pos
			printf("\e[2K\r"); // 2K clear entire line - cur pos dosn't change

			printf("Z1 > 0x%08x 0x%08x 0x%08x 0x%08x \n", msg[0], msg[1], msg[2], msg[3]);

			printf("\nZ%d > ",ZONE);
			printf("%s",&cmd_line[0]);
			printf("\e8");   // restore curs pos
			printf("\e[2B"); // curs down down
		}

		if (ECALL_RECV(2, msg)){

			printf("\e7"); // save curs pos
			printf("\e[2K\r"); // 2K clear entire line - cur pos dosn't change

			printf("Z2 > 0x%08x 0x%08x 0x%08x 0x%08x \n", msg[0], msg[1], msg[2], msg[3]);

			printf("\nZ%d > ",ZONE);
			printf("%s",&cmd_line[0]);
			printf("\e8");   // restore curs pos
			printf("\e[2B"); // curs down down
		}

		if (ECALL_RECV(3, msg)){

			printf("\e7"); // save curs pos
			printf("\e[2K\r"); // 2K clear entire line - cur pos dosn't change

			printf("Z3 > 0x%08x 0x%08x 0x%08x 0x%08x \n", msg[0], msg[1], msg[2], msg[3]);

			printf("\nZ%d > ",ZONE);
			printf("%s",&cmd_line[0]);
			printf("\e8");   // restore curs pos
			printf("\e[2B"); // curs down down
		}

		if (ECALL_RECV(4, msg)){

			printf("\e7"); // save curs pos
			printf("\e[2K\r"); // 2K clear entire line - cur pos dosn't change

			switch (msg[0]) {
			case 1   : printf("Z4 > USB DEVICE ATTACH VID=0x1267 PID=0x0000\n"); break;
			case 2   : printf("Z4 > USB DEVICE DETACH\n"); break;
			case 'p' : printf("Z4 > pong\n"); break;
			default  : printf("Z4 > ???\n"); break;
			}

			printf("\nZ%d > ",ZONE);
			printf("%s",&cmd_line[0]);
			printf("\e8");   // restore curs pos
			printf("\e[2B"); // curs down down
		}

		ECALL_YIELD();

	}

	for (int i = CMD_LINE_SIZE-1; i > 0; i--)
		if (cmd_line[i]==' ') cmd_line[i]='\0';	else break;

	if (strlen(cmd_line)>0)
		strcpy(history, cmd_line);

	return strlen(cmd_line);

}

// ------------------------------------------------------------------------
int main (void) {
// ------------------------------------------------------------------------

	//volatile int w=0; while(1){w++;}
	//while(1) ECALL_YIELD();

	ECALL_TRP_VECT(0x0, trap_0x0_handler); // 0x0 Instruction address misaligned
	ECALL_TRP_VECT(0x1, trap_0x1_handler); // 0x1 Instruction access fault
	ECALL_TRP_VECT(0x2, trap_0x2_handler); // 0x2 Illegal Instruction
	ECALL_TRP_VECT(0x3, trap_0x3_handler); // 0x3 Soft timer
    ECALL_TRP_VECT(0x4, trap_0x4_handler); // 0x4 Load address misaligned
    ECALL_TRP_VECT(0x5, trap_0x5_handler); // 0x5 Load access fault
    ECALL_TRP_VECT(0x6, trap_0x6_handler); // 0x6 Store/AMO address misaligned
	ECALL_TRP_VECT(0x7, trap_0x7_handler); // 0x7 Store access fault

	freopen("UART", "r+", stdin);
	freopen("UART", "a+", stdout);

	printf("\e[2J\e[H"); // clear terminal screen
	printf("=====================================================================\n");
	printf("      	           Hex Five MultiZone(TM) Security                   \n");
	printf("    Copyright (C) 2018 Hex Five Security Inc. All Rights Reserved    \n");
	printf("=====================================================================\n");
	printf("This version of MultiZone(TM) is meant for evaluation purposes only. \n");
	printf("As such, use of this software is governed by your Evaluation License.\n");
	printf("There may be other functional limitations as described in the        \n");
	printf("evaluation kit documentation. The full version of the software does  \n");
	printf("not have these restrictions.                                         \n");
	printf("=====================================================================\n");

    print_cpu_info();

	char cmd_line[CMD_LINE_SIZE+1]="";
	int msg[MSG_SIZE]={0,0,0,0};

	while(1){

		printf("\nZ%d > ",ZONE);

		readline(cmd_line);

		printf("\n");

		char * tk1 = strtok (cmd_line, " ");
		char * tk2 = strtok (NULL, " ");
		char * tk3 = strtok (NULL, " ");

		if (tk1 != NULL && strcmp(tk1, "load")==0){
			if (tk2 != NULL){
				uint8_t data = 0x00;
				const uint64_t addr = strtoull(tk2, NULL, 16);
				asm ("lbu %0, (%1)" : "+r"(data) : "r"(addr));
				printf("0x%08x : 0x%02x \n", (unsigned int)addr, data);
			} else printf("Syntax: load address \n");

		} else if (tk1 != NULL && strcmp(tk1, "store")==0){
			if (tk2 != NULL && tk3 != NULL){
				const uint32_t data = (uint32_t)strtoul(tk3, NULL, 16);
				const uint64_t addr = strtoull(tk2, NULL, 16);

				if ( strlen(tk3) <=2 )
					asm ( "sb %0, (%1)" : : "r"(data), "r"(addr));
				else if ( strlen(tk3) <=4 )
					asm ( "sh %0, (%1)" : : "r"(data), "r"(addr));
				else
					asm ( "sw %0, (%1)" : : "r"(data), "r"(addr));

				printf("0x%08x : 0x%02x \n", (unsigned int)addr, (unsigned int)data);
			} else printf("Syntax: store address data \n");

		} else if (tk1 != NULL && strcmp(tk1, "exec")==0){
			if (tk2 != NULL){
				const uint64_t addr = strtoull(tk2, NULL, 16);
			    asm ( "jr (%0)" : : "r"(addr));
		} else printf("Syntax: exec address \n");

		} else if (tk1 != NULL && strcmp(tk1, "send")==0){
			if (tk2 != NULL && tk2[0]>='0' && tk2[0]<='4' && tk3 != NULL){
				for (int i=0; i<MSG_SIZE; i++)
					msg[i] = i<strlen(tk3) ? (unsigned int)*(tk3+i) : 0x0;
				if (!ECALL_SEND(tk2[0]-'0', msg))
					printf("Error: Inbox full.\n");
			} else printf("Syntax: send {0|1|2|3|4} message \n");

		} else if (tk1 != NULL && strcmp(tk1, "recv")==0){
			if (tk2 != NULL && tk2[0]>='0' && tk2[0]<='4'){
				if (ECALL_RECV(tk2[0]-'0', msg))
					printf("msg : 0x%08x 0x%08x 0x%08x 0x%08x \n", msg[0], msg[1], msg[2], msg[3]);
				else
					printf("Error: Inbox empty.\n");
			} else printf("Syntax: recv {0|1|2|3|4} \n");

		} else if (tk1 != NULL && strcmp(tk1, "yield")==0){
			uint64_t C1 = ECALL_CSRR_MCYCLE();
			ECALL_YIELD();
			uint64_t C2 = ECALL_CSRR_MCYCLE();
			const int T = ((C2-C1)*1000000)/CPU_FREQ;
			printf( (T>0 ? "yield : elapsed time %dus \n" : "yield : n/a \n"), T);

		} else if (tk1 != NULL && strcmp(tk1, "stats")==0){
			print_stats();

		} else if (tk1 != NULL && strcmp(tk1, "restart")==0){
			asm ("j _start");

		} else if (tk1 != NULL && strcmp(tk1, "timer")==0){
			if (tk2 != NULL){
				const uint64_t ms = abs(strtoull(tk2, NULL, 10));
				const uint64_t T0 = ECALL_CSRR_MTIME();
				const uint64_t T1 = T0 + ms*RTC_FREQ/1000;
				ECALL_CSRW_MTIMECMP(T1);
				printf("timer set T0=%lu, T1=%lu \n", (unsigned long)(T0*1000/RTC_FREQ),
													  (unsigned long)(T1*1000/RTC_FREQ) );
			} else printf("Syntax: timer ms \n");

		} else if (tk1 != NULL && strcmp(tk1, "pmp")==0){
			print_pmp();

		} else
			printf("Commands: load store exec send recv yield pmp stats timer restart \n");

	}

}
