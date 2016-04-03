//
// NO PRAGMA ONCE!
//

#if defined(_MSC_VER)


#ifdef GENERIC_READ
#pragma push_macro("GENERIC_READ")
#undef GENERIC_READ
#define WINDOWS_H_GENERIC_READ_DEFINED
#endif

#ifdef DOMAIN
#pragma push_macro("DOMAIN")
#undef DOMAIN
#define MATH_H_DOMAIN_DEFINED
#endif

#ifdef ERROR
#pragma push_macro("ERROR")
#undef ERROR
#define WINDOWS_H_ERROR_DEFINED
#endif

#endif  //_MSC_VER


