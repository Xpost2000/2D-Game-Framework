// For cheap "template" type metaprogramming in C.
// I do not wish to write a preprocessor, and while I abhor macros

// For dynamic generic data structures (arrays basically)
// I don't wish to constantly rewrite them.

// Simple fixed size vectors I'm fine with rewriting since they don't take more than 10 minutes each
// but these take a little longer since they're a bit easier to mess up, and pointlessly use code.

// Basically this file only exists to facilitate dynamic arrays
// hashmaps are always going to be fixed and I'm not letting them grow generally unless it's a very specific circumstance.

#ifndef MACRO_HELPERS_H
#define MACRO_HELPERS_H

#define template_fn_macro_concatenate(a, b) a##_##b
#define template_fn_name(name, type) template_fn_macro_concatenate(name, type)
#define template_fn(name) template_fn_name(name, type)
#define stringify(x) #x
#define macro_stringify(x) stringify(x)
#define tokenize(x) x
#define macro_tokenize(x) tokenize(x)

#endif
