-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION array_textsort" to load this file. \quit

ALTER EXTENSION array_textsort ADD function array_textsort(text[]);
ALTER EXTENSION array_distinct ADD function array_textsort(text[]);

