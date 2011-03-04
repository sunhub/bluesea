#include <iostream>

typedef void (*callback)(const char* name);

class Funcpointer
{
	public:
		Funcpointer(const char* name, callback cb);
		void SayHello();
	private:
		callback cback;
		const char* name;
};
