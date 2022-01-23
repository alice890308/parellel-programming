d = dict()

for chunk in range(0, 10):
    for reducer in range(1, 5):
        with open(f'./interFile/chunk{chunk}_reducer{reducer}.txt') as f:
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
with open('check.txt', 'w') as f:
    for word, count in d:
        f.write(f'{word} {count}\n');