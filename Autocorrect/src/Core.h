#pragma once

#include "SymSpell/include/SymSpell.h"


class SymspellDictionary
{
	bool Lock = false;
	std::vector<const char *> Sources;
	SymSpell Dict;
};

struct Signal
{
public:
	template<typename Typ1, typename Typ2, typename Typ3>
	Signal Create (Typ1 a, Typ2 b, Typ3 c, void(*callbackFunc)(void *, void *, uint8_t *))
	{
		Typ1 *_a = new Typ1; Typ2 *_b = new Typ2; Typ3 *_c = new Typ3;
		*_a = std::move (a); *_b = std::move (b); *_c = std::move (c);
		return Signal (callbackFunc, _a, _b, _c);
	}

	Signal (void(*callbackFunc)(void *, void *, uint8_t *), void *a, void *b, uint8_t *c)
		:user_data ({ a,b,c }), callback_func (callbackFunc)
	{
#ifdef _DEBUG
		if (callback_func == nullptr) { __debugbreak(); }
		if (a == nullptr || b == nullptr || c == nullptr) { __debugbreak(); }
#endif // _DEBUG
	}
	~Signal ()
	{
		callback_func (user_data[0], user_data[1], user_data[2]);
#ifdef _DEBUG
	if (user_data[0] != nullptr) LOG_WARN("user_data[0] is not nullptr");
	if (user_data[1] != nullptr || user_data[2] != nullptr) { __debugbreak (); }
#endif // _DEBUG
	}
private:
	void *user_data[3] = {nullptr, nullptr, nullptr};
	void (*callback_func)(void *, void *, uint8_t *) = nullptr;
};

struct QueryBox
{
	QueryBox (std::queue<Signal> *signalQueue, std::string message, std::vector<std::pair<std::string, bool>> optionsWithDefaultState, void* data_ex, void* target_obj, void(*callbackFunc)(void *, void *, uint8_t *))
		:signal_queue (signalQueue), message ({ std::move (message), false }), query_options (std::move (optionsWithDefaultState)), callback_func (callbackFunc), callback_target (target_obj), callback_data_ex(data_ex)
	{
#ifdef _DEBUG
		if (callback_func == nullptr)__debugbreak ();
		if (signalQueue == nullptr)__debugbreak ();
		if (target_obj == nullptr)__debugbreak ();
#endif // _DEBUG
	}

	bool OnImGuiRender ();

	~QueryBox ()
	{
		uint8_t *data = new uint8_t[4 + (1 + query_options.size ())]; // sizeof(uint32_t)[For count of query_options] + bool[yes_no] + bool[query_options states].size ()
		*((uint32_t *)(data + 0)) = uint32_t(query_options.size ());

		*((bool *)(data + 4)) = message.second;
		uint32_t i = 0;
		for(std::pair<std::string, bool>& opt : query_options)
			*((uint32_t *)(data + 4 + 1 + i)) = query_options.size ();

		signal_queue->emplace (callback_func, callback_target, callback_data_ex, data); // it is up-to the callback function to delete or re-use
	}
private:
	std::pair<std::string, bool> message = {"", false};
	std::vector<std::pair<std::string, bool>> query_options;
	void (*callback_func)(void *, void *, uint8_t *) = nullptr;
	void *callback_target = nullptr;
	void *callback_data_ex = nullptr; // no-ownership
	std::queue<Signal> *signal_queue;
};