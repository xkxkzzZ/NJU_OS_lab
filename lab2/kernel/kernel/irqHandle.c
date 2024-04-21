#include "x86.h"
#include "device.h"

extern int displayRow;
extern int displayCol;

extern uint32_t keyBuffer[MAX_KEYBUFFER_SIZE];
extern int bufferHead;
extern int bufferTail;

int tail=0;

void GProtectFaultHandle(struct TrapFrame *tf);

void KeyboardHandle(struct TrapFrame *tf);

void syscallHandle(struct TrapFrame *tf);
void syscallWrite(struct TrapFrame *tf);
void syscallPrint(struct TrapFrame *tf);
void syscallRead(struct TrapFrame *tf);
void syscallGetChar(struct TrapFrame *tf);
void syscallGetStr(struct TrapFrame *tf);


void irqHandle(struct TrapFrame *tf) { // pointer tf = esp
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));

	switch(tf->irq) {
		// TODO: 填好中断处理程序的调用
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x21:
			KeyboardHandle(tf);
			break;
		case 0x80:
			syscallHandle(tf);
			break;
		default:break;
	}
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}

void KeyboardHandle(struct TrapFrame *tf){
	uint32_t code = getKeyCode();

	if(code == 0xe){ // 退格符
		//要求只能退格用户键盘输入的字符串，且最多退到当行行首
		if(displayCol>0&&displayCol>tail){
			displayCol--;
			uint16_t data = 0 | (0x0c << 8);
			int pos = (80*displayRow+displayCol)*2;
			asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
		}
	}else if(code == 0x1c){ // 回车符
		//处理回车情况
		keyBuffer[bufferTail++]='\n';
		displayRow++;
		displayCol=0;
		tail=0;
		if(displayRow==25){
			scrollScreen();
			displayRow=24;
			displayCol=0;
		}
	}else if(code < 0x81){ 
		// TODO: 处理正常的字符
		char ch = getChar(code);
		if (ch >= 0x20) {
			putChar(ch);
			keyBuffer[bufferTail++] = ch;
			int sel = USEL(SEG_UDATA);
			asm volatile("movw %0, %%es"::"m"(sel));

			uint16_t data = ch | (0x0c << 8);
			int pos = (80 * displayRow + displayCol) * 2;
			asm volatile("movw %0, (%1)"::"r"(data), "r"(pos + 0xb8000));
				
			displayCol ++;
			if (displayCol >= 80) {
				displayCol = 0;
				displayRow ++;
			}
			while (displayRow >= 25) {
				scrollScreen();
				displayRow --;
				displayCol = 0;
			}	
		}
	}
	updateCursor(displayRow, displayCol);
	
}

void syscallHandle(struct TrapFrame *tf) {
	switch(tf->eax) { // syscall number
		case 0:
			syscallWrite(tf);
			break; // for SYS_WRITE
		case 1:
			syscallRead(tf);
			break; // for SYS_READ
		default:break;
	}
}

void syscallWrite(struct TrapFrame *tf) {
	switch(tf->ecx) { // file descriptor
		case 0:
			syscallPrint(tf);
			break; // for STD_OUT
		default:break;
	}
}

void syscallPrint(struct TrapFrame *tf) {
	int sel =  USEL(SEG_UDATA);
	char *str = (char*)tf->edx;
	int size = tf->ebx;
	int i = 0;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es"::"m"(sel));
	for (i = 0; i < size; i++) {
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i));
		// TODO: 完成光标的维护和打印到显存
		if (character == '\n') {
			displayRow += 1;
			displayCol = 0;
		}
		else {
			data = character | (0x0c << 8);
			pos = (80 * displayRow + displayCol) * 2;
			asm volatile("movw %0, (%1)"::"r"(data), "r"(pos + 0xb8000));
			displayCol ++;
		}

		if (displayCol >= 80) {
			displayCol = 0;
			displayRow ++;
		}
		while (displayRow >= 25) {
			scrollScreen();
			displayRow --;
			displayCol = 0;
		} 


	}
	tail=displayCol;
	updateCursor(displayRow, displayCol);
}

void syscallRead(struct TrapFrame *tf){
	switch(tf->ecx){ //file descriptor
		case 0:
			syscallGetChar(tf);
			break; // for STD_IN
		case 1:
			syscallGetStr(tf);
			break; // for STD_STR
		default:break;
	}
}

void syscallGetChar(struct TrapFrame *tf){
	// TODO: 自由实现
	int flag = 0;
	if(keyBuffer[bufferTail-1] == '\n') flag = 1;
	while(bufferTail > bufferHead && keyBuffer[bufferTail-1] == '\n') 
		keyBuffer[--bufferTail] = '\0';
	if(bufferTail > bufferHead && flag){ 
		tf->eax = keyBuffer[bufferHead++];
		bufferHead = bufferTail;
	}
	else {
		tf->eax = 0;
	}

}

void syscallGetStr(struct TrapFrame *tf){
	// TODO: 自由实现
	int size = tf->ebx;
	char *str = (char*)tf->edx;
	int i = 0;
	
	int sel = USEL(SEG_UDATA);
	asm volatile("movw %0, %%es"::"m"(sel));

	int flag = 0;
	if(keyBuffer[bufferTail-1] == '\n') flag = 1;
	while(bufferTail > bufferHead && keyBuffer[bufferTail-1] == '\n') 
		keyBuffer[--bufferTail] = '\0';
	if(!flag && bufferTail-bufferHead < size) {
		tf->eax = 0;
	}
	else{
		for(i=0; i<size && i<bufferTail-bufferHead;i++){
			char ch = keyBuffer[bufferHead+i];
			asm volatile("movb %0, %%es:(%1)"::"r"(ch),"r"(str+i));
		}
		tf->eax = 1;
	}
}
