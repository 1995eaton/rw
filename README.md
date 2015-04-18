# rw
a unix command that allows redirecting files to themselves

# Usage

```bash
seq 100 > example.txt

# commands like this won't work in most shells:

# generate 10 lines of numbers of length 5
tr -cd '[:digit:]' < /dev/urandom | fold -w5 | head -10 > example.txt

# commands like these won't work:
cat example.txt | sort -n | uniq > example.txt
cat example.txt | grep -v '[2468]$' > example.txt
cat example.txt | cat > example.txt

# rw solves this problem:
cat example.txt | sort -n | rw example.txt
```
