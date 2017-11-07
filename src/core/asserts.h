#pragma once


void MyDebugBreak();
void MyOutputDebugString(const char* text);


#define ASSERT(cond) if(!(cond)) { MyDebugBreak(); MyOutputDebugString(#cond); }
#define ASSERT2(cond, text) if(!(cond)) { MyDebugBreak(); MyOutputDebugString(text); }