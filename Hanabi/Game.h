#include "GameConstants.h"
#include "Card.h"
#include "Events.h"
#include "Player.h"
#include "PlayerAndHand.h"
#include <ctime>
#include <iostream>

using std::vector;
using std::cout;
using std::endl;

#ifndef GAME_H
#define GAME_H

void swap(Card &c1, Card &c2)
{
	Card temp = c1;
	c1 = c2;
	c2 = temp;
}

class Game
{
public:
	Game();
	Game(bool chattyGame);
	Game(const Game& g);
	// Run once before playing a game.
	void setup(Player& p1, Player& p2);
	// Primary game action.
	int gameLoop();
	// Get the score of the last-concluded game.
	int getScore() const;
private:
	// Shuffles cards for a new game.
	void shuffle();
	// Return a single card from deck. Reduce deck size by one.
	Card dealCard();
	// Report an event to the other player.
	void tellOtherPlayer(Event* e);
	// Report an event to teh current player.
	void tellCurrentPlayer(Event* e);
	// Switch players.
	void nextPlayer();
	// Reports whether a card is legally playable (true) or not (false)
	bool isPlayable(const Card& c) const;
	// Announce information about an event to the console.
	// Only called for games where chatty == true.
	void announce(Event* e) const;

	vector<PlayerAndHand> players;
	vector<Card> deck;
	vector<int> board;
	int currentPlayer;
	int hints;
	int fuses;
	bool chatty;
	unsigned int endGameCounter;
};


Game::Game()
{
	srand(static_cast<unsigned int>(time(0)));
	players.clear();
	deck.clear();
	board.clear();
	currentPlayer = 0;
	hints = MAX_HINTS;
	fuses = 0;
	chatty = false;
	endGameCounter = 0;
}

Game::Game(bool chattyGame)
{
	srand(static_cast<unsigned int>(time(0)));
	players.clear();
	deck.clear();
	board.clear();
	currentPlayer = 0;
	hints = MAX_HINTS;
	fuses = 0;
	chatty = chattyGame;
	endGameCounter = 0;
}

Game::Game(const Game& g)
{
	srand(static_cast<unsigned int>(time(0)));
	players = g.players;
	deck = g.deck;
	board = g.board;
	currentPlayer = g.currentPlayer;
	hints = g.hints;
	fuses = g.fuses;
	chatty = g.chatty;
	endGameCounter = 0;
}

void Game::setup(Player& p1, Player& p2)
{
	players.clear();
	deck.clear();
	board.clear();
	currentPlayer = 0;
	hints = MAX_HINTS;
	endGameCounter = 0;
	int colors[] = {RED, YELLOW, BLUE, GREEN, WHITE};
	int numbers[] = {1, 1, 1, 2, 2, 3, 3, 4, 4, 5};
	const int NUMBERS_SIZE = 10;
	// Load deck with cards of appropriate colors and numbers.
	for (int i = 0; i < NUM_COLORS; i++)
	{
		for (int j = 0; j < NUMBERS_SIZE; j++)
		{
			Card temp(colors[i], numbers[j]);
			deck.push_back(temp);
		}
	}
	shuffle();

	fuses = 3;
	// All five suits begin with a board state of 0 (i.e., no cards played)
	for (int i = 0; i < NUM_COLORS; i++)
	{
		board.push_back(0);
	}

	vector<Card> hand1;
	vector<Card> hand2;
	const int handSize = 5;
	// Load both hands with starting cards, announcing each draw to the other 
	// player.
	for (int i = 0; i < handSize; i++)
	{
		hand1.push_back(dealCard());
		hand2.push_back(dealCard());

		DrawEvent* e1 = new DrawEvent(i, hand2[i]);
		p1.tell(e1, board, hints, fuses, hand2, deck.size());
		delete e1;

		DrawEvent* e2 = new DrawEvent(i, hand1[i]);
		p2.tell(e2, board, hints, fuses, hand1, deck.size());
		delete e2;
	}
	// Add both players to the array of players.
	players.push_back(PlayerAndHand());
	players.push_back(PlayerAndHand());
	players[0].p = p1;
	players[0].hand = hand1;
	players[1].p = p2;
	players[1].hand = hand2;

	//No-op event - just announces starting board state.
	Event* e = new Event();
	if (chatty)
	{
		announce(e);
	}
	delete e;


	return;
}

int Game::gameLoop()
{
	while (true)
	{
		Event *e = players[currentPlayer].p.ask();
		// Discard
		if (e->getAction() == DISCARD)
		{
			DiscardEvent *de = static_cast<DiscardEvent*>(e);
			// Find the card that was just discarded.
			de->c = players[currentPlayer].hand[de->position];
			// Report the discard to both players.
			de->wasItThisPlayer = false;
			tellOtherPlayer(de);
			de->wasItThisPlayer = true;
			tellCurrentPlayer(de);
			
			if (hints < MAX_HINTS)
			{
				hints++;
			}

			players[currentPlayer].hand.erase(players[currentPlayer].hand.begin() + de->position);
			delete e;
			
			// Replace the discarded card and announce to other player.
			if (deck.size() > 0)
			{
				Card c = dealCard();
				players[currentPlayer].hand.push_back(c);
				DrawEvent *drawE = new DrawEvent(players[currentPlayer].hand.size(), c);
				tellOtherPlayer(drawE);
				delete drawE;
			}
			else
			{
				Event *e = new Event();
				tellOtherPlayer(e);
				delete e;
				endGameCounter++;
			}
		}
		// Color hint or number hint
		else if ((e->getAction() == COLOR_HINT) || (e->getAction() == NUMBER_HINT))
		{
			if (hints <= 0)
			{
				cout << "Tried to hint with no hints remaining." << endl;
				exit(0);
			}
			if (deck.size() == 0)
			{
				endGameCounter++;
			}
			hints--;
			// Mark either all colors or all numbers, as appropriate.
			if (e->getAction() == COLOR_HINT)
			{
				ColorHintEvent* ce = static_cast<ColorHintEvent*>(e);
				ce->indices.clear();
				int otherPlayer = (currentPlayer + 1) % 2;
				for (unsigned int i = 0; i < players[otherPlayer].hand.size(); i++)
				{
					if (players[otherPlayer].hand[i].color == ce->color)
					{
						ce->indices.push_back(i);
					}
				}
				if (ce->indices.size() == 0)
				{
					cout << "Illegal hint - no cards of that color." << endl;
					exit(0);
				}
			}
			else
			{
				NumberHintEvent* ne = static_cast<NumberHintEvent*>(e);
				ne->indices.clear();
				int otherPlayer = (currentPlayer + 1) % 2;
				for (unsigned int i = 0; i < players[otherPlayer].hand.size(); i++)
				{
					if (players[otherPlayer].hand[i].number == ne->number)
					{
						ne->indices.push_back(i);
					}
				}
				if (ne->indices.size() == 0)
				{
					cout << "Illegal hint - no cards of that number." << endl;
					exit(0);
				}
			}
			tellOtherPlayer(e);
			delete e;
		}
		// Play
		else if (e->getAction() == PLAY)
		{
			PlayEvent* pe = static_cast<PlayEvent*>(e);
			pe->c = players[currentPlayer].hand[pe->position];
			pe->legal = isPlayable(pe->c);
			// If legal, update board; otherwise, burn a hint.
			if (pe->legal)
			{
				board[pe->c.color]++;
				// Playing a 5 recovers a hint.
				if ((board[pe->c.color] == 5) && (hints < MAX_HINTS))
				{
					hints++;
				}

			}
			else
			{
				fuses--;
			}
			// Announce play of card to both players.
			players[currentPlayer].hand.erase(players[currentPlayer].hand.begin() + pe->position);
			pe->wasItThisPlayer = false;
			tellOtherPlayer(e);
			pe->wasItThisPlayer = true;
			tellCurrentPlayer(e);
			delete e;

			// If there are still cards in the deck, draw to replace.
			if (deck.size() > 0)
			{
				Card c = dealCard();
				DrawEvent *drawE = new DrawEvent(players[currentPlayer].hand.size(), c);
				players[currentPlayer].hand.push_back(c);
				tellOtherPlayer(drawE);
				delete drawE;
			}
			else
			{
				Event *e = new Event();
				tellOtherPlayer(e);
				delete e;
				endGameCounter++;
			}
		}
		// Error
		else
		{
			cout << "Bad ASK action." << endl;
			exit(0);
		}

		// End of game checks
		// Out of fuses - lose, scoring 0 points.
		if (fuses == 0)
		{
			cout << "All fuses are gone. Game over." << endl;
			for (unsigned int i = 0; i < board.size(); i++)
			{
				board[i] = 0;
				return 0;
			}
		}
		// Game won - all tracks at 5.
		else if ((board[RED] == 5) && (board[GREEN] == 5) && (board[WHITE] == 5) && (board[YELLOW] == 5) && (board[BLUE] == 5))
		{
			cout << "Perfect score!" << endl;
			return getScore();
		}
		// Deck runs out & all players have gotten their final play.
		else if (deck.size() == 0 && endGameCounter >= players.size())
		{
			cout << "Deck ran out. Final score was " << getScore() << endl;
			return getScore();
		}
		nextPlayer();
	}
}

int Game::getScore() const
{
	int score = 0;
	for (unsigned int i = 0; i < board.size(); i++)
	{
		score += board[i];
	}
	return score;
}

void Game::shuffle()
{
	const int deckSize = deck.size();
	for (int i = deckSize; i > 1; i--)
	{
		int j = rand() % i;
		swap(deck[j], deck[i-1]);
	}
	return;
}

Card Game::dealCard()
{
	if (deck.size() == 0)
	{
		Card temp(-1, -1);
		return temp;
	}
	Card temp = deck.back();
	deck.pop_back();
	return temp;
}

void Game::tellOtherPlayer(Event* e)
{
	if (chatty)
	{
		announce(e);
	}

	players[(currentPlayer+1) % players.size()].p.tell(e, board, hints, fuses, players[currentPlayer].hand, deck.size());
	return;
}

void Game::tellCurrentPlayer(Event* e)
{
	players[currentPlayer].p.tell(e, board, hints, fuses, players[(currentPlayer + 1) % players.size()].hand, deck.size());
	return;
}

void Game::nextPlayer()
{
	currentPlayer = (currentPlayer + 1) % players.size();
}

bool Game::isPlayable(const Card& c) const
{
	return (board[c.color] == (c.number - 1));
}

void Game::announce(Event* e) const
{
	int action = e->getAction();
	if (action == PLAY)
	{
		PlayEvent* pe = static_cast<PlayEvent*>(e);
		cout << "Player " << currentPlayer + 1 << " has ";
		if (!(pe->legal))
		{
			cout << "illegally ";
		}
		cout << "played card #" << pe->position + 1 << ", a " 
			<< pe->c.toString() << "." << endl;
		return;
	}
	else if (action == DISCARD)
	{
		DiscardEvent* de = static_cast<DiscardEvent*>(e);
		cout << "Player " << currentPlayer + 1 << " has discarded card #" << de->position + 1 << ", a "
			<< players[currentPlayer].hand[de->position].toString() << "." << endl;
		return;
	}
	else if (action == COLOR_HINT)
	{
		ColorHintEvent* ce = static_cast<ColorHintEvent*>(e);
		cout << "Player " << currentPlayer + 1 << " hints that card";
		if (ce->indices.size() == 1)
		{
			cout << " #" << ce->indices[0] + 1 << " is ";
		}
		else if (ce->indices.size() == 2)
		{
			cout << " #" << ce->indices[0] + 1 << " and #" << ce->indices[1] + 1 << " are ";
		}
		else
		{
			for (unsigned int i = 0; i < ce->indices.size() - 1; i++)
			{
				cout << " #" << ce->indices[i] + 1<< ", ";
			}
			cout << "and #" << ce->indices[ce->indices.size() - 1] + 1 << " are ";
		}
		switch (ce->color)
		{
		case RED:
			cout << "red." << endl;
			break;
		case GREEN:
			cout << "green." << endl;
			break;
		case YELLOW:
			cout << "yellow." << endl;
			break;
		case WHITE:
			cout << "white." << endl;
			break;
		case BLUE:
			cout << "blue." << endl;
			break;
		}
	}
	else if (action == NUMBER_HINT)
	{
		NumberHintEvent* ne = static_cast<NumberHintEvent*>(e);
		cout << "Player " << currentPlayer + 1 << " hints that card";
		if (ne->indices.size() == 1)
		{
			cout << " #" << ne->indices[0] + 1 << " is ";
		}
		else if (ne->indices.size() == 2)
		{
			cout << " #" << ne->indices[0] + 1 << " and #" << ne->indices[1] + 1<< " are ";
		}
		else
		{
			for (unsigned int i = 0; i < ne->indices.size() - 1; i++)
			{
				cout << " #" << ne->indices[i] + 1 << ", ";
			}
			cout << "and #" << ne->indices[ne->indices.size() - 1] + 1 << " are ";
		}
		cout << ne->number << "." << endl;
	}
	else if (action == DRAW)
	{
		DrawEvent* de = static_cast<DrawEvent*>(e);
		cout << "Player " << currentPlayer + 1 << " draws a new card, a " << de->drawnCard.toString() << "." << endl;
	}
	for (unsigned int j = 0; j < players.size(); j++)
	{
		cout << "Player " << j+1 << "'s hand is currently: ";
		for (unsigned int i = 0; i < players[j].hand.size(); i++)
		{
			cout << players[j].hand[i].toString();
			if (i < players[j].hand.size() - 1)
			{
				cout << ", ";
			}
		}
		cout << endl;
	}
	cout << "The board is currently: Red " << board[RED] << ", Blue " << board[BLUE] << ", Green " << board[GREEN] << ", Yellow "
		<< board[YELLOW] << ", White " << board[WHITE] << "," << endl;
	cout << "with " << fuses << " fuses and " << hints << " hints." << endl;
	cout << endl;
}


#endif