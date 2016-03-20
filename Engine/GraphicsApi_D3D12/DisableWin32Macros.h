//
// NO PRAGMA ONCE!
//

#ifdef _MSC_VER


#ifdef GENERIC_READ
#pragma push_macro("GENERIC_READ")
#undef GENERIC_READ
#define WIN32_GENERIC_READ_DEFINED
#endif

#ifdef DOMAIN
#pragma push_macro("DOMAIN")
#undef DOMAIN
#define WIN32_DOMAIN_DEFINED
#endif


#endif // _MSC_VER


