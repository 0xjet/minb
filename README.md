# minb

A minimal bind shell. See [UNIX daemonization and the double fork](https://0xjet.github.io/3OHA/2022/04/11/post.html) for context.


## Instructions

Compile and run it with:

```
$ gcc -o minb minb.c
$ minb
```

Connect programmatically or with a standard `nc`

```
$ nc <HOSTNAME> <PORT>
```
