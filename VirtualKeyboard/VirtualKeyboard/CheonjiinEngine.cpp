#include "CheonjiinEngine.h"

namespace
{
/// <summary>
/// 호환 자모를 초성 인덱스 0부터 18로 바꾼다.
/// </summary>
int ChoseongOf(wchar_t jamo)
{
	switch (jamo)
	{
	case 0x3131: return 0;
	case 0x3132: return 1;
	case 0x3134: return 2;
	case 0x3137: return 3;
	case 0x3138: return 4;
	case 0x3139: return 5;
	case 0x3141: return 6;
	case 0x3142: return 7;
	case 0x3144: return 8;
	case 0x3145: return 9;
	case 0x3146: return 10;
	case 0x3147: return 11;
	case 0x3148: return 12;
	case 0x3149: return 13;
	case 0x314A: return 14;
	case 0x314B: return 15;
	case 0x314C: return 16;
	case 0x314D: return 17;
	case 0x314E: return 18;
	default: return -1;
	}
}

/// <summary>
/// 호환 자모를 종성 인덱스로 바꾼다. 종성이 될 수 없으면 -1이다.
/// </summary>
int JongOf(wchar_t jamo)
{
	switch (jamo)
	{
	case 0x3131: return 1;
	case 0x3132: return 2;
	case 0x3134: return 4;
	case 0x3137: return 7;
	case 0x3139: return 8;
	case 0x3141: return 16;
	case 0x3142: return 17;
	case 0x3145: return 19;
	case 0x3146: return 20;
	case 0x3147: return 21;
	case 0x3148: return 22;
	case 0x314A: return 23;
	case 0x314B: return 24;
	case 0x314C: return 25;
	case 0x314D: return 26;
	case 0x314E: return 27;
	default: return -1;
	}
}

/// <summary>
/// 종성 두 개를 겹받침 인덱스로 합친다. 겹받침이 아니면 -1이다.
/// </summary>
int CombineJong(wchar_t first, wchar_t second)
{
	if (first == 0x3131 && second == 0x3145) return 3;
	if (first == 0x3134 && second == 0x3148) return 5;
	if (first == 0x3134 && second == 0x314E) return 6;
	if (first == 0x3139 && second == 0x3131) return 9;
	if (first == 0x3139 && second == 0x3141) return 10;
	if (first == 0x3139 && second == 0x3142) return 11;
	if (first == 0x3139 && second == 0x3145) return 12;
	if (first == 0x3139 && second == 0x314C) return 13;
	if (first == 0x3139 && second == 0x314D) return 14;
	if (first == 0x3139 && second == 0x314E) return 15;
	if (first == 0x3142 && second == 0x3145) return 18;
	return -1;
}

/// <summary>
/// 중성 상태에서 획 하나를 받아 다음 중성을 돌려준다. 0은 ㅣ, 1은 ㆍ, 2는 ㅡ이다.
/// </summary>
int NextVowel(int state, int stroke)
{
	if (state == -1)
	{
		if (stroke == 0) return 20;
		if (stroke == 1) return -2;
		return 18;
	}
	if (state == 20 && stroke == 1) return 0;
	if (state == 0 && stroke == 1) return 2;
	if (state == 0 && stroke == 0) return 1;
	if (state == 2 && stroke == 0) return 3;
	if (state == -2 && stroke == 0) return 4;
	if (state == -2 && stroke == 1) return -3;
	if (state == -2 && stroke == 2) return 8;
	if (state == -3 && stroke == 0) return 6;
	if (state == -3 && stroke == 2) return 12;
	if (state == 4 && stroke == 0) return 5;
	if (state == 6 && stroke == 0) return 7;
	if (state == 8 && stroke == 0) return 11;
	if (state == 11 && stroke == 1) return 9;
	if (state == 9 && stroke == 0) return 10;
	if (state == 18 && stroke == 0) return 19;
	if (state == 18 && stroke == 1) return 13;
	if (state == 13 && stroke == 1) return 17;
	if (state == 13 && stroke == 0) return 16;
	if (state == 16 && stroke == 1) return 14;
	if (state == 14 && stroke == 0) return 15;
	return -99;
}

/// <summary>
/// 초성 인덱스를 호환 자모로 바꾼다.
/// </summary>
wchar_t ChoJamo(int cho)
{
	static const wchar_t table[] = L"\u3131\u3132\u3134\u3137\u3138\u3139\u3141\u3142\u3144\u3145\u3146\u3147\u3148\u3149\u314A\u314B\u314C\u314D\u314E";
	return table[cho];
}

/// <summary>
/// 중성 인덱스를 호환 자모로 바꾼다.
/// </summary>
wchar_t JungJamo(int jung)
{
	static const wchar_t table[] = L"\u314F\u3150\u3151\u3152\u3153\u3154\u3155\u3156\u3157\u3158\u3159\u315A\u315B\u315C\u315D\u315E\u315F\u3160\u3161\u3162\u3163";
	return table[jung];
}

/// <summary>
/// 자음 키의 순환 문자열을 돌려준다.
/// </summary>
const wchar_t* RingOf(CheonjiinKeyId key, int& length)
{
	switch (key)
	{
	case CheonjiinKeyId::Giyeok: length = 3; return L"\u3131\u314B\u3132";
	case CheonjiinKeyId::Nieun: length = 2; return L"\u3134\u3139";
	case CheonjiinKeyId::Digeut: length = 3; return L"\u3137\u314C\u3138";
	case CheonjiinKeyId::Bieup: length = 3; return L"\u3142\u314D\u3144";
	case CheonjiinKeyId::Siot: length = 3; return L"\u3145\u314E\u3146";
	case CheonjiinKeyId::Jieut: length = 3; return L"\u3148\u314A\u3149";
	case CheonjiinKeyId::Ieung: length = 2; return L"\u3147\u3141";
	default: length = 0; return nullptr;
	}
}

/// <summary>
/// 조합 중인 글자를 확정 문자열 뒤에 붙인다.
/// </summary>
void AppendSyllable(std::wstring& out, int cho, int jung, int jong)
{
	bool vowel = jung >= 0 && jung <= 20;
	if (cho >= 0 && vowel)
	{
		out.push_back((wchar_t)(0xAC00 + (cho * 21 * 28) + (jung * 28) + jong));
		return;
	}
	if (cho >= 0) out.push_back(ChoJamo(cho));
	if (jung == -2) out.push_back(0x318D);
	else if (jung == -3)
	{
		out.push_back(0x318D);
		out.push_back(0x318D);
	}
	else if (vowel) out.push_back(JungJamo(jung));
}
}

/// <summary>
/// 자음 연타 제한 시간을 흘린다. 0.6초가 지나면 다음 자음은 첫 글자부터 시작한다.
/// </summary>
void CheonjiinEngine::Tick(float dt)
{
	if (m_ringIndex < 0) return;
	m_ringTime += dt;
	if (m_ringTime > 0.6f) m_ringIndex = -1;
}

/// <summary>
/// 지금 조합 상태를 복사한다.
/// </summary>
CheonjiinEngine::State CheonjiinEngine::Capture() const
{
	State state;
	state.committed = m_committed;
	state.cho = m_cho;
	state.jung = m_jung;
	state.jong = m_jong;
	state.jongA = m_jongA;
	state.jongB = m_jongB;
	state.lastSlot = m_lastSlot;
	state.ringKey = m_ringKey;
	state.ringIndex = m_ringIndex;
	return state;
}

/// <summary>
/// 저장해 둔 조합 상태로 되돌린다.
/// </summary>
void CheonjiinEngine::Restore(const State& state)
{
	m_committed = state.committed;
	m_cho = state.cho;
	m_jung = state.jung;
	m_jong = state.jong;
	m_jongA = state.jongA;
	m_jongB = state.jongB;
	m_lastSlot = state.lastSlot;
	m_ringKey = state.ringKey;
	m_ringIndex = state.ringIndex;
	m_ringTime = 0.f;
}

/// <summary>
/// 조합 중인 글자를 확정 문자열 뒤에 붙이고 조합 칸을 비운다.
/// </summary>
void CheonjiinEngine::Commit()
{
	AppendSyllable(m_committed, m_cho, m_jung, m_jong);
	m_cho = -1;
	m_jung = -1;
	m_jong = 0;
	m_jongA = 0;
	m_jongB = 0;
	m_lastSlot = 0;
}

/// <summary>
/// 모음 획 하나를 현재 중성에 잇는다. 받침이 있으면 그 받침을 다음 글자의 초성으로 넘긴다.
/// </summary>
void CheonjiinEngine::AddVowel(int stroke)
{
	if (m_jong == 0)
	{
		int next = NextVowel(m_jung, stroke);
		if (next != -99)
		{
			m_jung = next;
			m_lastSlot = 0;
			return;
		}
	}

	wchar_t carry = 0;
	if (m_jongB)
	{
		carry = m_jongB;
		m_jong = JongOf(m_jongA);
		m_jongB = 0;
	}
	else if (m_jongA)
	{
		carry = m_jongA;
		m_jong = 0;
		m_jongA = 0;
	}
	Commit();
	if (carry)
	{
		m_cho = ChoseongOf(carry);
		m_lastSlot = 1;
	}
	m_jung = NextVowel(-1, stroke);
}

/// <summary>
/// 자음을 초성이나 종성에 넣는다. 겹받침이 되면 한 글자로 합친다.
/// </summary>
void CheonjiinEngine::AddConsonant(wchar_t jamo)
{
	if (m_cho >= 0 && m_jung >= 0 && m_jung <= 20 && m_jong == 0)
	{
		int jong = JongOf(jamo);
		if (jong > 0)
		{
			m_jong = jong;
			m_jongA = jamo;
			m_lastSlot = 2;
			return;
		}
	}
	else if (m_jong != 0)
	{
		int combined = m_jongB == 0 ? CombineJong(m_jongA, jamo) : -1;
		if (combined > 0)
		{
			m_jong = combined;
			m_jongB = jamo;
			m_lastSlot = 3;
			return;
		}
	}
	else if (m_jung < 0 && m_cho < 0)
	{
		m_cho = ChoseongOf(jamo);
		m_lastSlot = 1;
		return;
	}

	Commit();
	m_cho = ChoseongOf(jamo);
	m_lastSlot = 1;
}

/// <summary>
/// 방금 넣은 자음만 링의 다음 글자로 바꾼다.
/// </summary>
void CheonjiinEngine::ReplaceConsonant(wchar_t jamo)
{
	if (m_lastSlot == 1)
	{
		m_cho = ChoseongOf(jamo);
		return;
	}
	if (m_lastSlot == 2)
	{
		int jong = JongOf(jamo);
		if (jong < 0)
		{
			m_jong = 0;
			m_jongA = 0;
			m_jongB = 0;
			Commit();
			m_cho = ChoseongOf(jamo);
			m_lastSlot = 1;
			return;
		}
		m_jong = jong;
		m_jongA = jamo;
		return;
	}
	if (m_lastSlot == 3)
	{
		int combined = CombineJong(m_jongA, jamo);
		if (combined > 0)
		{
			m_jong = combined;
			m_jongB = jamo;
			return;
		}
		m_jong = JongOf(m_jongA);
		m_jongB = 0;
		Commit();
		m_cho = ChoseongOf(jamo);
		m_lastSlot = 1;
	}
}

/// <summary>
/// 같은 자음 키의 반복이면 순환하고, 아니면 새 자음으로 넣는다.
/// </summary>
void CheonjiinEngine::OnConsonant(CheonjiinKeyId key)
{
	int length = 0;
	const wchar_t* ring = RingOf(key, length);
	if (!ring) return;
	if (m_ringKey == (int)key && m_ringIndex >= 0)
	{
		m_ringIndex = (m_ringIndex + 1) % length;
		ReplaceConsonant(ring[m_ringIndex]);
	}
	else
	{
		m_ringKey = (int)key;
		m_ringIndex = 0;
		AddConsonant(ring[0]);
	}
	m_ringTime = 0.f;
}

/// <summary>
/// 자음 링 인덱스를 지운다.
/// </summary>
void CheonjiinEngine::ResetRing()
{
	m_ringKey = 0;
	m_ringIndex = -1;
	m_ringTime = 0.f;
}

/// <summary>
/// 확정 문자열 뒤에 조합 중인 음절을 붙여 화면 문자열을 만든다.
/// </summary>
void CheonjiinEngine::Rebuild()
{
	m_text = m_committed;
	AppendSyllable(m_text, m_cho, m_jung, m_jong);
}

/// <summary>
/// 키 하나를 조합한다. 검색, 한자, !#1은 무시한다.
/// </summary>
void CheonjiinEngine::Press(CheonjiinKeyId key)
{
	if (key == CheonjiinKeyId::None || key == CheonjiinKeyId::Search
		|| key == CheonjiinKeyId::Hanja || key == CheonjiinKeyId::Shift)
		return;

	if (key == CheonjiinKeyId::Backspace)
	{
		if (m_history.empty()) return;
		Restore(m_history.back());
		m_history.pop_back();
		Rebuild();
		return;
	}

	m_history.push_back(Capture());
	if (key == CheonjiinKeyId::Space)
	{
		Commit();
		m_committed.push_back(L' ');
		ResetRing();
	}
	else if (key == CheonjiinKeyId::Punct)
	{
		Commit();
		m_committed.push_back(L',');
		ResetRing();
	}
	else if (key == CheonjiinKeyId::I || key == CheonjiinKeyId::Dot || key == CheonjiinKeyId::Eu)
	{
		int stroke = 2;
		if (key == CheonjiinKeyId::I) stroke = 0;
		else if (key == CheonjiinKeyId::Dot) stroke = 1;
		AddVowel(stroke);
		ResetRing();
	}
	else
		OnConsonant(key);
	Rebuild();
}

/// <summary>
/// 가, 가나, 자음 순환, 받침 분리, 시간 초과를 확인한다.
/// </summary>
bool CheonjiinEngine::SelfCheck()
{
	CheonjiinEngine syllable;
	syllable.Press(CheonjiinKeyId::Giyeok);
	syllable.Press(CheonjiinKeyId::I);
	syllable.Press(CheonjiinKeyId::Dot);
	if (std::wstring(syllable.Text()) != L"\uAC00") return false;
	syllable.Press(CheonjiinKeyId::Nieun);
	syllable.Press(CheonjiinKeyId::I);
	syllable.Press(CheonjiinKeyId::Dot);
	if (std::wstring(syllable.Text()) != L"\uAC00\uB098") return false;
	syllable.Press(CheonjiinKeyId::Backspace);
	if (std::wstring(syllable.Text()) != L"\uAC00\uB2C8") return false;

	CheonjiinEngine ring;
	ring.Press(CheonjiinKeyId::Giyeok);
	ring.Press(CheonjiinKeyId::Giyeok);
	ring.Press(CheonjiinKeyId::Giyeok);
	if (std::wstring(ring.Text()) != L"\u3132") return false;
	ring.Tick(0.7f);
	ring.Press(CheonjiinKeyId::Giyeok);
	if (std::wstring(ring.Text()) != L"\u3132\u3131") return false;

	CheonjiinEngine split;
	split.Press(CheonjiinKeyId::Giyeok);
	split.Press(CheonjiinKeyId::I);
	split.Press(CheonjiinKeyId::Dot);
	split.Press(CheonjiinKeyId::Nieun);
	split.Press(CheonjiinKeyId::Nieun);
	split.Press(CheonjiinKeyId::Giyeok);
	if (std::wstring(split.Text()) != L"\uAC09") return false;
	split.Press(CheonjiinKeyId::I);
	split.Press(CheonjiinKeyId::Dot);
	if (std::wstring(split.Text()) != L"\uAC08\uAC00") return false;
	return true;
}
