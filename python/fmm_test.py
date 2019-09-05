import fmm
model = fmm.MapMatcher("fmm_config.xml")
### Run map matching
wkt = "LINESTRING (0.200812146892656 2.14088983050848,1.44262005649717 \
2.14879943502825,3.06408898305084 2.16066384180791,3.06408898305084 \
2.7103813559322,3.70872175141242 2.97930790960452, 4.11606638418078 \
2.62337570621469)"
result = model.match_wkt(wkt)

print "Matched path geometry",result.mgeom
print "Matched points geometry",result.pgeom
print "Matched edge id",list(result.opath)
print "Matched path edges",list(result.cpath)
