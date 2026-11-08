#pragma once
#include <windows.h>

struct KeyboardBitmap
{
	HBITMAP bitmap = nullptr;
	void* bits = nullptr;
	int width = 0;
	int height = 0;
	int pitch = 0;
};

enum class CheonjiinKeyId
{
	None = 0,
	I,
	Dot,
	Eu,
	Giyeok,
	Nieun,
	Digeut,
	Bieup,
	Siot,
	Jieut,
	Ieung,
	Backspace,
	Space,
	Punct,
	Search,
	Hanja,
	Shift
};

/// <summary>
/// 천지인 자판을 그리고, 위 빈칸에 label을 넣는다.
/// label이 비어 있으면 그 칸은 배경만 둔다.
/// </summary>
bool CreateBitmap(int width, int height, const wchar_t* label, KeyboardBitmap& out);

/// <summary>
/// CreateBitmap이 만든 비트맵을 지운다.
/// </summary>
void DestroyBitmap(KeyboardBitmap& image);

/// <summary>
/// 클라이언트 좌표가 들어 있는 키 번호를 돌려준다. 키가 아니면 None이다.
/// </summary>
CheonjiinKeyId HitKey(int width, int height, int x, int y);
