import sys
import random
import argparse
parser = argparse.ArgumentParser()
parser.add_argument('-t', '--train', required = True)
parser.add_argument('-e', '--test', required = True)
ns = parser.parse_args(sys.argv[1:])
lines = []
for line in sys.stdin:
    if line:
        lines.append(line)
random.shuffle( lines )
train_f = open(ns.train, 'w')
test_f = open(ns.test, 'w')
for line in lines:
    if random.randint(1,10) != 1:
        train_f.write(line)
    else:
        test_f.write(line)


train_f.close()
test_f.close()
    
