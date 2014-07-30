-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION array_textsort" to load this file. \quit

-- array_textsort() sorts textual arrays
CREATE FUNCTION array_textsort(_text)
RETURNS _text
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

-- array_distinct() sorts textual arrays and removes duplicates
CREATE FUNCTION array_distinct(_text)
RETURNS _text
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT IMMUTABLE;

