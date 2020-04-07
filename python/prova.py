import bottle
import bottle_mysql
from bottle import route, run
from bottle import template

app = bottle.Bottle()
# dbhost is optional, default is localhost
plugin = bottle_mysql.Plugin(dbuser='angelo', dbpass='angelo', dbname='angelo')
app.install(plugin)

@app.route('/show')
def show(item, db):
    db.execute('SELECT * from temperatura where basetta=".192.168.1.220."')
    row = db.fetchone()
    if row:
        return template('showitem', page=row)
    return HTTPError(404, "Page not found")

run(host='localhost', port=8000, debug=True)

