#pragma once
#include "KeyboardImage.h"
#include <string>
#include <vector>

class CheonjiinEngine
{
public:
	/// <summary>
	/// 자음 연타 제한 시간을 흘린다. 0.6초가 지나면 다음 자음은 첫 글자부터 시작한다.
	/// </summary>
	void Tick(float dt);

	/// <summary>
	/// 키 하나를 조합한다. 검색, 한자, !#1은 무시한다.
	/// </summary>
	void Press(CheonjiinKeyId key);

	/// <summary>
	/// 확정된 글자와 조합 중인 글자를 붙인 문자열이다.
	/// </summary>
	const wchar_t* Text() const { return m_text.c_str(); }

	/// <summary>
	/// 가, 가나, 자음 순환, 받침 분리, 시간 초과를 확인한다.
	/// </summary>
	static bool SelfCheck();

private:
	struct State
	{
		std::wstring committed;
		int cho = -1;
		int jung = -1;
		int jong = 0;
		wchar_t jongA = 0;
		wchar_t jongB = 0;
		int lastSlot = 0;
		int ringKey = 0;
		int ringIndex = -1;
	};

	/// <summary>
	/// 지금 조합 상태를 복사한다.
	/// </summary>
	State Capture() const;

	/// <summary>
	/// 저장해 둔 조합 상태로 되돌린다.
	/// </summary>
	void Restore(const State& state);

	/// <summary>
	/// 조합 중인 글자를 확정 문자열 뒤에 붙이고 조합 칸을 비운다.
	/// </summary>
	void Commit();

	/// <summary>
	/// 모음 획 하나를 현재 중성에 잇는다. 받침이 있으면 그 받침을 다음 글자의 초성으로 넘긴다.
	/// </summary>
	void AddVowel(int stroke);

	/// <summary>
	/// 자음을 초성이나 종성에 넣는다. 겹받침이 되면 한 글자로 합친다.
	/// </summary>
	void AddConsonant(wchar_t jamo);

	/// <summary>
	/// 방금 넣은 자음만 링의 다음 글자로 바꾼다.
	/// </summary>
	void ReplaceConsonant(wchar_t jamo);

	/// <summary>
	/// 같은 자음 키의 반복이면 순환하고, 아니면 새 자음으로 넣는다.
	/// </summary>
	void OnConsonant(CheonjiinKeyId key);

	/// <summary>
	/// 자음 링 인덱스를 지운다.
	/// </summary>
	void ResetRing();

	/// <summary>
	/// 확정 문자열 뒤에 조합 중인 음절을 붙여 화면 문자열을 만든다.
	/// </summary>
	void Rebuild();

	std::wstring m_committed;
	std::wstring m_text;
	std::vector<State> m_history;
	int m_cho = -1;
	int m_jung = -1;
	int m_jong = 0;
	wchar_t m_jongA = 0;
	wchar_t m_jongB = 0;
	int m_lastSlot = 0;
	int m_ringKey = 0;
	int m_ringIndex = -1;
	float m_ringTime = 0.f;
};
