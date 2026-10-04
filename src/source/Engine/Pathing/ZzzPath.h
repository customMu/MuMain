#ifdef CSK_DEBUG_MAP_PATHFINDING
#define SHOW_PATH_INFO
extern bool g_bShowPath;
#endif // CSK_DEBUG_MAP_PATHFINDING

#include <math.h>
#include <utility>
#include <vector>
#include "Core/Utilities/BaseCls.h"
#include "Core/Utilities/Log/ErrorReport.h"


class PATH
{
public:
    PATH();
    ~PATH();

public:
    void SetMapDimensions(int iWidth, int iHeight, WORD* pbyMap);

private:
    int	m_iWidth, m_iHeight;
    int m_iSize;
    WORD* m_pbyMap;

private:
    int m_iNumPath;
    BYTE m_xPath[MAX_COUNT_PATH];
    BYTE m_yPath[MAX_COUNT_PATH];
public:
    int GetPath(void) { return (std::min<int>(m_iNumPath, MAX_PATH_FIND)); }
    BYTE* GetPathX(void) { return (m_xPath + MAX_COUNT_PATH - m_iNumPath); }
    BYTE* GetPathY(void) { return (m_yPath + MAX_COUNT_PATH - m_iNumPath); }

    int GetIndex(int xPos, int yPos) { return (xPos + yPos * m_iWidth); }
    void GetXYPos(int iIndex, int* pxPos, int* pyPos) { *pxPos = iIndex % m_iWidth; *pyPos = iIndex / m_iWidth; }
    BOOL CheckXYPos(int xPos, int yPos) { return (xPos >= 0 && yPos >= 0 && xPos < m_iWidth && yPos < m_iHeight); }
    static int s_iDir[8][2];

private:
    EPathNodeState* m_pbyClosed;
    int m_iMinClosed, m_iMaxClosed;
    int* m_piCostToStart;
    int* m_pxPrev;
    int* m_pyPrev;
    // Open nodes as (estimated total cost, index), a min-heap; nodes which got a cheaper cost stay as stale entries.
    std::vector<std::pair<int, int>> m_openNodes;

    void Clear(void);
    bool AddClearPos(int iIndex);
    void Init(void);
    bool IsWalkable(int iIndex, int iWall) const;
    bool OpenNode(int iIndex, int iCostToStart, int xPrev, int yPrev, int xEnd, int yEnd);

public:
    bool FindPath(int xStart, int yStart, int xEnd, int yEnd, bool bErrorCheck, int iWall, bool Value, float fDistance = 0.0f);

private:
    void SetEndNodes(bool bErrorCheck, int iWall, int xEnd, int yEnd, float fDistance);
    bool MarkEndNodes(bool bErrorCheck, int iWall, bool Value, int xEnd, int yEnd, float fDistance);
    int CalculateCostToStartAddition(int xDir, int yDir);
    static int EstimateCostToGoal(int xEnd, int yEnd, int xNew, int yNew);
    bool GeneratePath(int xStart, int yStart, int xEnd, int yEnd);

#ifdef SHOW_PATH_INFO
public:
    BYTE GetClosedStatus(int iIndex) { return (m_pbyClosed[iIndex]); }
#endif // SHOW_PATH_INFO
};

inline PATH::PATH()
{
    m_pbyClosed = NULL;
    m_piCostToStart = NULL;
    m_pxPrev = NULL;
    m_pyPrev = NULL;
}

inline PATH::~PATH()
{
    Clear();
}

inline void PATH::Clear(void)
{
    if (m_pbyClosed)
    {
        delete[] m_pbyClosed;
        m_pbyClosed = NULL;
    }
    if (m_piCostToStart)
    {
        delete[] m_piCostToStart;
        m_piCostToStart = NULL;
    }
    if (m_pxPrev)
    {
        delete[] m_pxPrev;
        m_pxPrev = NULL;
    }
    if (m_pyPrev)
    {
        delete[] m_pyPrev;
        m_pyPrev = NULL;
    }
    m_iMinClosed = MAX_INT_FORPATH;
    m_iMaxClosed = -1;
}

inline void PATH::SetMapDimensions(int iWidth, int iHeight, WORD* pbyMap)
{
    Clear();

    m_iWidth = iWidth;
    m_iHeight = iHeight;
    m_pbyMap = pbyMap;
    m_iSize = m_iWidth * m_iHeight;

    m_pbyClosed = new EPathNodeState[m_iSize];
    m_piCostToStart = new int[m_iSize];
    m_pxPrev = new int[m_iSize];
    m_pyPrev = new int[m_iSize];
    ZeroMemory(m_pbyClosed, m_iSize * sizeof(BYTE));
}

inline bool PATH::AddClearPos(int iIndex)
{
    if (iIndex < 0 || iIndex >= m_iSize)
    {
        return false;
    }

    m_iMinClosed = std::min<int>(iIndex, m_iMinClosed);
    m_iMaxClosed = std::max<int>(iIndex, m_iMaxClosed);

    return true;
}

inline void PATH::Init(void)
{
    if (MAX_INT_FORPATH == m_iMinClosed)
    {
        return;
    }

    ZeroMemory(&(m_pbyClosed[m_iMinClosed]), (m_iMaxClosed - m_iMinClosed + 1) * sizeof(BYTE));
    m_iMinClosed = MAX_INT_FORPATH;
    m_iMaxClosed = -1;
}

inline void PATH::SetEndNodes(bool bErrorCheck, int iWall, int xEnd, int yEnd, float fDistance)
{
    int iDistance = (int)fDistance;
    for (int j = -iDistance; j <= iDistance; j++)
    {
        int xRange = iDistance - abs(j);
        for (int i = -xRange; i <= 0; i++)
        {
            int iEndIndex = GetIndex(xEnd + i, yEnd + j);
            if (iEndIndex >= 0 && iEndIndex < (m_iSize))
            {
                if (!(bErrorCheck && iWall <= m_pbyMap[iEndIndex]))
                {
                    m_pbyClosed[iEndIndex] = PATH_END;
                    AddClearPos(iEndIndex);
                }
            }
            else
            {
                g_ErrorReport.Write(L"Error Path : %d \r\n", iEndIndex);
            }
            iEndIndex = GetIndex(xEnd - i, yEnd + j);

            if (iEndIndex >= 0 && iEndIndex < (m_iSize))
            {
                if (!(bErrorCheck && iWall <= m_pbyMap[iEndIndex]))
                {
                    m_pbyClosed[iEndIndex] = PATH_END;
                    AddClearPos(iEndIndex);
                }
            }
            else
            {
                g_ErrorReport.Write(L"Error Path : %d \r\n", iEndIndex);
            }
        }

        for (int i = -iDistance; i < -xRange; i++)
        {
            if ((float)sqrt((float)(i * i + j * j)) < fDistance)
            {
                int iEndIndex = GetIndex(xEnd + i, yEnd + j);
                if (iEndIndex >= 0 && iEndIndex < (m_iSize))
                {
                    if (!(bErrorCheck && iWall <= m_pbyMap[iEndIndex]))
                    {
                        m_pbyClosed[iEndIndex] = PATH_END;
                        AddClearPos(iEndIndex);
                    }
                }
                else
                {
                    g_ErrorReport.Write(L"Error Path : %d \r\n", iEndIndex);
                }

                iEndIndex = GetIndex(xEnd - i, yEnd + j);
                if (iEndIndex >= 0 && iEndIndex < (m_iSize))
                {
                    if (!(bErrorCheck && iWall <= m_pbyMap[iEndIndex]))
                    {
                        m_pbyClosed[iEndIndex] = PATH_END;
                        AddClearPos(iEndIndex);
                    }
                }
                else
                {
                    g_ErrorReport.Write(L"Error Path : %d \r\n", iEndIndex);
                }
            }
        }
    }
}

inline int PATH::CalculateCostToStartAddition(int xDir, int yDir)
{
    return ((xDir == 0 || yDir == 0) ? FACTOR_PATH_DIST : FACTOR_PATH_DIST_DIAG);
}

inline bool PATH::GeneratePath(int xStart, int yStart, int xEnd, int yEnd)
{
    int xCurrent = xEnd;
    int yCurrent = yEnd;
    for (m_iNumPath = 0; m_iNumPath < MAX_COUNT_PATH; m_iNumPath++)
    {
        m_xPath[(MAX_COUNT_PATH - 1) - m_iNumPath] = xCurrent;
        m_yPath[(MAX_COUNT_PATH - 1) - m_iNumPath] = yCurrent;

        if (xCurrent == xStart && yCurrent == yStart)
        {
            m_iNumPath++;
            return (true);
        }

        int iIndex = GetIndex(xCurrent, yCurrent);
        xCurrent = m_pxPrev[iIndex];
        yCurrent = m_pyPrev[iIndex];
    }

    return (false);
}

inline POINT MovePoint(EPathDirection direction, POINT position)
{
    switch (direction)
    {
    case EPathDirection::WEST:       position.x--; position.y--; break;
    case EPathDirection::SOUTHWEST:  position.y--; break;
    case EPathDirection::SOUTH:      position.x++; position.y--; break;
    case EPathDirection::SOUTHEAST:  position.x++; break;
    case EPathDirection::EAST:       position.x++; position.y++; break;
    case EPathDirection::NORTHEAST:  position.y++; break;
    case EPathDirection::NORTH:      position.x--; position.y++; break;
    case EPathDirection::NORTHWEST:  position.x--; break;
    }
    return position;
}
