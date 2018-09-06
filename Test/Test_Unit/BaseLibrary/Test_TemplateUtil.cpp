#include <BaseLibrary/TemplateUtil.hpp>
#include <iostream>

#include <Catch2/catch.hpp>

using namespace inl;


class Dummy_Test_TemplateUtil {};

static_assert(templ::is_printable<float>::value);
static_assert(templ::is_printable<float&>::value);
static_assert(templ::is_printable<float&&>::value);
static_assert(templ::is_printable<const float&&>::value);
static_assert(templ::is_printable<const float&>::value);

static_assert(!templ::is_printable<Dummy_Test_TemplateUtil>::value);
static_assert(!templ::is_printable<Dummy_Test_TemplateUtil&>::value);
static_assert(!templ::is_printable<Dummy_Test_TemplateUtil&&>::value);
static_assert(!templ::is_printable<const Dummy_Test_TemplateUtil&>::value);


static_assert(templ::is_readable<float>::value);
static_assert(templ::is_readable<float&>::value);
static_assert(!templ::is_readable<float&&>::value);
static_assert(!templ::is_readable<const float&&>::value);
static_assert(!templ::is_readable<const float&>::value);

static_assert(!templ::is_readable<Dummy_Test_TemplateUtil>::value);
static_assert(!templ::is_readable<Dummy_Test_TemplateUtil&>::value);
static_assert(!templ::is_readable<Dummy_Test_TemplateUtil&&>::value);
static_assert(!templ::is_readable<const Dummy_Test_TemplateUtil&>::value);


static_assert(templ::is_equality_comparable<float>::value);
static_assert(!templ::is_equality_comparable<Dummy_Test_TemplateUtil>::value);


static_assert(templ::is_less_comparable<float>::value);
static_assert(!templ::is_less_comparable<Dummy_Test_TemplateUtil>::value);