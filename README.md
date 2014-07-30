array_textsort
==============

This extension provides two functions for handling of TEXT arrays:
- array_textsort(TEXT[]) returns the array, sorted according to the
  curent database locale. The input array must not contail NULLs.
- array_distinct(TEXT[]) returns an array of all the distinct values
  of the input array. As with array_textsort(), the input array must
  not contain NULLs.
