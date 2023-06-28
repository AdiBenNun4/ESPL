WRITE EQU 4
STDOUT EQU 1
STDIN  EQU 0

section .data
nl:  db 10,0
charBuff: times 2 db 0,0
index: dd 0
data_ptr: dd 0
msg db 'Not implemented succesfully', 0xa
format db "%02hhx",0
formatString db "%s",0
formatDec db "%d",0
formatU db "%u", 0   ; Format string for printf
modeRead db "r", 0
flag: db 0, 0
greaterFlag db 0
flagLastRead db 0
min db 0
max db 0
x_struct: db 5
x_num: db 0xaa, 0x01, 0x02, 0x44,0x4f
y_struct: db 6
y_num: db  0xaa,0x01, 0x02, 0x03, 0x44,0x4f
z_struct: db 0
z_num: times 255 db 0



section .text
    extern strlen
    extern printf
    extern fgets
    extern stdin
    extern strtol
    extern srand
    extern rand
    BUFSZ EQU 1000
    global main

main: push ebp
mov ebp, esp
push ebx
mov ecx, [ebp+8] ; Get first argument ac
mov edx, [ebp+12] ; Get 2nd argument av
mov ebx, 0
Next: mov eax, [edx+ebx*4]
push eax
call my_puts
add esp, 4
mov eax,nl
push eax
call my_puts 
add esp, 4
inc ebx
cmp ebx, ecx
jnz Next
push dword BUFSZ
add esp, 4
mov [data_ptr], eax
pop ebx
cmp byte[flag],0
jz addNprint

cmp byte[flag],1
jz readInput

push formatString
push msg
call printf
add esp, 8
jmp addNprint

readInput:
mov byte[x_struct],0
mov byte[y_struct],0
call readStructX
call readStructY

addNprint:
mov eax, [x_struct]
mov ebx, [y_struct]
call maxMin

mov [z_struct], al
push x_struct
call print_multi ; might need add esp, 4
add esp, 4
push y_struct
call print_multi
add esp, 4
call add_multi
push z_struct
call print_multi
add esp, 4
mov esp, ebp
pop ebp
ret

global my_puts ; void my_puts(char *p);
my_puts: push ebp
mov ebp, esp
pushad
mov ecx, [ebp+8] ; Get first argument p
call checkFlag
popad
mov esp, ebp
pop ebp
ret

call my_strlen
mov ecx, [ebp+8] ; Get first argument
mov edx, eax ; Count of bytes
mov ebx, 1
mov eax, WRITE
int 0x80 ; Linux system call
popad
mov esp, ebp
pop ebp
ret

my_strlen: mov eax,1
cont: cmp byte [ecx], 0
jz doneLen
inc ecx
inc eax
jmp cont
doneLen: ret

print_multi:
    push ebp
    mov ebp, esp
    pushad
                       ;mov esi,0
    mov eax,[ebp+8]
    mov edi,[eax]
    movzx ebx, byte [eax]
    printIter:
    mov ecx,[eax+ebx]
    cmp ebx,0
    jz donePrintIter
    call printCurr
    dec ebx
    jmp printIter
    donePrintIter: 
    push formatString
    push nl
    call printf
    add esp,8
    popad
    mov esp, ebp
    pop ebp
    ret

printCurr:
    pushad
    push ecx
    push format
    call printf
    add esp, 8
    popad
    ret

readStructX:
    pushad
    mov edi, 0
    mov byte[x_struct], 0
    startReadX:call readChar
    cmp byte[charBuff], 10
    jz finishStructReadX
    cmp byte[charBuff+1], 10
    jnz contStructReadX
    mov byte[flagLastRead],1

    contStructReadX:push 16
    push ecx 
    push charBuff
    call strtol
    add esp, 12
    
    mov ebx, eax
    lea esi, [x_num + edi]
    mov [esi], bl
    inc edi
    inc byte[x_struct]

    cmp byte[flagLastRead], 1
    jnz startReadX
    finishStructReadX:popad
    ret

readChar:
    push dword [stdin]
    push 3; size to read- too big should be fine
    push charBuff ;str pointer to fill with the characters read
    call fgets
    add esp, 12 ;should be proprtional to values pushed x4
    ret

readStructY:
    pushad
    mov edi, 0
    mov byte[y_struct], 0
    startReadY:call readChar
    cmp byte[charBuff], 10
    jz finishStructReadY
    cmp byte[charBuff+1], 10
    jnz contStructReadY
    mov byte[flagLastRead],2

    contStructReadY:push 16
    push ecx 
    push charBuff
    call strtol
    add esp, 12
    
    mov ebx, eax
    lea esi, [y_num + edi]
    mov [esi], bl
    inc edi
    inc byte[y_struct]

    cmp byte[flagLastRead], 2
    jnz startReadY
    finishStructReadY:popad
    ret

maxMin:

cmp al, bl
jge doneMinMaxing
mov ecx, eax
mov eax, ebx
mov ebx, ecx
inc byte[greaterFlag]
doneMinMaxing:
mov [max], al
mov [min], bl
ret

add_multi: pushad   ; pusha, make sure is in dwords 
    mov     edi, 0            ; set index to 0                                       
    and     cl, cl               ; clear CF 
    do_rep:     
    lea eax, [x_num+edi]
    lea ebx, [y_num+edi]
    mov ecx, [eax]
    cmp byte[min], 0
    jle checkGreater
    
    mov edx, [ebx]
    adc     ecx, edx 
    jmp contMulti

    checkGreater: cmp byte[greaterFlag], 0
    jz contMulti
    mov ecx, [ebx]

    contMulti:mov [z_num+edi], ecx ;stores the result back into memory              
    inc       edi      ; next item. CF unchanged!  
    dec byte[min]                
    dec      byte[max]     ; are we done?
    cmp byte[max],0
    jnz do_rep
    popad                 
    ret



checkFlag:
call checkInput
mov ecx, [ebp+8] 
call checkOutput
ret

checkInput:
cmp byte[ecx], '-' ; should represent minus -
jne skipIn
inc ecx                       ;advance in one memory block
cmp byte[ecx], 'I'
jne skipIn
inc ecx                       ;advance in one memory block
mov byte[flag], 1
skipIn: ret

checkOutput:
cmp byte[ecx], '-'
jne skipOut
inc ecx                       ;advance in one memory block
cmp byte[ecx], 'R'
jne skipOut
inc ecx                       ;advance in one memory block
mov byte[flag], 2
skipOut: ret

lfsr_fib:
    push ebp
    mov ebp, esp
    mov eax, 0xAC75     ; Start state
    mov ebx, eax        
    mov ecx, 0          ; Bit counter
    xor edx, edx        ; Period = 0

lfsr_loop:
    mov esi, eax        
    shr esi, 1         
    xor eax, eax        
    movzx edi, bx       ; Zero-extend LFSR to EDI

    shr edi, 5         
    xor edi, esi        
    shr esi, 2          
    xor edi, esi        
    shr esi, 1          
    xor edi, esi        
    and edi, 1          ; Get the least significant bit

    shl edi, 15         
    or esi, edi         ; Set the leftmost bit of LFSR
    mov ebx, esi        ; Update LFSR
    inc edx             ; Increment period
    cmp ebx, eax        ; Compare LFSR with start_state, repeat if not equal
    jne lfsr_loop       

    mov eax, edx        ; Move result to EAX for return
    leave               
    ret             

    