CREATE EXTENSION array_textsort;

SELECT ftest('{"a","b","ä",2,1,"@"}');

SELECT ftest('{"b","B","A","C","ä","Ä","a","b"}');

SELECT ftest('{"arbeit","Arbeit","aRbeit","atom"}');

SELECT ftest('{"aBc","AbC","Abc","aBC","abc","ABc","abC","ABC"}');

SELECT array_textsort('{"abc",NULL,"xyz"}');
