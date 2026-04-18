import sys

test_name = sys.argv[1]
input_file = sys.argv[2]
output_file = sys.argv[3]

with open(input_file, "r") as f:
    lines = [line.strip() for line in f if line.strip()]

with open(output_file, "a") as out:
    for line in lines:
        out.write(f"{test_name};{line}\n")