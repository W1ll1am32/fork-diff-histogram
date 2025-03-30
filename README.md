# fork-diff-histogram

Собирать:
```bash
g++ -std=c++17 -o app_name main.cpp Tokenizer.cpp Diff.cpp
```
На вход передавать файлы old, new. В ином случае будут использоваться файлы по умолчанию: 
```bash
./app_name old.txt new.txt
