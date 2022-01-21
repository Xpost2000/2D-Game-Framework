// macro sauce dynamic array.
// you don't include the header file while using this!
// all types should be declared at the top! So just include
// the C source file instead for proper usage and use macro sauce

// Other than macro sauce, this is exactly what you think it is,
// so you can kind of glance over this.

// dynamic_array(type_name) s;
// dynamic_array_push(type_name)(self, params);

// although the way this generates means the code resulting an just be written as
// so that's nice too.

// dynamic_array(type_name) s;
// dynamic_array_push_int(s, asdasdda);
#include "macro_helpers.h"

#ifndef type
#define type provide_typename
#error Please define a type!
#endif

#define dynamic_array_reserve(type)               template_fn_macro_concatenate(dynamic_array_reserve,               type)
#define dynamic_array_push(type)                  template_fn_macro_concatenate(dynamic_array_push,                  type)
#define dynamic_array_pop(type)                   template_fn_macro_concatenate(dynamic_array_pop,                   type)
#define dynamic_array_pop_and_retrieve_last(type) template_fn_macro_concatenate(dynamic_array_pop_and_retrieve_last, type)
#define dynamic_array_erase(type)                 template_fn_macro_concatenate(dynamic_array_erase,                 type)
#define dynamic_array_erase_ordered(type)         template_fn_macro_concatenate(dynamic_array_erase_ordered,                 type)
#define dynamic_array_erase_and_retrieve(type)    template_fn_macro_concatenate(dynamic_array_erase_and_retrieve,    type)
#define dynamic_array(type)                       template_fn_macro_concatenate(dynamic_array,                       type)
#define dynamic_array_new(type)                   template_fn_macro_concatenate(dynamic_array_new,                   type)
#define dynamic_array_duplicate(type)             template_fn_macro_concatenate(dynamic_array_duplicate,             type)
#define dynamic_array_copy(type)                  template_fn_macro_concatenate(dynamic_array_copy,                  type)
#define dynamic_array_free(type)                  template_fn_macro_concatenate(dynamic_array_free,                  type)

#include "dynamic_array.c"
