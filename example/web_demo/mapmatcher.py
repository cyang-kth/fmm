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
        if ("model" not in data):
            raise Exception("Model is missing.")
        if ("input" not in data):
            raise Exception("Input is missing.")
        if ("network" not in data["input"]):
            raise Exception("Network is missing.")
        if ("file" not in data["input"]["network"]):
            raise Exception("Network file is missing.")
        self.network_file = str(data["input"]["network"]["file"])
        if "id" in data["input"]["network"]:
            self.network_id = str(data["input"]["network"]["id"])
        else:
            self.network_id = "id"
        if "source" in data["input"]["network"]:
            self.network_source = str(data["input"]["network"]["source"])
        else:
            self.network_source = "source"
        if "target" in data["input"]["network"]:
            self.network_target = str(data["input"]["network"]["target"])
        else:
            self.network_target = "target"
        if str(data["model"])=="stmatch":
            self.model_tag = "stmatch"
            self.mm_config = STMATCHConfig()
            if "parameters" in data:
                if "k" in data["parameters"]:
                    self.mm_config.k = data["parameters"]["k"]
                if "r" in data["parameters"]:
                    self.mm_config.radius = data["parameters"]["r"]
                if "e" in data["parameters"]:
                    self.mm_config.gps_error = data["parameters"]["e"]
                if "f" in data["parameters"]:
                    self.mm_config.factor = data["parameters"]["f"]
                if "vmax" in data["parameters"]:
                    self.mm_config.vmax = data["parameters"]["vmax"]
                if "reverse_tolerance" in data["parameters"]:
                    self.mm_config.reverse_tolerance = data["parameters"]["reverse_tolerance"]
        elif (str(data["model"])=="fmm"):
            self.model_tag = "fmm"
            if ("ubodt" not in data["input"]):
                raise Exception("Ubodt is missing.")
            if ("file" not in data["input"]["ubodt"]):
                raise Exception("Ubodt file is missing.")
            self.ubodt_file = str(data["input"]["ubodt"]["file"])
            self.mm_config = FastMapMatchConfig()
            if "parameters" in data:
                if "k" in data["parameters"]:
                    self.mm_config.k = data["parameters"]["k"]
                if "r" in data["parameters"]:
                    self.mm_config.radius = data["parameters"]["r"]
                if "e" in data["parameters"]:
                    self.mm_config.gps_error = data["parameters"]["e"]
                if "reverse_tolerance" in data["parameters"]:
                    self.mm_config.reverse_tolerance = data["parameters"]["reverse_tolerance"]
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
