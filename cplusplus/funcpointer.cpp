#include "funcpointer.hpp"

Funcpointer::Funcpointer(const char* n, callback cb)
{
	name = n;
	cback = cb;
}
void Funcpointer::SayHello()
{
	if(cback != NULL)
		(*cback)(name);
	else
		std::cout<<"------"<<std::endl;
}
