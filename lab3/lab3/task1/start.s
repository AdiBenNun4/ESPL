WRITE EQU 4
STDOUT EQU 1
STDIN  EQU 0

section .data
encodedChar: db 0
msg db 'Hello', 0xa
outfile: dd STDOUT
infile: dd STDIN

section .text
global _start
global system_call
extern strlen


_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call   main       ; int main( int argc, char *argv[], char *envp[] )
    call encoder

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller
 
BUFSZ EQU 1000
global main
section .data
nl:  db 10,0
data_ptr: dd 0
section .text
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
mov esp, ebp
pop ebp
ret

encoder:
mov eax,3
mov ebx,[infile]
mov ecx, encodedChar
mov edx, 1
int 0x80
cmp eax,0
jz doneEncoding
mov ecx, encodedChar
cmp byte[encodedChar], 0
jz doneEncoding
call encodeByte
mov ecx, encodedChar ; Get first argument
mov edx, 1 ; Count of bytes
mov ebx, [outfile]
mov eax, WRITE
int 0x80
jmp encoder
doneEncoding: ret

encodeByte:
cmp byte[ecx], 'A'
jl doneEncodeByte
cmp byte[ecx], 'z'
jg doneEncodeByte
inc byte [ecx]
cmp byte[ecx], 91
je makeA
cmp byte[ecx], 123
je makea
doneEncodeByte: ret

makeA: mov byte[encodedChar], 65
ret
makea:mov byte[encodedChar], 97
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
cmp byte[ecx], 'i'
jne skipIn
inc ecx                       ;advance in one memory block
mov eax,5
mov ebx,ecx
mov ecx,2|64
mov edx,0777
int 0x80
mov [infile], eax
skipIn: ret

checkOutput:
cmp byte[ecx], '-'
jne skipOut
inc ecx                       ;advance in one memory block
cmp byte[ecx], 'o'
jne skipOut
inc ecx                       ;advance in one memory block
mov eax,5
mov ebx,ecx
mov ecx,2|64
mov edx,0777
int 0x80
mov [outfile], eax
skipOut: ret


global my_puts ; void my_puts(char *p);
section .text
my_puts: push ebp
mov ebp, esp
pushad
mov ecx, [ebp+8] ; Get first argument p
call checkFlag
mov ecx, [ebp+8]
call my_strlen
mov ecx, [ebp+8] ; Get first argument
mov edx, eax ; Count of bytes
mov ebx, [outfile]
mov eax, WRITE
int 0x80 ; Linux system call
popad
mov esp, ebp
pop ebp
ret
my_strlen: mov eax,1
cont: cmp byte [ecx], 0
jz done
inc ecx
inc eax
jmp cont
done: ret