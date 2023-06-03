extern "C" int kmain();
__declspec(naked) void startup()
{
	__asm {
		call kmain;
	}
}
#define IDT_TYPE_INTR (0x0E)
#define IDT_TYPE_TRAP (0x0F)
#define GDT_CS (0x8)	
// Структура описывает данные об обработчике прерывания
#pragma pack(push, 1) // Выравнивание членов структуры запрещено
struct idt_entry
{
	unsigned short base_lo; // Младшие биты адреса обработчика
	unsigned short segm_sel; // Селектор сегмента кода
	unsigned char always0; // Этот байт всегда 0
	unsigned char flags; // Флаги тип. Флаги: P, DPL, Типы - это константы - IDT_TYPE...
	unsigned short base_hi; // Старшие биты адреса обработчика
};
// Структура, адрес которой передается как аргумент команды lidt
struct idt_ptr
{
	unsigned short limit;
	unsigned int base;
};
struct idt_entry g_idt[256]; // Реальная таблица IDT
struct idt_ptr g_idtp; // Описатель таблицы для команды lidt
#pragma pack(pop)
typedef void (*intr_handler)();
void intr_reg_handler(int num, unsigned short segm_sel, unsigned short flags, intr_handler hndlr)
{
	unsigned int hndlr_addr = (unsigned int)hndlr;
	g_idt[num].base_lo = (unsigned short)(hndlr_addr & 0xFFFF);
	g_idt[num].segm_sel = segm_sel;
	g_idt[num].always0 = 0;
	g_idt[num].flags = flags;
	g_idt[num].base_hi = (unsigned short)(hndlr_addr >> 16);
}
__declspec(naked) void default_intr_handler()
{
	__asm {
		pusha
	}
	// ... (реализация обработки)
	__asm {
		popa
		iretd
	}
}
#define VIDEO_BUF_PTR (0xb8000)
unsigned int posY=0,  runner = 0, len=0,colour=0x07;
void cursor_moveto(unsigned int strnum, unsigned int pos);
bool FlagEmpty =1;
char outS [26][80];
int Colours[26];
void SpaceStr();
void clean(){
	unsigned char* video_buf = (unsigned char*)VIDEO_BUF_PTR;
	video_buf += 80 * 2 * 0;
	for (int i = 0;i<80*24;i++){
		video_buf[0] = ' ';
		video_buf[1] = 0x07;
		video_buf += 2;
	}
	posY = 0;
}
void out_str(int color, const char* ptr )
{
	int x = 0;
	unsigned char* video_buf = (unsigned char*)VIDEO_BUF_PTR;
	if(posY == 25){
		//up();
		clean();
		for(posY=0;posY<24;posY++) out_str(Colours[posY+1],outS[posY+1]);
		//0SpaceStr();
	}
	Colours[posY] = color;
	video_buf += 80 * 2 * posY;
	if(FlagEmpty){
		len=0;
	}
	while (*ptr)
	{
		if(FlagEmpty){
			len++;
		}
		outS[posY][x] = (unsigned char)*ptr;
		video_buf[0] = (unsigned char)*ptr; // Символ (код
		video_buf[1] = color; // Цвет символа и фона
		video_buf += 2;
		ptr++;
		x++;
	}
	outS[posY][x]='\0'; 
}
void intr_init()
{
	int i;
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);
	for (i = 0; i < idt_count; i++)
	intr_reg_handler(i, GDT_CS, 0x80 | IDT_TYPE_INTR,
	default_intr_handler); // segm_sel=0x8, P=1, DPL=0, Type=Intr
}
void intr_start()
{
	int idt_count = sizeof(g_idt) / sizeof(g_idt[0]);
	g_idtp.base = (unsigned int)(&g_idt[0]);
	g_idtp.limit = (sizeof(struct idt_entry) * idt_count) - 1;
	__asm {
		lidt g_idtp
	}
	//__lidt(&g_idtp);
}
void intr_enable()
{
	__asm sti;
}
void intr_disable()
{
	__asm cli;
}
__inline unsigned char inb(unsigned short port)
{
	unsigned char data;
	__asm {
		push dx
		mov dx, port
		in al, dx
		mov data, al
		pop dx
	}
	return data;
}
__inline void outb (unsigned short port, unsigned char data)
{
	__asm {
		push dx
		mov dx, port
		mov al, data
		out dx, al
		pop dx
	}
}
__inline void outw (unsigned short port, unsigned int data) {
	__asm {
		push dx
		mov dx, port
		mov eax, data
		out dx, eax
		pop dx
	}
}

void move(char* word, int position) {
    char tmp;
    int size = 0;
    for (; word[size] != '\0'; size++) {}
    for (int i = size; i != position; i--) {
        word[i + 1] = word[i];              
	}
}
void SpaceStr(){
	char space[79];
	for(int i = 0; i<79;i++)space[i]=' ';
	FlagEmpty =0;
	out_str(colour,space);
	FlagEmpty =1;
}
void __cpuid()
{
	int CPUInfo[4], InfoType=0;
	__asm 
	{
		mov    eax, InfoType
		xor    ecx, ecx   
		cpuid  
		mov    esi, CPUInfo
		mov    [esi +  0], eax
		mov    [esi +  4], ebx  
		mov    [esi +  8], ecx  
		mov    [esi + 12], edx  
	}
	char out[12];
	for(int j = 1; j<4;j++){
		for(int i = 0; i<4;i++){
			out[i+4*(j-1)]=CPUInfo[j]%100;
			CPUInfo[1]/=100;
		}
	}
	out[12]='\0';
	out_str(colour,out);
	posY++;
}
int c =0,amount=0, cur=0, mem = 0;
bool big=0, Cflag=0;
void CopyOneToAntother(char* source, char* destination){
	destination[0] = source[0];
	for(int i = 1; source[i-1]!='\0';i++)destination[i] = source[i];
	amount = mem;
}
unsigned long long int AmountOfTicks=0;
void ticks(void) {
	char strok[20];
	long int num = AmountOfTicks;
	int size = -1;
	while (num) {
		size++;
		num /= 10;
	}
	num = AmountOfTicks;
	strok[size + 1] = '\0';
	for (; size >= 0;size--) {
		strok[size] = num % 10 + 48;
		num /= 10;
	}
	out_str(colour,strok);
	posY++;
}
void RunningTime(){
	char out[40] ="Uptime is ";
	int number[2][3];
	int current =10;
	char tmp1,tmp2;
	char h[] = " hours";
	char m[] = " minutes";
	char s[] = " seconds";
	char a[] = "and ";
	bool check[2] = {0,0};
	number[0][0] =*(unsigned char*)0x8400/16*10 + *(unsigned char*)0x8400%16;
	number[0][1] =*(unsigned char*)0x8200/16*10 + *(unsigned char*)0x8200%16;;
	number[0][2] =*(unsigned char*)0x8000/16*10 + *(unsigned char*)0x8000%16;
	outb(0x70, 4);
	number[1][0] =(inb(0x71)+3)/16*10 + (inb(0x71)+3)%16;
	outb(0x70, 2);
	number[1][1] =inb(0x71)/16 *10 + inb(0x71)%16;
	outb(0x70, 0);
	number[1][2] =inb(0x71)/16 * 10 + inb(0x71)%16;
	for(int i =0;i<3;i++){
		if(number[1][i]<number[0][i]){
			if(i==2){
				number[1][i]+=24;
				} else{
				number[1][i+1]--;
				number[1][i]+=60;
			}
		}
		number[1][i] -= number[0][i];
	}
	for(int i = 0, flag = 0;i<3;i++,flag=0){
		if(number[1][i]){ // if !=0
			if(i>0&&check[i-1])for(int i = 0; i<4;i++,current++)out[current] = a[i];
			if(number[1][i]/10>0){ // if tens
				out[current]= number[1][i]/10+48;
				current++;
				
			}
			if (number[1][i]%10 == 1)flag=1;
			out[current]= number[1][i]%10+48;
			current++;
			switch (i){
				case 0:
				for(int i = 0; i<6-flag;i++,current++)out[current] = h[i];
				check[0]=1;
				out[current] =' ';
				current++;
				break;
				case 1:
				for(int i = 0; i<8-flag;i++,current++)out[current] = m[i];
				check[1]=1;
				out[current] =' ';
				current++;
				break;
				case 2:
				for(int i = 0; i<8-flag;i++,current++)out[current] = s[i];
				out[current] =' ';
				current++;
				break;
				
			}
		}
	}
	out[current] ='\0';
	out_str(colour,out);
	posY++;
}
void OutTime(unsigned char a, unsigned char b, unsigned char c) {
	char out[9];
	out[0] = a/16 + 48;
	out[1] = a%16 + 48;
	out[3] = b/16 + 48;
	out[4] = b%16 + 48;
	out[6] = c/16 + 48;
	out[7] = c%16 + 48;
	out[2] = ':';
	out[5] = ':';
	out[8] = '\0';
	out_str(colour,out);
	posY++;
}
void memory(){
	int mem = *(int*)0x8600;
	int size = -1;
	for(; mem;size++)mem/=10;
	mem = *(int*)0x8600;
	mem*=64;
	char out[30] = "Available memory: ";
	int curent=20;
	out[size+3+curent]='\0';
	curent+=size;
	for(int i =0; mem;i++){
		out[curent] = mem%10+48;
		mem/=10;
		curent--;
	}
	curent+=size;
	curent+=size;
	out[curent]='k';
	out[curent+1]='b';
	out_str(colour,out);
	posY++;
}
void comma(int com){
	switch (com){
		case 0: //info
		out_str(colour,"InfoOS, made by Goretskiy Ilya");
		posY++;
		SpaceStr();
		out_str(colour,"Group:4851003/90001");
		posY++;
		SpaceStr();
		out_str(colour,"OS: Windows");
		posY++;
		SpaceStr();
		out_str(colour,"YASM translatуr with intel syntax");
		posY++;
		SpaceStr();
		out_str(colour,"Complier:Microsoft C");
		posY++;
		SpaceStr();
		break;
		case 1: // clear
		SpaceStr();
		clean();
		posY =0;
		break;
		case 2: //ticks
		ticks();
		break;
		case 3: //loadtime
		OutTime(*(unsigned char*)0x8400,*(unsigned char*)0x8200,*(unsigned char*)0x8000);
		break;
		case 4: //curtime
		unsigned char  bag[3];
		outb(0x70, 0);
		bag[0] = inb(0x71);
		outb(0x70, 2);
		bag[1] = inb(0x71);
		outb(0x70, 4);
		bag[2] = inb(0x71);
		OutTime(bag[2]+3,bag[1],bag[0]);
		break;
		case 5: //uptime
		RunningTime();
		break;
		case 6: //meminfo
		memory();
		break;
		case 7: //cpuid
		__cpuid();
		break;
		case 8: //shutdown
		outw(0x501, 0x2000);
		break;
		case 9: // colour
		Cflag = 1;
		out_str(colour,"Plese select new colour:");
		posY++;
		out_str(0x01,"1:Blue");
		posY++;
		out_str(0x02,"2:Green");
		posY++;
		out_str(0x03,"3:Cyan");
		posY++;
		out_str(0x04,"4:Red");
		posY++;
		out_str(0x05,"5:Magenta");
		posY++;
		out_str(0x06,"6:Brown");
		posY++;
		out_str(0x07,"7:Light Gray");
		posY++;
		out_str(0x08,"8:Dark Gray");
		posY++;
		out_str(0x09,"9:Light Blue");
		posY++;
		out_str(0x0A,"10:Light Green");
		posY++;
		out_str(0x0B,"11:Light Cyan");
		posY++;
		out_str(0x0C,"12:Light Red");
		posY++;
		out_str(0x0D,"13:Light Magenta");
		posY++;
		out_str(0x0E,"14:Yellow");
		posY++;
		out_str(0x0F,"15:White");
		posY++;
		out_str(colour,"Print \"0\"to return ");
		posY++;
		break;
		case 10: //help
		char commands[11][9] = { "info","clear","ticks","loadtime","curtime","uptime","meminfo","cpuid","shutdown","colour","help" };
		for(int i = 0; i<11;i++){
			out_str(colour,commands[i]);
			posY++;
		}
		break;
		
	}
	
}
int FinColour=1;
bool parsing(char* str1) {
	posY++;
	char str[41];
	for(int i = 0;i<41;i++)str[i]='\0';
	for( int i= 0;str1[i];i++)str[i] =str1[i];
	int  size = 0;
	for (; str[size]; size++) {
		if (str[size] > 64 && str[size] < 91)str[size] += 32;
	}
	if(Cflag){
		switch (size)
		{
			case 1:
			if (str[0] > 47 && str[0] < 58){
				if(str[0] - 48){colour = str[0] - 48;
				}
				Cflag = 0;
				posY--;
			}
			else out_str(colour,"incorrect inputs, try again");
			break;
			case 2:
			if (str[0] == 49 && str[1] <54 && str[1] >47){
				colour = 10 + str[1] - 48;
				Cflag = 0;
				posY--;
			}
			else out_str(colour,"incorrect inputs, try again");
			break;
			default:
			out_str(colour,"incorrect inputs, try again");
			break;
		}
		return 1;
	}
	if (size < 4) {
		out_str(colour,"incorrect commant");
		return 1;
	};
	char commands[11][9] = { "info","clear","ticks","loadtime","curtime","uptime","meminfo","cpuid","shutdown","colour","help" }; //f e c a r t m u u l l
	int flag = 0;
	for (int i = 0; i < 7; i++) {
		if (str[2] == commands[i][2]) {
			flag = i + 1;
			break;
		}
	}
	if (flag) {
		for (int i = 0; i<=size; i++) {
			if (str[i] != commands[flag - 1][i]) {
				out_str(colour,"incorrect commant");
				return 1;
			}
		}
		comma(flag - 1);
		return 0;
	}
	if (str[0] == commands[7][0]&& size == 5) {
		for (int i = 0; i<=size; i++) {
			if (str[i] != commands[7][i]) {
				out_str(colour,"incorrect commant");
				return 1;
			}
		}
		comma(7);
		return 0;
	}
	if (str[0] == commands[8][0] && size == 8) {
		for (int i = 0; i<=size; i++) {
			if (str[i] != commands[8][i]) {
				out_str(colour,"incorrect commant");
				return 1;
			}
		}
		comma(8);
		return 0;
	}
	if (str[0] == commands[9][0] && size == 6) {
		for (int i = 0; i<=size; i++) {
			if (str[i] != commands[9][i]) {
				out_str(colour,"incorrect commant");
				return 1;
			}
		}
		comma(9);
		return 0;
	}
	if (str[0] == commands[10][0] && size == 4) {
		for (int i = 0; i<=size; i++) {
			if (str[i] != commands[10][i]) {
				out_str(colour,"incorrect commant");
				return 1;
			}
		}
		comma(10);
		return 0;
	}
	out_str(colour,"incorrect commant");
	return 1;
}
void CheckColour(char codeF, char codeS, int len){ // len = 1 work with codeS
	switch (len)
	{
		case 0:
		if (codeS > 47 && codeS  < 58){
			if(codeS  - 48)colour = codeS  - 48;
			else colour = FinColour;
		}
		break;
		case 1:
		if (codeF == 49 && codeS  <54 && codeS  >47)colour = 10 + codeS - 48;
		else colour = FinColour;
		break;
		default:
		colour = FinColour;
	}
}
#define PIC1_PORT (0x20)
void on_key(int scan_code){
	char scan_table[134];
	scan_table[0]='0';
	scan_table[1]='0';
	scan_table[2]='1';
	scan_table[3]='2';
	scan_table[4]='3';
	scan_table[5]='4';
	scan_table[6]='5';
	scan_table[7]='6';
	scan_table[8]='7';
	scan_table[9]='8';
	scan_table[10]='9';
	scan_table[11]='0';
	scan_table[12]='-';
	scan_table[13]='=';
	scan_table[14]='0';
	scan_table[15]='\t';
	scan_table[16]='q';
	scan_table[17]='w';
	scan_table[18]='e';
	scan_table[19]='r';
	scan_table[20]='t';
	scan_table[21]='y';
	scan_table[22]='u';
	scan_table[23]='i';
	scan_table[24]='o';
	scan_table[25]='p';
	scan_table[26]='[';
	scan_table[27]=']';
	scan_table[28]='\n';
	scan_table[29]='0';
	scan_table[30]='a';
	scan_table[31]='s';
	scan_table[32]='d';
	scan_table[33]='f';
	scan_table[34]='g';
	scan_table[35]='h';
	scan_table[36]='j';
	scan_table[37]='k';
	scan_table[38]='l';
	scan_table[39]=';';
	scan_table[40]='`';
	scan_table[41]='~';
	scan_table[42]='s';
	scan_table[43]='\\';
	scan_table[44]='z';
	scan_table[45]='x';
	scan_table[46]='c';
	scan_table[47]='v';
	scan_table[48]='b';
	scan_table[49]='n';
	scan_table[50]='m';
	scan_table[51]=',';
	scan_table[52]='.';
	scan_table[53]='/';
	scan_table[54]='0';
	scan_table[55]='0';
	scan_table[56]='0';
	scan_table[57]=' ';
	scan_table[58]='0';
	scan_table[59]='0';
	scan_table[60]='0';
	scan_table[61]='0';
	scan_table[62]='0';
	scan_table[63]='0';
	scan_table[64]='0';
	scan_table[65]='0';
	scan_table[66]='0';
	scan_table[67]='0';
	scan_table[68]='0';
	scan_table[69]='0';
	scan_table[70]='0';
	scan_table[71]='0';
	scan_table[72]='0';
	scan_table[73]='0';
	scan_table[74]='0';
	scan_table[75]='0';
	scan_table[76]='0';
	scan_table[77]='0';
	scan_table[78]='0';
	scan_table[79]='0';
	scan_table[80]='0';
	scan_table[81]='0';
	scan_table[82]='!';
	scan_table[83]='@';
	scan_table[84]='№';
	scan_table[85]='$';
	scan_table[86]='%';
	scan_table[87]='^';
	scan_table[88]='&';
	scan_table[89]='*';
	scan_table[90]='(';
	scan_table[91]=')';
	scan_table[92]='_';
	scan_table[93]='+';
	scan_table[94]='0';
	scan_table[95]='\t';
	scan_table[96]='Q';
	scan_table[97]='W';
	scan_table[98]='E';
	scan_table[99]='R';
	scan_table[100]='T';
	scan_table[101]='Y';
	scan_table[102]='U';
	scan_table[103]='I';
	scan_table[104]='O';
	scan_table[105]='P';
	scan_table[106]='{';
	scan_table[107]='}';
	scan_table[108]='\n';
	scan_table[109]='0';
	scan_table[110]='A';
	scan_table[111]='S';
	scan_table[112]='D';
	scan_table[113]='F';
	scan_table[114]='G';
	scan_table[115]='H';
	scan_table[116]='J';
	scan_table[117]='K';
	scan_table[118]='L';
	scan_table[119]=':';
	scan_table[120]='"';
	scan_table[121]='`';
	scan_table[122]='S';
	scan_table[123]='|';
	scan_table[124]='Z';
	scan_table[125]='X';
	scan_table[126]='C';
	scan_table[127]='V';
	scan_table[128]='B';
	scan_table[129]='N';
	scan_table[130]='M';
	scan_table[131]='<';
	scan_table[132]='>';
	scan_table[133]='?';
	char strok[26][80];
	if(big&&scan_code!=28&&scan_code!=14&&scan_code!=42&&scan_code!=54&&scan_code!=58&&scan_code!=0x48&&scan_code!=0x50&&scan_code!=0x4D&&scan_code!=0x4B)scan_code+=80;
	switch(scan_code){
		case 28:  //enter
		if(len==0)strok[mem][0]='\0';
		if (amount!=mem){
			if(parsing(strok[amount]))posY++;
			}else{
			if(parsing(strok[mem]))posY++;
		}
		FinColour = colour;
		SpaceStr();
		len=0;
		runner = len;	
		cur++;
		mem++;
		mem%=25;
		amount = mem;
		//posY++;
		break;
		case 14: //  backspace
		if(Cflag)CheckColour(strok[mem][0], strok[mem][1], len-2);
		if(runner>0){
			for(int i = 0, j = 0; i<len+1;i++){
				if(i != runner-1){
					strok[mem][j] = strok[amount][i];
					j++;
				}
			}
			if(Cflag)CheckColour(strok[mem][0], strok[mem][1], len-2);
			runner--;
			SpaceStr();
			out_str(colour,strok[mem]);
			amount = mem;
		}
		break;
		case 42: //Lshift
		case 54: //Rshift
		case 58: //Caps Lock
		big=!big;
		break;
		case 0x48: //up
		if(amount>0)amount--;
		else if (cur>24)amount = 24;
		out_str(colour,strok[amount]);
		SpaceStr();
		if(Cflag)CheckColour(strok[amount][0], strok[amount][1], len-1);
		out_str(colour,strok[amount]);
		runner = len;
		break;
		case 0x50: //down
		if(amount <= cur){
			amount++;
			amount%=25;
		}
		out_str(colour,strok[amount]);
		SpaceStr();
		if(Cflag)CheckColour(strok[amount][0], strok[amount][1], len-1);
		out_str(colour,strok[amount]);
		runner = len;
		break;
		case 0x4D: //right
		if(runner<len)runner++;
		break;
		case 0x4B: //left
		if(runner>0)runner--;
		break;
		default:
		if(len<40){
			if(Cflag)CheckColour(strok[mem][0],scan_table[scan_code], len);
			if(amount!=mem)CopyOneToAntother(strok[amount],strok[mem]);
			if(len==runner){
				strok[mem][runner] = scan_table[scan_code];
				runner++;
				strok[mem][runner] = '\0';
				out_str(colour,strok[amount]);
			}
			else {
				move(strok[mem],runner-1);
				strok[mem][runner]=scan_table[scan_code];
				runner++;
				out_str(colour,strok[mem]);
			}
			amount = mem;
		}
		break;
	}
	cursor_moveto(posY,runner);
}
void keyb_process_keys()
{
	// Проверка что буфер PS/2 клавиатуры не пуст (младший бит присутствует)
	if (inb(0x64) & 0x01)
	{
		unsigned char scan_code;
		unsigned char state;
		scan_code = inb(0x60); // Считывание символа с PS/2 клавиатуры
		if (scan_code < 128) // Скан-коды выше 128 - это отпускание клавиши
		{on_key(scan_code);}
		else if ( scan_code==170||scan_code==182){
			big=!big;
		}
	}
}
__declspec(naked) void keyb_handler()
{
	__asm pusha;
	// Обработка поступивших данных
	keyb_process_keys();
	// Отправка контроллеру 8259 нотификации о том, что прерывание обработано. Если не отправлять нотификацию, то контроллер не будет посылать новых сигналов о прерываниях до тех пор, пока ему не сообщать что прерывание обработано.
	outb(PIC1_PORT, 0x20);
	__asm {
		popa
		iretd
	}
}
__declspec(naked) void ticks_handler()
{
	__asm pusha;
	// Обработка поступивших данных
	AmountOfTicks++;
	// Отправка контроллеру 8259 нотификации о том, что прерывание обработано. Если не отправлять нотификацию, то контроллер не будет посылать новых сигналов о прерываниях до тех пор, пока ему не сообщать что прерывание обработано.
	outb(PIC1_PORT, 0x20);
	__asm {
		popa
		iretd
	}
}
void ticks_init()
{
	// Регистрация обработчика прерывания
	intr_reg_handler(0x08, GDT_CS, 0x80 | IDT_TYPE_INTR, ticks_handler);
	outb(PIC1_PORT + 1, 0xFF ^ 0x02 ^ 0x01); // 0xFF - все прерывания, 0x02 - бит IRQ1 (клавиатура).
}
void keyb_init()
{
	// Регистрация обработчика прерывания
	intr_reg_handler(0x09, GDT_CS, 0x80 | IDT_TYPE_INTR, keyb_handler); // segm_sel=0x8, P=1, DPL=0, Type=Intr
	// Разрешение только прерываний клавиатуры от контроллера 8259
	outb(PIC1_PORT + 1, 0xFF ^ 0x02); // 0xFF - все прерывания, 0x02 - бит IRQ1 (клавиатура).
	// Разрешены будут только прерывания, чьи биты установлены в 0
}
// Базовый порт управления курсором текстового экрана. Подходит для большинства, но может отличаться в других BIOS и в общем случае адрес должен быть прочитан из BIOS data area.
#define CURSOR_PORT (0x3D4)
#define VIDEO_WIDTH (80) // Ширина текстового экрана
// Функция переводит курсор на строку strnum (0 – самая верхняя) в позицию pos на этой строке (0 – самое левое положение).
void cursor_moveto(unsigned int strnum, unsigned int pos)
{
	unsigned short new_pos = (strnum * VIDEO_WIDTH) + pos;
	outb(CURSOR_PORT, 0x0F);
	outb(CURSOR_PORT + 1, (unsigned char)(new_pos & 0xFF));
	outb(CURSOR_PORT, 0x0E);
	outb(CURSOR_PORT + 1, (unsigned char)( (new_pos >> 8) & 0xFF));
}
void OutCpu(int a[4]){
	char out[12];
	int size=1;
	//for(;a[0];size++)a[2]/=10;
	for(int i = 3; i>=0;i--){
		out[i]=a[2]%100;
		a[2]/=100;
	}
	out_str(colour,out);
	posY++;
}
extern "C" int kmain()
{
	intr_disable();
	intr_init();
	keyb_init();
	intr_start();
	intr_enable();
	ticks_init();
	clean();
	switch ((*(int*)0x8800 - 48)){
		case 1:
		colour = 0x01;
		break;
		case 2:
		colour = 0x02;
		break;
		case 3:
		colour = 0x03;
		break;
		case 4:
		colour = 0x04;
		break;
		case 5:
		colour = 0x05;
		break;
		case 6:
		colour = 0x06;
		break;
		case 7:
		colour = 0x07;
		break;
		case 8:
		colour = 0x0E;
		break;
		case 9:
		colour = 0x0F;
		break;
		default:
		colour = 0x02;
		break;
	}
	out_str(colour,"Welcome to InfoOS");
			posY++;
	cursor_moveto(posY, 0);
	//OutCpu(a);
	while (1)
	{
		__asm hlt;
	}
	return 0;
}																																																															

