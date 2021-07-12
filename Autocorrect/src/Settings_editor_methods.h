#pragma once

static const int symspell_Max_Edit_Distance = 3, symspell_Prefix_length = 3;

#define MIN(x,y) (x > y ? y :  x)
#define MAX(x,y) (x > y ? x :  y)
#define ABS(x)   (x > 0 ? x : -x)
#define MOD(x,y) (x - ((int)(((float)x) / y) * y))


class new_MVC_Layer;
bool ChangeUnifiedDictionaryState (const bool state, new_MVC_Layer &layer);