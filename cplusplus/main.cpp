#include "funcpointer.hpp"
#include <iostream>
const char* name = "sunhai";
void say(const char* name)
{
	std::cout<<"Hello"<<name<<std::endl;
}

int main(int argc, char** argv)
{
	Funcpointer fp(name, say);
	fp.SayHello();
	return 0;
}
