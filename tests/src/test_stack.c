#define REX_IMPLEMENTATION
#include "rex.h"
#include <stdio.h>

static int
test_stack_uniform(
    rex_stack_t *stack,
    char * ctx
){
    int64_t i;
    int r = 0;
    uint64_t *val;
    for (i = 0; rex_stack_push(stack, &i, sizeof(int64_t)); i++);
    r = stack->top != stack->ceil;
    printf("Uniform %s Prevents Overflow: %s\n", ctx, !r ? "SUCCESS" : "FAIL");
    i--;
    for (; 
        (val =rex_stack_peek(stack, sizeof(int64_t))); 
        rex_stack_pop(stack, sizeof(int64_t)), i--
    )
        if ((*(int64_t*)val) != i)
        {
            printf("Uniform %s Data Uncorrupted: FAIL\n", ctx);
            r |= 1;
            break; 
        }
    r |= !(i<0);
    printf("Uniform %s Data Uncorrupted: %s\n", ctx, i<0? "SUCCESS" : "FAIL");

    r |= !(stack->top == stack->floor);
    printf("Uniform %s Prevents Underflow: %s\n", ctx,
        stack->top == stack->floor ? "SUCCESS" : "FAIL");

    return r;
}

static int
test_stack_nonuniform(
    rex_stack_t *stack,
    char * ctx
){
    int64_t i = 0;
    char * dp;
    int r = 0;
    char data[256] = {0};
    void * val;
    uint8_t * const ceil = stack->ceil;
    uint8_t * const floor = stack->floor;

    for (i = 1, dp = data; dp + i < data + 256; dp += i++)
    {
        memset(dp, i, i);
    }
    dp = data;

    for (i = 0; rex_stack_push(stack, dp, i);  dp+=i++);

    r =(floor < ceil) ?
        !((uint8_t*)stack->top < ceil && (uint8_t*)stack->top+i > ceil):
        !((uint8_t*)stack->top > ceil && (uint8_t*)stack->top-i < ceil);

    printf("Nonuniform %s Prevents Overflow: %s\n",
        ctx, !r ? "SUCCESS" : "FAIL");

    i--;
    dp -= i;
    for (; (val =rex_stack_peek(stack, i)); rex_stack_pop(stack, i--), dp -= i)
        if (memcmp(val, dp, i) != 0)
        {
            printf("Nonuniform %s Data Uncorrupted: FAIL\n", ctx);
            r = 1;
            break; 
        }

    r |= !(i<1);
    printf("Nonuniform %s Data Uncorrupted: %s\n", ctx, i<1? "SUCCESS" : "FAIL");

    r |= !(stack->top == floor);
    printf("Nonuniform %s Prevents Underflow: %s\n", ctx,
        stack->top == floor ? "SUCCESS" : "FAIL");

    return r;
}

int
main(void)
{
    int r = 0;
    uint8_t mem[256];
    rex_stack_t stack;

    stack.ceil = mem+256;
    stack.floor = mem;
    stack.top = stack.floor;
    r |= test_stack_uniform(&stack, "Low->High");
    stack.floor = mem+256;
    stack.ceil = mem;
    stack.top = stack.floor;
    r |= test_stack_uniform(&stack, "High->Low");

    stack.ceil = mem+256;
    stack.floor = mem;
    stack.top = stack.floor;
    r |= test_stack_nonuniform(&stack, "Low->High");
    stack.floor = mem+256;
    stack.ceil = mem;
    stack.top = stack.floor;
    r |= test_stack_nonuniform(&stack, "High->Low");
    
    return r;
}
