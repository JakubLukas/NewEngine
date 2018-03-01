#pragma once


namespace Veng
{


void MyDebugBreak();
void MyOutputDebugString(const char* text);


}


#define ASSERT(cond) if(!(cond)) { Veng::MyDebugBreak(); Veng::MyOutputDebugString(#cond); }
#define ASSERT2(cond, text) if(!(cond)) { Veng::MyDebugBreak(); Veng::MyOutputDebugString(text); }