d = dict()
dirname = input()
r = int(input())

for reducer in range(1, r+1):
    with open(f'./{dirname}/{dirname}-{reducer}.out') as f:
        lines = f.readlines()
        for line in lines:
            word, count = line.split()
            if word in d:
                d[word] += int(count)
        
            # if the dictionary does not have the key as "elements" 
            # then create a key "elements" and assign its value to 1.
            else:
                d.update({word: int(count)})

d = sorted(d.items())
with open('check_final.txt', 'w') as f:
    for word, count in d:
        f.write(f'{word} {count}\n');