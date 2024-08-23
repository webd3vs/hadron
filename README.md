<p align="center">
    <a href="https://github.com/webd3vs/hadron/build"><img src="https://img.shields.io/github/actions/workflow/status/webd3vs/hadron/c-cpp.yml?colorA=333&colorB=afa&style=for-the-badge"></a>
    <a href="https://github.com/webd3vs/hadron/stargazers"><img src="https://img.shields.io/github/stars/webd3vs/hadron?colorA=333&colorB=ffa&style=for-the-badge"></a>
    <a href="https://github.com/webd3vs/hadron/issues"><img src="https://img.shields.io/github/issues/webd3vs/hadron?colorA=333&colorB=faa&style=for-the-badge"></a>
    <a href="https://github.com/webd3vs/hadron/contributors"><img src="https://img.shields.io/github/contributors/webd3vs/hadron?colorA=333&colorB=aaf&style=for-the-badge"></a>
</p>
<h1 align="center">Hadron</h1>

# Hey!
### <font color="#ffc">If you have any ideas (syntax / features / improvement), please leave them [here](https://hadronlang.com/feedback), I would really appreciate that.</font>

### *Hello and welcome to Hadron :wave:, an early-stage language development project! We're thrilled that you've stopped by our project and hope you'll consider contributing to help us speed up development.*
### *We welcome any ideas, feedback, and contributions from the community. Whether you're a seasoned developer or just starting out, we'd love to hear from you.*
### *Please take a look at our documentation and feel free to reach out to us if you have any questions.*

---

Hadron is a custom programming language written in C. It is designed to be a versatile language that can be transcompiled to other languages or interpreted. It aims to provide a simple and intuitive syntax, while still being powerful and flexible.

## Planned Features

 - Simple and intuitive syntax
 - Static type system
 - On-demand garbage collection
 - Support for closures and anonymous functions
 - First-class functions
 - Modules and namespaces
 - Interoperability with C code
 - Transcompilation to C, JavaScript, Python, and other languages

## Getting Started

To build Hadron, you need to have `clang` installed on your system. You can build Hadron by running the following commands:

```sh
git clone https://github.com/hadron-lang/hadron.git
cd hadron
make
```

To run the interpreter, you can use the following command:

```sh
./hadron
```

To transcompile to C, you can use the following command:

```sh
./hadron -l c input.hdn -o output.c
```

To transcompile to other languages, you can replace c with the target language. For example:
```sh
./hadron -l js input.hdn -o output.js
```
## Examples
_Please note that the syntax may change in the future._

Here is an example of a "Hello, world!" program in Hadron:

```
fx main {
  IO:out("Hello, world!")
}
```

And here is an example of a function that calculates the nth Fibonacci number:

```
fx fib(i32 n) {
  if n < 2 { return n; }
  else { return fib(n-1) + fib(n-2); }
} i32

fx main() {
  IO:out(fib(10));
}
```


## Contributing

We welcome contributions to Hadron! If you find a bug or have an idea for a new feature, please open an issue on our GitHub repository. If you would like to contribute code, please submit a pull request. Before submitting a pull request, please make sure your code follows our coding style and passes our tests.

## License

Hadron is released under the MIT License. See LICENSE for details.
