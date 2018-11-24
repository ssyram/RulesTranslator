#  Rules Translator README

This project is a support tool project for the Mini_C project. Aiming at let the syntactic parse process of Mini_C can focus on the expression of information rather than the form. And this can translate all of the LR(1) grammer. Basically, it takes in the information that needed to distinguish a grammer, like: the terminate and nonterminate symbol set, the "production" list and the first nonterminate symbol, then output the ActionTable and GotoTable in the form of C++ code. So, it takes in a ".tsl" file and output a ".cpp" file.

## How to Use This Project

1. Write a valid ".tsl" file (Detail)
2. Compile the project with C++17 compiler.
3. `./<program name> <target ".tsl" file> <target ".cpp" file name>`
4. Use the ".cpp" file with a provided interface (Detail)

## Detail Description of the format of ".tsl" File

```
#include "rules_translator.h" // this file discribe the cpp std libary dependency

// ... Here, write some C++ codes
// like the declaration of definition of the class / function needed by the following part

// it's recommended that the \```tsl block wrapped by a namespace
namespace sample {
// pay attention that the begin and over of the block should not have any indentation
\```tsl
  // from here on, only "//" are accepted as line comment.
  // the terminate, token_type, get_type are 3 keywords here, please choose another name for other action
  // the following 3 sentences are required by tsl, do not oversee any of them
  terminate = enum class <the enum class name describe all the terminate symbols> {
    <symbol 1>, <symbol 2>, ...
  }; // pay attention that no comment are allowed in this block
  // this defines the type of token that are going to be put into this analyze program
  token_type = <a C++ type name that represent the type of token>
  // this defines the function that used to map token_type to terminate enum class
  // the function pointer should be like this:
  // <terminate enum class name> (*)(const token_type &);
  get_type = <the target map function>

  // the following "using" statement used to bind each of the nonterminate type with a C++ type name
  // this part is alternative
  using <a nonterminate type symbol name> = <a target C++ type name>;
  ...

  // here starts the description for all productions
  // the left symbol of the first production written will be automatically considered start nonterminate symbol
  // a terminate symbol can / should be wrapped by '"'
  <left nonterminate symbol> := <symbol 1> <symbol 2> ...   { <action block> }
  // the content in <action block> should be C++ codes that are supposed to be executed when this production reduce
  // the <action block> is alternative, which means the block follows each of the production can be replaced by semicolon(";")
  ...
\```
}

// ... C++ codes

```
See `samples/calculator.tsl` for a sample of this format.

## See `extern/TargetFormat.h` for what a target ".cpp" will look like

This project stems from the demand of the interpreter course in my curriculum that to implement a language interpreter, a new one or existed one. And the lexical analyzer has already been implemented. The next one is how to check the stream of token syntactically and semantically.

And, to program, in fact, according to my comprehension, is to express exact and sufficient information that aims to be executed by the machine. The more briefly sufficient information is given, the more it can get away from the trap of being haunted by its interpreter(here means the mechanism that use to interpret the given infomation).

This becomes increasingly eminent when it comes to the question of how to syntactically check and semantically execute a language. Intuitively, this task means an implementation like this:
```
  if (s[i] == IF) {
    if (s[i + 1] == LEFT_BRACKET) {
      ...
    }
  }
```
And this is already an explicit implementation. And here, the information is haunted by its way of interpretation. Here comes a problem -- it's hard to distinguish the information from its interpretation, which makes the information become ambiguious. So, I started to find a brief way to express the information and seperate it from the interpreter.



This, in fact, as for the syntactic rules, essentially comes into the question that how to express sufficient information of a certain language rule.
