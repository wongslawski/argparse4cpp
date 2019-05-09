# argparse4cpp
A simple yet applicable python-argparse-like c++11 header for command line argument parsing.  
It abandons positional arguments and **_supports only optional arguments_**(like -h/--help, with `-` or `--` prefix).

# Example
See [example.cpp](https://github.com/wongslawski/argparse4cpp/blob/master/example.cpp)
1. `make example`
2. try `./example -a 1 -b 0.1 0.5 1.0 --chi --delta --epsilon a b c -g true false`

# Quick Start
1. `make install` and `#include <argparse4cpp/argparse.hpp>`
2. copy `argparse.hpp` to your own project and `#include "argparse.hpp"` since header only.

# Creating a parser
```c++
ArgumentParser argparser("this is an example program for argparse");
```

# Add arguments
A simple example. This argument will be parsed with `-a` or `--alpha` from command line, 
and needs `one single value` of `integer type`.
It `can not be omitted` at the command line and `value must be among {0, 1, 2}`.

```c++
argparser.AddArgument("-a", "--alpha", "alpha", ValueType::Int, "fixed int illustration")
        .SetRequired(true).SetNargs(1).SetChoices<int>({0, 1, 2});
```

Method `AddArgument(...)` prototype is the following:

```
AddArgument(const string &short_name, const string &name, 
        const string &dest, ValueType type, const string &help);
```

* `short_name` arg name with `-` prefix
* `name` arg name with `--` prefix
* `dest` the key to get argument values
* `type` type of specified values
  * ValueType::Int
  * ValueType::Long
  * ValueType::Float
  * ValueType::Double
  * ValueType::String
  * ValueType::Bool
* `help` comment text to print

Configure other argument properties

* `SetRequired(bool required)` `true by default`.
* `SetNargs(int nargs)` specifies fixed number of values. `1 by default`.
* `SetNargs(char nargs)` specifies variable list of values. `'*' means >=0 and '+' means >= 1`.
* `SetDefault(const T &val)` specifies one default value when arg is omitted. (with `SetRequired(false)`)
* `SetDefault(const vector<T> &val)` specifies default value list when arg is omitted. (with `SetRequired(false)`)
* `SetChoices(const vector<T> &val)` specifies the valid value collection.
* `SetAction(ActionType action)` specifies arg action. 
  * `ActionType::Store` by default, read values at command line 
  * `ActionType::StoreTrue` if arg is present, value set to true. Otherwise, false. (with `ValueType::Bool`)
  * `ActionType::StoreFalse` if arg is present, value set to false. Otherwise, true. (with `ValueType::Bool`)
* `SetRange(double lower, double upper)` specifies the bound of values. (with `ValueType::Int,Long,Float,Double`)

#### Argument (optional) receiving a fixed number of doubles [0, 1]
```c++
argparser.AddArgument("-b", "--beta", "beta", ValueType::Double, "fixed double list")
        .SetRequired(false).SetNargs(3).SetRange(0, 1).SetDefault({0.1, 0.3, 0.9});
```

#### Argument (required) receiving variable number of strings
```c++
argparser.AddArgument("-e", "--epsilon", "epsilon", ValueType::String, "nargs*")
        .SetRequired(true).SetNargs('*').SetChoices<string>({"a", "b", "c", "d", "e", "f", "g"});
```

#### Argument (optional) work as a switch
```c++
argparser.AddArgument("-c", "--chi", "chi", ValueType::Bool, "action->store_true")
        .SetRequired(false).SetAction(ActionType::StoreTrue);
```

# Parse arguments
```c++
int ret = argparser.Parse(argc, argv); // argv[0] should be program path
```
* `ret = 0` means successfully parsing 
* `ret > 0` means help or usage is needed
* `ret < 0` means failure in parsing

To print help info, try
```
if (ret > 0) {
    argparser.PrintHelp();
    return 0;
}
```

# Get argument value
Get single value
```
int alpha = argparser.Get<int>("alpha");
```

Get a list of values
```c++
vector<string> epsilon = argparser.GetList<string>("epsilon");
```

# Just have fun!
