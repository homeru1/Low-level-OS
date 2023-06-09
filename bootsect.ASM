[BITS 16]
[ORG 0x7C00]
start:
mov ax, cs ; ���������� ������ �������� ���� � ax
mov ds, ax  ; ���������� ����� ������ ��� ������ �������� ������
mov ss, ax  ; � �������� �����
mov sp, start ; ���������� ������ ����� ��� ����� ������ ���������� ����� ����. ���� ����� ����� ����� � �� ��������� ���.
;call cls
;cls:
;  pusha
;  mov ah, 0x00
;  mov al, 0x03  ; text mode 80x25 16 colours
;  int 0x10
;  popa
;  ret 
mov ax, 0x1100
mov es, ax
mov bx, 0x0000
mov dl, 1 ; ����� �����
mov dh, 0 ; ����� �������
mov ch, 0 ; ����� �������� (������� 2 ���� cl/ch)
mov cl, 2 ; � ������ �������
mov al, 23 ; ���������� ��������  2E00
mov ah, 0x02 ; �������� 
int 0x13 ; ���������� i/o �����

mov ax, 0x1400
mov es, ax
mov bx, 0x0000
mov dl, 1
mov dh, 0
mov ch, 0
mov cl, 25
mov al, 11 ;1600
mov ah, 0x02
int 0x13
mov ah, 0x02 
int 0x1A
add ch, 3
mov [0x8000], dh ; �������
mov [0x8200], cl ; ������
mov [0x8400], ch ; ����
mov ax, 0xe801
int 0x15
mov [0x8600], dx
pusha
mov ah, 0x00
mov al, 0x03  ; text mode 80x25 16 colours
int 0x10
popa
mov ah, 0x0e
mov al, '1'
int 0x10
mov al, ':'
int 0x10
mov al, 'b'
int 0x10
mov al, 'l'
int 0x10
mov al, 'u'
int 0x10
mov al, 'e'
int 0x10
mov al, 0x0d 
int 0x10
mov al, 0xa
int 0x10
mov al, '2'
int 0x10
mov al, ':'
int 0x10
mov al, 'g'
int 0x10
mov al, 'r'
int 0x10
mov al, 'e'
int 0x10
mov al, 'e'
int 0x10
mov al, 'n'
int 0x10
mov al, 0x0d 
int 0x10
mov al, 0xa
int 0x10
mov al, '3'
int 0x10
mov al, ':'
int 0x10
mov al, 'c'
int 0x10
mov al, 'y'
int 0x10
mov al, 'a'
int 0x10
mov al, 'n'
int 0x10
mov al, 0x0d 
int 0x10
mov al, 0xa
int 0x10
mov al, '4'
int 0x10
mov al, ':'
int 0x10
mov al, 'r'
int 0x10
mov al, 'e'
int 0x10
mov al, 'd'
int 0x10
mov al, 0x0d 
int 0x10
mov al, 0xa
int 0x10
mov al, '5'
int 0x10
mov al, ':'
int 0x10
mov al, 'm'
int 0x10
mov al, 'a'
int 0x10
mov al, 'g'
int 0x10
mov al, 'n'
int 0x10
mov al, 'e'
int 0x10
mov al, 't'
int 0x10
mov al, 'a'
int 0x10
mov al, 0x0d 
int 0x10
mov al, 0xa
int 0x10
mov al, '6'
int 0x10
mov al, ':'
int 0x10
mov al, 'b'
int 0x10
mov al, 'r'
int 0x10
mov al, 'o'
int 0x10
mov al, 'w'
int 0x10
mov al, 'n'
int 0x10
mov al, 0x0d 
int 0x10
mov al, 0xa
int 0x10
mov al, '7'
int 0x10
mov al, ':'
int 0x10
mov al, 'g'
int 0x10
mov al, 'r'
int 0x10
mov al, 'e'
int 0x10
mov al, 'y'
int 0x10
mov al, 0x0d 
int 0x10
mov al, 0xa
int 0x10
mov al, '8'
int 0x10
mov al, ':'
int 0x10
mov al, 'y'
int 0x10
mov al, 'e'
int 0x10
mov al, 'l'
int 0x10
mov al, 'l'
int 0x10
mov al, 'o'
int 0x10
mov al, 'w'
int 0x10
mov al, 0x0d 
int 0x10
mov al, 0xa
int 0x10
mov al, '9'
int 0x10
mov al, ':'
int 0x10
mov al, 'w'
int 0x10
mov al, 'h'
int 0x10
mov al, 'i'
int 0x10
mov al, 't'
int 0x10
mov al, 'e'
int 0x10
mov al, 0x0d 
int 0x10
mov al, 0xa
int 0x10
inf_loop:
mov al, 0x0d 
int 0x10
mov ah, 0
int 0x16
cmp al, 0x0d 
je endi
cmp al, '1'
jl inf_loop
cmp  al, ':'
jle number
jmp inf_loop
number:
mov [0x8800], al
mov ah, 0x0e
int 0x10
jmp inf_loop
endi:
; ���������� ����������
cli
; �������� ������� � ������ ������� ������������
lgdt [gdt_info] ; ��� GNU assembler ������ ���� "lgdt gdt_info"
; ��������� �������� ����� �20
in al, 0x92
or al, 2
out 0x92, al    
; ��������� ���� PE �������� CR0 - ��������� �������� � ���������� �����
mov eax, cr0
or al, 1
mov cr0, eax
jmp 0x8:protected_mode ; "�������" ������� ��� �������� ���������� ���������� � cs (������������� ����������� �� ��������� ����� ������� ��������).
gdt: ;����������� ������� ������������ ������ �������� �� ���� �������:
; ������� ����������
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
; ������� ����: base=0, size=4Gb, P=1, DPL=0, S=1(user),
; Type=1(code), Access=00A, G=1, B=32bit
db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00
; ������� ������: base=0, size=4Gb, P=1, DPL=0, S=1(user),
; Type=0(data), Access=0W0, G=1, B=32bit
db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00
gdt_info:  ; ������ � ������� GDT (������, ��������� � ������)
dw gdt_info - gdt ; ������ ������� (2 �����)
dw gdt, 0 ; 32-������ ���������� ����� �������.
[BITS 32]
protected_mode:
; �������� ���������� ��������� ��� ����� � ������ � ��������
mov ax, 0x10 ; ������������ ���������� � ������� 2 � GDT
mov es, ax
mov ds, ax
mov ss, ax
call 0x11000 ; �������� ���������� ������������ ����
times (512 - ($ - start) - 2) db 0 
db 0x55, 0xAA