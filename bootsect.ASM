[BITS 16]
[ORG 0x7C00]
start:
mov ax, cs ; Сохранение адреса сегмента кода в ax
mov ds, ax  ; Сохранение этого адреса как начало сегмента данных
mov ss, ax  ; И сегмента стека
mov sp, start ; Сохранение адреса стека как адрес первой инструкции этого кода. Стек будет расти вверх и не перекроет код.
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
mov dl, 1 ; Номер диска
mov dh, 0 ; Номер головки
mov ch, 0 ; Номер цилиндра (старшие 2 бита cl/ch)
mov cl, 2 ; С какого сектора
mov al, 23 ; Количество секторов  2E00
mov ah, 0x02 ; Параметр 
int 0x13 ; прерывание i/o диска

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
mov [0x8000], dh ; секунды
mov [0x8200], cl ; минуты
mov [0x8400], ch ; часы
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
; Отключение прерываний
cli
; Загрузка размера и адреса таблицы дескрипторов
lgdt [gdt_info] ; Для GNU assembler должно быть "lgdt gdt_info"
; Включение адресной линии А20
in al, 0x92
or al, 2
out 0x92, al    
; Установка бита PE регистра CR0 - процессор перейдет в защищенный режим
mov eax, cr0
or al, 1
mov cr0, eax
jmp 0x8:protected_mode ; "Дальний" переход для загрузки корректной информации в cs (архитектурные особенности не позволяют этого сделать напрямую).
gdt: ;Минимальная таблица дескрипторов должна состоять из трех записей:
; Нулевой дескриптор
db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
; Сегмент кода: base=0, size=4Gb, P=1, DPL=0, S=1(user),
; Type=1(code), Access=00A, G=1, B=32bit
db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9A, 0xCF, 0x00
; Сегмент данных: base=0, size=4Gb, P=1, DPL=0, S=1(user),
; Type=0(data), Access=0W0, G=1, B=32bit
db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xCF, 0x00
gdt_info:  ; Данные о таблице GDT (размер, положение в памяти)
dw gdt_info - gdt ; Размер таблицы (2 байта)
dw gdt, 0 ; 32-битный физический адрес таблицы.
[BITS 32]
protected_mode:
; Загрузка селекторов сегментов для стека и данных в регистры
mov ax, 0x10 ; Используется дескриптор с номером 2 в GDT
mov es, ax
mov ds, ax
mov ss, ax
call 0x11000 ; Передача управления загруженному ядру
times (512 - ($ - start) - 2) db 0 
db 0x55, 0xAA