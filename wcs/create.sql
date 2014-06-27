SET ROLE casuadmin;

CREATE OR REPLACE FUNCTION angdist (double precision, double precision, double precision, double precision)
returns double precision
as '/Users/eglez/Development/SoftwareC/postgres/wcslib.so', 'angdist'
language 'c';

CREATE OR REPLACE FUNCTION onchip (double precision, double precision, integer, integer, double precision, double precision, double precision, double precision, double precision, double precision, double precision, double precision)
RETURNS boolean
AS '/Users/eglez/Development/SoftwareC/postgres/wcslib', 'onchip'
LANGUAGE 'c';

CREATE OR REPLACE FUNCTION onchip (double precision, double precision, integer, integer, double precision, double precision, double precision, double precision, double precision, double precision, double precision, double precision, double precision, double precision, double precision)
RETURNS boolean
AS '/Users/eglez/Development/SoftwareC/postgres/wcslib', 'onchip'
LANGUAGE 'c';



