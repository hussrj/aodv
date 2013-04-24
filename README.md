aodv
====

OPNET implementations of location aided routing protocols for MANET (LAR distance/zone, GeoAODV static/rotate, etc.).

Initial codebase pulled from https://code.google.com/p/ru-compsci-sr-project-aodv/ and further modified such that alpha and beta used in the LAR distance rebroadcast calculation have been parameterized, as well as the number of degrees by which to expand the flooding angle after a route discovery failure when using GeoAODV.
