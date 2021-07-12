import sys
if (1>=len(sys.argv)):
    exit()
with open(sys.argv[1], 'r') as data:
    plaintext = data.read()

plaintext = plaintext.lower()
plaintext = plaintext.replace('  ', '\\x')
plaintext = plaintext.replace(' ', '\\x')

print(plaintext)
