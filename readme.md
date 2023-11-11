# risc v simulater with intgrated cache
steps to run code:
- add the code t0 run in editor.txt 
- run assemble.cpp
- the assembled code gets stored in machinecode.txt
- run simulator.cpp

### Instructions supported:
instruction-->format
```
add     add x1,x2,x3
sub     sub x1,x2,x3
and     and x1,x2,x3
or      or x1,x2,x3
addi    addi x1,x2,1
lw      lw x1,x4,4
sw      sw x2,4,x3
beq     beq x1,x2,label
bge     blt x1,x2,label
blt     bge x1,x2,label
labels for branch also supported
```
### code tested on
- prime
```
addi s1,x0,7
addi t0,x0,2
loop:
bge t0,s1,prime
add t1,s1,x0
continued_sub:
sub t1,t1,t0
blt x0,t1,continued_sub
beq t1,x0,not_prime
addi t0,t0,1
beq x0,x0,loop
prime:
addi s2,x0,1
beq x0,x0,not_prime
not_prime:
```
- gcd and lcm
```
#code for gcd lcm
addi x1,x0,8
addi x2,x0,2
blt x1,x2,x1_lt_x2
add s1,x1,x0
add s2,x2,x0
beq x0,x0,compare_exit
x1_lt_x2:
add s1,x2,x0
add s2,x1,x0
compare_exit:
add t0,s1,x0
add t1,s2,x0
gcd_loop:
sub t0,t0,t1
blt t1,t0,gcd_loop
beq t0,x0,gcd_exit
add t2,t1,x0
add t1,t0,x0
add t0,t2,x0
beq x0,x0,gcd_loop
gcd_exit:
add t2,x0,x0
loop:
add t2,t2,x1
addi x2,x2,-1
blt x0,x2,loop
addi t3,x0,1
loop2:
sub t2,t2,t1
addi t3,t3,1
blt t1,t2,loop2
add x0,x0,x0
```
- factorial
```
addi s1,x0,1
addi s2,x0,7
addi x3,x0,2
fact_loop:
add x2,x3,x0
loop:
add t0,t0,s1
addi x2,x2,-1
blt x0,x2,loop
addi x3,x3,1
add s1,t0,x0
add t0,x0,x0
bge s2,x3,fact_loop
```
