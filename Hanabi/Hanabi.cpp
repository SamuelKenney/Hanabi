#include <iostream>
#include "Player.h"
#include "Game.h"

using std::cout;
using std::endl;

int main()
{
	int total = 0;
	// Set to false if you don't want a "chatty" game
	Game g(true);
	// Will be 1000 for final test.
	int iterations = 1;
	for (int i = 0; i < iterations; i++)
	{
		Player p1;
		Player p2;
		g.setup(p1, p2);
		total += g.gameLoop();
	}
	cout << "Average score " << total/(static_cast<double>(iterations)) 
		<< "." << endl;
	return 0;
}