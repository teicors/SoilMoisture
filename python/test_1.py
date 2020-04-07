from bottle import request, response
from bottle import post, get, put, delete
import re, json

_names = set()                    # the set of names

namepattern = re.compile(r'^[a-zA-Z\d]{1,64}$')

@post('/names')
def creation_handler():
    '''Handles name creation'''

    try:
        # parse input data
        try:
            data = request.json()
        except:
            raise ValueError

        if data is None:
            raise ValueError

        # extract and validate name
        try:
            if namepattern.match(data['name']) is None:
                raise ValueError
            name = data['name']
        except (TypeError, KeyError):
            raise ValueError

        # check for existence
        if name in _names:
            raise KeyError

    except ValueError:
        # if bad request data, return 400 Bad Request
        response.status = 400
        return
    
    except KeyError:
        # if name already exists, return 409 Conflict
        response.status = 409
        return

    # add name
    _names.add(name)
    
    # return 200 Success
    response.headers['Content-Type'] = 'application/json'
    return json.dumps({'name': name})
