#pragma once
#include "SymSpell/include/SymSpell.h"
#include <string>

static const int symspell_Max_Edit_Distance = 3, symspell_Prefix_length = 4;

#define MIN(x,y) (x > y ? y :  x)
#define MAX(x,y) (x > y ? x :  y)
#define ABS(x)   (x > 0 ? x : -x)
#define MOD(x,y) (x - ((int)(((float)x) / y) * y))


class new_MVC_Layer;
bool ChangeUnifiedDictionaryState (const bool state, new_MVC_Layer &layer);

struct SymSpell_Dictionary
{
	bool Lock = false;
	SymSpell symspell = SymSpell(1, symspell_Max_Edit_Distance, symspell_Prefix_length);
	std::vector<std::string> Sources = std::vector<std::string>();
	std::queue<std::string> SourceLoadQueue;
};

// For Query-box
struct UserQuery
{
public:
	static UserQuery Create (void *target_object_ptr, void(*static_callback_fn)(void*,bool,uint32_t,const bool*), const char *message_c_str, std::vector<std::string_view> query_options, const bool* start_states = nullptr)
	{
#ifdef _DEBUG
		if (message_c_str == nullptr) { LOG_ERROR ("message is NULL"); __debugbreak (); }
#endif // _DEBUG
		bool *bools = nullptr;
		if (query_options.size () > 0) bools = new bool[query_options.size ()];
		if (start_states)
			memcpy_s (bools, query_options.size (), start_states, query_options.size ());
		return UserQuery(std::string_view(message_c_str), query_options, bools, static_callback_fn, target_object_ptr);
	}
	
	void Callback (bool yes_no) { if(_callback_fn) _callback_fn(_callback_target, yes_no, _query_options.size (), _query_options_states); };
	
	inline const char*    Message()     { return _message.data (); }	
	inline const uint32_t OptionSize () { return _query_options.size (); }
	
	std::pair<const char *, bool*> OptionAt (uint32_t posn)
	{
#ifdef _DEBUG
		if (posn >= _query_options.size ()) { LOG_ERROR ("Accessing Out Of Bound Elements"); __debugbreak (); }
		if (_query_options.size () > 0) if(_query_options_states == nullptr) { LOG_ERROR ("Query Not Alive"); __debugbreak (); }
		if (&_query_options_states[posn] != (_query_options_states + posn)) { LOG_ERROR ("Posn Addn to ptr is not good"); __debugbreak (); }

#endif // _DEBUG
		return{ _query_options[posn].data () , _query_options_states + posn };
	}

	// TODO FIXME, manually clear before deleting
	void Clear () { 
		if (_query_options.size ()) { _query_options.clear (), delete[] _query_options_states; } 
	}

	~UserQuery () {} //if (_query_options_states) delete[] _query_options_states; _query_options_states = nullptr;	}
private :
	UserQuery () = default;
	UserQuery (std::string_view message, std::vector<std::string_view> &query_options, bool *query_options_states, void(*callback_fn)(void *, bool, uint32_t, const bool *), void *callback_target)
		:_message (message), _query_options (std::move(query_options)), _query_options_states (std::move(query_options_states)), _callback_fn (callback_fn), _callback_target (callback_target)
	{}

	const std::string_view _message;
	std::vector<std::string_view> _query_options;
	bool *_query_options_states; // keep thil nullptr

	
	void(*_callback_fn)(void*, bool, uint32_t, const bool *); // yes_no, option_count, option_states
	void *_callback_target;
};