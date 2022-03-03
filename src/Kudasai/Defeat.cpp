
#include "Kudasai/Defeat.h"

namespace Kudasai
{
	namespace Defeat
	{
		void defeat(RE::Actor* subject)
		{
			logger::info("Defeating Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
			Srl::GetSingleton()->defeats.emplace(subject->GetFormID());

			pacify(subject);

			if (subject->IsPlayerRef()) {
				using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
				auto cmap = RE::ControlMap::GetSingleton();
				cmap->ToggleControls(UEFlag::kActivate, false);
				cmap->ToggleControls(UEFlag::kJumping, false);
				cmap->ToggleControls(UEFlag::kMenu, false);
			}
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			subject->boolFlags.set(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
			subject->boolFlags.set(RE::Actor::BOOL_FLAGS::kInBleedoutAnimation);
			auto args = RE::MakeFunctionArguments(std::move(subject));
			vm->DispatchStaticCall("KudasaiInternal", "FinalizeDefeat", args, callback);

			static const auto defeatkeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x7946FF, "YKudasai.esp");
			auto ref = subject->GetObjectReference();
			if (!ref) {
				logger::warn("BoundObject does not exist? Skipping Keyword application");
				return;
			}
			ref->As<RE::BGSKeywordForm>()->AddKeyword(defeatkeyword);

			logger::info("Defeat completed");
		}

		void rescue(RE::Actor* subject, const bool undo_pacify)
		{
			logger::info("Rescueing Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
			if (Srl::GetSingleton()->defeats.erase(subject->GetFormID()) == 0)
				return;

			subject->boolFlags.reset(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
			if (subject->IsPlayerRef()) {
				using UEFlag = RE::UserEvents::USER_EVENT_FLAG;
				auto cmap = RE::ControlMap::GetSingleton();
				cmap->ToggleControls(UEFlag::kActivate, true);
				cmap->ToggleControls(UEFlag::kJumping, true);
				cmap->ToggleControls(UEFlag::kMenu, true);
			}
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			auto args = RE::MakeFunctionArguments(std::move(subject));
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			vm->DispatchStaticCall("KudasaiInternal", "FinalizeRescue", args, callback);

			if (undo_pacify)
				undopacify(subject);

			const auto defeatkeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x7946FF, "YKudasai.esp");
			auto ref = subject->GetObjectReference();
			if (!ref) {
				logger::warn("BoundObject does not exist? Skipping Keyword application");
				return;
			}
			ref->As<RE::BGSKeywordForm>()->RemoveKeyword(defeatkeyword);

			logger::info("Rescue completed");
		}


		void pacify(RE::Actor* subject)
		{
			logger::info("Pacyfying Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
			Srl::GetSingleton()->pacifies.emplace(subject->GetFormID());
			RE::ProcessLists::GetSingleton()->StopCombatAndAlarmOnActor(subject, false);
			subject->StopCombat();

			static const auto defeatkeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x7D1354, "YKudasai.esp");
			auto ref = subject->GetObjectReference();
			if (!ref) {
				logger::warn("BoundObject does not exist? Skipping Keyword application");
				return;
			}
			ref->As<RE::BGSKeywordForm>()->AddKeyword(defeatkeyword);

			logger::info("Pacification completed");
		}

		void undopacify(RE::Actor* subject)
		{
			logger::info("Undoing Pacify Actor: {} ( {} )", subject->GetDisplayFullName(), subject->GetFormID());
			if (Srl::GetSingleton()->pacifies.erase(subject->GetFormID()) == 0)
				return;

			const auto defeatkeyword = RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x7D1354, "YKudasai.esp");
			auto ref = subject->GetObjectReference();
			if (!ref) {
				logger::warn("BoundObject does not exist? Skipping Keyword application");
				return;
			}
			ref->As<RE::BGSKeywordForm>()->RemoveKeyword(defeatkeyword);

			logger::info("UndoPacify completed");
		}

		bool isdefeated(RE::Actor* subject)
		{
			auto srl = Srl::GetSingleton();
			return srl->defeats.contains(subject->GetFormID());
		}

		bool ispacified(RE::Actor* subject)
		{
			auto srl = Srl::GetSingleton();
			return srl->pacifies.contains(subject->GetFormID());
		}

		void setdamageimmune(RE::Actor* subject, bool immune)
		{
			auto srl = Srl::GetSingleton();
			auto key = subject->GetFormID();
			if (immune)
				srl->defeats.emplace(key);
			else
				srl->defeats.erase(key);
		}

	}  // namespace Defeat

}  // namespace Kudasai
