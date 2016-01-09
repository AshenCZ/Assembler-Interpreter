### Assembler interpreter

I wrote for [Mr. Yaghob's](http://ulita.ms.mff.cuni.cz/~yaghob/) C++ course as a homework. The whole specification can be found on: [this site](http://ulita.ms.mff.cuni.cz/~yaghob/cppdu-asm/) in Czech language.

##### The assembler
It has 3 types of registers - integer Rx, float Fx and bool Px (unwriteable to except with CMP).

```
* LDC R1 = 5 
* ADD R1 = R3,R2 
* CMPGE P5,P6 = R10,R20   // GE, GT, EQ, NE, LE, LT
* ST [R10] = R20
* LD R10 = [R20]
* IN R10 , OUT 10
* CVRT F10 = R10 (and vice versa)
* JMP WHERE
* MOV R10 = 333
```

Every command can be predicated by doing  ```JMP.P1 WHERE.```
You can find examples in the ```assembler_programs/``` folder.

##### The program

Everyone is free to use this code or submit an issue.

It has 2 types of parsing, one is significantly slower done with RegExes and the second one is pure parsing via tokenization, fast but barely readable. The program uses templates, that is why there is no class implementation in ```instruction.cpp```.
