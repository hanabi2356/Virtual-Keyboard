#include "KeyboardImage.h"

namespace
{
/// <summary>
/// 둥근 키 하나를 칠하고 가운데에 글자를 넣는다.
/// </summary>
void DrawKey(HDC dc, const RECT& rc, COLORREF fill, const wchar_t* text)
{
    HBRUSH brush = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_NULL, 0, 0);
    HGDIOBJ oldBrush = SelectObject(dc, brush);
    HGDIOBJ oldPen = SelectObject(dc, pen);
    int radius = (rc.bottom - rc.top) / 4;
    if (radius < 12) radius = 12;
    RoundRect(dc, rc.left, rc.top, rc.right, rc.bottom, radius, radius);
    SelectObject(dc, oldPen);
    SelectObject(dc, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);

    if (!text || !text[0]) return;
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(32, 32, 32));
    RECT textRc = rc;
    UINT format = DT_CENTER | DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE;
    if (wcschr(text, L'\n'))
        format = DT_CENTER | DT_NOPREFIX;
    DrawTextW(dc, text, -1, &textRc, format);
}

/// <summary>
/// 검색 키 한가운데에 파란 돋보기를 그린다.
/// </summary>
void DrawSearchIcon(HDC dc, const RECT& rc)
{
    int cx = (rc.left + rc.right) / 2 - 2;
    int cy = (rc.top + rc.bottom) / 2 - 4;
    HPEN pen = CreatePen(PS_SOLID, 3, RGB(48, 120, 236));
    HGDIOBJ old = SelectObject(dc, pen);
    SelectObject(dc, GetStockObject(NULL_BRUSH));
    Ellipse(dc, cx - 11, cy - 11, cx + 11, cy + 11);
    MoveToEx(dc, cx + 8, cy + 8, nullptr);
    LineTo(dc, cx + 16, cy + 16);
    SelectObject(dc, old);
    DeleteObject(pen);
}

/// <summary>
/// 같은 간격의 키 칸을 계산한다.
/// </summary>
RECT Cell(int x, int y, int colW, int rowH, int gap, int col, int row)
{
    RECT rc;
    rc.left = x + col * (colW + gap);
    rc.top = y + row * (rowH + gap);
    rc.right = rc.left + colW;
    rc.bottom = rc.top + rowH;
    return rc;
}
}

/// <summary>
/// 천지인 자판을 사진과 같은 칸 배치로 그린 32비트 비트맵을 만든다.
/// 오른쪽 열은 지우기, 검색, 세로로 긴 문장부호, 한자다.
/// </summary>
bool CreateCheonjiinBitmap(int width, int height, KeyboardBitmap& out)
{
    if (width < 64 || height < 64) return false;

    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(nullptr, &info, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmap || !bits) return false;

    HDC dc = CreateCompatibleDC(nullptr);
    HGDIOBJ oldBitmap = SelectObject(dc, bitmap);

    const COLORREF background = RGB(236, 237, 240);
    const COLORREF whiteKey = RGB(255, 255, 255);
    const COLORREF grayKey = RGB(222, 224, 228);
    HBRUSH bg = CreateSolidBrush(background);
    RECT all = { 0, 0, width, height };
    FillRect(dc, &all, bg);
    DeleteObject(bg);

    int margin = width / 36;
    int gap = width / 70;
    if (gap < 6) gap = 6;
    int top = height / 7;
    int gridW = width - margin * 2;
    int gridH = height - top - margin;
    int colW = (gridW - gap * 3) / 4;
    int rowH = (gridH - gap * 3) / 4;
    int x = margin;
    int y = top;

    HFONT letterFont = CreateFontW(-rowH / 3, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        HANGUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Malgun Gothic");
    HFONT labelFont = CreateFontW(-rowH / 5, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        HANGUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Malgun Gothic");
    SelectObject(dc, letterFont);

    const wchar_t* letters[3][3] = {
        { L"\u3163", L"\u318D", L"\u3161" },
        { L"\u3131\n\u314B", L"\u3134\n\u3139", L"\u3137\n\u314C" },
        { L"\u3142\n\u314D", L"\u3145\n\u314E", L"\u3148\n\u314A" },
    };
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
            DrawKey(dc, Cell(x, y, colW, rowH, gap, col, row), whiteKey, letters[row][col]);
    }

    RECT backspace = Cell(x, y, colW, rowH, gap, 3, 0);
    RECT search = Cell(x, y, colW, rowH, gap, 3, 1);
    RECT hanja = Cell(x, y, colW, rowH, gap, 3, 3);
    hanja.top = hanja.bottom - (rowH * 3) / 4;
    RECT punct = Cell(x, y, colW, rowH, gap, 3, 2);
    punct.bottom = hanja.top - gap;

    DrawKey(dc, backspace, grayKey, L"\u2190");
    DrawKey(dc, search, whiteKey, L"");
    DrawSearchIcon(dc, search);
    SelectObject(dc, labelFont);
    DrawKey(dc, punct, grayKey, L",,?!");
    DrawKey(dc, hanja, grayKey, L"\uD55C\uC790");

    RECT bottom = Cell(x, y, colW, rowH, gap, 0, 3);
    int bottomH = bottom.bottom - bottom.top;
    int narrow = colW * 9 / 10;
    int spaceW = gridW - narrow * 2 - colW - gap * 3;
    RECT shift = { x, bottom.top, x + narrow, bottom.top + bottomH };
    RECT ieung = { shift.right + gap, bottom.top, shift.right + gap + narrow, bottom.top + bottomH };
    RECT space = { ieung.right + gap, bottom.top, ieung.right + gap + spaceW, bottom.top + bottomH };
    DrawKey(dc, shift, grayKey, L"!#1");
    DrawKey(dc, ieung, whiteKey, L"\u3147\n\u3141");
    DrawKey(dc, space, grayKey, L"KR");

    SelectObject(dc, oldBitmap);
    DeleteObject(letterFont);
    DeleteObject(labelFont);
    DeleteDC(dc);

    out.bitmap = bitmap;
    out.bits = bits;
    out.width = width;
    out.height = height;
    out.pitch = width * 4;
    return true;
}

/// <summary>
/// CreateCheonjiinBitmap이 만든 비트맵을 지운다.
/// </summary>
void DestroyCheonjiinBitmap(KeyboardBitmap& image)
{
    if (image.bitmap) DeleteObject(image.bitmap);
    image.bitmap = nullptr;
    image.bits = nullptr;
    image.width = 0;
    image.height = 0;
    image.pitch = 0;
}
