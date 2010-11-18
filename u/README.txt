No library is complete without a testsuite. However, with the PDK
being mostly a data access library, testing it is not easy without,
well, data.

This directory contains the humble beginnings of a testsuite. It
doesn't yet test very much.

However, the PDK (in combination with VGAPLisp) is also used to test
PHost. As of 18/Nov/2010, that testsuite consists of over 1600 tests
with over 200000 individual checks. In addition, I'm testing the
combat code in various versions against the PCC2 combat code. This
should give us some confidence that it actually works.


--Stefan
