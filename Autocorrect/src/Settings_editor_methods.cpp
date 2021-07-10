#include "new_MVC_Layer.h"
#include "Text_Object.h"
#include "Settings_editor_methods.h"

////////////////////////////////////
///
///  Methods When Changing Settings

bool ChangeUnifiedDictionaryState (const bool state, new_MVC_Layer &layer)
{
	if (layer.m_TextFileObjects.size () < 2) // Early return
		return state;

	if (state) { // Available to Unify
		// cancel all lookups
		for (auto &obj : layer.m_TextFileObjects)
			obj.m_CancelLookup = true;

		std::vector<std::string> locationOfDictionaries;
		for (uint32_t i = 1; i < layer.m_Dictionaries.size (); i++) {
			for (std::string &dict_path : layer.m_Dictionaries[i].second)
				locationOfDictionaries.emplace_back (std::move (dict_path));
			layer.m_Dictionaries[i].second.clear ();
		} // pushed all locations

		// filter duplicates and remove those already loaded in 1st dictionary
		{
			bool *non_duplicates = new bool[locationOfDictionaries.size ()];

			uint32_t i = 0;
			for (std::string &str_ref1 : locationOfDictionaries) { // assuming no duplicates in layer.m_Dictionaries[0].second
				non_duplicates[i] = true;
				for (uint32_t j = i + 1; j < locationOfDictionaries.size () + layer.m_Dictionaries[0].second.size (); j++) {
					std::string *str_ref2 = nullptr;
					if (j < locationOfDictionaries.size ())
						str_ref2 = &locationOfDictionaries[j];
					else str_ref2 = &layer.m_Dictionaries[0].second[j - locationOfDictionaries.size ()];

					if (str_ref1 == *str_ref2)
						non_duplicates[i] = false;
				}
				i++;
			}

			for (i = locationOfDictionaries.size () - 1; i != 0; i--)
				if (non_duplicates[i] == false)
					locationOfDictionaries.erase (locationOfDictionaries.begin () + i);
			delete[] non_duplicates;
		}

		for (std::string &filePath : locationOfDictionaries) { // Unify dictionary
			layer.m_Dictionaries[0].first.LoadDictionary (filePath, 0, 1, XL (' '));
			layer.m_Dictionaries[0].second.push_back (std::move (filePath));
		}
		locationOfDictionaries.clear ();

		for (uint32_t i = 1; i < layer.m_TextFileObjects.size (); i++) {
			layer.m_TextFileObjects[i].m_Symspell = &layer.m_Dictionaries[0].first;
			layer.m_TextFileObjects[i].m_LoadedDictionaries = &layer.m_Dictionaries[0].second;
		} // Link all

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
				save_state[i] = uint32_t(obj.m_Symspell - &layer.m_Dictionaries[0].first);
				i++;
			}
		}
		while (layer.m_Dictionaries.size () < layer.m_TextFileObjects.size ()) {
			layer.m_Dictionaries.push_back (layer.m_Dictionaries[0]);

			// Link
			save_state[layer.m_Dictionaries.size () - 1] = layer.m_Dictionaries.size () - 1;
		}

		for (uint32_t i = 0; i < layer.m_TextFileObjects.size (); i++)
			layer.m_TextFileObjects[i].m_Symspell = &layer.m_Dictionaries[save_state[i]].first, layer.m_TextFileObjects[i].m_LoadedDictionaries = &layer.m_Dictionaries[save_state[i]].second;
		
		if (layer.m_TextFileObjects.size () > 1)
			delete[] save_state;
		else delete save_state;

		for (auto &obj : layer.m_TextFileObjects)
			obj.m_CancelLookup = false;
	}
	return state;
}

////////////////////////////////////