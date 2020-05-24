# PasswordGuesser
C Program to brute force passwords for a given info and it will tell you in how much time he cracked or if didnt find it
It uses parallel programming with pthread.h library

# How to execute it
Install folder in your computer and put your basic info (things a hacker would know about you: name, football team, city, street...) in the file `files/victim.txt`

- `gcc parallel.c -o exe -lpthread`

- `exe yourPassword`
