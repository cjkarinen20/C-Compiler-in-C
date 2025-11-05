#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h> 
#include <sys/stat.h>


// --- Token Definitions --- 
// int (A keyword)
// main (An identifier, whose value is “main”)
// (
// void (A keyword)
// )
// {
// return (A keyword)
// 2 (A constant, whose value is “2”)
// ; 
// }

// ---Regular Expressions---
// Identifier [a-zA-Z_]\w*\b
// Constant [0-9]+\b
// int keyword int\b
// void keyword void\b
// return keyword return\b
// Open parenthesis \(
// Close parenthesis \)
// Open brace {
// Close brace }
// Semicolon ;