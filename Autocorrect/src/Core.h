#pragma once
#include <vector>
#include <string>
#include <tuple>
#include <future>
#include <queue>
#include <optional>
#include "SymSpell/include/SymSpell.h"

static const int symspell_Max_Edit_Distance = 3, symspell_Prefix_length = 4;

// MACROS
#define MIN(x,y) (x > y ? y :  x)
#define MAX(x,y) (x > y ? x :  y)
#define ABS(x)   (x > 0 ? x : -x)
#define MOD(x,y) (x - ((int)(((float)x) / y) * y))

#ifdef MODE_DEBUG
#define MY_ASSERT(x)  if(!(x))__debugbreak ();
#else				 
#define MY_ASSERT(x)  ;
#endif // _DEBUG


struct SymSpell_Dictionary
{
	bool Lock = false;
	
	struct
	{
		uint32_t val = 0;
		uint32_t max = 0;
		float offset_min = 0.0f;
		float offset_max = 0.0f;
		const char *for_src;
		float percent ()
		{
			return (offset_min + (double (val)/max)*(offset_max - offset_min))*100;
		}
	} progress;
	
	std::vector<std::string> Sources = std::vector<std::string> ();
	SymSpell symspell = SymSpell (1, symspell_Max_Edit_Distance, symspell_Prefix_length);
	
	bool IsSourceExist (std::string &src)
	{
		for (std::string &val: Sources) {
			if (val == src)
				return true;
		}
		return false;
	}
};

struct Signal
{
public:
	template<typename Typ1, typename Typ2, typename Typ3>
	Signal Create (Typ1 *a, Typ2 b, Typ3 c, void(*callbackFunc)(void *, void *, uint8_t *, bool))
	{
		Typ1 *_a = a; Typ2 *_b = new Typ2; Typ3 *_c = new Typ3;
		*_b = std::move (b); *_c = std::move (c);
		return Signal (callbackFunc, _a, _b, _c);
	}
	Signal (void(*callbackFunc)(void *, void *, uint8_t *, bool), void *a, void *b, uint8_t *c)
		:user_data { a,b,c }, callback_func (callbackFunc), isValid(true)
	{
		MY_ASSERT (callback_func != nullptr);
		MY_ASSERT (a != nullptr && b != nullptr && c != nullptr);
	}
	Signal (Signal& other)
		: user_data {other.user_data[0], other.user_data[1], other.user_data[2]}, callback_func (other.callback_func), isValid(other.isValid)
	{
		other.isValid = false;
		other.user_data[0] = nullptr, other.user_data[1] = nullptr, other.user_data[2] = nullptr;
		callback_func = nullptr;
	}

	void Invalidate ()
	{
		isValid = false;
	}
	bool IsValid ()
	{
		return isValid;
	}
	bool Has (void *ptr)
	{
		bool has = false;
		for (void *val: user_data)
			has |= (val != nullptr ? (ptr == val) : false);
		return has;
	}

	~Signal ()
	{
		if(callback_func) // NOTE: call_back function needs to delete user_data[1], user_data[2];
			callback_func (user_data[0], user_data[1], (uint8_t*)user_data[2], isValid);
		MY_ASSERT(callback_func != nullptr && user_data[0] != nullptr /* && user_data[1] == nullptr && user_data[2] != nullptr*/);
	}
private:
	bool isValid = false;
	void *user_data[3] = {nullptr, nullptr, nullptr};
	void (*callback_func)(void *, void *, uint8_t *, bool) = nullptr;
};

struct QueryBox
{
	QueryBox (std::queue<Signal> *signalQueue, std::string message, std::vector<std::pair<std::string, bool>> optionsWithDefaultState, void *data1, void* data2, void(*callbackFunc)(void *, void *, uint8_t *, bool))
		:signal_queue (signalQueue), message ({ std::move (message), false }), query_options (std::move (optionsWithDefaultState)), callback_func (callbackFunc), data_main (data1), data_ex(data2)
	{
		// At the least
		MY_ASSERT(callback_func != nullptr);
		MY_ASSERT(signalQueue != nullptr);
		MY_ASSERT(data1 != nullptr);
	}
	QueryBox (QueryBox& other)
		:signal_queue (other.signal_queue), message ({ std::move (other.message.first), other.message.second }), query_options (std::move (other.query_options)), callback_func (other.callback_func), data_main (other.data_main), data_ex(other.data_ex)
	{
		other.callback_func = nullptr;
		other.signal_queue  = nullptr;
		other.data_main  = nullptr;
		other.data_ex  = nullptr;
	}

	bool OnImGuiRender ();

	~QueryBox ()
	{
		if (callback_func) {
			uint32_t size_of_ptr = (sizeof (uint32_t)/sizeof (uint8_t)) + size_t ((sizeof (bool)/float (sizeof (uint8_t))) * (1 + query_options.size ()));
			uint8_t *option_data_gen = new uint8_t[size_of_ptr]; // uint32_t[For count of query_options] + bool[yes_no] + bool[query_options states].size ()
			*((uint32_t *)(option_data_gen + 0)) = uint32_t (query_options.size ());

			*((bool *)(option_data_gen + 4)) = message.second;
			
			for (uint32_t i = 0; i < query_options.size (); i++) {
				*((bool *)(option_data_gen + 4 + 1 + i)) = query_options[i].second;
			}
			signal_queue->emplace (callback_func, data_main, data_ex, option_data_gen); // it is up-to the callback function to delete or re-use

			MY_ASSERT (!message.first.empty ()) ;
			MY_ASSERT (!query_options.empty ());
			MY_ASSERT (data_main) ;
			// MY_ASSERT(!data_ex); // not nessarily
			MY_ASSERT (signal_queue);
		}
		MY_ASSERT (!(callback_func == nullptr && message.first.empty () ));
		MY_ASSERT (!(callback_func == nullptr && query_options.empty () ));
		MY_ASSERT (!(callback_func == nullptr && data_main == nullptr   ));
		MY_ASSERT (!(callback_func == nullptr && data_ex == nullptr     ));
		MY_ASSERT (!(callback_func == nullptr && signal_queue == nullptr));
	}
private:
	// no-ownership of any element
	std::pair<std::string, bool> message = {"", false};
	std::vector<std::pair<std::string, bool>> query_options;
	void (*callback_func)(void *, void *, uint8_t *, bool) = nullptr;
	void *data_main = nullptr;
	void *data_ex = nullptr;
	std::queue<Signal> *signal_queue;
};

std::optional<std::string> AbsoluteDirectoryPath (std::string orignal);

// create file at given address with unique name, if orignal.empty() create file at same directory as exe
std::string CreateNewFile (std::string orignal = std::string());

// Combine above 2
std::string CreateNewFileIfNotExists (std::string orignal = std::string());

// deduce extension for file
std::string GetFileExtension (const char *fileName_or_path, uint32_t size = 0);

// deduce file-name from file-path
std::string GetFileName (const char *fileName_or_path, uint32_t size = 0);