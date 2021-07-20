#pragma once
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <cctype>
#include <locale>
#include <regex>
#include <iostream>

//#define UNICODE_SUPPORT
#include "Helpers.h"

// Copyright (C) 2019 Wolf Garbe
// Version: 6.5
// Orignal Author: Wolf Garbe wolf.garbe@faroo.com
// Maintainer: Wolf Garbe wolf.garbe@faroo.com
// URL: https://github.com/wolfgarbe/symspell
// Description: https://medium.com/@wolfgarbe/1000x-faster-spelling-correction-algorithm-2012-8701fcd87a5f

#define DEFAULT_SEPARATOR_CHAR XL('\t')
#define DEFAULT_MAX_EDIT_DISTANCE 2
#define DEFAULT_PREFIX_LENGTH 7
#define DEFAULT_COUNT_THRESHOLD 1
#define DEFAULT_INITIAL_CAPACITY 82765
#define DEFAULT_COMPACT_LEVEL 5
#define min3(a, b, c) (MIN(a, MIN(b, c)))
#define MAXINT LLONG_MAX
#define M
#define MAXLONG MAXINT
#define uint unsigned int
static inline void ltrim (xstring &s)
{

	s.erase (s.begin (), find_if (s.begin (), s.end (), [](xchar ch) {
		return !isxspace (ch);
								  }));
}

static inline void rtrim (xstring &s)
{
	s.erase (find_if (s.rbegin (), s.rend (), [](xchar ch) {
		return !isxspace (ch);
					  }).base (), s.end ());
}

static inline void trim (xstring &s)
{
	ltrim (s);
	rtrim (s);
}

class Info
{
private:

	xstring segmentedstring;
	xstring correctedstring;
	int distanceSum;
	double probabilityLogSum;
public:
	void set (xstring &seg, xstring &cor, int d, double prob)
	{
		segmentedstring = seg;
		correctedstring = cor;
		distanceSum = d;
		probabilityLogSum = prob;
	};
	xstring getSegmented ()
	{
		return segmentedstring;
	};
	xstring getCorrected ()
	{
		return correctedstring;
	};
	int getDistance ()
	{
		return distanceSum;
	};
	double getProbability ()
	{
		return probabilityLogSum;
	};
};

enum Verbosity
{
	Top,
	Closest,
	All
};

class SymSpell
{
private:
	int initialCapacity;
	int maxDictionaryEditDistance;
	int prefixLength;
	long countThreshold;
	int compactMask;
	DistanceAlgorithm distanceAlgorithm = DistanceAlgorithm::DamerauOSADistance;
	int maxDictionaryWordLength;
	Dictionary<int, vector<xstring>> *deletes = NULL;
	Dictionary<xstring, int64_t> words;
	Dictionary<xstring, int64_t> belowThresholdWords;

public:
	int MaxDictionaryEditDistance ();

	int PrefixLength ();

	int MaxLength ();

	long CountThreshold ();

	int WordCount ();

	int EntryCount ();

	SymSpell (int initialCapacity = DEFAULT_INITIAL_CAPACITY, int maxDictionaryEditDistance = DEFAULT_MAX_EDIT_DISTANCE
			  , int prefixLength = DEFAULT_PREFIX_LENGTH, int countThreshold = DEFAULT_COUNT_THRESHOLD
			  , unsigned char compactLevel = DEFAULT_COMPACT_LEVEL);

	bool CreateDictionaryEntry (xstring key, int64_t count, SuggestionStage *staging);

	Dictionary<xstring, long> bigrams;
	int64_t bigramCountMin = MAXLONG;

	bool LoadBigramDictionary (string corpus, int termIndex, int countIndex, xchar separatorChars = DEFAULT_SEPARATOR_CHAR);

	bool LoadBigramDictionary (xifstream &corpusStream, int termIndex, int countIndex, xchar separatorChars = DEFAULT_SEPARATOR_CHAR);

	bool LoadDictionary (string corpus, int termIndex, int countIndex, xchar separatorChars = DEFAULT_SEPARATOR_CHAR);

	bool LoadDictionary (xifstream &corpusStream, int termIndex, int countIndex, xchar separatorChars = DEFAULT_SEPARATOR_CHAR);

	bool LoadDictionaryWithPB (string corpus, int termIndex, int countIndex, xchar separatorChars, uint32_t *progress_amount, uint32_t *max_progress_amount, float *progress_contribution_min, float *progress_contribution_max);

	bool LoadDictionaryWithPB (xifstream &corpusStream, int termIndex, int countIndex, xchar separatorChars, uint32_t *progress_amount, uint32_t *max_progress_amount, float *progress_contribution_min, float *progress_contribution_max);

	bool CreateDictionary (string corpus);

	bool CreateDictionary (xifstream &corpusStream);

	void PurgeBelowThresholdWords ();

	/// <summary>Commit staged dictionary additions.</summary>
	/// <remarks>Used when you write your own process to load multiple words into the
	/// dictionary, and as part of that process, you first created a SuggestionsStage 
	/// object, and passed that to CreateDictionaryEntry calls.</remarks>
	/// <param name="staging">The SuggestionStage object storing the staged data.</param>
	void CommitStaged (SuggestionStage *staging);

	/// <summary>Find suggested spellings for a given input word, using the maximum
	/// edit distance specified during construction of the SymSpell dictionary.</summary>
	/// <param name="input">The word being spell checked.</param>
	/// <param name="verbosity">The value controlling the quantity/closeness of the retuned suggestions.</param>
	/// <returns>A List of SuggestItem object representing suggested correct spellings for the input word, 
	/// sorted by edit distance, and secondarily by count frequency.</returns>
	vector<SuggestItem> Lookup (xstring input, Verbosity verbosity);

	/// <summary>Find suggested spellings for a given input word, using the maximum
	/// edit distance specified during construction of the SymSpell dictionary.</summary>
	/// <param name="input">The word being spell checked.</param>
	/// <param name="verbosity">The value controlling the quantity/closeness of the retuned suggestions.</param>
	/// <param name="maxEditDistance">The maximum edit distance between input and suggested words.</param>
	/// <returns>A List of SuggestItem object representing suggested correct spellings for the input word, 
	/// sorted by edit distance, and secondarily by count frequency.</returns>
	vector<SuggestItem> Lookup (xstring input, Verbosity verbosity, int maxEditDistance);

	/// <summary>Find suggested spellings for a given input word.</summary>
	/// <param name="input">The word being spell checked.</param>
	/// <param name="verbosity">The value controlling the quantity/closeness of the retuned suggestions.</param>
	/// <param name="maxEditDistance">The maximum edit distance between input and suggested words.</param>
	/// <param name="includeUnknown">Include input word in suggestions, if no words within edit distance found.</param>																													   
	/// <returns>A List of SuggestItem object representing suggested correct spellings for the input word, 
	/// sorted by edit distance, and secondarily by count frequency.</returns>
	vector<SuggestItem> Lookup (xstring input, Verbosity verbosity, int maxEditDistance, bool includeUnknown);


	/* For Multi-threading and support for safe thread cancel */
	/// <summary>Find suggested spellings for a given input word.</summary>
	/// <param name="input">The word being spell checked.</param>
	/// <param name="verbosity">The value controlling the quantity/closeness of the retuned suggestions.</param>
	/// <param name="maxEditDistance">The maximum edit distance between input and suggested words.</param>
	/// <param name="includeUnknown">Include input word in suggestions, if no words within edit distance found.</param>																													   
	/// <param name="instant_access_to_suggestions">NOTE: will be turned nullptr the moment function returns, 
	/// reference to a pointer pointing to suggestions being worked upon in real-time </param>																													   
	/// <param name="safe_cancel_signal">NOTE: it won't turn false when function returns, 
	/// pointer to bool, when turns true Lookup is safely canceled.</param>																													   
	/// <returns>A List of SuggestItem object representing suggested correct spellings for the input word, 
	/// sorted by edit distance, and secondarily by count frequency.</returns>
	vector<SuggestItem> Lookup (xstring input, Verbosity verbosity, int maxEditDistance, bool includeUnknown, vector<SuggestItem> **instant_access_to_suggestions, bool *safe_cancel_signal);

private:
	//check whether all delete chars are present in the suggestion prefix in correct order, otherwise this is just a hash collision
	bool DeleteInSuggestionPrefix (xstring deleteSugg, int deleteLen, xstring suggestion, int suggestionLen);

	//create a non-unique wordlist from sample text
	//language independent (e.g. works with Chinese characters)
	vector<xstring> ParseWords (xstring text);

	//inexpensive and language independent: only deletes, no transposes + replaces + inserts
	//replaces and inserts are expensive and language dependent (Chinese has 70,000 Unicode Han characters)
	HashSet<xstring> *Edits (xstring word, int editDistance, HashSet<xstring> *deleteWords);

	HashSet<xstring> EditsPrefix (xstring key);

	int GetstringHash (xstring s);

public:
	//######################

	//LookupCompound supports compound aware automatic spelling correction of multi-word input strings with three cases:
	//1. mistakenly inserted space into a correct word led to two incorrect terms 
	//2. mistakenly omitted space between two correct words led to one incorrect combined term
	//3. multiple independent input terms with/without spelling errors

	/// <summary>Find suggested spellings for a multi-word input string (supports word splitting/merging).</summary>
	/// <param name="input">The string being spell checked.</param>																										   
	/// <returns>A List of SuggestItem object representing suggested correct spellings for the input string.</returns> 
	vector<SuggestItem> LookupCompound (xstring input);

	/// <summary>Find suggested spellings for a multi-word input string (supports word splitting/merging).</summary>
	/// <param name="input">The string being spell checked.</param>
	/// <param name="maxEditDistance">The maximum edit distance between input and suggested words.</param>																											   
	/// <returns>A List of SuggestItem object representing suggested correct spellings for the input string.</returns> 
	vector<SuggestItem> LookupCompound (xstring input, int editDistanceMax);

	//######

	//WordSegmentation divides a string into words by inserting missing spaces at the appropriate positions
	//misspelled words are corrected and do not affect segmentation
	//existing spaces are allowed and considered for optimum segmentation

	//SymSpell.WordSegmentation uses a novel approach *without* recursion.
	//https://medium.com/@wolfgarbe/fast-word-segmentation-for-noisy-text-2c2c41f9e8da
	//While each string of length n can be segmentend in 2^n−1 possible compositions https://en.wikipedia.org/wiki/Composition_(combinatorics)
	//SymSpell.WordSegmentation has a linear runtime O(n) to find the optimum composition

	//number of all words in the corpus used to generate the frequency dictionary
	//this is used to calculate the word occurrence probability p from word counts c : p=c/N
	//N equals the sum of all counts c in the dictionary only if the dictionary is complete, but not if the dictionary is truncated or filtered
	static const int64_t N = 1024908267229L;

	/// <summary>Find suggested spellings for a multi-word input string (supports word splitting/merging).</summary>
	/// <param name="input">The string being spell checked.</param>
	/// <returns>The word segmented string, 
	/// the word segmented and spelling corrected string, 
	/// the Edit distance sum between input string and corrected string, 
	/// the Sum of word occurrence probabilities in log scale (a measure of how common and probable the corrected segmentation is).</returns> 
	Info WordSegmentation (xstring input);

	/// <summary>Find suggested spellings for a multi-word input string (supports word splitting/merging).</summary>
	/// <param name="input">The string being spell checked.</param>
	/// <param name="maxEditDistance">The maximum edit distance between input and corrected words 
	/// (0=no correction/segmentation only).</param>	
	/// <returns>The word segmented string, 
	/// the word segmented and spelling corrected string, 
	/// the Edit distance sum between input string and corrected string, 
	/// the Sum of word occurrence probabilities in log scale (a measure of how common and probable the corrected segmentation is).</returns> 
	Info WordSegmentation (xstring input, int maxEditDistance);

	/// <summary>Find suggested spellings for a multi-word input string (supports word splitting/merging).</summary>
	/// <param name="input">The string being spell checked.</param>
	/// <param name="maxSegmentationWordLength">The maximum word length that should be considered.</param>	
	/// <param name="maxEditDistance">The maximum edit distance between input and corrected words 
	/// (0=no correction/segmentation only).</param>	
	/// <returns>The word segmented string, 
	/// the word segmented and spelling corrected string, 
	/// the Edit distance sum between input string and corrected string, 
	/// the Sum of word occurrence probabilities in log scale (a measure of how common and probable the corrected segmentation is).</returns> 
	Info WordSegmentation (xstring input, int maxEditDistance, int maxSegmentationWordLength);
};
