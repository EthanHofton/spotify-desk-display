import random
import string

def random_string(n: int):
    """
    Generate a random string of length `n`
    """
    return ''.join(random.choices(string.ascii_letters + string.digits, k=n))
