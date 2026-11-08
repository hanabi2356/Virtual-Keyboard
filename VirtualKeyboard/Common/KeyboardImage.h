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

/// <summary>
/// 천지인 자판을 사진과 같은 칸 배치로 그린 32비트 비트맵을 만든다.
/// 오른쪽 열은 지우기, 검색, 세로로 긴 문장부호, 한자다.
/// </summary>
bool CreateCheonjiinBitmap(int width, int height, KeyboardBitmap& out);

/// <summary>
/// CreateCheonjiinBitmap이 만든 비트맵을 지운다.
/// </summary>
void DestroyCheonjiinBitmap(KeyboardBitmap& image);
