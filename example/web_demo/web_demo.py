'''
    FMM web application
    Author: Can Yang
'''

import os
import tornado.wsgi
import tornado.httpserver
import time
import optparse
import logging
import flask
from flask import jsonify
from mapmatcher import MapMatcher

import numpy as np

app = flask.Flask(__name__,static_url_path='/static')
app.config['SEND_FILE_MAX_AGE_DEFAULT'] = 0

@app.route('/demo')
def index():
    return flask.render_template('demo.html')

#@app.route('/static/<path:path>')
#def send_js(path):
    #return send_from_directory('static', path)

@app.route('/match_wkt', methods=['GET'])
def match_url():
    # print(flask.request.args)
    wkt = str(flask.request.args.get('wkt', ''))
    logging.info('WKT get in python: %s', wkt)
    starttime = time.time()
    result = app.mapmatcher.match_wkt(wkt)
    mgeom_wkt = ""
    if (result.mgeom.get_num_points()>0):
        mgeom_wkt = result.mgeom.export_wkt()
    # logging.info('Probs %s',probs)
    endtime = time.time()
    # logging.info('%s', result)
    # logging.info('Time cost: %s', result[2])
    # print "Result is ",result
    # print "Result geom is ",result.mgeom
    if (mgeom_wkt!=""):
        print "Matched"
        response_json = {"wkt":mgeom_wkt,"state":1}
        return jsonify(response_json)
    else:
        print "Not matched"
        return jsonify({"state":0})

def start_tornado(app, port=5000):
    http_server = tornado.httpserver.HTTPServer(
        tornado.wsgi.WSGIContainer(app))
    http_server.listen(port)
    print("Tornado server starting on port {}".format(port))
    print("Visit http://localhost:{}/demo to check the demo".format(port))
    tornado.ioloop.IOLoop.instance().start()
def start_from_terminal(app):
    """
    Parse command line options and start the server.
    """
    parser = optparse.OptionParser()
    # Store the file argument into the filename attr
    # parser.add_option("-f", "--file", action="store", type="string", dest="filename")
    parser.add_option(
        '-d', '--debug',
        help="enable debug mode",
        action="store_true", default=False)
    parser.add_option(
        '-p', '--port',
        help="which port to serve content on", action="store",dest="port", type='int', default=5000)
    parser.add_option(
        '-c', '--config',
        help="the model configuration file",action="store",dest="config_file",
        type='string', default="config.json")
    opts, args = parser.parse_args()
    app.mapmatcher = MapMatcher(opts.config_file)
    if opts.debug:
        app.run(debug=True, host='0.0.0.0', port=opts.port)
    else:
        start_tornado(app, opts.port)
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
    start_from_terminal(app)
