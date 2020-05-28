#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "type.h"
#include "tree.h"
#include "list.h"
#include "hashtab.h"
#include "array.h"
#include "bigint.h"

typedef struct array_value {
    value_t value;
    _Bool exist;
} array_value;

typedef struct array_stack {
    size_t begin;
    size_t end;
    size_t top;
} array_stack;

typedef struct Array {
    vtype_t type;
    array_value *buffer;
    array_stack stack;
    size_t size;
} Array;

static void _print_node_array(Array *array, size_t index);
static void _set_node_array(Array *array, size_t index, void *value);
static _Bool _cmp_node_array(Array *x, Array *y, size_t ix, size_t iy);
static void _free_array(Array *array);
static void _free_node_array(Array *array, size_t index);

extern Array *new_array(size_t size, vtype_t type) {
    switch(type) {
        case DECIMAL_ELEM: 
        case REAL_ELEM: 
        case STRING_ELEM: 
        case LIST_ELEM: 
        case TREE_ELEM: 
        case HASHTAB_ELEM: 
        case ARRAY_ELEM:
        case BIGINT_ELEM:
            break;
        default:
            fprintf(stderr, "%s\n", "type not supported");
            return NULL;
    }
    Array *array = (Array*)malloc(sizeof(Array));
    array->buffer = (array_value*)malloc(size * sizeof(array_value));
    memset(array->buffer, 0, size * sizeof(array_value));
    array->stack.top = 0;
    array->stack.begin = 0;
    array->stack.end = size;
    array->size = size;
    array->type = type;
    return array;
};

extern value_t get_array(Array *array, size_t index) {
    if (index >= array->size) {
        fprintf(stderr, "%s\n", "array overflow");
        value_t none;
        return none;
    }
    return array->buffer[index].value;
}

extern int8_t set_array(Array *array, size_t index, void *value) {
    if (index >= array->size) {
        fprintf(stderr, "%s\n", "array overflow");
        return 1;
    }
    _set_node_array(array, index, value);
    return 0;
}

extern int8_t del_array(Array *array, size_t index) {
    if (index >= array->size) {
        fprintf(stderr, "%s\n", "array overflow");
        return 1;
    }
    _free_node_array(array, index);
    array->buffer[index].exist = 0;
    return 0;
}

extern int32_t in_array(Array *array, void *value) {
    for (size_t index = 0; index < array->size; ++index) {
        if (!array->buffer[index].exist) {
            continue;
        }
        _Bool flag = 0;
        switch(array->type) {
            case DECIMAL_ELEM:
                flag = (int32_t)(intptr_t)value == array->buffer[index].value.decimal;
            break;
            case REAL_ELEM:
                flag = *(double*)value == array->buffer[index].value.real;
                if(flag) {
                    free((double*)value);
                }
            break;
            case STRING_ELEM:
                flag = strcmp((uint8_t*)value, array->buffer[index].value.string) == 0;
            break;
            case LIST_ELEM:
                flag = cmp_list((List*)value, array->buffer[index].value.list) == 0;
            break;
            case TREE_ELEM:
                flag = cmp_tree((Tree*)value, array->buffer[index].value.tree) == 0;
            break;
            case HASHTAB_ELEM:
                flag = cmp_hashtab((HashTab*)value, array->buffer[index].value.hashtab) == 0;
            break;
            case ARRAY_ELEM:
                flag = cmp_array((Array*)value, array->buffer[index].value.array) == 0;
            break;
            case BIGINT_ELEM:
                flag = cmp_bigint((BigInt*)value, array->buffer[index].value.bigint) == 0;
            break;
        }
        if (flag) {
            return index;
        }
    }
    if(array->type == REAL_ELEM) {
        free((double*)value);
    }
    return -1;
}

extern int8_t cmp_stack(Array *x, Array *y) {
    if (x->type != y->type) {
        return -1;
    }
    if (x->stack.end - x->stack.begin != y->stack.end - y->stack.begin) {
        return -2;
    }
    if (x->stack.top - x->stack.begin != y->stack.top - y->stack.begin) {
        return -3;
    }
    size_t i = x->stack.begin;
    size_t j = y->stack.begin;
    for (; i < x->stack.top; ++i, ++j) {
        if (!_cmp_node_array(x, y, i, j)) {
            return 1;
        }
    }
    return 0;
}

extern int8_t cmp_array(Array *x, Array *y) {
    if (x->type != y->type) {
        return -1;
    }
    if (x->size != y->size) {
        return -2;
    }
    for (size_t i = 0; i < x->size; ++i) {
        if (!x->buffer[i].exist && !y->buffer[i].exist) {
            continue;
        }
        if (!x->buffer[i].exist || !y->buffer[i].exist) {
            return 2;
        }
        if (!_cmp_node_array(x, y, i, i)) {
            return 1;
        }
    }
    return 0;
}

extern size_t size_stack(Array *array) {
    return array->stack.top - array->stack.begin;
}

extern int8_t set_stack(Array *array, size_t top, size_t begin, size_t end) {
    if (end > array->size) {
        return 1;
    }
    if (begin > end) {
        return 2;
    }
    if (top > end || top < begin) {
        return 3;
    }
    array->stack.begin = begin;
    array->stack.end = end;
    array->stack.top = top;
    return 0;
}

extern size_t size_array(Array *array) {
    return array->size;
}

extern size_t sizeof_array(void) {
    return sizeof(Array);
}

extern void print_stack(Array *array) {
    printf("#S[ ");
    for (size_t i = array->stack.begin; i < array->stack.top; ++i) {
        _print_node_array(array, i);
        putchar(' ');
    }
    putchar(']');
}

extern void println_stack(Array *array) {
    print_stack(array);
    putchar('\n');
}

extern void print_array(Array *array) {
    printf("#A[ ");
    for (size_t i = 0; i < array->size; ++i) {
        if (!array->buffer[i].exist) {
            continue;
        }
        printf("(%d :: ", i);
        _print_node_array(array, i);
        printf(") ");
    }
    putchar(']');
}

extern void println_array(Array *array) {
    print_array(array);
    putchar('\n');
}

extern int8_t push_stack(Array *array, void *value) {
    if (array->stack.top == array->stack.end) {
        fprintf(stderr, "%s\n", "stack overflow");
        return 1;
    }
    _set_node_array(array, array->stack.top, value);
    array->stack.top += 1;
    return 0;
}

extern value_t pop_stack(Array *array) {
    if (array->stack.top == array->stack.begin) {
        fprintf(stderr, "%s\n", "stack overflow");
        value_t none;
        return none;
    }
    return array->buffer[--array->stack.top].value;
}

extern void free_array(Array *array) {
    switch(array->type) {
        case LIST_ELEM: case TREE_ELEM: case HASHTAB_ELEM: case ARRAY_ELEM: case BIGINT_ELEM:
            _free_array(array);
        break;
    }
    free(array->buffer);
    free(array);
}

static void _print_node_array(Array *array, size_t index) {
    switch(array->type) {
        case DECIMAL_ELEM:
            printf("%d", array->buffer[index].value.decimal);
        break;
        case REAL_ELEM:
            printf("%lf", array->buffer[index].value.real);
        break;
        case STRING_ELEM:
            printf("'%s'", array->buffer[index].value.string);
        break;
        case LIST_ELEM:
            print_list(array->buffer[index].value.list);
        break;
        case TREE_ELEM:
            print_tree(array->buffer[index].value.tree);
        break;
        case HASHTAB_ELEM:
            print_hashtab(array->buffer[index].value.hashtab);
        break;
        case ARRAY_ELEM:
            print_array(array->buffer[index].value.array);
        break;
        case BIGINT_ELEM:
            print_bigint(array->buffer[index].value.bigint);
        break;
    }
}

static void _set_node_array(Array *array, size_t index, void *value) {
    if (array->buffer[index].exist) {
        _free_node_array(array, index);
    }
    switch(array->type) {
        case DECIMAL_ELEM:
            array->buffer[index].value.decimal = (int32_t)(intptr_t)value;
        break;
        case REAL_ELEM:
            array->buffer[index].value.real = *(double*)value;
            free((double*)value);
        break;
        case STRING_ELEM:
            array->buffer[index].value.string = (uint8_t*)value;
        break;
        case LIST_ELEM:
            array->buffer[index].value.list = (struct List*)value;
        break;
        case TREE_ELEM:
            array->buffer[index].value.tree = (struct Tree*)value;
        break;
        case HASHTAB_ELEM:
            array->buffer[index].value.hashtab = (struct HashTab*)value;
        break;
        case ARRAY_ELEM:
            array->buffer[index].value.array = (struct Array*)value;
        break;
        case BIGINT_ELEM:
            array->buffer[index].value.bigint = (struct BigInt*)value;
        break;
    }
    array->buffer[index].exist = 1;
}

static _Bool _cmp_node_array(Array *x, Array *y, size_t ix, size_t iy) {
    _Bool flag = 0;
    switch(x->type) {
        case DECIMAL_ELEM:
            flag = x->buffer[ix].value.decimal == y->buffer[iy].value.decimal;
        break;
        case REAL_ELEM:
            flag = x->buffer[ix].value.real == y->buffer[iy].value.real;
        break;
        case STRING_ELEM:
            flag = strcmp(x->buffer[ix].value.string, y->buffer[iy].value.string) == 0;
        break;
        case LIST_ELEM:
            flag = cmp_list(x->buffer[ix].value.list, y->buffer[iy].value.list) == 0;
        break;
        case TREE_ELEM:
            flag = cmp_tree(x->buffer[ix].value.tree, y->buffer[iy].value.tree) == 0;
        break;
        case HASHTAB_ELEM:
            flag = cmp_hashtab(x->buffer[ix].value.hashtab, y->buffer[iy].value.hashtab) == 0;
        break;
        case ARRAY_ELEM:
            flag = cmp_array(x->buffer[ix].value.array, y->buffer[iy].value.array) == 0;
        break;
        case BIGINT_ELEM:
            flag = cmp_bigint(x->buffer[ix].value.bigint, y->buffer[iy].value.bigint) == 0;
        break;
    }
    return flag;
}

static void _free_array(Array *array) {
    for (size_t i = 0; i < array->size; ++i) {
        if (!array->buffer[i].exist) {
            continue;
        }
        _free_node_array(array, i);
    }
}

static void _free_node_array(Array *array, size_t index) {
    switch(array->type) {
        case LIST_ELEM:
            free_list(array->buffer[index].value.list);
        break;
        case TREE_ELEM: 
            free_tree(array->buffer[index].value.tree);
        break;
        case HASHTAB_ELEM: 
            free_hashtab(array->buffer[index].value.hashtab);
        break;
        case ARRAY_ELEM: 
            free_array(array->buffer[index].value.array);
        break;
        case BIGINT_ELEM:
            free_bigint(array->buffer[index].value.bigint);
        break;
    }
}