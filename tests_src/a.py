import random
import string

def random_word(length=5):
    return ''.join(random.choice(string.ascii_lowercase) for _ in range(length))

words = (random_word(random.randint(3,10)) for _ in range(1_000_000))

with open("words.txt", "w") as f:
    for w in words:
        f.write(w + "\n")
