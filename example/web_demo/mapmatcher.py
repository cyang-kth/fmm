import os
import json
from fmm import Network, NetworkGraph
from fmm import FastMapMatch, FastMapMatchConfig, UBODT
from fmm import STMATCH, STMATCHConfig

class MapMatcherConfig(object):
    def __init__(self, config_json_file):
        if not os.path.exists(config_json_file):
            raise Exception(
                "File for {} is missing.".format(config_json_file))
        with open(config_json_file) as f:
            data = json.load(f)
        if (not data.has_key("model")):
            raise Exception("Model is missing.")
        if (not data.has_key("input")):
            raise Exception("Input is missing.")
        if (not data["input"].has_key("network")):
            raise Exception("Network is missing.")
        if (not data["input"]["network"].has_key("file")):
            raise Exception("Network file is missing.")
        self.network_file = str(data["input"]["network"]["file"])
        if data["input"]["network"].has_key("id"):
            self.network_id = str(data["input"]["network"]["id"])
        else:
            self.network_id = "id"
        if data["input"]["network"].has_key("source"):
            self.network_source = str(data["input"]["network"]["source"])
        else:
            self.network_source = "source"
        if data["input"]["network"].has_key("target"):
            self.network_target = str(data["input"]["network"]["target"])
        else:
            self.network_target = "target"
        if str(data["model"])=="stmatch":
            self.model_tag = "stmatch"
            self.mm_config = STMATCHConfig()
            if data.has_key("parameters"):
                if data["parameters"].has_key("k"):
                    self.mm_config.k = data["parameters"]["k"]
                if data["parameters"].has_key("r"):
                    self.mm_config.radius = data["parameters"]["r"]
                if data["parameters"].has_key("e"):
                    self.mm_config.gps_error = data["parameters"]["e"]
                if data["parameters"].has_key("f"):
                    self.mm_config.factor = data["parameters"]["f"]
                if data["parameters"].has_key("vmax"):
                    self.mm_config.vmax = data["parameters"]["vmax"]
        elif (str(data["model"])=="fmm"):
            self.model_tag = "fmm"
            if (not data["input"].has_key("ubodt")):
                raise Exception("Ubodt is missing.")
            if (not data["input"]["ubodt"].has_key("file")):
                raise Exception("Ubodt file is missing.")
            self.ubodt_file = str(data["input"]["ubodt"]["file"])
            self.mm_config = FastMapMatchConfig()
            if data.has_key("parameters"):
                if data["parameters"].has_key("k"):
                    self.mm_config.k = data["parameters"]["k"]
                if data["parameters"].has_key("r"):
                    self.mm_config.radius = data["parameters"]["r"]
                if data["parameters"].has_key("e"):
                    self.mm_config.gps_error = data["parameters"]["e"]
        else:
            raise Exception(
                "Model not found for {} ".format(
                    data["model"]))

class MapMatcher(object):
    def __init__(self, config_json_file):
        if not os.path.exists(config_json_file):
            raise Exception(
                "File for {} is missing.".format(config_json_file))
        config = MapMatcherConfig(config_json_file)
        self.network = Network(
            config.network_file,config.network_id,
            config.network_source,config.network_target)
        self.graph = NetworkGraph(
            self.network)
        if config.model_tag=="stmatch":
            self.model = STMATCH(self.network,self.graph)
            self.mm_config = config.mm_config
        elif (config.model_tag=="fmm"):
            self.ubodt = UBODT.read_ubodt_file(
                config.ubodt_file)
            self.model = FastMapMatch(self.network, self.graph, self.ubodt)
            self.mm_config = config.mm_config
        else:
            raise Exception(
                "Model not found for {} ".format(
                    data["model"]))

    def match_wkt(self, wkt):
        return self.model.match_wkt(wkt,self.mm_config)
