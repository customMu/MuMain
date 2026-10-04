#include "stdafx.h"
#include "Engine/Pathing/ZzzPath.h"

#ifdef CSK_DEBUG_MAP_PATHFINDING
bool g_bShowPath = false;
#endif // CSK_DEBUG_MAP_PATHFINDING

int PATH::s_iDir[8][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

namespace
{
// Min-heap order of the open nodes: the lowest estimated total cost first.
bool IsMoreExpensive(const std::pair<int, int>& lhs, const std::pair<int, int>& rhs)
{
    return lhs.first > rhs.first;
}
} // namespace

bool PATH::IsWalkable(int iIndex, int iWall) const
{
    int byMapAttribute = m_pbyMap[iIndex];
    byMapAttribute &= ~(TW_ACTION | TW_HEIGHT | TW_CAMERA_UP);
    return iWall > byMapAttribute;
}

bool PATH::OpenNode(int iIndex, int iCostToStart, int xPrev, int yPrev, int xEnd, int yEnd)
{
    if (!AddClearPos(iIndex))
    {
        return false;
    }

    int x, y;
    GetXYPos(iIndex, &x, &y);
    m_pbyClosed[iIndex] |= PATH_INTESTLIST;
    m_piCostToStart[iIndex] = iCostToStart;
    m_pxPrev[iIndex] = xPrev;
    m_pyPrev[iIndex] = yPrev;
    m_openNodes.emplace_back(iCostToStart + EstimateCostToGoal(xEnd, yEnd, x, y), iIndex);
    std::push_heap(m_openNodes.begin(), m_openNodes.end(), IsMoreExpensive);
    return true;
}

bool PATH::MarkEndNodes(bool bErrorCheck, int iWall, bool Value, int xEnd, int yEnd, float fDistance)
{
    if (0.0f != fDistance)
    {
        SetEndNodes(bErrorCheck, iWall, xEnd, yEnd, fDistance);
        return true;
    }

    int iEndIndex = GetIndex(xEnd, yEnd);
    if (iEndIndex < 0 || iEndIndex >= m_iSize)
    {
        return false;
    }

    if (Value)
    {
        m_pbyMap[iEndIndex] = 0;
    }

    if (bErrorCheck && (iWall <= m_pbyMap[iEndIndex] && (m_pbyMap[iEndIndex] & TW_ACTION) != TW_ACTION))
    {
        return false;
    }

    m_pbyClosed[iEndIndex] = PATH_END;
    return AddClearPos(iEndIndex);
}

// A* search over the walk map. With bErrorCheck the path must reach an end node; without it,
// the path leads to the reachable node which is the closest to the end (e.g. when the end is a wall).
bool PATH::FindPath(int xStart, int yStart, int xEnd, int yEnd, bool bErrorCheck, int iWall, bool Value,
                    float fDistance)
{
    Init();
    m_openNodes.clear();

    if (xStart == 0 || yStart == 0 || !CheckXYPos(xStart, yStart))
    {
        return false;
    }

    if (!MarkEndNodes(bErrorCheck, iWall, Value, xEnd, yEnd, fDistance))
    {
        return false;
    }

    if (!OpenNode(GetIndex(xStart, yStart), 0, xStart, yStart, xEnd, yEnd))
    {
        return false;
    }

    // Without a path to the end, the search may stop right next to an end which can't be entered.
    const bool bEndBlocked = !bErrorCheck && 0.0f == fDistance && !IsWalkable(GetIndex(xEnd, yEnd), iWall);
    int iCostToGoalOfNearest = MAX_INT_FORPATH;
    int xNearest = xStart;
    int yNearest = yStart;

    for (int iExpanded = 0; !m_openNodes.empty() && iExpanded < MAX_PATH_SEARCH_NODES;)
    {
        std::pop_heap(m_openNodes.begin(), m_openNodes.end(), IsMoreExpensive);
        const int iIndex = m_openNodes.back().second;
        m_openNodes.pop_back();
        if (PATH_TESTED & m_pbyClosed[iIndex])
        {
            continue; // a stale entry of a node which was reached cheaper before
        }

        m_pbyClosed[iIndex] |= PATH_TESTED;
        ++iExpanded;

        int xTest, yTest;
        GetXYPos(iIndex, &xTest, &yTest);
        if (PATH_END & m_pbyClosed[iIndex])
        {
            m_openNodes.clear();
            return GeneratePath(xStart, yStart, xTest, yTest);
        }

        if (!bErrorCheck)
        {
            const int iCostToGoal = EstimateCostToGoal(xEnd, yEnd, xTest, yTest);
            if (iCostToGoal < iCostToGoalOfNearest)
            {
                iCostToGoalOfNearest = iCostToGoal;
                xNearest = xTest;
                yNearest = yTest;
            }

            if (bEndBlocked && abs(xEnd - xTest) <= 1 && abs(yEnd - yTest) <= 1)
            {
                break;
            }
        }

        for (int i = 0; i < 8; i++)
        {
            const int xNew = xTest + s_iDir[i][0];
            const int yNew = yTest + s_iDir[i][1];
            if (!CheckXYPos(xNew, yNew))
            {
                continue;
            }

            const int iNewIndex = GetIndex(xNew, yNew);
            if ((PATH_TESTED & m_pbyClosed[iNewIndex]) || !IsWalkable(iNewIndex, iWall))
            {
                continue;
            }

            const int iNewCost = m_piCostToStart[iIndex] + CalculateCostToStartAddition(s_iDir[i][0], s_iDir[i][1]);
            if ((PATH_INTESTLIST & m_pbyClosed[iNewIndex]) && iNewCost >= m_piCostToStart[iNewIndex])
            {
                continue;
            }

            if (!OpenNode(iNewIndex, iNewCost, xTest, yTest, xEnd, yEnd))
            {
                m_openNodes.clear();
                return false;
            }
        }
    }

    m_openNodes.clear();
    return !bErrorCheck && GeneratePath(xStart, yStart, xNearest, yNearest);
}

int PATH::EstimateCostToGoal(int xEnd, int yEnd, int xNew, int yNew)
{
    const int xDist = abs(xNew - xEnd);
    const int yDist = abs(yNew - yEnd);
    return abs(xDist - yDist) * FACTOR_PATH_DIST + std::min<int>(xDist, yDist) * FACTOR_PATH_DIST_DIAG;
}
