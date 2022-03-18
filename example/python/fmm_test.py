from __future__ import print_function
from fmm import Network,NetworkGraph,STMATCH,STMATCHConfig
network = Network("../data/edges.shp")
graph = NetworkGraph(network)
print(graph.get_num_vertices())
model = STMATCH(network,graph)
wkt = "LINESTRING(0.200812146892656 2.14088983050848,1.44262005649717 2.14879943502825,3.06408898305084 2.16066384180791,3.06408898305084 2.7103813559322,3.70872175141242 2.97930790960452,4.11606638418078 2.62337570621469)"
config = STMATCHConfig()
config.k = 4
config.gps_error = 0.5
config.radius = 0.4
config.vmax = 30;
config.factor =1.5
result = model.match_wkt(wkt,config)
print(type(result))
print("Opath ",list(result.opath))
print("Cpath ",list(result.cpath))
print("WKT ",result.mgeom.export_wkt())
