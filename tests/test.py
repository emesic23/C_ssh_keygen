import random as rand
import math


def numberToBase(n, b):
    if n == 0:
        return [0]
    digits = []
    while n:
        digits.append(int(n % b))
        n //= b
    return digits[::-1]

file = open('nums.txt', 'w')
file2 = open('nums_dec.txt', 'w')
for i in range(100):
    bits = rand.getrandbits(rand.randint(1, 300))
    file.write("{:X}\n".format(bits))
    file2.write("{}\n".format(bits))
file.close()
file2.close()

file = open('nums_div.txt', 'w')
file2 = open('nums_div_dec.txt', 'w')
for i in range(100):
    bits = rand.getrandbits(rand.randint(300, 400))
    bits2 = rand.getrandbits(rand.randint(1, 199))
    while(bits2 == 0):
        bits2 = rand.getrandbits(rand.randint(1, 199))
    file.write("{:X}\n".format(bits))
    file.write("{:X}\n".format(bits2))
    file2.write("{}\n".format(bits))
    file2.write("{}\n".format(bits2))
file.close()
file2.close()

file = open('nums_mont.txt', 'w')
file2 = open('nums_mont_dec.txt', 'w')
for i in range(100):
    A = rand.getrandbits(rand.randint(1, 200))
    E = rand.getrandbits(rand.randint(1, 6))
    M = rand.getrandbits(rand.randint(1, 100))
    while(M == 0 or M % 2 == 0):
        M = rand.getrandbits(rand.randint(1, 100))
    file.write("{:X}\n".format(A))
    file.write("{:X}\n".format(E))
    file.write("{:X}\n".format(M))
    file2.write("{}\n".format(A))
    file2.write("{}\n".format(E))
    file2.write("{}\n".format(M))
file.close()
file2.close()

ref_add = open('ref_add.txt', 'w')
ref_sub = open('ref_sub.txt', 'w')
ref_mul = open('ref_mul.txt', 'w')
ref_div = open('ref_div.txt', 'w')
ref_cmp = open('ref_cmp.txt', 'w')
ref_gcd = open('ref_gcd.txt', 'w')
ref_mont = open('ref_mont.txt', 'w')
ref_modinv = open('ref_modinv.txt', 'w')
with open('nums.txt') as f:
    lines = f.readlines()
    prev = int(lines[0], 16)
    for line in range(1, len(lines)):
        num = int(lines[line], 16)
        ref_add.write("{:X}\n".format(prev + num))
        ref_sub.write("{:X}\n".format(prev - num))
        ref_mul.write("{:X}\n".format(prev * num))
        if prev > num:
            ref_cmp.write("1\n")
        if prev < num:
            ref_cmp.write("-1\n")
        if prev == num:
            ref_cmp.write("0\n")
        ref_gcd.write("{:X}\n".format(math.gcd(prev, num)))
        try:
            ref_modinv.write("{:X}\n".format(pow(prev, -1, num)))
        except:
            ref_modinv.write("{:X}\n".format(0))
        prev = num

with open('nums_div.txt') as f:
    lines = f.readlines()
    for line in range(0, len(lines), 2):
        top = int(lines[line], 16)
        bottom = int(lines[line+1], 16)
        ref_div.write("{:X}\n".format(top // bottom))
        ref_div.write("{:X}\n".format(top % bottom))

with open('nums_mont.txt') as f:
    lines = f.readlines()
    for line in range(0, len(lines), 3):
        A = int(lines[line], 16)
        E = int(lines[line + 1], 16)
        M = int(lines[line + 2], 16)
        ref_mont.write("{:X}\n".format((A ** E)%M))

file = open('nums_binary.txt', 'w')
fileref = open('ref_binary.txt', 'w')
for i in range(100):
    bits = rand.getrandbits(rand.randint(1, 300))
    bytes = numberToBase(bits, 256)
    s = ""
    for byte in bytes:
        if byte == 10 or byte == 14 or byte == 13:
            i -= 1
            continue
        s += "{:c}".format(byte)
    s += "\n"
    file.write("{:X}\n".format(bits))
    fileref.write(s)
file.close()

ref_add.close()
ref_sub.close()
ref_mul.close()
ref_div.close()
ref_cmp.close()
ref_gcd.close()
        