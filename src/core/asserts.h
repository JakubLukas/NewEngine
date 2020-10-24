#pragma once


namespace Veng
{


void MyDebugBreak();
void MyOutputDebugString(const char* text);


}


#define ASSERT(cond) do{ if(!(cond)) { Veng::MyDebugBreak(); Veng::MyOutputDebugString(#cond); } }while(0)
#define ASSERT2(cond, text) do{ if(!(cond)) { Veng::MyDebugBreak(); Veng::MyOutputDebugString(text); } }while(0)
