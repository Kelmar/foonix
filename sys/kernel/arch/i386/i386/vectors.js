// There isn't much need to generate this over and over, but I've left this file here just in case.

console.log("# Generated from vectors.js");
console.log();
console.log(".globl isr_stub");
console.log();

for (var i = 0; i < 256; ++i)
{
    console.log(".globl vector" + i);
    console.log(`vector${i}:`);
    if (!(i == 8 || (i >= 10 && i <= 14) || i == 17))
        console.log("    pushl $0");
    console.log("    pushl $" + i);
    console.log("    jmp   isr_stub")
    console.log();
}

console.log("# Table of all vectors for ease of use.");
console.log(".data");
console.log(".globl vectors");
console.log("vectors:");

for (var i = 0; i < 256; ++i)
    console.log("    .long vector" + i);
