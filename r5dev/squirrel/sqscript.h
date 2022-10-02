#pragma once
#include "squirrel/sqtype.h"
#include "squirrel/sqvm.h"

struct ScriptFunctionBinding_t
{
	const SQChar* _scriptname; // 00
	const SQChar* _nativename; // 08
	const SQChar* _helpstring; // 10
	const SQChar* _returntype; // 18
	const SQChar* _parameters; // 20
	std::int16_t unk28;        // 28
	std::int16_t padding1;     // 2A
	std::int32_t unk2c;        // 2C
	const SQChar* _codehook;   // 30
	std::int32_t unk38;        // 38
	SQInteger _nparamscheck;   // 3C
	std::int64_t unk40;        // 40
	std::int64_t unk48;        // 48
	std::int64_t unk50;        // 50
	std::int32_t unk58;        // 58
	std::int32_t padding3;     // 5C
	void* _functor;            // 60

	ScriptFunctionBinding_t()
	{
		_scriptname = nullptr;
		_nativename = nullptr;
		_helpstring = nullptr;
		_returntype = nullptr;
		_parameters = nullptr;
		unk28 = 0;
		padding1 = 0;
		unk2c = 0;
		_codehook = nullptr;
		unk38 = 0;
		_nparamscheck = 6;
		unk40 = 0;
		unk48 = 0;
		unk50 = 0;
		unk58 = 0;
		padding3 = 0;
		_functor = nullptr;
	}
};

#pragma pack(push, 4)
class CSquirrelVM
{
public:
	HSQUIRRELVM GetVM() const
	{
		return m_sqVM;
	}

private:
	SQChar pad0[0x8];
	HSQUIRRELVM m_sqVM;
	SQChar pad1[0x8];
	SQInteger m_nFlags;
	SQChar pad2[4];
	SQChar pad3[16];
#if !defined (GAMEDLL_S0) && !defined (GAMEDLL_S1)
	SQChar pad4[4];
#endif
	SQInteger m_nTick;
	SQChar pad5[4];
	SQCONTEXT m_iContext;
#if !defined (GAMEDLL_S2) && !defined (GAMEDLL_S3)
	SQChar pad6[4];
#endif
	void* m_pCompareFunc;
};
#pragma pack(pop)

inline CMemory p_Script_RegisterFunction;
inline auto v_Script_RegisterFunction = p_Script_RegisterFunction.RCast<SQRESULT(*)(CSquirrelVM* s, ScriptFunctionBinding_t* binding, SQInteger a1)>();

inline CMemory p_Script_RegisterConstant;
inline auto v_Script_RegisterConstant = p_Script_RegisterConstant.RCast<SQRESULT(*)(CSquirrelVM* s, const SQChar* name, SQInteger value)>();
#if !defined (CLIENT_DLL)
inline CMemory p_Script_InitializeSVGlobalStructs;
inline auto v_Script_InitializeSVGlobalStructs = p_Script_InitializeSVGlobalStructs.RCast<SQRESULT(*)(HSQUIRRELVM v)>();
#endif // !CLIENT_DLL
#if !defined (DEDICATED)
inline CMemory p_Script_InitializeCLGlobalStructs;
inline auto v_Script_InitializeCLGlobalStructs = p_Script_InitializeCLGlobalStructs.RCast<SQRESULT(*)(HSQUIRRELVM v, SQCONTEXT context)>();
#endif // !DEDICATED
#if !defined (CLIENT_DLL) && defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
inline CMemory p_Script_CreateServerVM;
inline auto v_Script_CreateServerVM = p_Script_CreateServerVM.RCast<SQBool(*)(void)>();
#elif !defined (CLIENT_DLL) && defined (GAMEDLL_S3) || defined (GAMEDLL_S2)
inline CMemory p_Script_CreateServerVM;
inline auto v_Script_CreateServerVM = p_Script_CreateServerVM.RCast<SQBool(*)(void)>();
#endif
#if !defined (DEDICATED) && defined (GAMEDLL_S0) || defined (GAMEDLL_S1) || defined (GAMEDLL_S2)
inline CMemory p_Script_CreateClientVM;
inline auto v_Script_CreateClientVM = p_Script_CreateClientVM.RCast<SQBool(*)(CHLClient* hlclient)>();
#elif !defined (DEDICATED) && defined (GAMEDLL_S3)
inline CMemory p_Script_CreateClientVM;
inline auto v_Script_CreateClientVM = p_Script_CreateClientVM.RCast<SQBool(*)(CHLClient* hlclient)>();
#endif
#if !defined (DEDICATED)
inline CMemory p_Script_CreateUIVM;
inline auto v_Script_CreateUIVM = p_Script_CreateUIVM.RCast<SQBool(*)(void)>();
#endif // !DEDICATED
inline CMemory p_Script_DestroySignalEntryListHead;
inline auto v_Script_DestroySignalEntryListHead = p_Script_DestroySignalEntryListHead.RCast<SQBool(*)(CSquirrelVM* s, HSQUIRRELVM v, SQFloat f)>();

inline CMemory p_Script_LoadRson;
inline auto v_Script_LoadRson = p_Script_LoadRson.RCast<SQInteger(*)(const SQChar* rsonfile)>();

inline CMemory p_Script_LoadScript;
inline auto v_Script_LoadScript = p_Script_LoadScript.RCast<SQBool(*)(HSQUIRRELVM v, const SQChar* path, const SQChar* name, SQInteger flags)>();

#if !defined (CLIENT_DLL)
inline CMemory g_pServerScript;
#endif // !CLIENT_DLL
#if !defined (DEDICATED)
inline CMemory g_pClientScript;
inline CMemory g_pUIScript;
#endif // !DEDICATED

SQRESULT Script_RegisterConstant(CSquirrelVM* s, const SQChar* name, SQInteger value);
SQRESULT Script_RegisterFunction(CSquirrelVM* s, const SQChar* scriptname, const SQChar* nativename,
	const SQChar* helpstring, const SQChar* returntype, const SQChar* arguments, void* functor);
void Script_RegisterServerFunctions(CSquirrelVM* s);
void Script_RegisterClientFunctions(CSquirrelVM* s);
void Script_RegisterUIFunctions(CSquirrelVM* s);

SQRESULT Script_InitializeCLGlobalStructs(HSQUIRRELVM v, SQCONTEXT context);
void Script_InitializeSVGlobalStructs(HSQUIRRELVM v);

SQBool Script_CreateServerVM();
#ifndef DEDICATED
SQBool Script_CreateClientVM(CHLClient* hlclient);
#endif // !DEDICATED
SQBool Script_CreateUIVM();
CSquirrelVM* Script_GetContextObject(const SQCONTEXT context);

SQInteger Script_LoadRson(const SQChar* rsonfile);
SQBool Script_LoadScript(HSQUIRRELVM v, const SQChar* path, const SQChar* name, SQInteger flags);

void Script_Execute(const SQChar* code, const SQCONTEXT context);

void SQScript_Attach();
void SQScript_Detach();
///////////////////////////////////////////////////////////////////////////////
class VSquirrelVM : public IDetour
{
	virtual void GetAdr(void) const
	{
		spdlog::debug("| FUN: Script_RegisterConstant              : {:#18x} |\n", p_Script_RegisterConstant.GetPtr());
		spdlog::debug("| FUN: Script_RegisterFunction              : {:#18x} |\n", p_Script_RegisterFunction.GetPtr());
#ifndef CLIENT_DLL
		spdlog::debug("| FUN: Script_InitializeSVGlobalStructs     : {:#18x} |\n", p_Script_InitializeSVGlobalStructs.GetPtr());
#endif // !CLIENT_DLL
#ifndef DEDICATED
		spdlog::debug("| FUN: Script_InitializeCLGlobalStructs     : {:#18x} |\n", p_Script_InitializeCLGlobalStructs.GetPtr());
#endif // !DEDICATED
#ifndef CLIENT_DLL
		spdlog::debug("| FUN: Script_CreateServerVM                : {:#18x} |\n", p_Script_CreateServerVM.GetPtr());
#endif // !CLIENT_DLL
#ifndef DEDICATED
		spdlog::debug("| FUN: Script_CreateClientVM                : {:#18x} |\n", p_Script_CreateClientVM.GetPtr());
		spdlog::debug("| FUN: Script_CreateUIVM                    : {:#18x} |\n", p_Script_CreateUIVM.GetPtr());
#endif // !DEDICATED
		spdlog::debug("| FUN: Script_DestroySignalEntryListHead    : {:#18x} |\n", p_Script_DestroySignalEntryListHead.GetPtr());
		spdlog::debug("| FUN: Script_LoadRson                      : {:#18x} |\n", p_Script_LoadRson.GetPtr());
		spdlog::debug("| FUN: Script_LoadScript                    : {:#18x} |\n", p_Script_LoadScript.GetPtr());
#ifndef CLIENT_DLL
		spdlog::debug("| VAR: g_pServerScript                      : {:#18x} |\n", g_pServerScript.GetPtr());
#endif // !CLIENT_DLL
#ifndef DEDICATED
		spdlog::debug("| VAR: g_pClientScript                      : {:#18x} |\n", g_pClientScript.GetPtr());
		spdlog::debug("| VAR: g_pUIScript                          : {:#18x} |\n", g_pUIScript.GetPtr());
#endif // !DEDICATED
		spdlog::debug("+----------------------------------------------------------------+\n");
	}
	virtual void GetFun(void) const
	{
		p_Script_RegisterConstant = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x30\x4C\x8B"), "xxxx?xxxx?xxxx?xxxxxxx");
		p_Script_RegisterFunction = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x83\xEC\x38\x45\x0F\xB6\xC8"), "xxxxxxxx");
#if !defined (CLIENT_DLL)
		p_Script_InitializeSVGlobalStructs = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x30\x48\x8B\x3D\x00\x00\x00\x00\x48\x8B\xF1"), "xxxx?xxxxxxxx????xxx");
#endif // !CLIENT_DLL
#if !defined (DEDICATED)
		p_Script_InitializeCLGlobalStructs = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x83\xEC\x30\x48\x63\xC2\x48\x8D\x3D\x00\x00\x00\x00"), "xxxx?xxxx?xxxxxxxxxxxx????");
#endif // !DEDICATED
#if !defined (CLIENT_DLL) && defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		p_Script_CreateServerVM = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x40\x53\x48\x83\xEC\x50\x48\x8D\x0D\x00\x00\x00\x00"), "xxxxxxxxx????");
#elif !defined (CLIENT_DLL) && defined (GAMEDLL_S3) || defined (GAMEDLL_S2)
		p_Script_CreateServerVM = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x40\x53\x56\x48\x83\xEC\x48\x48\x8D\x0D\x00\x00\x00\x00"), "xxxxxxxxxx????");
#endif
#if !defined (DEDICATED) && defined (GAMEDLL_S0) || defined (GAMEDLL_S1) || defined (GAMEDLL_S2)
		p_Script_CreateClientVM = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x83\xEC\x58\x48\x83\x3D\x00\x00\x00\x00\x00\x74\x05"), "xxxxxxx?????xx");
#elif !defined (DEDICATED) && defined (GAMEDLL_S3)
		p_Script_CreateClientVM = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x40\x53\x41\x57\x48\x83\xEC\x68\x48\x83\x3D\x00\x00\x00\x00\x00"), "xxxxxxxxxxx?????");
#endif
#if !defined (DEDICATED)
		p_Script_CreateUIVM = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x40\x53\x48\x83\xEC\x20\x48\x8B\x1D\x00\x00\x00\x00\xC6\x05\x00\x00\x00\x00\x00"), "xxxxxxxxx????xx?????");
#endif // !DEDICATED
		p_Script_DestroySignalEntryListHead = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x56\x57\x41\x56\x48\x83\xEC\x50\x44\x8B\x42\x78"), "xxxx?xxxx?xxxxxxxxxxxx");
		p_Script_LoadRson = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x4C\x8B\xDC\x49\x89\x5B\x08\x57\x48\x81\xEC\xA0\x00\x00\x00\x33"), "xxxxxxxxxxxxxxxx");
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
		p_Script_LoadScript = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x89\x5C\x24\x10\x48\x89\x74\x24\x18\x48\x89\x7C\x24\x20\x48\x89\x4C\x24\x08\x55\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\x6C"), "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
#elif defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
		p_Script_LoadScript = g_GameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x8B\xC4\x48\x89\x48\x08\x55\x41\x56\x48\x8D\x68"), "xxxxxxxxxxxxx");
#endif
		v_Script_RegisterConstant = p_Script_RegisterConstant.RCast<SQRESULT(*)(CSquirrelVM*, const SQChar*, SQInteger)>();              /*48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 48 83 EC 30 4C 8B*/
		v_Script_RegisterFunction = p_Script_RegisterFunction.RCast<SQRESULT(*)(CSquirrelVM*, ScriptFunctionBinding_t*, SQInteger)>();   /*48 83 EC 38 45 0F B6 C8*/
#if !defined (CLIENT_DLL)
		v_Script_InitializeSVGlobalStructs = p_Script_InitializeSVGlobalStructs.RCast<SQRESULT(*)(HSQUIRRELVM)>();                       /*48 89 74 24 ?? 57 48 83 EC 30 48 8B 3D ?? ?? ?? ?? 48 8B F1*/
#endif // !CLIENT_DLL
#if !defined (DEDICATED)
		v_Script_InitializeCLGlobalStructs = p_Script_InitializeCLGlobalStructs.RCast<SQRESULT(*)(HSQUIRRELVM, SQCONTEXT)>();            /*48 89 74 24 ?? 48 89 7C 24 ?? 41 56 48 83 EC 30 48 63 C2 48 8D 3D ?? ?? ?? ??*/
#endif // !DEDICATED
#if !defined (CLIENT_DLL)
		v_Script_CreateServerVM = p_Script_CreateServerVM.RCast<SQBool(*)(void)>();                                                      /*40 53 56 48 83 EC 48 48 8D 0D ?? ?? ?? ??*/
#endif // !CLIENT_DLL
#if !defined (DEDICATED)
		v_Script_CreateClientVM = p_Script_CreateClientVM.RCast<SQBool(*)(CHLClient*)>();                                                /*40 53 41 57 48 83 EC 68 48 83 3D ?? ?? ?? ?? ??*/
		v_Script_CreateUIVM = p_Script_CreateUIVM.RCast<SQBool(*)(void)>();                                                              /*40 53 48 83 EC 20 48 8B 1D ?? ?? ?? ?? C6 05 ?? ?? ?? ?? ??*/
#endif // !DEDICATED
		v_Script_DestroySignalEntryListHead = p_Script_DestroySignalEntryListHead.RCast<SQBool(*)(CSquirrelVM*, HSQUIRRELVM, SQFloat)>();/*48 89 5C 24 ?? 48 89 6C 24 ?? 56 57 41 56 48 83 EC 50 44 8B 42*/
		v_Script_LoadRson = p_Script_LoadRson.RCast<SQInteger(*)(const SQChar*)>();                                                      /*4C 8B DC 49 89 5B 08 57 48 81 EC A0 00 00 00 33*/
		v_Script_LoadScript = p_Script_LoadScript.RCast<SQBool(*)(HSQUIRRELVM, const SQChar*, const SQChar*, SQInteger)>();              /*48 8B C4 48 89 48 08 55 41 56 48 8D 68*/
	}
	virtual void GetVar(void) const
	{
#if !defined (CLIENT_DLL)
		g_pServerScript = p_Script_CreateServerVM.FindPatternSelf("48 89 1D", CMemory::Direction::DOWN, 150).ResolveRelativeAddressSelf(0x3, 0x7);
#endif // !CLIENT_DLL
#if !defined (DEDICATED)
		g_pClientScript = p_Script_CreateClientVM.FindPatternSelf("48 83 3D", CMemory::Direction::DOWN, 150).ResolveRelativeAddressSelf(0x3, 0x8);
		g_pUIScript = p_Script_CreateUIVM.FindPatternSelf("48 8B 1D", CMemory::Direction::DOWN, 150).ResolveRelativeAddressSelf(0x3, 0x7);
#endif // !DEDICATED
	}
	virtual void GetCon(void) const { }
	virtual void Attach(void) const { }
	virtual void Detach(void) const { }
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(VSquirrelVM);