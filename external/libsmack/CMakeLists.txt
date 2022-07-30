
project(smacker
    LANGUAGES C
    VERSION 1.2.0.43 #r43
)

set(LIBSMACKER_SOURCES
    smacker.c
    smacker.h
    smk_malloc.h
)

add_library(libsmacker OBJECT ${LIBSMACKER_SOURCES})
