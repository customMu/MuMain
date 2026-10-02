#pragma once

#include <functional>
#include <array>
#include <set>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

#include "MuHelperData.h"

namespace MUHelper
{
	class CMuHelper
	{
	public:
		CMuHelper() = default;
		~CMuHelper() = default;

	public:
		static void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

		ConfigData GetConfig() const;
		void Save(const ConfigData& config);
		void Load(const ConfigData& config);
		void Start();
		void Stop();
		void Toggle();
		void TriggerStart();
		void TriggerStop();
		bool IsActive() { return m_bActive; }
		void AddCost(int iCost) { m_iTotalCost += iCost; }
		int GetTotalCost() { return m_iTotalCost; }

		// Session statistics (shown under the helper buttons, for balance tests): reset on start, kept after stop until the next start.
		void RecordKill() { if (m_bActive) ++m_iSessionKills; }
		void AddExperience(int64_t iExperience, bool bMaster) { if (m_bActive) (bMaster ? m_iSessionMasterExperience : m_iSessionExperience) += iExperience; }
		void AddZen(int64_t iZen) { if (m_bActive) m_iSessionZen += iZen; }
		bool HasSessionStats() const { return m_ullSessionStart != 0; }
		uint64_t GetSessionMilliseconds() const { return (m_bActive ? GetTickCount64() : m_ullSessionStop.load()) - m_ullSessionStart; }
		int GetSessionKills() const { return m_iSessionKills; }
		int64_t GetSessionExperience() const { return m_iSessionExperience; }
		int64_t GetSessionMasterExperience() const { return m_iSessionMasterExperience; }
		int64_t GetSessionZen() const { return m_iSessionZen; }

		// Hits: dealt by the hero to monsters, and taken by the hero (damage 0 = miss).
		void AddHitDealt(int64_t iDamage) { if (m_bActive) { ++m_iSessionHitsDealt; if (iDamage > 0) m_iSessionDamageDealt += iDamage; else ++m_iSessionMissesDealt; } }
		void AddHitTaken(int64_t iDamage) { if (m_bActive) { ++m_iSessionHitsTaken; if (iDamage > 0) m_iSessionDamageTaken += iDamage; else ++m_iSessionMissesTaken; } }
		// Records the death and keeps the time (local clock) and the killer of the last one.
		void RecordDeath(const wchar_t* szKiller);
		bool GetLastDeath(std::wstring& time, std::wstring& killer) const;
		// iSize: 0 small, 1 medium, 2 large; iPrice: shop price of one potion
		void RecordPotion(bool bMana, int iSize, int iPrice) { if (m_bActive && iSize >= 0 && iSize < 3) { ++m_aiSessionPotions[(bMana ? 3 : 0) + iSize]; m_iSessionPotionCost += iPrice; } }
		int64_t GetSessionDamageDealt() const { return m_iSessionDamageDealt; }
		int GetSessionHitsDealt() const { return m_iSessionHitsDealt; }
		int GetSessionMissesDealt() const { return m_iSessionMissesDealt; }
		int64_t GetSessionDamageTaken() const { return m_iSessionDamageTaken; }
		int GetSessionHitsTaken() const { return m_iSessionHitsTaken; }
		int GetSessionMissesTaken() const { return m_iSessionMissesTaken; }
		int GetSessionDeaths() const { return m_iSessionDeaths; }
		int GetSessionPotions(bool bMana, int iSize) const { return m_aiSessionPotions[(bMana ? 3 : 0) + iSize]; }
		int64_t GetSessionPotionCost() const { return m_iSessionPotionCost; }

		// Jewels picked up during the session (only shown after the first one).
		static constexpr int JewelTypeCount = 10;
		static const std::array<int, JewelTypeCount>& GetJewelTypes();
		void RecordPickedItem(int iItemType, int iCount);
		int GetSessionJewels(int iJewelIndex) const { return m_aiSessionJewels[iJewelIndex]; }

		void AddTarget(int iTargetId, bool bIsAttacking);
		void DeleteTarget(int iTargetId);
		void DeleteAllTargets();

		void AddItem(int iItemId, POINT posDropped);
		void DeleteItem(int iItemId);

	private:
		void WorkLoop(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
		void Work();
		int ActivatePet();
		int Buff();
		int BuffTarget(CHARACTER* pTargetChar, ActionSkillType iBuffSkill);
		int RecoverHealth();
		int Heal();
		int HealSelf(ActionSkillType iHealingSkill);
		int DrainLife();
		int ConsumePotion();
		int Attack();
		int RepairEquipments();
		int Regroup();
		ActionSkillType SelectAttackSkill();
		int SimulateAttack(ActionSkillType iSkill);
		int SimulateSkill(ActionSkillType iSkill, bool bTargetRequired, int iTarget);
		int SimulateBasicAttack(int iTarget);
		int SimulateComboAttack();
		int GetNearestTarget();
		int GetFarthestAttackingTarget();
		void CleanupTargets();
		int ComputeDistanceByRange(int iRange);
		int ComputeDistanceFromTarget(CHARACTER* pTarget);
		int ComputeDistanceBetween(POINT posA, POINT posB);
		int SimulateMove(POINT posMove);
		int ObtainItem();
		int SelectItemToObtain();
		bool ShouldObtainItem(int iItemId);
		ActionSkillType GetHealingSkill();
		ActionSkillType GetDrainLifeSkill();
		bool HasAssignedBuffSkill();
		bool IsSelfPositionSkill(ActionSkillType iSkill);

	private:
		ConfigData m_config;
		POINT m_posOriginal;
		std::thread m_timerThread;
		std::atomic<bool> m_bActive;
		std::set<int> m_setTargets;
		std::set<int> m_setTargetsAttacking;
		std::set<int> m_setItems;
		int m_iCurrentItem;
		int m_iCurrentTarget;
		int m_iCurrentBuffIndex;
		int m_iCurrentBuffPartyIndex;
		int m_iCurrentHealPartyIndex;
		int m_iComboState;
		ActionSkillType m_iCurrentSkill;
		int m_iHuntingDistance;
		int m_iObtainingDistance;
		int m_iLoopCounter;
		int m_iSecondsElapsed;
		int m_iSecondsAway;
		bool m_bTimerActivatedBuffOngoing;
		bool m_bPetActivated;
		int m_iTotalCost;
		std::atomic<uint64_t> m_ullSessionStart{ 0 };
		std::atomic<uint64_t> m_ullSessionStop{ 0 };
		std::atomic<int> m_iSessionKills{ 0 };
		std::atomic<int64_t> m_iSessionExperience{ 0 };
		std::atomic<int64_t> m_iSessionMasterExperience{ 0 };
		std::atomic<int64_t> m_iSessionZen{ 0 };
		std::atomic<int64_t> m_iSessionDamageDealt{ 0 };
		std::atomic<int> m_iSessionHitsDealt{ 0 };
		std::atomic<int> m_iSessionMissesDealt{ 0 };
		std::atomic<int64_t> m_iSessionDamageTaken{ 0 };
		std::atomic<int> m_iSessionHitsTaken{ 0 };
		std::atomic<int> m_iSessionMissesTaken{ 0 };
		std::atomic<int> m_iSessionDeaths{ 0 };
		std::array<std::atomic<int>, 6> m_aiSessionPotions{};
		std::atomic<int64_t> m_iSessionPotionCost{ 0 };
		std::array<std::atomic<int>, JewelTypeCount> m_aiSessionJewels{};
		mutable std::mutex m_lastDeathMutex;
		std::wstring m_strLastDeathTime;
		std::wstring m_strLastDeathKiller;
	};

	extern CMuHelper g_MuHelper;
}