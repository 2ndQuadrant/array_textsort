-- This function takes a TEXT[] and sorts it with array_textsort(),
-- then UNNEST()s it for comparison with the result of sorting via ORDER BY,
-- which we assume to be correct. This way, we avoid hard coding sort results
-- into the test cases, as sorting is locale dependend.
CREATE FUNCTION ftest(arr TEXT[]) RETURNS BOOLEAN AS $$
DECLARE
	ret	BOOLEAN;
BEGIN
WITH
	orig(c) AS (
		SELECT
			UNNEST(arr)
		),
	sorted(c, r) AS (
		SELECT
			c, RANK() OVER (ORDER BY c) FROM orig
		),
	arraysorted(c, r) AS (
		SELECT
			UNNEST(array_textsort(arr)),
			GENERATE_SERIES(1, ARRAY_LENGTH(arr, 1))
		)
SELECT INTO ret
	CASE COUNT(*) WHEN ARRAY_LENGTH(arr, 1) THEN TRUE ELSE FALSE END FROM sorted JOIN arraysorted USING (c, r);

RETURN ret;
END;
$$ LANGUAGE plpgsql;
