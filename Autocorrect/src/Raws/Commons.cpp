#include "new_MVC_Layer.h"
#include "Text_Object.h"
#include "Commons.h"
#include <thread>

////////////////////////////////////
///
///  Methods When Changing Settings

bool ChangeUnifiedDictionaryState (const bool state, new_MVC_Layer &layer)
{
	if (layer.m_TextFileObjects.size () < 2) // Early return
		return state;

	for (auto &obj : layer.m_Dictionaries)
		if (obj.Lock == true) { // No Change
			new_MVC_Layer::s_Settings_changed = true;
			return new_MVC_Layer::s_Settings_Global.UnifiedDictionary;
		}

	if (state) { // Available to Unify
		// cancel all lookups
		for (auto &obj : layer.m_TextFileObjects)
			obj.m_CancelLookup = true;

		std::vector<std::string> locationOfDictionaries;
		for (uint32_t i = 1; i < layer.m_Dictionaries.size (); i++) {
			for (std::string &dict_path : layer.m_Dictionaries[i].Sources)
				locationOfDictionaries.emplace_back (std::move (dict_path));
			layer.m_Dictionaries[i].Sources.clear ();
		} // pushed all locations

		// filter duplicates and remove those already loaded in 1st dictionary
		{
			bool *non_duplicates = new bool[locationOfDictionaries.size ()];

			int i = 0;
			for (std::string &str_ref1 : locationOfDictionaries) { // assuming no duplicates in layer.m_Dictionaries[0].second
				non_duplicates[i] = true;
				for (uint32_t j = i + 1; j < locationOfDictionaries.size () + layer.m_Dictionaries[0].Sources.size (); j++) {
					std::string *str_ref2 = nullptr;
					if (j < locationOfDictionaries.size ())
						str_ref2 = &locationOfDictionaries[j];
					else str_ref2 = &layer.m_Dictionaries[0].Sources[j - locationOfDictionaries.size ()];

					if (str_ref1 == *str_ref2)
						non_duplicates[i] = false;
				}
				i++;
			}

			for (i = locationOfDictionaries.size () - 1; i > -1; i--)
				if (non_duplicates[i] == false)
					locationOfDictionaries.erase (locationOfDictionaries.begin () + i);
			delete[] non_duplicates;
		}

		std::thread add_dict (
			[](SymSpell_Dictionary &dictionary, std::vector<std::string> LocationOfDictionaries) {
				for (std::string &filePath : LocationOfDictionaries) { // Unify dictionary
					dictionary.Lock = true;
					dictionary.symspell.LoadDictionary (filePath, 0, 1, XL (' '));
					dictionary.Sources.push_back (std::move (filePath));
					dictionary.Lock = false;
				}
			}, layer.m_Dictionaries[0], std::move(locationOfDictionaries));
		add_dict.detach ();
		locationOfDictionaries.clear ();

		for (uint32_t i = 1; i < layer.m_TextFileObjects.size (); i++) // Link all
			layer.m_TextFileObjects[i].m_MyDictionary = &layer.m_Dictionaries[0];

		// Pop Extras: Hope all lookups are canceled by this time
		while (layer.m_Dictionaries.size () > 1)
			layer.m_Dictionaries.pop_back ();

		for (auto &obj : layer.m_TextFileObjects) {
			// For those that didn't even had ongoing lookups
			obj.m_CancelLookup = false;
		}
	} else { // Split
		// cancel all lookups // No Need though
		for (auto &obj : layer.m_TextFileObjects)
			obj.m_CancelLookup = true;

		uint32_t *save_state = new uint32_t[layer.m_TextFileObjects.size () + 1];
		{
			uint32_t i = 0;
			for (auto &obj: layer.m_TextFileObjects) {
				save_state[i] = uint32_t(obj.m_MyDictionary - &layer.m_Dictionaries[0]);
				i++;
			}
		}
		while (layer.m_Dictionaries.size () < layer.m_TextFileObjects.size ()) {
			layer.m_Dictionaries.push_back (layer.m_Dictionaries[0]);

			// Link
			save_state[layer.m_Dictionaries.size () - 1] = layer.m_Dictionaries.size () - 1;
		}

		for (uint32_t i = 0; i < layer.m_TextFileObjects.size (); i++)
			layer.m_TextFileObjects[i].m_MyDictionary = &layer.m_Dictionaries[save_state[i]];
		
		delete[] save_state;

		for (auto &obj : layer.m_TextFileObjects)
			obj.m_CancelLookup = false;
	}
	return state;
}

////////////////////////////////////