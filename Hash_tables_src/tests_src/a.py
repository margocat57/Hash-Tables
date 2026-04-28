import random
import string

def random_word(length=5):
    s = ""
    for i in range(length):
        s += random.choice(string.ascii_lowercase)
    s += "\n"
    return s


with open("words.txt", "w") as f:
    for i in range(1_000_000):
        f.write(random_word(random.randint(3, 10)))