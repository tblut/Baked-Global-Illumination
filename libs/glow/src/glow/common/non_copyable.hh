#pragma once

/// This macro delete copy and move c'tor and assignment op
#define GLOW_NON_COPYABLE(CLASS)              \
    CLASS(CLASS const &) = delete;            \
    CLASS(CLASS &&) = delete;                 \
    CLASS &operator=(CLASS const &) = delete; \
    CLASS &operator=(CLASS &&) = delete // force ;

/// This macro is basically non-copyable for the RAII case, i.e. including move c'tor
#define GLOW_RAII_CLASS(CLASS)                \
    CLASS(CLASS const &) = delete;            \
    CLASS &operator=(CLASS const &) = delete; \
    CLASS &operator=(CLASS &&) = delete // force ;
