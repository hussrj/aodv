aodv
====

OPNET implementations of location aided routing protocols for MANET (LAR distance and zone as well as GeoAODV static and rotate).

Initial codebase pulled from https://github.com/ecks/ru-compsci-sr-project-aodv (google-code prior) and further modified such that alpha and beta used in the LAR distance rebroadcast calculation have been parameterized, as well as the number of degrees by which to expand the flooding angle after a route discovery failure when using GeoAODV.
