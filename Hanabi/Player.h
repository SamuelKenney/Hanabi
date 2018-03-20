#include "GameConstants.h"
#include "Card.h"
#include "Events.h"
#include <vector>
#include <iostream>
#include <map>
#include <algorithm>
#include <list>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

using std::vector;
using std::cout;
using std::endl;

#ifndef PLAYER_H
#define PLAYER_H




class Player
{
public:
	Player();
	Player(const Player& p);
	void tell(Event* e, vector<int> board, int hints, int fuses, vector<Card> oHand, int deckSize);
	Event* ask();

	bool contains(int in);
protected:
	// players hand that holds every possible card
	std::map<int, std::map<int, std::list<int>>> playerHand;
	// deck that holds every possible card
	std::map<int, std::list<int>> deck;
	// map that holds the tableau
	std::vector<int>tableau;
	// discard pile
	std::vector<Card> discardPile;

	std::vector<int> pastColorHintMoves;
	std::vector<int> pastNumberHintMoves;


	// elements for the Knowledge base
	int hints;
	int fuses;
	// easily can store other hand
	vector<Card> oHand;
	int deckSize;

	vector<std::pair<bool, bool>> hintedAt;
	vector<std::pair<bool, bool>> playerHintedAtStored;

	bool canBePlayed(Card c);
	int chooseDiscard(bool uncertain=false);
	bool numberCanBePlayed(int n);
	void playerHintedAt();

	int getCardColor(int index);
	int getCardNumber(int index);

	Card cardPlay();

	int turns;
};

Player::Player()
{
	// Start with 8 hints
	hints = 8;
	// Start with 3 fuses
	fuses = 3;
	// 50 cards to start the deck
	deckSize = 50;

	// fill vector with possible cards for each color
	// 3 - 1's, 2 - 2's, 2 - 3's, 2 - 4's, 1 - 5
	std::list<int> possiblecards;
	possiblecards.push_back(1);
	possiblecards.push_back(1);
	possiblecards.push_back(1);
	possiblecards.push_back(2);
	possiblecards.push_back(2);
	possiblecards.push_back(3);
	possiblecards.push_back(3);
	possiblecards.push_back(4);
	possiblecards.push_back(4);
	possiblecards.push_back(5);

	// fill the deck with every possible card
	deck[0] = possiblecards; //RED
	deck[1] = possiblecards; //BLUE
	deck[2] = possiblecards; //GREEN
	deck[3] = possiblecards; //YELLOW
	deck[4] = possiblecards; //WHITE

	// fill the player's hand with all the possible cards
	// RED 0 BLUE 1 GREEN 2 YELLOW 3 WHITE 4
	playerHand[0] = deck; //Position 0
	playerHand[1] = deck; //Position 1
	playerHand[2] = deck; //Position 2
	playerHand[3] = deck; //Position 3
	playerHand[4] = deck; //Position 4

													// color, number
	std::pair<bool, bool> pos0 = std::pair<bool, bool>(false, false);
	std::pair<bool, bool> pos1 = std::pair<bool, bool>(false, false);
	std::pair<bool, bool> pos2 = std::pair<bool, bool>(false, false);
	std::pair<bool, bool> pos3 = std::pair<bool, bool>(false, false);
	std::pair<bool, bool>pos4 = std::pair<bool, bool>(false, false);

	hintedAt.push_back(pos0);
	hintedAt.push_back(pos1);
	hintedAt.push_back(pos2);
	hintedAt.push_back(pos3);
	hintedAt.push_back(pos4);

	playerHintedAtStored.resize(5);

	turns = 0;
}

Player::Player(const Player& p)
{
	hints = p.hints;
	fuses = p.fuses;
	oHand = p.oHand;
	deckSize = p.deckSize;

	playerHand = p.playerHand;
	deck = p.deck;
	tableau = p.tableau;
	discardPile = p.discardPile;
	pastColorHintMoves = p.pastColorHintMoves;
	pastNumberHintMoves = p.pastNumberHintMoves;

	hintedAt = p.hintedAt;
	turns = p.turns;
}

void Player::tell(Event* e, vector<int> board, int hints, int fuses, vector<Card> oHand, int deckSize)
{
	// ============================================================================================
	// ========This function will store the events and the results into the knowledge base=========
	// ============================================================================================

	// go through deck and remove each instance of possible card from the deck from other Player's card
	int currentAction = e->getAction();
	/*
	PLAY 11
	DISCARD 12
	COLOR_HINT 13
	DRAW 14
	NUMBER_HINT 15
	NO_OP 0
	*/
	if (currentAction == 11){
		PlayEvent* pe = (PlayEvent*) e;
		//only play card if legal
		if (pe->legal){
			// take away from player hand or other hand
			if (pe->wasItThisPlayer){
				// remove all options from this position in the players hand-- will be replaced by draw
				playerHand.at(pe->position) = deck; // cards that are left over in the deck, not all 50 cards
			}

			// push card onto the tableau when it is a valid move
			tableau = board;

			// first  red                   green                blue
			// <0 < 0 <1 1 1 2 2 3 3 4 4 5> 1 <1 1 1 2 2 3 3 4 4 5> ... > > >
			// need to update own hand to reflect that it is no longer an option
			for (int i = 0; i < playerHand.size(); i++)
			{
				// Removes the card from the map that holds the deck
				if (!playerHand.at(i).empty() && !playerHand.at(i).at(pe->c.color).empty()){
					// Removes the card from the map that holds the deck
					std::list<int>::iterator it = std::find(playerHand.at(i).at(pe->c.color).begin(), playerHand.at(i).at(pe->c.color).end(), pe->c.number);
					if (it != playerHand.at(i).at(pe->c.color).end())
						playerHand.at(i).at(pe->c.color).erase(it);		
				}	
			}

			// Removes the card from the map that holds the deck
			std::list<int>::iterator it = std::find(deck.at(pe->c.color).begin(), deck.at(pe->c.color).end(),pe->c.number);
			// makes sure there are no out of bound errors
			if (it != deck.at(pe->c.color).end())
				deck.at(pe->c.color).erase(it);

		} else { // if not legal, take away a fuse
			// don't need to worry about the game ending, this is for my knowledge base
			// if it is not a legal move, needs to be discarded

			// take away from player hand or other hand
			if (pe->wasItThisPlayer){
				// remove all options from this position in the players hand-- will be replaced by draw
				playerHand.at(pe->position) = deck;
			} 

			//add discarded card to the pile
			discardPile.push_back(pe->c);

			// first  red                   green                blue
			// <0 < 0 <1 1 1 2 2 3 3 4 4 5> 1 <1 1 1 2 2 3 3 4 4 5> ... > > >
			// need to update own hand to reflect that it is no longer an option
			for (int i = 0; i < playerHand.size(); i++)
			{
				if (!playerHand.at(i).empty() && !playerHand.at(i).at(pe->c.color).empty()){
					// Removes the card from the map that holds the deck
					std::list<int>::iterator it = std::find(playerHand.at(i).at(pe->c.color).begin(), playerHand.at(i).at(pe->c.color).end(), pe->c.number);
					if (it != playerHand.at(i).at(pe->c.color).end())
						playerHand.at(i).at(pe->c.color).erase(it);			
				}
			}

			// Removes the card from the map that holds the deck
			std::list<int>::iterator it = std::find(deck.at(pe->c.color).begin(), deck.at(pe->c.color).end(), pe->c.number);
			// makes sure there are no out of bound errors
			if (it != deck.at(pe->c.color).end())
				deck.at(pe->c.color).erase(it);
		}
	}
	else if(currentAction == 12){
		DiscardEvent* de = (DiscardEvent*) e;
		// depending on who it is, the card is removed from the hand and the deck
		if (de->wasItThisPlayer){
			// remove all options from this position in the players hand-- will be replaced by draw
			playerHand.at(de->position).at(de->c.color).clear();
		} else {
			oHand.at(de->position).color = -1; // TODO: figure out a better way than sentinel value for no card in oHand
		}

		// add discarded card to the pile
		discardPile.push_back(de->c);

		// update the players hand based on what is discarded
		for (int i = 0; i < playerHand.size(); i++)
		{
			if (!playerHand.at(i).empty() && !playerHand.at(i).at(de->c.color).empty()){
				// Removes the card from the map that holds the deck
				std::list<int>::iterator it = std::find(playerHand.at(i).at(de->c.color).begin(), playerHand.at(i).at(de->c.color).end(), de->c.number);
				if (it != playerHand.at(i).at(de->c.color).end())
					playerHand.at(i).at(de->c.color).erase(it);		
			}
		}

		// Removes the card from the map that holds the deck
		std::list<int>::iterator it = std::find(deck.at(de->c.number).begin(), deck.at(de->c.number).end(), de->c.color);
		// makes sure there are no out of bound errors
		if (it != deck.at(de->c.color).end())
			deck.at(de->c.color).erase(it);
	}
	else if (currentAction == 13){
		ColorHintEvent* ce = (ColorHintEvent*) e;
		for (int i = 0; i < ce->indices.size(); i++)
		{
			// first  red                   green                blue
			// <0 < 0 <1 1 1 2 2 3 3 4 4 5> 1 <1 1 1 2 2 3 3 4 4 5> ... > > >
			for (int j = 0; j < playerHand.at(i).size(); j++)
			{
				// for each color not in hint, remove from the map
				// grab the position of the index specified in indices at i
				if (j != ce->color){
					playerHand.at(ce->indices.at(i)).at(j).clear();
					hintedAt[i].first = true; // hand at this position has been hinted a color
				}
			}
		}
	}
	else if (currentAction == 14){
		// will always be for the other player since we can see it
		DrawEvent* dre = (DrawEvent*) e;

		// removes drawn card from the player's hand
		for (int i = 0; i < playerHand.size(); i++)
		{
			if (!playerHand.at(i).empty() && !playerHand.at(i).at(dre->drawnCard.color).empty()){
				// Removes the card from the map that holds the deck
				std::list<int>::iterator it = std::find(playerHand.at(i).at(dre->drawnCard.color).begin(), playerHand.at(i).at(dre->drawnCard.color).end(), dre->drawnCard.number);
				if (it != playerHand.at(i).at(dre->drawnCard.color).end()){
					playerHand.at(i).at(dre->drawnCard.color).erase(it);
				}
			}
		}

		// Removes the card from the map that holds the deck
		std::list<int>::iterator it = std::find(deck.at(dre->drawnCard.color).begin(), deck.at(dre->drawnCard.color).end(), dre->drawnCard.number);
		// makes sure there are no out of bound errors
		if (it != deck.at(dre->drawnCard.color).end())
			deck.at(dre->drawnCard.color).erase(it);
	}
	else if (currentAction == 15){
		NumberHintEvent* ne = (NumberHintEvent*) e;

		for (int i = 0; i < ne->indices.size(); i++)
		{
			// first  red                   green                blue
			// <0 < 0 <1 1 1 2 2 3 3 4 4 5> 1 <1 1 1 2 2 3 3 4 4 5> ... > > >
			for (int j = 0; j < playerHand.at(i).size(); j++)
			{
				// grab the position of the index specified in indices at i
				playerHand.at(ne->indices.at(i)).at(j).clear();
				playerHand.at(ne->indices.at(i)).at(j).push_back(ne->number);
				hintedAt[i].second = true; // hand at this position has been hinted a number
			}
		}
	}
	else if (currentAction == 0){
		// TODO: Probably nothing
	}

	this->hints = hints;
	this->fuses = fuses;
	this->oHand = oHand;
	this->deckSize = deckSize;


	/*  Possible kinds of event:
		DiscardEvent - can be for us or other player
			c - the card discarded
			wasItThisPlayer - true if we discarded, false otherwise
			position - the index in hand of the discarded card (0 base)
		ColorHintEvent - always for other player
			indices - all indices of the chosen color
			color - the color in question
		NumberHintEvent - always for the other player
			indices - all indices of the chosen number
			color - the number in question
		PlayEvent - can be for us or other player
			position - the index in hand of the discarded card (0 base)
			c - the card played
			legal - whether the card was a legal play
			wasItThisPlayer - true if we played, false otherwise
	*/
}

Event* Player::ask()
{

	// ============================================================================================
	// ==========This function will figure out the next best move to be made by the player=========
	// ============================================================================================

	/*
	 What do I have in my knowledge base to make a decision?
		- Player's Hand: or what could be
		- Deck: Cards left in the deck
		- Tableau: cards played to win the game
		- Discard Pile: cards that have been discarded or poorly played

	// elements for the Knowledge base
	int hints;
	int fuses;
	// easily can store other hand
	vector<Card> oHand;
	int deckSize;
	*/

	// The following hold each of the instantiations of the Event class with all needed arguments
	// Copy and paste from here to each decision made
	/*
	int card = 0;
	ColorHintEvent* colorEvent = new ColorHintEvent(vector<int>(), card);
	return colorEvent;

	int number = 1;
	NumberHintEvent* numberEvent = new NumberHintEvent(vector<int>(), number);
	return numberEvent;

	int index = 0;
	PlayEvent* playEvent = new PlayEvent(index);
	return playEvent;

	int indexDis = 0;
	DiscardEvent* discardEvent = new DiscardEvent(indexDis);
	return discardEvent;
	*/
	
	//1 Save Hint
	if (turns > 5){ // wait till second stage of game
		for (int i = 0; i < hintedAt.size(); i++)
		{
			if (!hintedAt[i].first && !hintedAt[i].second){ // if both false, high chance of discard
				// TODO:
			}
		}
	}
	playerHintedAt();
	cardPlay();


	//2 Good play
	
		/* Looking at every card in MY hand */
		// If fully known card, discard (if hints < 8) or play accordingly

	//3 Bad play
		// If number known & playable, play it
		// Perhaps play the rarest card possible? eg play a 5 over a 3
		// If only color known, skip it

	//4 Play Hint
		/* Looking at every PLAYABLE card in THEIR hand */
		// Hint at the number of the card in the smallest group
			// Eg if there is a playable 1 and 2, but there are more 1's than 2's, hint the 2

	//5 Discard
		

	turns++;


	PlayEvent* playEvent = new PlayEvent(0);
	return playEvent;








	
	/*


	srand (time(NULL));
	int choice = rand()%10;

	
	Three main moves
		1. Give a hint -- based on other player's hand
			- if hint given, go through and update which cards have been hinted
			- if they have been hinted, do not hint color or number again
			- if 1 white, find 2 or white
			- if 3 blue, find 4 or blue

		2. Play a card -- based on what we know about our hand
			- go through, if there are any ones and are playable, play them 

		3. Discard a card -- based on what we know about our hand

	

	// at all times, and in all places, play a 1
	for (int i = 0; i < playerHand.size(); i++)
	{
		// first  red                   green                blue
		// <0 < 0 <1 1 1 2 2 3 3 4 4 5> 1 <1 1 1 2 2 3 3 4 4 5> ... > > >
		for (int j = 0; j < playerHand.at(i).size(); j++)
		{
			if (playerHand.at(i).at(j).size() == 1){
				if (playerHand.at(i).at(j).front() == 1){
					PlayEvent* playEvent = new PlayEvent(i);
					return playEvent;
				}
			}
		}

	}
	

	
	// maybe do some sort of check on if there are ones
	// and then if there are any ones played, look for twos etc.
	// if there are any played on the board, look for that color

	// make hints based on the tableau and what you want on it
	for (int i = 0; i < oHand.size(); i++)
	{
		if (oHand[i].number == 1 && hintedAt[i].second != true){
			NumberHintEvent* numberEvent = new NumberHintEvent(vector<int>(), 1);
			return numberEvent;
		}
	}

	  You must produce an event of the appropriate type. Not all member
		variables of a given event type need to be filled in; some will be
		ignored even if they are. Summary follows.
		Options:
			ColorHintEvent - you must declare a color; no other member variables
				necessary.
			NumberHintEvent - you must declare a number; no other member variables
				necessary.
			PlayEvent - you must declare the index to be played; no other member
				variables necessary.
			DiscardEvent - you must declare the index to be discarded; no other
				member variables necessary.
	*/
}

bool Player::canBePlayed(Card c){
	// goes to specific spot in tableau and if the played card is one lower than possible played card, return true
	if (tableau[c.color] == c.number--)
		return true;
		
	return false;
}
bool Player::numberCanBePlayed(int n){
	// goes through tableau and if there is a value anywhere that the number can be played on, it returns true
	for (int i = 0; i < tableau.size(); i++)
	{
		if (tableau[i] == n--)
			return true;
	}

	return false;
}

Card Player::cardPlay(){
	int cols = 0;
	int in = 0;

	for (int i = 0; i < playerHand.size(); i++)
	{
		cols = 0;
		// check if the color is known
		for (int j = 0; j < playerHand.at(i).size(); j++)
		{
			if (playerHand.at(i).at(j).empty()){
				cols++;
			}
		}
		if (cols == 4){ // if color has been known, store which card is known
			in = i;
		}
	}

	// sentinel values for if there is no card to be played
	int color = -1;
	int number = -1;
	// go through the card that has a known color
	for (int j = 0; j < playerHand.at(in).size(); j++)
	{
		if (playerHand.at(in).at(j).size() == 1){
			color = j;
			number = playerHand.at(in).at(j).front();
		}
	}

	Card c = Card(color, number); // return card that is playable since color and number is known at the same time
	return c;
}

/// Returns the index of the best card to discard
///	if uncertain = true than 
int Player::chooseDiscard(bool uncertain = false) {
	// This case is assuming we have already discarded any obvious discards
	/* Looking a the cards in order of least information known first */
	
	// Any known safe discard
	playerHintedAt();
	for (int i = 0; i < hintedAt.size(); i++)
	{
		// if we know the card and it can't be played
		// TODO: Consider if the card is numbered, but we cannot play it
		if (hintedAt[i].first && hintedAt[i].second
			&& (tableau[i] >= getCardNumber(i)) {
			return i;
		}

		// 1. Any color xor number we know to be 'used up'
	}

	// Uncertain discards
	if (uncertain) {
		
		// 2. Unknown
		// 3. Color Known
		// 4. Number Known
	}
	
}

/// Returns the color of the card if it is definite, returns -1 otherwise
int Player::getCardColor(int card) {
	// Sentinel value
	int color = -1;
	int cols = 0;

	// check if the color is known
	for (int j = 0; j < playerHand.at(card).size(); j++)
	{
		if (playerHand.at(card).at(j).empty()) {
			cols++;
		}
		else {
			color = j;
		}
	}
	if (cols == 4) { // color has been hinted
		return color;
	} else {
		return -1;
	}
}

/// Returns the number of the card if it is definite, returns -1 otherwise
int Player::getCardNumber(int card) {
	// Sentinel value
	int num = -1;

	for (int i = 0; i < playerHand.at(card).size(); i++)
	{
		if (playerHand.at(card).at(i).size() != 1) {
			return -1;
		}
	}
	return playerHand.at(card).at(0).front;
}



void Player::playerHintedAt(){
	// updates vector of pairs that let the player know what parts of their hand have been hinted at
	// Possible values are of the form (color, number) which could be FF, FT, TF, or TT

	int cols = 0;
	int nums = 0;

	bool colorHinted = false;
	bool numberHinted = false;

	for (int i = 0; i < playerHand.size(); i++)
	{
		cols = 0;
		colorHinted = false;
	    numberHinted = false;

		// check if the color is known
		for (int j = 0; j < playerHand.at(i).size(); j++)
		{
			if (playerHand.at(i).at(j).empty()){
				cols++;
			}
		}
		if (cols == 4){ // color has been hinted
			colorHinted = true;
		}
		// check if the color is known
		for (int j = 0; j < playerHand.at(i).size(); j++)
		{
			if (playerHand.at(i).at(j).size() == 1){
				nums++;
			}
		}
		if (nums == 1 || nums == 5){
			numberHinted = true;
		}

		// keep track of each hand whether that be FF, TF, FT, or TT
		std::pair<bool, bool> temp = std::pair<bool, bool>(colorHinted, numberHinted);
		playerHintedAtStored[i] = temp;
	}
}
#endif