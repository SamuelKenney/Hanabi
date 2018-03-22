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
protected:
	// players hand that holds every possible card
	std::map<int, std::map<int, std::list<int>>> playerHand;

	// partner hand that holds every possible card
	std::map<int, std::map<int, std::list<int>>> partnerHand;

	// deck that holds every possible card
	std::map<int, std::list<int>> deck;
	// map that holds the tableau
	std::vector<int>tableau;
	// discard pile
	std::vector<Card> discardPile;

	std::vector<int> pastColorHintMoves;
	std::vector<int> pastNumberHintMoves;

	bool lastCard(Card c);

	// elements for the Knowledge base
	int hints;
	int fuses;
	// easily can store other hand
	vector<Card> oHand;
	int deckSize;

	// Managing hand knowledge
	void removePossibilityFromHand(std::map<int, std::map<int, std::list<int>>> &hand, Card c);
	void removeCardFromHand(std::map<int, std::map<int, std::list<int>>> &hand, int i);
	void setCardHandColor(std::map<int, std::map<int, std::list<int>>> &hand, int cardIndex, int color);
	void setCardHandNumber(std::map<int, std::map<int, std::list<int>>> &hand, int cardIndex, int number);

	bool canBePlayed(Card c);
	int chooseDiscard(std::map<int, std::map<int, std::list<int>>> hand, bool uncertain = false);
	int choosePlay(std::map<int, std::map<int, std::list<int>>> hand);
	bool numberCanBePlayed(int n);
	int getCardColor(std::map<int, std::map<int, std::list<int>>> hand, int index);
	int getCardNumber(std::map<int, std::map<int, std::list<int>>> hand, int index);
	int getPlayableCard(std::map<int, std::map<int, std::list<int>>> hand);
	int getCleanNumberHint();

	vector<std::pair<bool, bool>> hintedAt;
	vector<std::pair<bool, bool>> playerHintedAtStored;
	
	int turns;

	void print();
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

	partnerHand[0] = deck; //Position 0
	partnerHand[1] = deck; //Position 1
	partnerHand[2] = deck; //Position 2
	partnerHand[3] = deck; //Position 3
	partnerHand[4] = deck; //Position 4

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
		std::pair<bool, bool> foo = std::pair<bool, bool>(false, false);
		if (pe->legal){
			// take away from player hand or other hand
			// take away from player hand or other hand
			if (pe->wasItThisPlayer) {
				// removes the card from the play, and shifts everything down
				std::map<int, std::map<int, std::list<int>>> temp;
				removeCardFromHand(playerHand, pe->position);

				// need to update own hand to reflect that it is no longer an option
				removePossibilityFromHand(playerHand, pe->c);

				playerHintedAtStored.erase(playerHintedAtStored.begin() + pe->position);
				playerHintedAtStored.push_back(foo);
			}
			else {
				// need to update own hand to reflect that it is no longer an option
				removePossibilityFromHand(partnerHand, pe->c);
				removeCardFromHand(partnerHand, pe->position);

				hintedAt.erase(hintedAt.begin() + pe->position);
				hintedAt.push_back(foo);
			}

			// push card onto the tableau when it is a valid move
			tableau = board;

			// Removes the card from the map that holds the deck
			std::list<int>::iterator it = std::find(deck.at(pe->c.color).begin(), deck.at(pe->c.color).end(), pe->c.number);
			// makes sure there are no out of bound errors
			if (it != deck.at(pe->c.color).end())
				deck.at(pe->c.color).erase(it);

		}
		else { // if not legal, take away a fuse
			// don't need to worry about the game ending, this is for my knowledge base
			// if it is not a legal move, needs to be discarded

			// take away from player hand or other hand
			if (pe->wasItThisPlayer){
				// removes the card from the play, and shifts everything down
				std::map<int,std::map<int, std::list<int>>> temp;
				removeCardFromHand(playerHand, pe->position);
				
				// need to update own hand to reflect that it is no longer an option
				removePossibilityFromHand(playerHand, pe->c);

				playerHintedAtStored.erase(playerHintedAtStored.begin() + pe->position);
				playerHintedAtStored.push_back(foo);
			} else {
				// need to update own hand to reflect that it is no longer an option
				removePossibilityFromHand(partnerHand, pe->c);
				removeCardFromHand(partnerHand, pe->position);

				hintedAt.erase(hintedAt.begin() + pe->position);
				hintedAt.push_back(foo);
			}

			//add discarded card to the pile
			discardPile.push_back(pe->c);


			// Removes the card from the map that holds the deck
			std::list<int>::iterator it = std::find(deck.at(pe->c.color).begin(), deck.at(pe->c.color).end(), pe->c.number);
			// makes sure there are no out of bound errors
			if (it != deck.at(pe->c.color).end())
				deck.at(pe->c.color).erase(it);
		}
	}
	else if (currentAction == 12) {
		DiscardEvent* de = (DiscardEvent*)e;
		// depending on who it is, the card is removed from the hand and the deck
		std::pair<bool, bool> foo = std::pair<bool, bool>(false, false);
		if (de->wasItThisPlayer) {
			// removes the card from the play, and shifts everything down
			std::map<int, std::map<int, std::list<int>>> temp;
			removeCardFromHand(playerHand, de->position);

			playerHintedAtStored.erase(playerHintedAtStored.begin() + de->position);
			playerHintedAtStored.push_back(foo);

			// update the players hand based on what is discarded
			removePossibilityFromHand(playerHand, de->c);
		}
		else {
			removePossibilityFromHand(partnerHand, de->c);
			removeCardFromHand(partnerHand, de->position);

			hintedAt.erase(hintedAt.begin() + de->position);
			hintedAt.push_back(foo);
		}

		// add discarded card to the pile
		discardPile.push_back(de->c);


		// Removes the card from the map that holds the deck
		std::list<int>::iterator it = std::find(deck.at(de->c.color).begin(), deck.at(de->c.color).end(), de->c.number);
		// makes sure there are no out of bound errors
		if (it != deck.at(de->c.color).end())
			deck.at(de->c.color).erase(it);
	}
	else if (currentAction == 13) {
		ColorHintEvent* ce = (ColorHintEvent*)e;
		for (int i = 0; i < ce->indices.size(); i++)
		{
			setCardHandColor(playerHand, ce->indices.at(i), ce->color);
		}
	}
	else if (currentAction == 14) {
		// will always be for the other player since we can see it
		DrawEvent* dre = (DrawEvent*)e;

		// removes drawn card from the player's hand
		removePossibilityFromHand(playerHand, dre->drawnCard);

		// Removes the card from the map that holds the deck
		std::list<int>::iterator it = std::find(deck.at(dre->drawnCard.color).begin(), deck.at(dre->drawnCard.color).end(), dre->drawnCard.number);
		// makes sure there are no out of bound errors
		if (it != deck.at(dre->drawnCard.color).end())
			deck.at(dre->drawnCard.color).erase(it);
	}
	else if (currentAction == 15){
		NumberHintEvent* ne = (NumberHintEvent*) e;
		// TODO: How many of the number were present before hint

		for (int i = 0; i < ne->indices.size(); i++)
		{
			setCardHandNumber(playerHand, ne->indices.at(i), ne->number);
		}
	}
	else if (currentAction == 0){
		// TODO: Probably nothing
	}

	this->tableau = board;
	this->hints = hints;
	this->fuses = fuses;
	this->oHand = oHand;
	this->deckSize = deckSize;

}

Event* Player::ask()
{

	// ============================================================================================
	// ==========This function will figure out the next best move to be made by the player=========
	// ============================================================================================

	print();
	// needs to be called every time to update values based on hints
	//playerHintedAt();
	
	turns++;

	bool hint = true;
	//1 Save Hint
	if (turns >= 2){ // wait till second stage of game TODO:
		int in = chooseDiscard(partnerHand);
		// TODO: DO WE HAVE ANY HINTS TO SPEND!!!
		if (in != -1){
			if (!hintedAt[in].first && !hintedAt[in].second){ // if both false, high chance of discard
				// TODO: PLAYED TOO MANY LAST CARDS
				if (lastCard(oHand[in]) && hints != 0){ // if this is the last card, give a color hint
					ColorHintEvent* colorEvent = new ColorHintEvent(vector<int>(), oHand[in].color);
					for (int j = 0; j < oHand.size(); j++)
					{
						if (oHand[j].color == oHand[in].color)
							hintedAt[j].first = true;
					}
					return colorEvent;
				}
			}
		}
	}

	if (hints < 8) {
		int c = chooseDiscard(playerHand);
		if (c != -1) { // if there is a good guaranteed card to get rid of, do it
			DiscardEvent* discardEvent = new DiscardEvent(c);
			return discardEvent;
		}
	}

	//2 Play
	int c = choosePlay(playerHand);
	if (c != -1) {
		PlayEvent* playEvent = new PlayEvent(c); // if a real card that can be played, play it
		return playEvent;
	}
	/* Looking at every card in MY hand */
	// If fully known card, discard (if hints < 8) or play accordingly

	// 
	for (int i = 0; i < hintedAt.size(); i++) // TODO: HINTS NEED TO NOT BE 0!!!!
	{
		if (hintedAt[i].second == true && hints != 0){
			ColorHintEvent* colorEvent = new ColorHintEvent(vector<int>(), oHand[i].color);

			for (int j = 0; j < oHand.size(); j++)
			{
				if (oHand[j].color == oHand[i].color)
					hintedAt[j].first = true;
			}
			return colorEvent;
		}

	}

	//3 Uncertain play
		// If number known & playable, play it
		// Perhaps play the rarest card possible? eg play a 5 over a 3
		// If only color known, skip it
	for (int i = 0; i < playerHand.size(); i++)
	{
		int num = getCardNumber(playerHand, i);
		if (numberCanBePlayed(num) && getCardColor(playerHand, i) == -1){
			PlayEvent* playEvent = new PlayEvent(i); // if a real card that can be played, play it
			return playEvent;
		}
	}

	//4 Play Hint
		/* Looking at every PLAYABLE card in THEIR hand */
		// Hint at the number of the card in the smallest group
			// Eg if there is a playable 1 and 2, but there are more 1's than 2's, hint the 2
	for (int i = 0; i < oHand.size(); i++)
	{
		// TODO: whittle down options
			// Look at all playable numbers, pick 'clean' numbers over 'dirty'
				// A dirty number whose clue would mark unplayable cards to be played.

		if (canBePlayed(oHand[i])) {
			if (hintedAt[i].second != true && hints != 0) {
				NumberHintEvent* numberEvent = new NumberHintEvent(vector<int>(), oHand[i].number);
				for (int j = 0; j < oHand.size(); j++)
				{
					if (oHand[j].number == oHand[i].number)
						hintedAt[j].second = true;
				}
				return numberEvent;
			}
		}
	}

	// If we have max hints, give a hint
	if (hints == 8){
		for (int i = 0; i < hintedAt.size(); i++)
		{
			// If we have hinted at this card's number (So high chance of a play hint), hint at its color bc we have no need for more hints
			if (hintedAt[i].second == true && hints != 0){
				ColorHintEvent* colorEvent = new ColorHintEvent(vector<int>(), oHand[i].color);

				for (int j = 0; j < oHand.size(); j++)
				{
					if (oHand[j].color == oHand[i].color)
						hintedAt[j].first = true;
				}
				return colorEvent;
			}

		}
	} // Otherwise, discard what we have to

	//5 Discard
	DiscardEvent* discardEvent = new DiscardEvent(chooseDiscard(playerHand, true));
	return discardEvent;
	
}

void Player::removePossibilityFromHand(std::map<int, std::map<int, std::list<int>>> &hand, Card c)
{
	for (int i = 0; i < hand.size(); i++)
	{
		if (!hand.at(i).empty() && !hand.at(i).at(c.color).empty()) {
			// Removes the card from the map that holds the deck
			std::list<int>::iterator it = std::find(hand.at(i).at(c.color).begin(), hand.at(i).at(c.color).end(), c.number);
			if (it != hand.at(i).at(c.color).end())
				hand.at(i).at(c.color).erase(it);
		}
	}
}

void Player::removeCardFromHand(std::map<int, std::map<int, std::list<int>>> &hand, int cardIndex)
{
	std::map<int, std::map<int, std::list<int>>> temp;
	int count = 0;
	for (int i = 0; i < hand.size(); i++)
	{
		if (i != cardIndex) {
			temp[count] = hand[i];
		}
		else {
			count--;
		}
		count++;
	}
	temp[4] = deck;
	hand = temp;
}

/// Taske a hand and sets the card to the color
void Player::setCardHandColor(std::map<int, std::map<int, std::list<int>>> &hand, int cardIndex, int color)
{
	for (int colorIndex = 0; colorIndex < hand.at(cardIndex).size(); colorIndex++)
	{
		// for each color not in hint, remove from the map
		// grab the position of the index specified in indices at i
		if (colorIndex != color) {
			hand.at(cardIndex).at(colorIndex).clear();
		}
		playerHintedAtStored[colorIndex].first = true; // hand at this position has been hinted a number
	}
}

void Player::setCardHandNumber(std::map<int, std::map<int, std::list<int>>> &hand, int cardIndex, int number)
{
	std::list<int> choosen;
	for (int j = 0; j < playerHand.at(cardIndex).size(); j++)
	{
		int count = 0;
		for (auto v : playerHand.at(cardIndex).at(j)) {
			if (v == number) {
				count++;
			}
		}

		choosen.clear();

		for (int k = 0; k < count; k++) {
			choosen.push_back(number);
		}

		// grab the position of the index specified in indices at cardIndex
		playerHand.at(cardIndex).at(j) = choosen; // set the new vector there to be the possible options
	}
	playerHintedAtStored[cardIndex].second = true;
}

/// Returns true if the card can be played
bool Player::canBePlayed(Card c) {
	int val = c.number - 1;
	// goes to specific spot in tableau and if the played card is one lower than possible played card, return true
	if (tableau[c.color] == val)
		return true;

	return false;
}

/// Returns true if the number is a valid play on some stack
bool Player::numberCanBePlayed(int n) {
	// goes through tableau and if there is a value anywhere that the number can be played on, it returns true
	int val = n - 1;
	for (int i = 0; i < tableau.size(); i++)
	{
		if (tableau[i] == val)
			return true;
	}

	return false;
}

/// Returns a card that can be played, if no cards playable, return -1 for color and number
int Player::getPlayableCard(std::map<int, std::map<int, std::list<int>>> hand) {
	for (int i = 0; i < hand.size(); i++)
	{
		int colr = getCardColor(hand, i);
		int num = getCardNumber(hand, i);
		Card c(colr, num);
		if (colr != -1 && num != -1) {
			if (canBePlayed(c)) {
				return i;
			}
		}
	}

	return -1;
}

inline int Player::getCleanNumberHint()
{
	std::vector<Card> playable;
	// Get all playable cards from opponents hand
	for (int c = 0; c < oHand.size(); c++) {
		if (canBePlayed(oHand[c])) {
			//playable.push_back();
		}
	}

	// For each count up the number of unplayable cards that would also be hinted at

	// Return any card that doesn't give any misleading hints (hinting at a card that cannot be played

	std::vector<int> count = { 0,0,0,0,0 };
	for (int cardIndex = 0; cardIndex < hintedAt.size(); cardIndex++) {
		if (!hintedAt[cardIndex].second && true) {
			count[cardIndex]++;
		}
	}
	
	for (int i = 0; i < count.size(); i++) {
		if (count[i] == 1) {
		}
	}

	return -1;
}

/// Returns true if the given card is the last of its kind
bool Player::lastCard(Card c) {
	int counter = 0;
	if (c.number == 5) {
		return true;
	}

	std::list<int>::iterator it = std::find(deck.at(c.color).begin(), deck.at(c.color).end(), c.number);
	if (it == deck.at(c.color).end()) {
		return true;
	}
	return false;

}

/// Returns the index of the best card to discard from the given hand
///	if uncertain than it will pick a card garuanteed
/// if certain than it will return -1 if no safe discards exists
int Player::chooseDiscard(std::map<int, std::map<int, std::list<int>>> hand, bool uncertain) {
	// This case is assuming we have already discarded any obvious discards
	/* Looking a the cards in order of least information known first */

	// Any known safe discard
	for (int cardIndex = 0; cardIndex < hand.size(); cardIndex++)
	{
		int color = getCardColor(hand, cardIndex);
		int number = getCardNumber(hand, cardIndex);

		// if we know the card and it can't be played
		// TODO: Consider if the card is numbered, but we cannot play it
		if (number != -1 && color != -1 && tableau[color] >= number)
		{
			return cardIndex;
		}

		// 1. Any color xor number we know to be 'used up' 
		if (color != -1 && tableau[color] == 5) {
			return cardIndex; // discard this card
		}

		int min = 5;
		for (int j = 0; j < tableau.size(); j++)
		{
			if (tableau[j] < min) {
				min = tableau[j];
			}
		}
		if (number != -1 && number <= min) {
			return cardIndex;
		}
	}
	// Uncertain discards
	if (uncertain) {
		int in = -1;
		for (int cardIndex = 0; cardIndex < hand.size(); cardIndex++)
		{
			// 2. Unknown
			if (getCardColor(hand, cardIndex) == -1 && getCardNumber(hand, cardIndex) == -1) {
				return cardIndex;
			}
		}
		for (int cardIndex = 0; cardIndex < hand.size(); cardIndex++)
		{
			// 3. Number Known
			if (!getCardColor(hand, cardIndex) == -1 && getCardNumber(hand, cardIndex) == -1) {
				return cardIndex;
			}
		}
		return 0;
	}
	return -1;
}

/// Returns the index of the best card to play from the give hand
/// Returns -1 if there is no play to be made
int Player::choosePlay(std::map<int, std::map<int, std::list<int>>> hand) {
	int cardIndex = getPlayableCard(hand);
	if (cardIndex != -1) { // if its a real card
		return cardIndex;
	}

	return -1;
}

/// Returns the color of the card from the hand if it is definite, returns -1 otherwise
int Player::getCardColor(std::map<int, std::map<int, std::list<int>>> hand, int card) {
	// Sentinel value
	int color = -1;
	int cols = 0;

	// check if the color is known
	for (int j = 0; j < hand.at(card).size(); j++)
	{
		if (hand.at(card).at(j).empty()) {
			cols++;
		}
		else {
			color = j;
		}
	}
	if (cols == 4) { // color has been hinted
		return color;
	}
	else {
		return -1;
	}
}

/// Returns the number of the card from the hand if it is definite, returns -1 otherwise
int Player::getCardNumber(std::map<int, std::map<int, std::list<int>>> hand, int card) {
	// Sentinel value
	int num = -1;

	// TODO: not used right now
	//for (int i = 0; i < playerHand.at(card).size(); i++)
	//{
	//	for (auto v : playerHand.at(card).at(i))
	//	{
	//		if (numberCanBePlayed(v))
	//			return v;
	//	}
	//}

	for (int i = 0; i < playerHand.at(card).size(); i++){
		bool same = true;
		std::list<int> test = playerHand.at(card).at(i);

		if (!test.empty()){
			int prev = test.front();
			for (auto v :test)
			{
				if (v != prev)
					same = false;
			}
			if (same){
				return prev;
			}
		}
	}

	return -1;
}

void Player::print(){
	std::cout << "Other hints: \n";
	for (int i = 0; i < hintedAt.size(); i++)
	{
		switch (i){
		case 0:
			std::cout << "First Card: ";
			break;
		case 1:
			std::cout << "Second Card: ";
			break;
		case 2:
			std::cout << "Third Card: ";
			break;
		case 3:
			std::cout << "Fourth Card: ";
			break;
		case 4:
			std::cout << "Fifth Card: ";
			break;
		}
		switch(hintedAt[i].first){
		case (true):
			std::cout << "T ";
			break;
		case (false):
			std::cout << "F ";
			break;
		}

		switch(hintedAt[i].second){
		case (true):
			std::cout << "T ";
			break;
		case (false):
			std::cout << "F ";
			break;
		}
	}
	std::cout << "\nOur hints: " << std::endl;
	for (int i = 0; i < playerHintedAtStored.size(); i++)
	{
		switch (i){
		case 0:
			std::cout << "First Card: ";
			break;
		case 1:
			std::cout << "Second Card: ";
			break;
		case 2:
			std::cout << "Third Card: ";
			break;
		case 3:
			std::cout << "Fourth Card: ";
			break;
		case 4:
			std::cout << "Fifth Card: ";
			break;
		}
		switch(playerHintedAtStored[i].first){
		case (true):
			std::cout << "T ";
			break;
		case (false):
			std::cout << "F ";
			break;
		}

		switch(playerHintedAtStored[i].second){
		case (true):
			std::cout << "T ";
			break;
		case (false):
			std::cout << "F ";
			break;
		}
	}
	std::cout << std::endl;
	std::cout << std::endl;
	for (int i = 0; i < playerHand.size(); i++)
	{
		switch (i){
		case 0:
			std::cout << "First Card: \n";
			break;
		case 1:
			std::cout << "Second Card: \n";
			break;
		case 2:
			std::cout << "Third Card: \n";
			break;
		case 3:
			std::cout << "Fourth Card: \n";
			break;
		case 4:
			std::cout << "Fifth Card: \n";
			break;
		}
		for (int j = 0; j < playerHand.at(i).size(); j++)
		{
			switch(j){
			case 0:
				std::cout << "Red: ";
				break;
			case 1:
				std::cout << "Blue: ";
				break;
			case 2:
				std::cout << "Green: ";
				break;
			case 3:
				std::cout << "Yellow: ";
				break;
			case 4:
				std::cout << "White: ";
				break;
			}
			for (auto v : playerHand.at(i).at(j))
			{
				std::cout << v << " ";
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void otherPlayerHintedAt(){

}
#endif