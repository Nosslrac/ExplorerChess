#include "../inc/Engine.h"

int main()
{
	std::unique_ptr<Engine> engine = std::make_unique<Engine>();
	engine->run();
}