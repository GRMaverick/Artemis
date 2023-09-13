#pragma once

#include "Logging\Logger.h"

#if defined(LogInfo)
#undef LogInfo
#endif
#define LogInfo(pFormat,...)			Artemis::Utilities::Logger::Log(Artemis::Utilities::SEVERITY_INFO, Artemis::Utilities::CATEGORY_APP, pFormat, __VA_ARGS__)

#if defined(LogWarning)
#undef LogWarning
#endif
#define LogWarning(pFormat, ...)		Artemis::Utilities::Logger::Log(Artemis::Utilities::SEVERITY_WARN, Artemis::Utilities::CATEGORY_APP, pFormat, __VA_ARGS__)

#if defined(LogError)
#undef LogError
#endif
#define LogError(pFormat, ...)			Artemis::Utilities::Logger::Log(Artemis::Utilities::SEVERITY_ERR, Artemis::Utilities::CATEGORY_APP, pFormat, __VA_ARGS__)

#if defined(LogFatal)
#undef LogFatal
#endif
#define LogFatal(pFormat, ...)			Artemis::Utilities::Logger::Log(Artemis::Utilities::SEVERITY_FATAL, Artemis::Utilities::CATEGORY_APP, pFormat, __VA_ARGS__)

#define PRAGMA_TODO(todo)	__pragma(message("[TODO]: "todo));
