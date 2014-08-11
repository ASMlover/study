#include <stdlib.h>  
#include <string.h>  
#include <stdio.h>  

#define TOKENS "><+-.,[]"  

#define CODE_SEGMENT_SIZE 30000  
#define STACK_SEGMENT_SIZE 1000  
#define DATA_SEGMENT_SIZE 30000  

typedef void (*Callback)(void);  

struct {  
	char cs[CODE_SEGMENT_SIZE];   /* Code Segment */  
	long ip;                      /* Instruction Pointer */  

	char ss[STACK_SEGMENT_SIZE];  /* Stack Segment */  
	long sp;                      /* Stack Pointer */  

	char ds[DATA_SEGMENT_SIZE];   /* Data Segment */  
	long bp;                      /* Base Pointer */  

	Callback fn[128];  
} vm;  

void vm_forward() {  
	vm.bp = (vm.bp + 1) % DATA_SEGMENT_SIZE;  
}  

void vm_backward() {  
	vm.bp = (vm.bp + DATA_SEGMENT_SIZE - 1) % DATA_SEGMENT_SIZE;  
}  

void vm_increment() {  
	vm.ds[vm.bp]++;  
}  

void vm_decrement() {  
	vm.ds[vm.bp]--;  
}  

void vm_input() {  
	vm.ds[vm.bp] = getchar();  
}  

void vm_output() {  
	putchar(vm.ds[vm.bp]);  
}  

void vm_while_entry() {  
	if (vm.ds[vm.bp]) {  
		vm.ss[vm.sp] = (char)(vm.ip - 1);  
		vm.sp++;  
	} else {  
		int c = 1;  
		for (vm.ip++; vm.cs[vm.ip] && c; vm.ip++) {  
			if (vm.cs[vm.ip] == '[') {  
				c++;  
			} else if (vm.cs[vm.ip] == ']') {  
				c--;  
			}  
		}  
	}  
}  

void vm_while_exit() {  
	if (vm.ds[vm.bp]) {  
		vm.sp--;  
		vm.ip = vm.ss[vm.sp];  
	}  
}  

void init(void) {
	memset(&vm, 0, sizeof(vm));  
	vm.fn['>'] = vm_forward;  
	vm.fn['<'] = vm_backward;  
	vm.fn['+'] = vm_increment;  
	vm.fn['-'] = vm_decrement;  
	vm.fn['.'] = vm_output;  
	vm.fn[','] = vm_input;  
	vm.fn['['] = vm_while_entry;  
	vm.fn[']'] = vm_while_exit;  
}

void setup(FILE* stream) {  
	int c;  
	int i;  

	if (NULL == stream)
		return;
	init();
	for (i = 0; (c = fgetc(stream)) != EOF;) {  
		if (strchr(TOKENS, c)) {  
			vm.cs[i] = c;  
			i++;  
		}  
	}  
} 

void run() {  
	while (vm.cs[vm.ip]) {  
		vm.fn[vm.cs[vm.ip]]();  
		vm.ip++;  
	}  
}  

int main(int argc, char* argv[]) {
	FILE* stream = stdin;
	int file_flag = 0;

	if (argc > 1) {
		stream = fopen(argv[1], "r");
		if (NULL != stream)
			file_flag = 1;
		else 
			stream = stdin;
	}

	setup(stream);  
	run();  

	if (file_flag)
		fclose(stream);

	return 0;  
} 

