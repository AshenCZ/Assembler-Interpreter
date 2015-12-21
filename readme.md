### Assembler interpreter

I wrote for Mr. Yaghob's C++ course as homework. The instructions are:

It has 3 types of registers - integer Rx, float Fx and bool Px (unwriteable to except with CMP).

```
* LDC R1 = 5 
* ADD R1 = R3,R2 
* CMPGE P1,P2 = R10,R20
* ST [R10] = R20
* LD R10 = [R20]
* IN R10 , OUT 10
* CVRT F10 = R10 (and vice versa)
* JMP WHERE
```

Every command can be predicated by doing  ```JMP.P1 WHERE.```
You can find examples in the ```assembler_programs/``` folder.
