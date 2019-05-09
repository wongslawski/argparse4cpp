#ifndef WONGSLAWSKI_ARG_PARSE_H
#define WONGSLAWSKI_ARG_PARSE_H

#include <assert.h>
#include <map>
#include <vector>
#include <set>
#include <string>
#include <regex>
#include <sstream>
#include <exception>

using namespace std;

namespace ArgParse {


enum class ValueType: int {
    Int = 0,
    Long,
    Float,
    Double,
    Bool,
    String
};

enum class ActionType: int {
    Store = 0,
    StoreTrue,
    StoreFalse
};


class ArgumentParser;

class Argument {
public:
    friend class ArgumentParser;

    Argument() {}

    Argument(const Argument &src):
        short_name(src.short_name),
        name(src.name),
        dest(src.dest),
        help(src.help),
        required(src.required),
        fixed(src.fixed),
        fixed_nargs(src.fixed_nargs),
        variable_nargs(src.variable_nargs),
        type(src.type),
        defaults(src.defaults),
        choices(src.choices),
        values(src.values),
        loaded(src.loaded),
        action(src.action),
        range(src.range) {}

    Argument(const string &short_name,
            const string &name,
            const string &dest,
            ValueType type,
            const string &help): 
        short_name(short_name), 
        name(name), 
        dest(dest), 
        help(help),
        type(type) {
        defaults.clear();
        choices.clear();
        values.clear();
    }

    Argument& operator =(const Argument& src) {
        if (this != &src) {
            short_name = src.short_name;
            name = src.name;
            dest = src.dest;
            help = src.help;
            required = src.required;
            fixed = src.fixed;
            fixed_nargs = src.fixed_nargs;
            variable_nargs = src.variable_nargs;
            type = src.type;
            defaults = src.defaults;
            choices = src.choices;
            values = src.values;
            loaded = src.loaded;
            action = src.action;
            range = src.range;
        }
        return *this;
    }

    string GetName() const { return this->name; }
    string GetShortName() const { return this->short_name; }
    string GetDest() const { return this->dest; }
    string GetHelp() const { return this->help; }
    ValueType GetType() const { return this->type; }

    Argument& SetRequired(bool required) { this->required = required; return *this; }
    bool GetRequired() const { return this->required; }

    Argument& SetNargs(int nargs) { 
        assert(nargs >= 0);
        this->fixed = true; 
        this->fixed_nargs = nargs; 
        return *this; 
    }
    Argument& SetNargs(char nargs) { 
        assert(nargs == '*' || nargs == '+');
        this->fixed = false; 
        this->variable_nargs = nargs; 
        return *this; 
    }
    bool GetFixed() const { return this->fixed; }
    int GetFixedNargs() const { return this->fixed_nargs; }
    char GetVariableNargs() const { return this->variable_nargs; }
    
    template <class T>
    Argument& SetDefault(const T &val) { 
        this->defaults.push_back(Cast<T>(val)); 
        return *this; 
    }
    template <class T>
    Argument& SetDefault(const vector<T> &val) { 
        this->defaults = Fix(Cast<T>(val));
        return *this; 
    }
    const vector<string>& GetDefault() const { return this->defaults; }
    
    template <class T>
    Argument& SetChoices(const vector<T> &val) { 
        for (int i = 0; i < val.size(); ++i) {
            this->choices.push_back(Cast<T>(val[i]));
        }
        return *this; 
    }
    const vector<string>& GetChoices() const { return this->choices; }

    Argument& SetAction(ActionType action) { this->action = action; return *this; }
    ActionType GetAction() const { return this->action; }

    Argument& SetPrint(bool print) { this->print = print; return *this; }

    Argument& SetRange(double lower, double upper) {
        assert(type == ValueType::Int || type == ValueType::Long || type == ValueType::Double || type == ValueType::Float);
        range[0] = lower;
        range[1] = upper;
        return *this;
    }

private:
    template <class T>
    string Cast(const T &val) {
        stringstream stream;
        stream << val;
        string res;
        stream >> res;
        return res;
    }

    template <class T>
    vector<string> Cast(const vector<T> &val) {
        vector<string> res;
        for (int i = 0; i < val.size(); ++i) {
            res.push_back(Cast<T>(val[i]));
        }
        return res;
    }

    bool CheckRange(double x) {
        if (range[0] < range[1]) {
            return x >= range[0] && x <= range[1];
        }
        return true;
    }  

    bool CheckValue(const string &val) {
        if (this->type == ValueType::Int || this->type == ValueType::Long) {
            return std::regex_match(val, reg_integer) && CheckRange(atof(val.c_str()));
        } else if (this->type == ValueType::Float || this->type == ValueType::Double) {
            return std::regex_match(val, reg_float) && CheckRange(atof(val.c_str()));
        } else if (this->type == ValueType::Bool) {
            return val == "true" || val == "false";
        }
        return true;
    }

    bool CheckValue(const vector<string> &val) {
        for (int i = 0; i < val.size(); ++i) {
            if (!CheckValue(val[i])) {
                return false;
            }
        }
        return true;
    }

    int AcceptValue(const string &k, const vector<string> &v) {
    
        if (action == ActionType::Store) {
            // 检查值个数是否符合nargs
            if (fixed && v.size() != fixed_nargs) {
                fprintf(stderr, "argparse error! value number mismatch for %s (expected:%d, actual:%d)\n",
                        k.c_str(), fixed_nargs, v.size());
                return -1;
            }
            if (!fixed && variable_nargs == '+' && v.empty()) {
                fprintf(stderr, "argparse error! value number mismatch for %s (expected: >0, actual:%d)\n",
                        k.c_str(), v.size());
                return -1;
            }
            
            // 检查取值是否在choices中
            if (!choices.empty()) {
                for (int i = 0; i < v.size(); ++i) {
                    if (!std::any_of(std::begin(choices), std::end(choices), [v, i](string &ele) { return ele == v[i]; })) {
                        fprintf(stderr, "argparse error! value %s of key %s is not among specified choices.\n",
                                v[i].c_str(), k.c_str());
                        return -1;
                    }
                }
            }

            // 类型检查
            if (!CheckValue(v)) {
                fprintf(stderr, "argparse error! key %s is followed by invalid formatted values\n", k.c_str());
                return -1;
            }
            
            vector<string> fix_v = Fix(v);
            this->values.assign(fix_v.begin(), fix_v.end());

        }
        loaded = true;
        return 0;
    }

private:
    
    string Fix(const string &v) {
        if (type == ValueType::Bool) {
            return v == "true" ? "1" : "0";
        }
        return v;
    }

    vector<string> Fix(const vector<string> &v) {
        vector<string> new_v;
        for (int i = 0; i < v.size(); ++i) {
            new_v.push_back(Fix(v[i]));
        }
        return new_v;
    }

    string name{ "" };
    string short_name{ "" };
    string dest{ "" };
    bool required{ true };
    bool fixed{ true  };
    int fixed_nargs{ 1 };
    char variable_nargs { '*' };
    vector<string> defaults;
    vector<string> choices;
    string help{ "" };
    ValueType type;
    vector<string> values;
    bool loaded{ false };
    ActionType action{ ActionType::Store };
    std::regex reg_integer{ "-?\\d+" };
    std::regex reg_float{ "-?\\d+(\\.\\d+)?" };
    bool print{ true };
    vector<double> range {0, -1};
};

class ArgumentParser {
public:
    ArgumentParser(const string &desc): description(desc) {
        AddArgument("-h", "--help", "help", ValueType::Bool, "show help info and exit")
            .SetRequired(false).SetAction(ActionType::StoreTrue).SetPrint(false);
    }
    
    Argument& AddArgument(const string &short_name,
            const string &name,
            const string &dest,
            ValueType type,
            const string &help) {
        // name和short_name至少一个不为空
        assert(!short_name.empty() || !name.empty());
        // dest和help必不为空
        assert(!dest.empty() && !help.empty());
        // dest不允许重复
        assert(args_map.count(dest) == 0);
        // short_name不允许重复
        if (!short_name.empty()) {
            assert(short_name.length() == 2 && short_name[0] == '-');
            assert(this->name_map.count(short_name) == 0);
            this->name_map[short_name] = dest;
        }
        // name不允许重复
        if (!name.empty()) {
            assert(name.length() > 2 && name.substr(0, 2) == "--");
            assert(name_map.count(name) == 0);
            this->name_map[name] = dest;
        }

        Argument arg(short_name, name, dest, type, help);
        this->args_map[dest] = arg;
        return this->args_map[dest];
    }

    int Parse(int argc, char* argv[]) {
        this->program = string(argv[0]);
        string key;
        vector<string> vals;
        
        for (int i = 1; i < argc; ++i) {
            string arg(argv[i]);
            if (arg.empty()) { continue; }

            // 如果是kv中的v，临时记录到vals
            if (arg[0] != '-') {
                // 保证不可能出现有v无k
                if (key.empty()) {
                    fprintf(stderr, "argparse error! value %s matched no key\n", arg.c_str());
                    return -1;
                }
                vals.push_back(arg);
                continue;
            }

            // 如果是kv中的k，需要先将上一轮记录的kv进行处理
            if (!key.empty() && HandleKV(key, vals) != 0) {
                fprintf(stderr, "argparse error! fail to add key %s\n", key.c_str());
                return -1;
            }

            key = arg;
            vals.clear();
        }

        if (!key.empty() && HandleKV(key, vals) != 0) {
            fprintf(stderr, "argparse error! fail to add key %s\n", key.c_str());
            return -1;
        }

        if (args_map.find("help")->second.loaded) {
            return 1;
        } else if (CheckArguments()) {
            return 0;
        }
        return -1;
    }

    template <class T>
    T Get(const string &key) {
        map<string, Argument>::const_iterator iter = args_map.find(key);
        if (iter == args_map.end()) {
            throw std::invalid_argument("argparse error! key not found");
        }
        const Argument &arg = (*iter).second;
        if (arg.action == ActionType::StoreTrue) {
            string val = arg.loaded ? "1": "0";
            return Cast<T>(val);
        } else if (arg.action == ActionType::StoreFalse) {
            string val = arg.loaded ? "0": "1";
            return Cast<T>(val);
        } else if (arg.action == ActionType::Store) {
            if (!arg.values.empty()) {
                return Cast<T>(arg.values[0]);
            }
            if (!arg.defaults.empty()) {
                return Cast<T>(arg.defaults[0]);
            }
        }
        throw std::bad_exception();
    }

    template <class T>
    vector<T> GetList(const string &key) {
        map<string, Argument>::const_iterator iter = args_map.find(key);
        if (iter == args_map.end()) {
            throw std::invalid_argument("argparse error! key not found");
        }
        const Argument &arg = (*iter).second;
      
        if (arg.action == ActionType::Store) {
            // 对于可空数组，如果命令行参数为空，就不再读取defaults了
            if ((arg.loaded && !arg.fixed && arg.variable_nargs == '*') || !arg.values.empty()) {
                return Cast<T>(arg.values);
            }

            if (!arg.defaults.empty()) {
                return Cast<T>(arg.defaults);
            }
            if (!arg.fixed && arg.variable_nargs == '*') {
                return vector<T>();
            }
        }
        throw std::bad_exception();
    }

    void PrintHelp () {
        fprintf(stdout, "usage: <yourscript> [-h/--help] [options]\n\n");
        fprintf(stdout, "%s\n\n", description.c_str());
        fprintf(stdout, "options (required):\n");
        map<string, Argument>::const_iterator iter = args_map.begin();
        while(iter != args_map.end()) {
            const Argument &arg = (*iter).second;
            if (arg.required && arg.print) {
                fprintf(stdout, "\t%s, %s\t\t%s\n", 
                        arg.short_name.c_str(),
                        arg.name.c_str(),
                        arg.help.c_str());
            }
            ++iter;
        }
        fprintf(stdout, "options (optional):\n");
        iter = args_map.begin();
        while(iter != args_map.end()) {
            const Argument &arg = (*iter).second;
            if (!arg.required && arg.print) {
                fprintf(stdout, "\t%s, %s\t\t%s\n",
                        arg.short_name.c_str(),
                        arg.name.c_str(),
                        arg.help.c_str());
            }
            ++iter;
        }
        fprintf(stdout, "\n");
    }

private:
    
    bool CheckArguments() {
        map<string, Argument>::const_iterator iter = args_map.begin();
        while(iter != args_map.end()) {
            const Argument &arg = (*iter).second;
            if (arg.required && !arg.loaded) {
                fprintf(stderr, "argparse error! missing argument %s %s\n", 
                        arg.short_name.c_str(), arg.name.c_str());
                return false;
            }
            ++iter;
        }
        return true;
    }

    int HandleKV(const string &k, const vector<string> &v) {
        
        map<string, string>::const_iterator iter = name_map.find(k);
        if (iter == name_map.end()) {
            fprintf(stderr, "argparse warn! received key %s is not defined\n", k.c_str());
            return 0;
        }
        Argument &arg = args_map[(*iter).second];
        return arg.AcceptValue(k, v);
    }

    template <class T>
    T Cast(const string &val) {
        stringstream stream;
        T res;
        stream << val;
        stream >> res;
        return res;
    }

    template <class T>
    vector<T> Cast(const vector<string> &val) {
        vector<T> res;
        for (int i = 0; i < val.size(); ++i) {
            res.push_back(Cast<T>(val[i]));
        }
        return res;
    }

    string program;
    string description;
    map<string, Argument> args_map;
    map<string, string> name_map;
}; 

};

#endif
