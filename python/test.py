from bottle import Bottle, run, post, error
from bottle import template
import bottle_mysql

plugin = bottle_mysql.Plugin(dbuser='angelo', dbpass='angelo', dbname='angelo')

app = Bottle()
app.install(plugin)

@app.route('/')
@app.route('/hello/<name>')
def greet(name='Stranger'):
    return template('Hello {{name}}, how are you?', name=name)

@post('/login') # or @route('/login', method='POST')
def do_login():
    username = request.forms.get('username')
    password = request.forms.get('password')
    if check_login(username, password):
        return "<p>Your login information was correct.</p>"
    else:
        return "<p>Login failed.</p>"

@app.route('/show/:<item>')
def show(item):
    dbname.execute('SELECT * from angelo.temperatura') # where sensorid="%s"', (item,))
    row = dbname.fetchone()
    return row
    if row:
        return template('showitem', page=row)
    return error(404, "Page not found")

@error(404)
def error404(error):
    return 'Nothing here, sorry'


run(app, host='localhost', port=8080)

