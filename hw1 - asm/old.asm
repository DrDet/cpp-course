                section         .text

                global          _start
_start:

                sub             rsp, 4 * 128 * 8
                lea             rdi, [rsp + 3 * 128 * 8]
                mov             rcx, 128
                call            read_long
	lea	rdi, [rsp + 2 * 128 * 8]
                call            read_long
                lea             rsi, [rsp + 3 * 128 * 8]
	lea	rbx, [rsp + 2 * 128 * 8]
	
	mov	rdi, rsp
	mov	rcx, 2 * 128
	call	set_zero
	
                call            mul_long_long

                call            write_long

                mov             al, 0x0a
                call            write_char

                jmp             exit
	
; multiplication of two long numbers
; rsi -- #1
; rbx -- #2
; rdi -- result
mul_long_long:
                push            rsi
                push            rbx
	push            rcx	
	clc
	
	mov	r8, 0 			;set counter for an outside cycle .. rsi
.loop1:
;{
	mov	r9, 0 			;set counter for an inside cycle .. rbx
.loop2:
;{
	mov	rax, [rsi + 8 * r8]     		;rax <- rsi[i]
	mov	rdx, [rbx + 8 * r9]		;rdx <- rbx[j]
	mul	rdx			;rdx:rax = rsi[i] * rbx[j]
	lea	r10, [r8 + r9]			;r10 = i + j
	
	clc	
	xor	r15, r15			;clear r15
	add	[rdi + 8 * r10], rax		;rdi[i+j] += rax;
	adc	r15, 0			;save carry in r15
	
	inc	r10			;set r10 = i+j+1			
	call	push_carry			;push from i+j+1 to the end
	
	clc
	xor	r15, r15			;upd
	add	[rdi + 8 * r10], rdx		;rdi[i+j+1] += rdx
	adc	r15, 0			;upd
	
	inc	r10			;set r10 = i+j+2
	call	push_carry			;push from i+j+2 to the end

	inc	r9
	cmp 	r9, 128
	jnz	.loop2
	;} end of inside_loop
	
	inc	r8
	cmp	r8, 128
	jnz	.loop1
	;} end of outside_loop
	
                pop             rcx
                pop             rbx
                pop             rsi
                ret
	
;pushes carry in r15
;    from idx in r10 to the end
push_carry:
	push	r10
	push	r15
	
	cmp	r10, 2 * 128
	jz	.skip
.loop:
	clc
	add	[rdi + 8 * r10], r15 		;rdi[i] += carry
	mov	r15, 0			;upd carry
	adc	r15, 0			;upd carry
	
	inc	r10
	cmp	r10, 2 * 128
	jnz	.loop
.skip:	
	pop	r15
	pop	r10
	ret
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; adds 64-bit number to long number
;    rdi -- address of summand #1 (long number)
;    rax -- summand #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    sum is written to rdi
add_long_short:
                push            rdi
                push            rcx
                push            rdx

                xor             rdx,rdx
.loop:
                add             [rdi], rax
                adc             rdx, 0
                mov             rax, rdx
                xor             rdx, rdx
                add             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rdx
                pop             rcx
                pop             rdi
                ret

; multiplies long number by a short
;    rdi -- address of multiplier #1 (long number)
;    rbx -- multiplier #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    product is written to rdi
mul_long_short:
                push            rax
                push            rdi
                push            rcx

                xor             rsi, rsi
.loop:
                mov             rax, [rdi]
                mul             rbx
                add             rax, rsi
                adc             rdx, 0
                mov             [rdi], rax
                add             rdi, 8
                mov             rsi, rdx
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             rdi
                pop             rax
                ret

; divides long number by a short
;    rdi -- address of dividend (long number)
;    rbx -- divisor (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    quotient is written to rdi
;    rdx -- remainder
div_long_short:
                push            rdi
                push            rax
                push            rcx

                lea             rdi, [rdi + 8 * rcx - 8]
                xor             rdx, rdx

.loop:
                mov             rax, [rdi]
                div             rbx
                mov             [rdi], rax
                sub             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             rax
                pop             rdi
                ret

; assigns a zero to long number
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
set_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep stosq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; checks if a long number is a zero
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
; result:
;    ZF=1 if zero
is_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep scasq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; read long number from stdin
;    rdi -- location for output (long number)
;    rcx -- length of long number in qwords
read_long:
                push            rcx
                push            rdi

                call            set_zero
.loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              .done
                cmp             rax, '0'
                jb              .invalid_char
                cmp             rax, '9'
                ja              .invalid_char

                sub             rax, '0'
                mov             rbx, 10
                call            mul_long_short
                call            add_long_short
                jmp             .loop

.done:
                pop             rdi
                pop             rcx
                ret

.invalid_char:
                mov             rsi, invalid_char_msg
                mov             rdx, invalid_char_msg_size
                call            print_string
                call            write_char
                mov             al, 0x0a
                call            write_char

.skip_loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              exit
                jmp             .skip_loop

; write long number to stdout
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
write_long:
                push            rax
                push            rcx

                mov             rax, 20
                mul             rcx
                mov             rbp, rsp
                sub             rsp, rax

                mov             rsi, rbp

.loop:
                mov             rbx, 10
                call            div_long_short
                add             rdx, '0'
                dec             rsi
                mov             [rsi], dl
                call            is_zero
                jnz             .loop

                mov             rdx, rbp
                sub             rdx, rsi
                call            print_string

                mov             rsp, rbp
                pop             rcx
                pop             rax
                ret

; read one char from stdin
; result:
;    rax == -1 if error occurs
;    rax \in [0; 255] if OK
read_char:
                push            rcx
                push            rdi

                sub             rsp, 1
                xor             rax, rax
                xor             rdi, rdi
                mov             rsi, rsp
                mov             rdx, 1
                syscall

                cmp             rax, 1
                jne             .error
                xor             rax, rax
                mov             al, [rsp]
                add             rsp, 1

                pop             rdi
                pop             rcx
                ret
.error:
                mov             rax, -1
                add             rsp, 1
                pop             rdi
                pop             rcx
                ret

; write one char to stdout, errors are ignored
;    al -- char
write_char:
                sub             rsp, 1
                mov             [rsp], al

                mov             rax, 1
                mov             rdi, 1
                mov             rsi, rsp
                mov             rdx, 1
                syscall
                add             rsp, 1
                ret

exit:
                mov             rax, 60
                xor             rdi, rdi
                syscall

; print string to stdout
;    rsi -- string
;    rdx -- size
print_string:
                push            rax

                mov             rax, 1
                mov             rdi, 1
                syscall

                pop             rax
                ret


                section         .rodata
invalid_char_msg:
                db              "Invalid character: "
invalid_char_msg_size: equ             $ - invalid_char_msg