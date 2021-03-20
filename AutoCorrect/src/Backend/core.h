#pragma once

#define HZ_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#define BIT(x) (1 << x)
#define MIN(x,y) (x > y ? y:x)
#define MAX(x,y) (x > y ? x:y)