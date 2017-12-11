#pragma once

#define STACK_INIT_SIZE 1000
#define STACK_INCREMENT 50
#include "Error.h"
#include "HTMLElement.h"
//自定义栈
typedef HTMLElement ElemType;

typedef struct
{
    ElemType* base;
    ElemType* top;
    int stacksize;
} Stack;

void InitStack(Stack& S);
bool IsEmpty(Stack S);
int StackLength(Stack S);
ElemType GetTop(Stack S);
void Push(Stack& S, ElemType e);
void Pop(Stack& S, ElemType& e);
void ClearStack(Stack &S);
