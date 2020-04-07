from bottle import Bottle, run, post, error
import bottle
import bottle_mysql
from bottle import template

app = bottle.Bottle()
# dbhost is optional, default is localhost
plugin = bottle_mysql.Plugin(dbuser='angelo', dbpass='angelo', dbname='angelo')
app.install(plugin)

@app.route('/show')
def show(item, db):
    db.execute('SELECT * from angelo.temperatura where sensorid="led0"')
    row = db.fetchone()
    if row:
        return template('showitem', page=row)
    return HTTPError(404, "Page not found")

run(app, host='localhost', port=8080)
