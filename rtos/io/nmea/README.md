NMEA Parser
===========
This implements parsers for devices talking in NMEA standard.

To compile the parser run:
`
ragel -G2 -L nmea_parser.rl -o nmea_parser.c
`

To generate DOT file run:
`
ragel -G2 -L nmea_parser.rl -V -o nmea_parser.dot
`
or for compact size
`
ragel -T1 -L nmea_parser.rl -V -o nmea_parser.dot
`
After that replace tabs with 4 spaces if needed.
Use http://www.webgraphviz.com/ or similar to view the genearted graph.