#pragma once

namespace GLCore
{
	class FileDialogs
	{
	public:
		// returns Empty string if canceled
		static std::string OpenFile (const char *filter);

		// returns Empty string if canceled
		static std::string SaveFile (const char *filter);
	};
}
