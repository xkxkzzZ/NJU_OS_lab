#include "x86.h"
#include "device.h"

extern TSS tss;
extern ProcessTable pcb[MAX_PCB_NUM];
extern int current;

extern int displayRow;
extern int displayCol;

void GProtectFaultHandle(struct StackFrame *sf);
void timerHandle(struct StackFrame *sf);
void syscallHandle(struct StackFrame *sf);

void syscallWrite(struct StackFrame *sf);
void syscallPrint(struct StackFrame *sf);
void syscallFork(struct StackFrame *sf);
void syscallSleep(struct StackFrame *sf);
void syscallExit(struct StackFrame *sf);

void irqHandle(struct StackFrame *sf)
{ // pointer sf = esp
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds" ::"a"(KSEL(SEG_KDATA)));
	/*XXX Save esp to stackTop */
	uint32_t tmpStackTop = pcb[current].stackTop;
	pcb[current].prevStackTop = pcb[current].stackTop;
	pcb[current].stackTop = (uint32_t)sf;

	switch (sf->irq)
	{
	case -1:
		break;
	case 0xd:
		GProtectFaultHandle(sf);
		break;
	case 0x20:
		timerHandle(sf);
		break;
	case 0x80:
		syscallHandle(sf);
		break;
	default:
		assert(0);
	}
	/*XXX Recover stackTop */
	pcb[current].stackTop = tmpStackTop;
}

void GProtectFaultHandle(struct StackFrame *sf)
{
	assert(0);
	return;
}

void switch_between_process(){
	uint32_t tmpStackTop = pcb[current].stackTop;
	pcb[current].stackTop = pcb[current].prevStackTop; 
	tss.esp0 = (uint32_t)&(pcb[current].stackTop);  
	asm volatile("movl %0, %%esp"::"m"(tmpStackTop)); 
	asm volatile("popl %gs");
	asm volatile("popl %fs");
	asm volatile("popl %es");
	asm volatile("popl %ds");
	asm volatile("popal");
	asm volatile("addl $8, %esp");
	asm volatile("iret");
}

void timerHandle(struct StackFrame *sf)
{
	// TODO
	for(int i = 0; i < MAX_PCB_NUM; i++){
		if(pcb[i].state == STATE_BLOCKED){
			pcb[i].sleepTime--;
			if(pcb[i].sleepTime == 0){
				pcb[i].state = STATE_RUNNABLE;
			}
		}
	}

	if(pcb[current].state == STATE_RUNNING){
		pcb[current].timeCount++;
		if (pcb[current].timeCount < MAX_TIME_COUNT){
			return;
		}
		else{
			pcb[current].timeCount = 0;
			pcb[current].state = STATE_RUNNABLE;
		}
	}

	for (int i = 1; i < MAX_PCB_NUM; i++){
		if (pcb[i].state == STATE_RUNNABLE && i != current){
			current = i;
			break;
		}
	}
	if (pcb[current].state != STATE_RUNNABLE){
		current = 0;
	}

	pcb[current].state = STATE_RUNNING;
	switch_between_process();
}

void syscallHandle(struct StackFrame *sf)
{
	switch (sf->eax)
	{ // syscall number
	case 0:
		syscallWrite(sf);
		break; // for SYS_WRITE
	/*TODO Add Fork,Sleep... */
	case 1:
		syscallFork(sf);
		break;
	case 3:
		syscallSleep(sf);
		break;
	case 4:
		syscallExit(sf);
		break;
	default:
		break;
	}
}

void syscallWrite(struct StackFrame *sf)
{
	switch (sf->ecx)
	{ // file descriptor
	case 0:
		syscallPrint(sf);
		break; // for STD_OUT
	default:
		break;
	}
}

void syscallPrint(struct StackFrame *sf)
{
	int sel = sf->ds; // segment selector for user data, need further modification
	char *str = (char *)sf->edx;
	int size = sf->ebx;
	int i = 0;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es" ::"m"(sel));
	for (i = 0; i < size; i++)
	{
		asm volatile("movb %%es:(%1), %0" : "=r"(character) : "r"(str + i));
		if (character == '\n')
		{
			displayRow++;
			displayCol = 0;
			if (displayRow == 25)
			{
				displayRow = 24;
				displayCol = 0;
				scrollScreen();
			}
		}
		else
		{
			data = character | (0x0c << 8);
			pos = (80 * displayRow + displayCol) * 2;
			asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
			displayCol++;
			if (displayCol == 80)
			{
				displayRow++;
				displayCol = 0;
				if (displayRow == 25)
				{
					displayRow = 24;
					displayCol = 0;
					scrollScreen();
				}
			}
		}
		// asm volatile("int $0x20"); //XXX Testing irqTimer during syscall
		// asm volatile("int $0x20":::"memory"); //XXX Testing irqTimer during syscall
	}

	updateCursor(displayRow, displayCol);
	// take care of return value
	return;
}

// TODO syscallFork ...
void syscallFork(struct StackFrame* sf){
	putStr("syscallFork\n");

	int new_index = 0;
	while(new_index < MAX_PCB_NUM && pcb[new_index].state != STATE_DEAD){
		new_index++;
	}
	if(new_index == MAX_PCB_NUM){
		pcb[current].regs.eax = -1;
		return;
	}

	for(int i = 0; i < 0x100000; i++){
		*(unsigned char *)(i + (new_index + 1) * 0x100000) = *(unsigned char *)(i + (current + 1) * 0x100000);
	}

	pcb[new_index].pid = new_index;
	pcb[new_index].state = STATE_RUNNABLE;
	pcb[new_index].timeCount = 0;
	pcb[new_index].sleepTime = 0;

	pcb[new_index].stackTop = pcb[current].stackTop - (uint32_t)&(pcb[current]) + (uint32_t)&(pcb[new_index]);
	pcb[new_index].prevStackTop = pcb[current].prevStackTop - (uint32_t)&(pcb[current]) + (uint32_t)&(pcb[new_index]);

	pcb[new_index].regs.edi = pcb[current].regs.edi;
	pcb[new_index].regs.esi = pcb[current].regs.esi;
	pcb[new_index].regs.ebp = pcb[current].regs.ebp;
	pcb[new_index].regs.xxx = pcb[current].regs.xxx;
	pcb[new_index].regs.ebx = pcb[current].regs.ebx;
	pcb[new_index].regs.edx = pcb[current].regs.edx;
	pcb[new_index].regs.ecx = pcb[current].regs.ecx;
	pcb[new_index].regs.eax = pcb[current].regs.eax;
	pcb[new_index].regs.irq = pcb[current].regs.irq;
	pcb[new_index].regs.error = pcb[current].regs.error;
	pcb[new_index].regs.eip = pcb[current].regs.eip;
	pcb[new_index].regs.eflags = pcb[current].regs.eflags;
	pcb[new_index].regs.esp = pcb[current].regs.esp;

	pcb[new_index].regs.cs = USEL(2*new_index + 1);
	pcb[new_index].regs.ss = USEL(2*new_index + 2);
	pcb[new_index].regs.ds = USEL(2*new_index + 2);
	pcb[new_index].regs.es = USEL(2*new_index + 2);
	pcb[new_index].regs.fs = USEL(2*new_index + 2);
	pcb[new_index].regs.gs = USEL(2*new_index + 2);

	pcb[current].regs.eax = new_index;
	pcb[new_index].regs.eax = 0;
	return;
}

void syscallSleep(struct StackFrame *sf){
	putStr("syscallSleep\n");

	pcb[current].state = STATE_BLOCKED;
	assert(sf->ecx > 0);
	pcb[current].sleepTime = sf->ecx;
	asm volatile("int $0x20");
}

void syscallExit(struct StackFrame *sf){
	putStr("syscallExit\n");

	pcb[current].state = STATE_DEAD;
	asm volatile("int $0x20");
}