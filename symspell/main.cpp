// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "include/SymSpell.h"

#include <iostream>
#include <fcntl.h>
#include <io.h>

void main()
{
	const int initialCapacity = 82765; // hard-coded
	const int maxEditDistance = 2;
	const int prefixLength = 3;
	SymSpell symSpell (initialCapacity, maxEditDistance, prefixLength);
	int start = clock ();
	symSpell.LoadDictionary (".\\data\\frequency_dictionary_en_82_765.txt", 0, 1, XL (' '));
	int end = clock ();
	float time = (float)((end - start) / (CLOCKS_PER_SEC / 1000));
	xcout << XL ("Library loaded: ") << time << XL (" ms") << endl;

	xcout << XL ("-------Testing English word segmentation-------") << endl;
	vector<xstring> sentences = { XL ("thequickbrownfoxjumpsoverthelazydog"),
		XL ("itwasabrightcolddayinaprilandtheclockswerestrikingthirteen"),
		XL ("itwasthebestoftimesitwastheworstoftimesitwastheageofwisdomitwastheageoffoolishness") };
	for (int i = 0; i < sentences.size (); i++) {
		Info results = symSpell.WordSegmentation (sentences[i]);
		xcout << sentences[i] << XL (" -> ") << results.getCorrected () << endl;
	}

	xcout << XL ("-------Testing English word correction-------") << endl;
	sentences = { XL ("tke"), XL ("abolution"), XL ("intermedaite") };
	for (int i = 0; i < sentences.size (); i++) {
		vector<SuggestItem> results = symSpell.Lookup (sentences[i], Verbosity::Closest);
		xcout << sentences[i] << XL (" -> ") << results[0].term << endl;
	}

	xcout << XL ("-------Testing English compound correction-------") << endl;
	sentences = { XL ("whereis th elove hehad dated forImuch of thepast who couqdn'tread in sixthgrade and ins pired him"),
		XL ("in te dhird qarter oflast jear he hadlearned ofca sekretplan"),
		XL ("the bigjest playrs in te strogsommer film slatew ith plety of funn") };
	for (int i = 0; i < sentences.size (); i++) {
		vector<SuggestItem> results = symSpell.LookupCompound (sentences[i]);
		xcout << sentences[i] << endl << XL (" -> ") << results[0].term << endl << XL (" -> ") << results[1].term << endl << XL (" -> ") << results[2].term << endl;
	}
}