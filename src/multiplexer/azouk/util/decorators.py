
import sys
import traceback
from functools import wraps

def parametrizable_decorator(decorator):
    @wraps(decorator)
    def wrapper(f = None, *args, **kwargs):
        if f is not None:
            return wraps(f)(decorator(f, *args, **kwargs))
        else:
            return lambda f: wraps(f)(decorator(f, *args, **kwargs))
    return wrapper

@parametrizable_decorator
def never_throw(f, default = None):
    def wrapper(*args, **kwargs):
        try:
            return f(*args, **kwargs)
        except Exception:
            traceback.print_stack()
            traceback.print_exc()
            print >> sys.stderr, "Ignored."
            return default
    return wrapper
