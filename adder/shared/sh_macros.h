#ifndef SH_MACROS_H__
#define SH_MACROS_H__

#define VA_GET_1(X, ...) X
#define VA_GET_2(X, ...) VA_GET_1(__VA_ARGS__)
#define VA_GET_3(X, ...) VA_GET_2(__VA_ARGS__)
#define VA_GET_4(X, ...) VA_GET_3(__VA_ARGS__)
#define VA_GET_5(X, ...) VA_GET_4(__VA_ARGS__)
#define VA_GET_6(X, ...) VA_GET_5(__VA_ARGS__)
#define VA_GET_7(X, ...) VA_GET_6(__VA_ARGS__)
#define VA_GET_8(X, ...) VA_GET_7(__VA_ARGS__)
#define VA_GET(i, ...)   VA_GET_##i(__VA_ARGS__)

#define VA_ARRAY(TYPE, ...)     (TYPE []){ __VA_ARGS__ }
#define VA_ARRAYLEN(TYPE, ...)  (sizeof (const TYPE []){ __VA_ARGS__ } / sizeof (TYPE))

#endif // SH_MACROS_H__