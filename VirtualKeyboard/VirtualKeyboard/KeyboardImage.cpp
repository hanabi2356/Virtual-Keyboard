#include "KeyboardImage.h"

namespace
{
struct CheonjiinKey
{
    RECT bounds;
    const wchar_t* draw;
    CheonjiinKeyId id;
    bool gray;
};

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

/// <summary>
/// 그리기와 클릭이 같이 쓰는 16개 키 사각형을 채운다.
/// </summary>
int LayoutCheonjiinKeys(int width, int height, CheonjiinKey* keys)
{
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

    const wchar_t* draw[3][3] = {
        { L"\u3163", L"\u318D", L"\u3161" },
        { L"\u3131\n\u314B", L"\u3134\n\u3139", L"\u3137\n\u314C" },
        { L"\u3142\n\u314D", L"\u3145\n\u314E", L"\u3148\n\u314A" },
    };
    const CheonjiinKeyId ids[3][3] = {
        { CheonjiinKeyId::I, CheonjiinKeyId::Dot, CheonjiinKeyId::Eu },
        { CheonjiinKeyId::Giyeok, CheonjiinKeyId::Nieun, CheonjiinKeyId::Digeut },
        { CheonjiinKeyId::Bieup, CheonjiinKeyId::Siot, CheonjiinKeyId::Jieut },
    };

    int count = 0;
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            keys[count].bounds = Cell(x, y, colW, rowH, gap, col, row);
            keys[count].draw = draw[row][col];
            keys[count].id = ids[row][col];
            keys[count].gray = false;
            ++count;
        }
    }

    RECT hanja = Cell(x, y, colW, rowH, gap, 3, 3);
    hanja.top = hanja.bottom - (rowH * 3) / 4;
    RECT punct = Cell(x, y, colW, rowH, gap, 3, 2);
    punct.bottom = hanja.top - gap;

    keys[count++] = { Cell(x, y, colW, rowH, gap, 3, 0), L"\u2190", CheonjiinKeyId::Backspace, true };
    keys[count++] = { Cell(x, y, colW, rowH, gap, 3, 1), L"", CheonjiinKeyId::Search, false };
    keys[count++] = { punct, L",,?!", CheonjiinKeyId::Punct, true };
    keys[count++] = { hanja, L"\uD55C\uC790", CheonjiinKeyId::Hanja, true };

    RECT bottom = Cell(x, y, colW, rowH, gap, 0, 3);
    int bottomH = bottom.bottom - bottom.top;
    int narrow = colW * 9 / 10;
    int spaceW = gridW - narrow * 2 - colW - gap * 3;
    RECT shift = { x, bottom.top, x + narrow, bottom.top + bottomH };
    RECT ieung = { shift.right + gap, bottom.top, shift.right + gap + narrow, bottom.top + bottomH };
    RECT space = { ieung.right + gap, bottom.top, ieung.right + gap + spaceW, bottom.top + bottomH };
    keys[count++] = { shift, L"!#1", CheonjiinKeyId::Shift, true };
    keys[count++] = { ieung, L"\u3147\n\u3141", CheonjiinKeyId::Ieung, false };
    keys[count++] = { space, L"KR", CheonjiinKeyId::Space, true };
    return count;
}
}

/// <summary>
/// 천지인 자판을 그리고, 위 빈칸에 label을 넣는다.
/// label이 비어 있으면 그 칸은 배경만 둔다.
/// </summary>
bool CreateBitmap(int width, int height, const wchar_t* label, KeyboardBitmap& out)
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

    CheonjiinKey keys[16];
    int count = LayoutCheonjiinKeys(width, height, keys);
    int rowH = keys[0].bounds.bottom - keys[0].bounds.top;
    HFONT letterFont = CreateFontW(-rowH / 3, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        HANGUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Malgun Gothic");
    HFONT labelFont = CreateFontW(-rowH / 5, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        HANGUL_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Malgun Gothic");
    SelectObject(dc, letterFont);

    for (int i = 0; i < count; ++i)
    {
        DrawKey(dc, keys[i].bounds, keys[i].gray ? grayKey : whiteKey, keys[i].draw);
        if (!keys[i].draw[0]) DrawSearchIcon(dc, keys[i].bounds);
    }

    if (label && label[0])
    {
        SelectObject(dc, letterFont);
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, RGB(32, 32, 32));
        RECT band = { width / 36, 0, width - width / 36, height / 7 };
        DrawTextW(dc, label, -1, &band, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

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
/// 클라이언트 좌표가 들어 있는 키 번호를 돌려준다. 키가 아니면 None이다.
/// </summary>
CheonjiinKeyId HitKey(int width, int height, int x, int y)
{
    CheonjiinKey keys[16];
    int count = LayoutCheonjiinKeys(width, height, keys);
    for (int i = 0; i < count; ++i)
    {
        if (x >= keys[i].bounds.left && x < keys[i].bounds.right
            && y >= keys[i].bounds.top && y < keys[i].bounds.bottom)
            return keys[i].id;
    }
    return CheonjiinKeyId::None;
}

/// <summary>
/// CreateBitmap이 만든 비트맵을 지운다.
/// </summary>
void DestroyBitmap(KeyboardBitmap& image)
{
    if (image.bitmap) DeleteObject(image.bitmap);
    image.bitmap = nullptr;
    image.bits = nullptr;
    image.width = 0;
    image.height = 0;
    image.pitch = 0;
}
