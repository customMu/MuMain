#include "stdafx.h"
#include "GameLogic/Events/KalimaEntry.h"

#include <utility>

namespace GameLogic::Events::KalimaEntry
{
    namespace
    {
        Info s_info;
    }

    void SetInfo(Info info)
    {
        s_info = std::move(info);
    }

    const Info& GetInfo()
    {
        return s_info;
    }

    bool CanEnter()
    {
        return s_info.canReenter || (s_info.tierLevel > 0 && s_info.entriesLeft > 0);
    }
}
