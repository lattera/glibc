#define NTRANSLIT 20
static const uint32_t translit_from_idx[] =
{
     0,    2,    4,    6,    8,   10,   12,   14,   16,   18,   20,   22,
    24,   26,   28,   30,   32,   34,   36,   38
};
static const wchar_t translit_from_tbl[] =
  L"\xa9" L"\0" L"\xab" L"\0" L"\xae" L"\0" L"\xbb" L"\0" L"\xbc" L"\0"
  L"\xbd" L"\0" L"\xbe" L"\0" L"\xc4" L"\0" L"\xc5" L"\0" L"\xc6" L"\0"
  L"\xd6" L"\0" L"\xdc" L"\0" L"\xdf" L"\0" L"\xe4" L"\0" L"\xe5" L"\0"
  L"\xe6" L"\0" L"\xf6" L"\0" L"\xfc" L"\0" L"\x201c" L"\0" L"\x201d";
static const uint32_t translit_to_idx[] =
{
     0,    5,    9,   14,   18,   23,   28,   33,   37,   41,   45,   49,
    53,   57,   61,   65,   69,   73,   77,   80
};
static const wchar_t translit_to_tbl[] =
  L"(C)\0" L"\0" L"<<\0" L"\0" L"(R)\0" L"\0" L">>\0" L"\0" L"1/4\0" L"\0"
  L"1/2\0" L"\0" L"3/4\0" L"\0" L"AE\0" L"\0" L"AA\0" L"\0" L"AE\0" L"\0"
  L"OE\0" L"\0" L"UE\0" L"\0" L"ss\0" L"\0" L"ae\0" L"\0" L"aa\0" L"\0"
  L"ae\0" L"\0" L"oe\0" L"\0" L"ue\0" L"\0" L"\"\0" L"\0" L"\"\0";
